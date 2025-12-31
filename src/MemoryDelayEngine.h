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

    enum class ScanMode
    {
        Manual = 0,
        Auto
    };

    enum class RoutingMode
    {
        In = 0,
        Out,
        Feed
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

    void prepare(double newSampleRate, int maxBlockSize, float maxBufferSeconds)
    {
        sampleRate = newSampleRate;
        bufferMaxSeconds = juce::jmax(kMemorySeconds, maxBufferSeconds);
        maxBlock = juce::jmax(1, maxBlockSize);
        sizeSecondsTarget = juce::jlimit(kMinSizeSeconds,
                                         juce::jmin(kMaxSizeSeconds, bufferMaxSeconds),
                                         sizeSecondsTarget);
        sizeSecondsCurrent = sizeSecondsTarget;
        sizeSecondsPrevious = sizeSecondsTarget;
        sizeCrossfadeSamplesRemaining = 0;
        sizeCrossfadeSamplesTotal = 0;

        buffer.prepare(sampleRate, bufferMaxSeconds);
        primary.setMemoryBuffer(&buffer);
        secondary.setMemoryBuffer(&buffer);
        primary.setMaxDelaySeconds(sizeSecondsCurrent);
        secondary.setMaxDelaySeconds(sizeSecondsCurrent);
        updateSpreadSeconds();

        modifierBankA.prepare(sampleRate, maxBlock, 2);
        modifierBankB.prepare(sampleRate, maxBlock, 2);

        resetVisualState();
        autoScanOffset = manualScan;
        autoScanSamplesRemaining = 0;
        autoScanSamplesTotal = 0;
        requestReseed = true;
    }

    void reset()
    {
        buffer.prepare(sampleRate, bufferMaxSeconds);
        sizeSecondsCurrent = sizeSecondsTarget;
        sizeSecondsPrevious = sizeSecondsTarget;
        sizeCrossfadeSamplesRemaining = 0;
        sizeCrossfadeSamplesTotal = 0;
        primary.setMaxDelaySeconds(sizeSecondsCurrent);
        secondary.setMaxDelaySeconds(sizeSecondsCurrent);
        updateSpreadSeconds();
        modifierBankA.reset();
        modifierBankB.reset();
        resetVisualState();
        if (tapeMode)
            resetTapeState();
    }

    void setMix(float newMix) { mix = juce::jlimit(0.0f, 1.0f, newMix); }

    void setScan(float newScan)
    {
        manualScan = juce::jlimit(0.0f, 1.0f, newScan);
        if (autoScanRateHz <= 0.0f)
            autoScanOffset = manualScan;
        else
            autoScanSamplesRemaining = 0;
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
        autoScanSamplesTotal = 0;
    }

    void setSpread(float newSpreadNormalized)
    {
        spreadNormalized = juce::jlimit(0.0f, 1.0f, newSpreadNormalized);
        updateSpreadSeconds();
    }

    void setFeedback(float newFeedback)
    {
        feedback = juce::jlimit(0.0f, 0.98f, newFeedback);
    }

    void setTapeMode(bool enabled)
    {
        tapeMode = enabled;
        if (!tapeMode)
            return;

        spreadNormalized = 0.0f;
        feedback = kTapeFeedback;
        mode = FeedbackMode::Feed;
        scanMode = ScanMode::Manual;
        autoScanRateHz = 0.0f;
        manualScan = 0.0f;
        alwaysRecord = true;
        routingModeA = RoutingMode::Out;
        routingModeB = RoutingMode::Out;
        modifierBankA.setModValues(0.0f, 0.0f, 0.0f);
        modifierBankB.setModValues(0.0f, 0.0f, 0.0f);
        character = 0.0f;
        setTapeWindowSeconds(tapeWindowSeconds);
        resetTapeState();
    }

    void setTapeWindowSeconds(float seconds)
    {
        tapeWindowSeconds = juce::jlimit(0.0f, kTapeMaxWindowSeconds, seconds);
        if (!tapeMode)
            return;

        sizeSecondsTarget = tapeWindowSeconds;
        sizeSecondsCurrent = tapeWindowSeconds;
        sizeSecondsPrevious = tapeWindowSeconds;
        sizeCrossfadeSamplesRemaining = 0;
        sizeCrossfadeSamplesTotal = 0;
        primary.setMaxDelaySeconds(sizeSecondsCurrent);
        secondary.setMaxDelaySeconds(sizeSecondsCurrent);
        updateSpreadSeconds();

        if (sizeSecondsCurrent <= 0.0f)
        {
            tapeOffsetSecondsCurrent = 0.0f;
            tapeOffsetSecondsStart = 0.0f;
            tapeOffsetSecondsTarget = 0.0f;
        }
        else
        {
            tapeOffsetSecondsCurrent = juce::jlimit(0.0f, sizeSecondsCurrent, tapeOffsetSecondsCurrent);
            tapeOffsetSecondsStart = tapeOffsetSecondsCurrent;
            tapeOffsetSecondsTarget = tapeOffsetSecondsCurrent;
        }
    }

    void setSize(float newSizeSeconds)
    {
        const float clamped = juce::jlimit(kMinSizeSeconds,
                                           juce::jmin(kMaxSizeSeconds, bufferMaxSeconds),
                                           newSizeSeconds);

        if (std::abs(clamped - sizeSecondsTarget) < kSizeEpsilon)
            return;

        if (sizeCrossfadeSamplesRemaining > 0 && sizeCrossfadeSamplesTotal > 0)
        {
            const float progress = 1.0f - (static_cast<float>(sizeCrossfadeSamplesRemaining)
                                           / static_cast<float>(sizeCrossfadeSamplesTotal));
            sizeSecondsCurrent = sizeSecondsPrevious + (sizeSecondsTarget - sizeSecondsPrevious) * progress;
        }

        sizeSecondsPrevious = sizeSecondsCurrent;
        sizeSecondsTarget = clamped;
        sizeCrossfadeSamplesTotal = juce::jmax(1, static_cast<int>(sampleRate * kSizeCrossfadeSeconds));
        sizeCrossfadeSamplesRemaining = sizeCrossfadeSamplesTotal;
    }

    void setStereoMode(int modeIndex)
    {
        stereoMode = static_cast<StereoMode>(juce::jlimit(0, 2, modeIndex));
    }

    void setMode(int modeIndex)
    {
        mode = static_cast<FeedbackMode>(juce::jlimit(0, 2, modeIndex));
    }

    void setScanMode(int modeIndex)
    {
        scanMode = static_cast<ScanMode>(juce::jlimit(0, 1, modeIndex));
        if (scanMode == ScanMode::Manual)
            autoScanOffset = manualScan;
        autoScanSamplesRemaining = 0;
        autoScanSamplesTotal = 0;
    }

    void setRoutingModeA(int modeIndex)
    {
        routingModeA = static_cast<RoutingMode>(juce::jlimit(0, 2, modeIndex));
    }

    void setRoutingModeB(int modeIndex)
    {
        routingModeB = static_cast<RoutingMode>(juce::jlimit(0, 2, modeIndex));
    }

    void setModifierBankA(float mod1, float mod2, float mod3)
    {
        modifierBankA.setModValues(mod1, mod2, mod3);
    }

    void setModifierBankB(float mod1, float mod2, float mod3)
    {
        modifierBankB.setModValues(mod1, mod2, mod3);
    }

    void setAlwaysRecord(bool shouldAlwaysRecord)
    {
        alwaysRecord = shouldAlwaysRecord;
    }

    void setDryKill(bool shouldDryKill)
    {
        dryKill = shouldDryKill;
    }

    void setLatch(bool shouldLatch)
    {
        latchEnabled = shouldLatch;
    }

    void setTrails(bool shouldTrail)
    {
        trailsEnabled = shouldTrail;
    }

    void setMemoryDry(bool shouldMemoryDry)
    {
        memoryDryEnabled = shouldMemoryDry;
    }

    void setWipe(bool shouldWipe)
    {
        wipeEnabled = shouldWipe;
    }

    void setBypassed(bool isBypassed)
    {
        bypassed = isBypassed;
    }

    void setCharacter(float newCharacter)
    {
        character = juce::jlimit(0.0f, 1.0f, newCharacter);
        modifierBankA.setCharacter(character);
        modifierBankB.setCharacter(character);
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

        if (bypassed != lastBypassed)
        {
            const bool allowTrails = trailsEnabled && !memoryDryEnabled;
            if (bypassed && !alwaysRecord && mode != FeedbackMode::Collect && !allowTrails)
                buffer.clear();
            lastBypassed = bypassed;
        }

        const int numSamples = audioBuffer.getNumSamples();
        const float dryMix = dryKill ? 0.0f : (1.0f - mix);
        const float wetMix = mix;
        float energySum = 0.0f;

        const int readChannelLeft = getReadChannel(0);
        const int readChannelRight = getReadChannel(1);

        float lastOffset = manualScan;
        constexpr float kCollectDecay = 0.98f;

        if (latchEnabled && !lastLatchEnabled)
        {
            latchedOffset = (scanMode == ScanMode::Manual) ? manualScan : autoScanOffset;
            lastLatchEnabled = true;
        }
        else if (!latchEnabled)
        {
            lastLatchEnabled = false;
        }

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float offset = latchEnabled ? latchedOffset : getNextScanOffset();
            lastOffset = offset;
            primary.setOffsetNormalized(offset);
            secondary.setOffsetNormalized(offset);

            const float inLeft = audioBuffer.getSample(0, sample);
            const float inRight = audioBuffer.getSample(1, sample);

            float rawEffectLeft = 0.0f;
            float rawEffectRight = 0.0f;
            if (sizeCrossfadeSamplesRemaining > 0 && sizeCrossfadeSamplesTotal > 0)
            {
                float rawALeft = 0.0f;
                float rawARight = 0.0f;
                float rawBLeft = 0.0f;
                float rawBRight = 0.0f;
                computeRawEffect(readChannelLeft, readChannelRight, sizeSecondsPrevious, rawALeft, rawARight);
                computeRawEffect(readChannelLeft, readChannelRight, sizeSecondsTarget, rawBLeft, rawBRight);
                const float progress = 1.0f - (static_cast<float>(sizeCrossfadeSamplesRemaining)
                                               / static_cast<float>(sizeCrossfadeSamplesTotal));
                rawEffectLeft = rawALeft + (rawBLeft - rawALeft) * progress;
                rawEffectRight = rawARight + (rawBRight - rawARight) * progress;

                --sizeCrossfadeSamplesRemaining;
                if (sizeCrossfadeSamplesRemaining <= 0)
                {
                    sizeSecondsCurrent = sizeSecondsTarget;
                    sizeSecondsPrevious = sizeSecondsTarget;
                    sizeCrossfadeSamplesTotal = 0;
                    updateSpreadSeconds();
                    primary.setMaxDelaySeconds(sizeSecondsCurrent);
                    secondary.setMaxDelaySeconds(sizeSecondsCurrent);
                }
            }
            else
            {
                computeRawEffect(readChannelLeft, readChannelRight, sizeSecondsCurrent, rawEffectLeft, rawEffectRight);
            }

            float effectLeft = rawEffectLeft;
            float effectRight = rawEffectRight;

            applyModifierBanks(RoutingMode::Out, effectLeft, effectRight);

            float outLeft = effectLeft;
            float outRight = effectRight;

            if (wipeEnabled)
            {
                outLeft = wetMix * effectLeft;
                outRight = wetMix * effectRight;
            }
            else if (bypassed)
            {
                if (trailsEnabled && !memoryDryEnabled)
                {
                    outLeft = (dryKill ? 0.0f : inLeft) + wetMix * effectLeft;
                    outRight = (dryKill ? 0.0f : inRight) + wetMix * effectRight;
                }
                else
                {
                    outLeft = inLeft;
                    outRight = inRight;
                }
            }
            else
            {
                outLeft = dryMix * inLeft + wetMix * effectLeft;
                outRight = dryMix * inRight + wetMix * effectRight;
            }

            audioBuffer.setSample(0, sample, outLeft);
            audioBuffer.setSample(1, sample, outRight);

            energySum += 0.5f * (std::abs(effectLeft) + std::abs(effectRight));

            const bool shouldWrite = !wipeEnabled && !latchEnabled
                                     && (!bypassed || alwaysRecord || mode == FeedbackMode::Collect);
            if (!shouldWrite)
                continue;

            float writeLeft = inLeft;
            float writeRight = inRight;

            applyModifierBanks(RoutingMode::In, writeLeft, writeRight);

            float feedbackSourceLeft = 0.0f;
            float feedbackSourceRight = 0.0f;

            if (!bypassed)
            {
                switch (mode)
                {
                    case FeedbackMode::Collect:
                        break;
                    case FeedbackMode::Feed:
                        // Feed mode recirculates the processed playback (no dry mix).
                        feedbackSourceLeft = effectLeft;
                        feedbackSourceRight = effectRight;
                        break;
                    case FeedbackMode::Closed:
                        // Closed mode feeds the full output (mix + modifiers) for accumulation.
                        feedbackSourceLeft = outLeft;
                        feedbackSourceRight = outRight;
                        break;
                }
            }

            float feedbackLeft = feedback * feedbackSourceLeft;
            float feedbackRight = feedback * feedbackSourceRight;

            applyModifierBanks(RoutingMode::Feed, feedbackLeft, feedbackRight);

            writeLeft += feedbackLeft;
            writeRight += feedbackRight;

            if (mode == FeedbackMode::Collect)
            {
                const int writeIndex = buffer.getWritePosition();
                const float existingLeft = buffer.getSample(0, writeIndex);
                const float existingRight = buffer.getSample(1, writeIndex);
                writeLeft = existingLeft * kCollectDecay + writeLeft;
                writeRight = existingRight * kCollectDecay + writeRight;
            }

            writeLeft = std::tanh(writeLeft);
            writeRight = std::tanh(writeRight);

            writeToMemory(writeLeft, writeRight);
        }

        const int index = visualWriteIndex.load();
        visualEnergy[static_cast<size_t>(index)].store(energySum / static_cast<float>(juce::jmax(1, numSamples)));
        visualWriteIndex.store((index + 1) % kVisualBins);

        const float spreadNorm = spreadNormalized;
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
        if (tapeMode)
            return getTapeOffset();

        if (scanMode == ScanMode::Manual || autoScanRateHz <= 0.0f)
            return manualScan;

        const int samplesPerCycle = juce::jmax(1, static_cast<int>(sampleRate / autoScanRateHz));
        if (autoScanSamplesRemaining <= 0 || samplesPerCycle != autoScanSamplesTotal)
        {
            autoScanSamplesTotal = samplesPerCycle;
            autoScanSamplesRemaining = samplesPerCycle;
            autoScanTarget = random.nextFloat01() * manualScan;
        }

        const float phase = 1.0f - (static_cast<float>(autoScanSamplesRemaining)
                                    / static_cast<float>(autoScanSamplesTotal));
        const float triangle = (phase <= 0.5f) ? (phase * 2.0f) : (2.0f - phase * 2.0f);
        autoScanOffset = triangle * autoScanTarget;
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

    void computeRawEffect(int readChannelLeft, int readChannelRight, float sizeSecondsForRead,
                          float& rawLeft, float& rawRight) const
    {
        const float spreadSeconds = spreadNormalized * sizeSecondsForRead;
        const float primaryLeft = primary.readSample(readChannelLeft, sampleRate, sizeSecondsForRead, 0.0f);
        const float primaryRight = primary.readSample(readChannelRight, sampleRate, sizeSecondsForRead, 0.0f);
        const float secondaryLeft = secondary.readSample(readChannelLeft, sampleRate, sizeSecondsForRead, spreadSeconds);
        const float secondaryRight = secondary.readSample(readChannelRight, sampleRate, sizeSecondsForRead, spreadSeconds);
        rawLeft = 0.5f * (primaryLeft + secondaryLeft);
        rawRight = 0.5f * (primaryRight + secondaryRight);
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

    void applyModifierBanks(RoutingMode routing, float& left, float& right)
    {
        if (routingModeA == routing)
        {
            left = modifierBankA.processSample(left, 0, random);
            right = modifierBankA.processSample(right, 1, random);
        }
        if (routingModeB == routing)
        {
            left = modifierBankB.processSample(left, 0, random);
            right = modifierBankB.processSample(right, 1, random);
        }
    }

    void updateSpreadSeconds()
    {
        const float spreadSeconds = spreadNormalized * sizeSecondsCurrent;
        secondary.setSpread(spreadSeconds);
    }

    float getTapeOffset()
    {
        if (sizeSecondsCurrent <= 0.0f)
            return 0.0f;

        if (tapeSlewSamplesRemaining > 0)
        {
            const float progress = 1.0f - (static_cast<float>(tapeSlewSamplesRemaining)
                                           / static_cast<float>(tapeSlewSamplesTotal));
            tapeOffsetSecondsCurrent = tapeOffsetSecondsStart
                                       + (tapeOffsetSecondsTarget - tapeOffsetSecondsStart) * progress;
            --tapeSlewSamplesRemaining;
            if (tapeSlewSamplesRemaining <= 0)
                tapeHoldSamplesRemaining = nextTapeHoldSamples();
        }
        else
        {
            if (tapeHoldSamplesRemaining <= 0)
                startTapeJump();
            else
                --tapeHoldSamplesRemaining;
        }

        return juce::jlimit(0.0f, 1.0f, tapeOffsetSecondsCurrent / sizeSecondsCurrent);
    }

    void startTapeJump()
    {
        if (sizeSecondsCurrent <= 0.0f)
        {
            tapeOffsetSecondsStart = 0.0f;
            tapeOffsetSecondsTarget = 0.0f;
            tapeSlewSamplesRemaining = 0;
            tapeSlewSamplesTotal = 0;
            tapeHoldSamplesRemaining = nextTapeHoldSamples();
            return;
        }

        tapeOffsetSecondsStart = tapeOffsetSecondsCurrent;
        const bool deepJump = random.nextFloat01() < kTapeDeepChance;
        const float minRatio = deepJump ? kTapeDeepMinRatio : kTapeNearMinRatio;
        const float maxRatio = deepJump ? kTapeDeepMaxRatio : kTapeNearMaxRatio;
        const float targetRatio = minRatio + random.nextFloat01() * (maxRatio - minRatio);
        tapeOffsetSecondsTarget = juce::jlimit(0.0f, sizeSecondsCurrent, targetRatio * sizeSecondsCurrent);
        tapeSlewSamplesTotal = juce::jmax(1, static_cast<int>(sampleRate * kTapeSlewSeconds));
        tapeSlewSamplesRemaining = tapeSlewSamplesTotal;
    }

    int nextTapeHoldSamples()
    {
        const float holdSeconds = kTapeHoldMinSeconds
                                  + random.nextFloat01() * (kTapeHoldMaxSeconds - kTapeHoldMinSeconds);
        return juce::jmax(1, static_cast<int>(holdSeconds * sampleRate));
    }

    void resetTapeState()
    {
        if (sizeSecondsCurrent <= 0.0f)
        {
            tapeOffsetSecondsCurrent = 0.0f;
            tapeOffsetSecondsStart = 0.0f;
            tapeOffsetSecondsTarget = 0.0f;
        }
        else
        {
            tapeOffsetSecondsCurrent = sizeSecondsCurrent * kTapeNearMinRatio;
            tapeOffsetSecondsStart = tapeOffsetSecondsCurrent;
            tapeOffsetSecondsTarget = tapeOffsetSecondsCurrent;
        }
        tapeSlewSamplesRemaining = 0;
        tapeSlewSamplesTotal = 0;
        tapeHoldSamplesRemaining = nextTapeHoldSamples();
    }

    static constexpr float kMinSizeSeconds = 0.05f;
    static constexpr float kMaxSizeSeconds = 60.0f;
    static constexpr float kMemorySeconds = 180.0f;
    static constexpr float kSizeCrossfadeSeconds = 0.05f;
    static constexpr float kSizeEpsilon = 1.0e-4f;
    static constexpr float kTapeFeedback = 0.65f;
    static constexpr float kTapeMaxWindowSeconds = 30.0f;
    static constexpr float kTapeDefaultWindowSeconds = 3.0f;
    static constexpr float kTapeNearMinRatio = 0.7f;
    static constexpr float kTapeNearMaxRatio = 1.0f;
    static constexpr float kTapeDeepMinRatio = 0.25f;
    static constexpr float kTapeDeepMaxRatio = 0.9f;
    static constexpr float kTapeDeepChance = 0.2f;
    static constexpr float kTapeHoldMinSeconds = 2.0f;
    static constexpr float kTapeHoldMaxSeconds = 6.0f;
    static constexpr float kTapeSlewSeconds = 0.25f;

    double sampleRate { 44100.0 };
    int maxBlock { 512 };
    float bufferMaxSeconds { kMemorySeconds };
    float sizeSecondsTarget { 1.0f };
    float sizeSecondsCurrent { 1.0f };
    float sizeSecondsPrevious { 1.0f };
    int sizeCrossfadeSamplesRemaining { 0 };
    int sizeCrossfadeSamplesTotal { 0 };

    MemoryBuffer buffer;
    Playhead primary;
    Playhead secondary;
    ModifierChain modifierBankA;
    ModifierChain modifierBankB;
    RandomGenerator random;

    float mix { 0.5f };
    float manualScan { 0.0f };
    float autoScanRateHz { 0.0f };
    float autoScanOffset { 0.0f };
    float autoScanTarget { 0.0f };
    int autoScanSamplesTotal { 0 };
    int autoScanSamplesRemaining { 0 };
    float spreadNormalized { 0.0f };
    float feedback { 0.0f };
    float character { 0.0f };

    StereoMode stereoMode { StereoMode::Independent };
    FeedbackMode mode { FeedbackMode::Feed };
    ScanMode scanMode { ScanMode::Manual };
    RoutingMode routingModeA { RoutingMode::Out };
    RoutingMode routingModeB { RoutingMode::Out };
    bool alwaysRecord { false };
    bool dryKill { false };
    bool latchEnabled { false };
    bool trailsEnabled { false };
    bool memoryDryEnabled { false };
    bool wipeEnabled { false };
    bool bypassed { false };
    bool lastBypassed { false };
    float latchedOffset { 0.0f };
    bool lastLatchEnabled { false };
    bool tapeMode { false };
    float tapeWindowSeconds { kTapeDefaultWindowSeconds };
    float tapeOffsetSecondsCurrent { 0.0f };
    float tapeOffsetSecondsStart { 0.0f };
    float tapeOffsetSecondsTarget { 0.0f };
    int tapeHoldSamplesRemaining { 0 };
    int tapeSlewSamplesRemaining { 0 };
    int tapeSlewSamplesTotal { 0 };

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
