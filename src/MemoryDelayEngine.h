// MemoryDelayEngine.h
//
// High-level class that manages the MemoryBuffer, playheads, modifiers,
// and feedback modes for the stereo memory delay.

#pragma once

#include <JuceHeader.h>
#include "MemoryBuffer.h"
#include "Playhead.h"
#include "RandomGenerator.h"
#include "Modifiers.h"
#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <vector>

class MemoryDelayEngine
{
public:
    enum class StereoMode
    {
        Independent = 0,
        Linked,
        Cross
    };

    enum class FeedbackMode
    {
        Collect = 0,
        Feed,
        Closed
    };

    static constexpr int kVisualBins = 96;

    struct VisualSnapshot
    {
        std::array<float, kVisualBins> energy {};
        float primaryPosition { 0.0f };
        float secondaryPosition { 0.0f };
        int writeIndex { 0 };
    };

    MemoryDelayEngine() = default;
    ~MemoryDelayEngine() = default;

    void prepare(double newSampleRate, int maxBlockSize, float maxDelaySeconds)
    {
        sampleRate = newSampleRate;
        bufferMaxSeconds = juce::jmax(0.1f, maxDelaySeconds);
        maxBlock = juce::jmax(1, maxBlockSize);
        maxDelaySeconds = bufferMaxSeconds;

        buffer.prepare(sampleRate, bufferMaxSeconds);
        primary.setMemoryBuffer(&buffer);
        secondary.setMemoryBuffer(&buffer);
        primary.setMaxDelaySeconds(maxDelaySeconds);
        secondary.setMaxDelaySeconds(maxDelaySeconds);

        primaryModifiers.prepare(sampleRate, maxBlock, 2);
        secondaryModifiers.prepare(sampleRate, maxBlock, 2);

        resetVisualState();
        autoScanOffset = manualScan;
        autoScanSamplesRemaining = 0;
        autoScanRampRemaining = 0;
        requestReseed = true;
    }

    void reset()
    {
        buffer.prepare(sampleRate, bufferMaxSeconds);
        primary.setMaxDelaySeconds(maxDelaySeconds);
        secondary.setMaxDelaySeconds(maxDelaySeconds);
        primaryModifiers.reset();
        secondaryModifiers.reset();
        resetVisualState();
    }

    void setMix(float newMix) { mix = juce::jlimit(0.0f, 1.0f, newMix); }

    void setScan(float newScan)
    {
        manualScan = juce::jlimit(0.0f, 1.0f, newScan);
        if (autoScanRateHz <= 0.0f)
            autoScanOffset = manualScan;
    }

    void setAutoScanRate(float newRateHz)
    {
        const bool wasAuto = autoScanRateHz > 0.0f;
        const float clampedRate = juce::jmax(0.0f, newRateHz);
        if (std::abs(clampedRate - autoScanRateHz) < 0.0001f)
            return;

        autoScanRateHz = clampedRate;
        if (!wasAuto && autoScanRateHz > 0.0f)
            autoScanOffset = manualScan;

        autoScanSamplesRemaining = 0;
        autoScanRampRemaining = 0;
    }

    void setSpread(float newSpreadSeconds)
    {
        spreadSeconds = newSpreadSeconds;
        secondary.setSpread(newSpreadSeconds);
    }

    void setFeedback(float newFeedback)
    {
        feedback = juce::jlimit(0.0f, 0.98f, newFeedback);
    }

    void setTime(float newTimeSeconds)
    {
        maxDelaySeconds = juce::jlimit(0.1f, bufferMaxSeconds, newTimeSeconds);
        primary.setMaxDelaySeconds(maxDelaySeconds);
        secondary.setMaxDelaySeconds(maxDelaySeconds);
    }

    void setStereoMode(int modeIndex)
    {
        stereoMode = static_cast<StereoMode>(juce::jlimit(0, 2, modeIndex));
    }

    void setMode(int modeIndex)
    {
        mode = static_cast<FeedbackMode>(juce::jlimit(0, 2, modeIndex));
    }

    void setCharacter(float newCharacter)
    {
        character = juce::jlimit(0.0f, 1.0f, newCharacter);
        primaryModifiers.setCharacter(character);
        secondaryModifiers.setCharacter(character);
    }

    void setRandomSeed(int newSeed)
    {
        const uint32_t nextSeed = static_cast<uint32_t>(newSeed);
        if (nextSeed != userSeed)
        {
            userSeed = nextSeed;
            requestReseed = true;
        }
    }

    void setTransportPosition(int64_t timeInSamples, bool isPlaying)
    {
        transportSample = timeInSamples;
        if (isPlaying && (!transportPlaying || (transportSample >= 0 && transportSample < lastTransportSample)))
            requestReseed = true;

        transportPlaying = isPlaying;
        lastTransportSample = transportSample;
    }

    void processBlock(juce::AudioBuffer<float>& audioBuffer)
    {
        updateRandomSeedIfNeeded();

        const int numSamples = audioBuffer.getNumSamples();
        const float dryMix = 1.0f - mix;
        float energySum = 0.0f;

        const int readChannelLeft = getReadChannel(0);
        const int readChannelRight = getReadChannel(1);

        float lastOffset = manualScan;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float offset = getNextScanOffset();
            lastOffset = offset;
            primary.setOffsetNormalized(offset);
            secondary.setOffsetNormalized(offset);

            const float inLeft = audioBuffer.getSample(0, sample);
            const float inRight = audioBuffer.getSample(1, sample);

            const float rawPrimaryLeft = primary.readSample(readChannelLeft, sampleRate);
            const float rawPrimaryRight = primary.readSample(readChannelRight, sampleRate);
            const float rawSecondaryLeft = secondary.readSample(readChannelLeft, sampleRate);
            const float rawSecondaryRight = secondary.readSample(readChannelRight, sampleRate);

            const float rawEffectLeft = 0.5f * (rawPrimaryLeft + rawSecondaryLeft);
            const float rawEffectRight = 0.5f * (rawPrimaryRight + rawSecondaryRight);

            const float procPrimaryLeft = primaryModifiers.processSample(rawPrimaryLeft, 0, random);
            const float procPrimaryRight = primaryModifiers.processSample(rawPrimaryRight, 1, random);
            const float procSecondaryLeft = secondaryModifiers.processSample(rawSecondaryLeft, 0, random);
            const float procSecondaryRight = secondaryModifiers.processSample(rawSecondaryRight, 1, random);

            const float procEffectLeft = 0.5f * (procPrimaryLeft + procSecondaryLeft);
            const float procEffectRight = 0.5f * (procPrimaryRight + procSecondaryRight);

            const float outLeft = dryMix * inLeft + mix * procEffectLeft;
            const float outRight = dryMix * inRight + mix * procEffectRight;

            audioBuffer.setSample(0, sample, outLeft);
            audioBuffer.setSample(1, sample, outRight);

            float feedbackSourceLeft = 0.0f;
            float feedbackSourceRight = 0.0f;

            switch (mode)
            {
                case FeedbackMode::Collect:
                    break;
                case FeedbackMode::Feed:
                    // Feed mode keeps modifiers static by feeding raw playhead output.
                    feedbackSourceLeft = rawEffectLeft;
                    feedbackSourceRight = rawEffectRight;
                    break;
                case FeedbackMode::Closed:
                    // Closed mode feeds the full output (mix + modifiers) for accumulation.
                    feedbackSourceLeft = outLeft;
                    feedbackSourceRight = outRight;
                    break;
            }

            float writeLeft = inLeft + feedback * feedbackSourceLeft;
            float writeRight = inRight + feedback * feedbackSourceRight;

            writeLeft = std::tanh(writeLeft);
            writeRight = std::tanh(writeRight);

            writeToMemory(writeLeft, writeRight);

            energySum += 0.5f * (std::abs(procEffectLeft) + std::abs(procEffectRight));
        }

        const int index = visualWriteIndex.load();
        visualEnergy[static_cast<size_t>(index)].store(energySum / static_cast<float>(juce::jmax(1, numSamples)));
        visualWriteIndex.store((index + 1) % kVisualBins);

        const float spreadNorm = (maxDelaySeconds > 0.0f) ? (spreadSeconds / maxDelaySeconds) : 0.0f;
        visualPrimary.store(lastOffset);
        visualSecondary.store(juce::jlimit(0.0f, 1.0f, lastOffset + spreadNorm));
    }

    void getVisualSnapshot(VisualSnapshot& snapshot) const
    {
        for (size_t i = 0; i < snapshot.energy.size(); ++i)
            snapshot.energy[i] = visualEnergy[i].load();

        snapshot.primaryPosition = visualPrimary.load();
        snapshot.secondaryPosition = visualSecondary.load();
        snapshot.writeIndex = visualWriteIndex.load();
    }

    int getMaxSamples() const { return buffer.getBufferSize(); }
    int getWriteIndex() const { return buffer.getWritePosition(); }
    float debugGetMemorySample(int channel, int index) const { return buffer.getSample(channel, index); }

private:
    float getNextScanOffset()
    {
        if (autoScanRateHz <= 0.0f)
            return manualScan;

        if (autoScanSamplesRemaining <= 0)
        {
            autoScanSamplesRemaining = juce::jmax(1, static_cast<int>(sampleRate / autoScanRateHz));
            autoScanTarget = random.nextFloat01();
            const int rampSamples = juce::jmax(1, static_cast<int>(sampleRate * 0.05f));
            autoScanStep = (autoScanTarget - autoScanOffset) / static_cast<float>(rampSamples);
            autoScanRampRemaining = rampSamples;
        }

        if (autoScanRampRemaining > 0)
        {
            autoScanOffset += autoScanStep;
            --autoScanRampRemaining;
        }

        --autoScanSamplesRemaining;
        return juce::jlimit(0.0f, 1.0f, autoScanOffset);
    }

    void updateRandomSeedIfNeeded()
    {
        if (!requestReseed)
            return;

        uint32_t combinedSeed = 0x6d2b79f5u;
        combinedSeed ^= userSeed + 0x9e3779b9u + (combinedSeed << 6) + (combinedSeed >> 2);

        if (transportSample >= 0)
        {
            const uint32_t low = static_cast<uint32_t>(transportSample & 0xffffffff);
            const uint32_t high = static_cast<uint32_t>((transportSample >> 32) & 0xffffffff);
            combinedSeed ^= low + 0x9e3779b9u + (combinedSeed << 6) + (combinedSeed >> 2);
            combinedSeed ^= high + 0x9e3779b9u + (combinedSeed << 6) + (combinedSeed >> 2);
        }

        random.setSeed(combinedSeed);
        requestReseed = false;
    }

    int getReadChannel(int outputChannel) const
    {
        switch (stereoMode)
        {
            case StereoMode::Independent:
                return outputChannel;
            case StereoMode::Linked:
                return 0;
            case StereoMode::Cross:
                return 1 - outputChannel;
        }

        return outputChannel;
    }

    void writeToMemory(float left, float right)
    {
        switch (stereoMode)
        {
            case StereoMode::Independent:
                buffer.writeSample(left, right);
                break;
            case StereoMode::Linked:
            {
                const float mono = 0.5f * (left + right);
                buffer.writeSample(mono, mono);
                break;
            }
            case StereoMode::Cross:
                buffer.writeSample(left, right);
                break;
        }
    }

    void resetVisualState()
    {
        for (auto& energy : visualEnergy)
            energy.store(0.0f);
        visualWriteIndex.store(0);
        visualPrimary.store(0.0f);
        visualSecondary.store(0.0f);
    }

    double sampleRate { 44100.0 };
    int maxBlock { 512 };
    float bufferMaxSeconds { 1.0f };
    float maxDelaySeconds { 1.0f };

    MemoryBuffer buffer;
    Playhead primary;
    Playhead secondary;
    ModifierChain primaryModifiers;
    ModifierChain secondaryModifiers;
    RandomGenerator random;

    float mix { 0.5f };
    float manualScan { 0.0f };
    float autoScanRateHz { 0.0f };
    float autoScanOffset { 0.0f };
    float autoScanTarget { 0.0f };
    float autoScanStep { 0.0f };
    int autoScanSamplesRemaining { 0 };
    int autoScanRampRemaining { 0 };
    float spreadSeconds { 0.0f };
    float feedback { 0.0f };
    float character { 0.0f };

    StereoMode stereoMode { StereoMode::Independent };
    FeedbackMode mode { FeedbackMode::Feed };

    uint32_t userSeed { 0 };
    int64_t transportSample { -1 };
    int64_t lastTransportSample { -1 };
    bool transportPlaying { false };
    bool requestReseed { true };

    std::array<std::atomic<float>, kVisualBins> visualEnergy {};
    std::atomic<int> visualWriteIndex { 0 };
    std::atomic<float> visualPrimary { 0.0f };
    std::atomic<float> visualSecondary { 0.0f };
};

namespace echoform {

enum class ScanMode {
    Auto,
    Manual
};

enum class RoutingMode {
    In,
    Out,
    Feed
};

struct ModifierBank {
    float modifier1 = 0.0f;
    float modifier2 = 0.0f;
    float modifier3 = 0.0f;
    RoutingMode routing = RoutingMode::Out;
};

struct MemoryDelayParameters {
    float sizeMs = 500.0f;
    float repeats = 0.35f;
    float scan = 0.0f;
    float spread = 0.0f;
    ScanMode scanMode = ScanMode::Manual;
    bool collect = false;
    bool alwaysRecord = false;
    bool wipe = false;
    bool inspectEnabled = false;
    bool bypassed = false;
    ModifierBank bankA{};
    ModifierBank bankB{};
};

struct InspectState {
    std::vector<float> energy;
    float primaryPlayhead = 0.0f;
    float secondaryPlayhead = 0.0f;
};

class MemoryDelayEngine {
public:
    MemoryDelayEngine();

    void prepare(double sampleRate, int maxBlockSize, double maxSeconds = 180.0);
    void reset();

    void setParameters(const MemoryDelayParameters& params);

    void processBlock(const float* const* input, float* const* output, int numSamples);

    const InspectState& getInspectState() const;

    int getMaxSamples() const;
    int getWriteIndex() const;
    float debugGetMemorySample(int channel, int index) const;

private:
    float applyBank(float sample, float& filterState, const ModifierBank& bank) const;
    float readInterpolated(const std::vector<float>& buffer, double position) const;
    double wrapPosition(double position) const;

    double sampleRate_ = 44100.0;
    int maxSamples_ = 0;
    int writeIndex_ = 0;

    std::vector<float> bufferL_;
    std::vector<float> bufferR_;

    MemoryDelayParameters params_{};
    InspectState inspect_{};

    double sizeSamplesCurrent_ = 0.0;
    double sizeSamplesTarget_ = 0.0;
    double sizeSlewPerSample_ = 0.0;

    double scanPhase_ = 0.0;

    float filterStateA_ = 0.0f;
    float filterStateB_ = 0.0f;
    float filterStateASecondary_ = 0.0f;
    float filterStateBSecondary_ = 0.0f;
};

} // namespace echoform
