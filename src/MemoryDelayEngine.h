#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

#include "RandomGenerator.h"

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
    void setRandomSeed(double randomSeed, double transportPosition);

    void processBlock(const float* const* input, float* const* output, int numSamples);

    const InspectState& getInspectState() const;
    float getAutoScanTarget() const noexcept;
    float nextModifierRandom() noexcept;

    int getMaxSamples() const;
    int getWriteIndex() const;
    float debugGetMemorySample(int channel, int index) const;

private:
    float applyBank(float sample, float& filterState, const ModifierBank& bank) const;
    float readInterpolated(const std::vector<float>& buffer, double position) const;
    double wrapPosition(double position) const;

    RandomGenerator randomGenerator;
    float autoScanTarget_ = 0.0f;

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

inline MemoryDelayEngine::MemoryDelayEngine() = default;

inline void MemoryDelayEngine::prepare(double sampleRate, int maxBlockSize, double maxSeconds)
{
    sampleRate_ = sampleRate;
    maxSamples_ = static_cast<int>(sampleRate * maxSeconds);
    bufferL_.assign(static_cast<std::size_t>(maxSamples_), 0.0f);
    bufferR_.assign(static_cast<std::size_t>(maxSamples_), 0.0f);
    inspect_.energy.assign(static_cast<std::size_t>(maxBlockSize), 0.0f);
    reset();
}

inline void MemoryDelayEngine::reset()
{
    std::fill(bufferL_.begin(), bufferL_.end(), 0.0f);
    std::fill(bufferR_.begin(), bufferR_.end(), 0.0f);
    writeIndex_ = 0;
    scanPhase_ = 0.0;
    autoScanTarget_ = 0.0f;
    filterStateA_ = 0.0f;
    filterStateB_ = 0.0f;
    filterStateASecondary_ = 0.0f;
    filterStateBSecondary_ = 0.0f;
}

inline void MemoryDelayEngine::setParameters(const MemoryDelayParameters& params)
{
    params_ = params;
}

inline void MemoryDelayEngine::setRandomSeed(double randomSeed, double transportPosition)
{
    randomGenerator.setSeed(randomSeed + transportPosition);
}

inline void MemoryDelayEngine::processBlock(const float* const* input, float* const* output, int numSamples)
{
    if (params_.scanMode == ScanMode::Auto)
        autoScanTarget_ = static_cast<float>(randomGenerator.nextFloat01());

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float inL = input[0][sample];
        const float inR = input[1][sample];
        output[0][sample] = inL;
        output[1][sample] = inR;
    }
}

inline const InspectState& MemoryDelayEngine::getInspectState() const
{
    return inspect_;
}

inline float MemoryDelayEngine::getAutoScanTarget() const noexcept
{
    return autoScanTarget_;
}

inline float MemoryDelayEngine::nextModifierRandom() noexcept
{
    return static_cast<float>(randomGenerator.nextFloat01());
}

inline int MemoryDelayEngine::getMaxSamples() const
{
    return maxSamples_;
}

inline int MemoryDelayEngine::getWriteIndex() const
{
    return writeIndex_;
}

inline float MemoryDelayEngine::debugGetMemorySample(int channel, int index) const
{
    if (channel == 0 && !bufferL_.empty())
        return bufferL_[static_cast<std::size_t>(index % maxSamples_)];
    if (channel == 1 && !bufferR_.empty())
        return bufferR_[static_cast<std::size_t>(index % maxSamples_)];
    return 0.0f;
}

inline float MemoryDelayEngine::applyBank(float sample, float& filterState, const ModifierBank& bank) const
{
    (void)bank;
    filterState = sample;
    return sample;
}

inline float MemoryDelayEngine::readInterpolated(const std::vector<float>& buffer, double position) const
{
    if (buffer.empty())
        return 0.0f;
    const auto index = static_cast<std::size_t>(static_cast<int>(position) % maxSamples_);
    return buffer[index];
}

inline double MemoryDelayEngine::wrapPosition(double position) const
{
    if (maxSamples_ == 0)
        return 0.0;
    while (position < 0.0)
        position += maxSamples_;
    while (position >= maxSamples_)
        position -= maxSamples_;
    return position;
}
