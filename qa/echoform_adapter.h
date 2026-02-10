// echoform_adapter.h
//
// DspUnderTest adapter for Echoform MemoryDelayEngine
// Wraps the JUCE-dependent engine for framework-agnostic QA testing

#pragma once

#include "../external/qa_harness/core/dsp_under_test.h"
#include "../external/qa_harness/core/effect_capabilities.h"
#include "../src/MemoryDelayEngine.h"
#include <memory>
#include <string>

namespace echoform {

class EchoformAdapter : public qa::DspUnderTest
{
public:
    // Parameter indices (28 total parameters)
    enum ParameterIndex
    {
        MIX = 0,              // 0.0–1.0
        SCAN,                 // 0.0–1.0
        AUTO_SCAN_RATE,       // 0.0–2.0 Hz
        SPREAD,               // 0.0–1.0 (normalized)
        FEEDBACK,             // 0.0–0.98
        SIZE,                 // 0.05–60.0 seconds (normalized)
        CHARACTER,            // 0.0–1.0
        STEREO_MODE,          // 0–2 (Independent/Linked/Cross)
        FEEDBACK_MODE,        // 0–2 (Collect/Feed/Closed)
        SCAN_MODE,            // 0–1 (Manual/Auto)
        ROUTING_MODE_A,       // 0–2 (In/Out/Feed)
        ROUTING_MODE_B,       // 0–2 (In/Out/Feed)
        BANK_A_MOD1,          // 0.0–1.0 (wow/flutter)
        BANK_A_MOD2,          // 0.0–1.0 (dropout)
        BANK_A_MOD3,          // 0.0–1.0 (tone)
        BANK_B_MOD1,          // 0.0–1.0 (wow/flutter)
        BANK_B_MOD2,          // 0.0–1.0 (dropout)
        BANK_B_MOD3,          // 0.0–1.0 (tone)
        ALWAYS_RECORD,        // 0/1 (bool)
        DRY_KILL,             // 0/1 (bool)
        LATCH,                // 0/1 (bool)
        TRAILS,               // 0/1 (bool)
        MEMORY_DRY,           // 0/1 (bool)
        WIPE,                 // 0/1 (bool)
        BYPASSED,             // 0/1 (bool)
        RANDOM_SEED,          // 0.0–1.0 (normalized to 0–65535)
        TAPE_MODE,            // 0/1 (bool)
        TAPE_WINDOW_SECONDS,  // 0.0–30.0 seconds (normalized)

        NUM_PARAMETERS
    };

    EchoformAdapter();
    ~EchoformAdapter() override = default;

    // DspUnderTest interface
    void prepare(double sampleRate, int maxBlockSize, int numChannels) noexcept override;
    void release() noexcept override;
    void processBlock(float** channelData, int numChannels, int numSamples) noexcept override;
    void setParameter(int parameterIndex, qa::NormalizedParam normalizedValue) noexcept override;
    void reset() noexcept override;

    bool getCapabilities(qa::EffectCapabilities& caps) const override;
    qa::OptionalFeatures getOptionalFeatures() const override;
    int getParameterCount() const override { return NUM_PARAMETERS; }
    const char* getParameterName(int index) const override;

private:
    std::unique_ptr<MemoryDelayEngine> engine_;
    std::unique_ptr<juce::AudioBuffer<float>> juceBuffer_;

    double currentSampleRate_ = 44100.0;
    int currentMaxBlockSize_ = 512;
    int currentNumChannels_ = 2;

    // Cached parameter values for denormalization
    float bankAMod1_ = 0.0f;
    float bankAMod2_ = 0.0f;
    float bankAMod3_ = 0.0f;
    float bankBMod1_ = 0.0f;
    float bankBMod2_ = 0.0f;
    float bankBMod3_ = 0.0f;

    // Helper functions
    float denormalizeSize(float normalized) const;
    float denormalizeAutoScanRate(float normalized) const;
    int denormalizeRandomSeed(float normalized) const;
    float denormalizeTapeWindow(float normalized) const;
};

} // namespace echoform
