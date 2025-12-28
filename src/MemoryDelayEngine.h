#pragma once

#include <cstddef>
#include <vector>

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
