// echoform_adapter.cpp

#include "echoform_adapter.h"
#include <cstring>
#include <cmath>

namespace echoform {

EchoformAdapter::EchoformAdapter()
    : engine_(std::make_unique<MemoryDelayEngine>())
    , juceBuffer_(std::make_unique<juce::AudioBuffer<float>>())
{
}

void EchoformAdapter::prepare(double sampleRate, int maxBlockSize, int numChannels) noexcept
{
    currentSampleRate_ = sampleRate;
    currentMaxBlockSize_ = maxBlockSize;
    currentNumChannels_ = numChannels;

    // Echoform MemoryDelayEngine is stereo-only, but we'll adapt
    const int engineChannels = 2;
    juceBuffer_->setSize(engineChannels, maxBlockSize, false, true, false);

    // Prepare with 180-second buffer (kMemorySeconds)
    engine_->prepare(sampleRate, maxBlockSize, 180.0f);

    // Initialize with sensible defaults for testing
    // (until parameter_variations are properly supported in scenarios)
    engine_->setMix(0.5f);              // 50% wet/dry mix
    engine_->setScan(0.5f);             // Center scan
    engine_->setFeedback(0.3f);         // 30% feedback
    engine_->setSize(0.1f);             // 100ms delay (short for testing)
    engine_->setAlwaysRecord(true);     // Enable recording
    engine_->setCharacter(0.5f);        // Neutral character
    engine_->setSpread(0.5f);           // Medium spread
}

void EchoformAdapter::release() noexcept
{
    // No explicit release needed for MemoryDelayEngine
    juceBuffer_.reset();
}

void EchoformAdapter::processBlock(float** channelData, int numChannels, int numSamples) noexcept
{
    // Echoform is stereo-only, adapt mono/multichannel input
    juceBuffer_->clear();

    if (numChannels == 1)
    {
        // Mono → duplicate to stereo
        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = channelData[0][i];
            juceBuffer_->setSample(0, i, sample);
            juceBuffer_->setSample(1, i, sample);
        }
    }
    else
    {
        // Stereo or multichannel → copy first 2 channels
        for (int ch = 0; ch < 2; ++ch)
        {
            const int srcCh = (ch < numChannels) ? ch : 0;
            for (int i = 0; i < numSamples; ++i)
                juceBuffer_->setSample(ch, i, channelData[srcCh][i]);
        }
    }

    // Process through engine
    engine_->processBlock(*juceBuffer_);

    // Copy back to output
    if (numChannels == 1)
    {
        // Stereo → mono (mix down)
        for (int i = 0; i < numSamples; ++i)
        {
            const float left = juceBuffer_->getSample(0, i);
            const float right = juceBuffer_->getSample(1, i);
            channelData[0][i] = 0.5f * (left + right);
        }
    }
    else
    {
        // Copy stereo output back
        for (int ch = 0; ch < numChannels && ch < 2; ++ch)
        {
            for (int i = 0; i < numSamples; ++i)
                channelData[ch][i] = juceBuffer_->getSample(ch, i);
        }
    }
}

void EchoformAdapter::setParameter(int parameterIndex, qa::NormalizedParam normalizedValue) noexcept
{
    const float value = normalizedValue; // NormalizedParam is just a float alias

    switch (parameterIndex)
    {
        case MIX:
            engine_->setMix(value);
            break;

        case SCAN:
            engine_->setScan(value);
            break;

        case AUTO_SCAN_RATE:
            engine_->setAutoScanRate(denormalizeAutoScanRate(value));
            break;

        case SPREAD:
            engine_->setSpread(value);
            break;

        case FEEDBACK:
            engine_->setFeedback(value * 0.98f); // Scale to 0.0–0.98
            break;

        case SIZE:
            engine_->setSize(denormalizeSize(value));
            break;

        case CHARACTER:
            engine_->setCharacter(value);
            break;

        case STEREO_MODE:
            engine_->setStereoMode(static_cast<int>(value * 2.999f)); // 0–2
            break;

        case FEEDBACK_MODE:
            engine_->setMode(static_cast<int>(value * 2.999f)); // 0–2
            break;

        case SCAN_MODE:
            engine_->setScanMode(static_cast<int>(value * 1.999f)); // 0–1
            break;

        case ROUTING_MODE_A:
            engine_->setRoutingModeA(static_cast<int>(value * 2.999f)); // 0–2
            break;

        case ROUTING_MODE_B:
            engine_->setRoutingModeB(static_cast<int>(value * 2.999f)); // 0–2
            break;

        case BANK_A_MOD1:
            bankAMod1_ = value;
            engine_->setModifierBankA(bankAMod1_, bankAMod2_, bankAMod3_);
            break;

        case BANK_A_MOD2:
            bankAMod2_ = value;
            engine_->setModifierBankA(bankAMod1_, bankAMod2_, bankAMod3_);
            break;

        case BANK_A_MOD3:
            bankAMod3_ = value;
            engine_->setModifierBankA(bankAMod1_, bankAMod2_, bankAMod3_);
            break;

        case BANK_B_MOD1:
            bankBMod1_ = value;
            engine_->setModifierBankB(bankBMod1_, bankBMod2_, bankBMod3_);
            break;

        case BANK_B_MOD2:
            bankBMod2_ = value;
            engine_->setModifierBankB(bankBMod1_, bankBMod2_, bankBMod3_);
            break;

        case BANK_B_MOD3:
            bankBMod3_ = value;
            engine_->setModifierBankB(bankBMod1_, bankBMod2_, bankBMod3_);
            break;

        case ALWAYS_RECORD:
            engine_->setAlwaysRecord(value >= 0.5f);
            break;

        case DRY_KILL:
            engine_->setDryKill(value >= 0.5f);
            break;

        case LATCH:
            engine_->setLatch(value >= 0.5f);
            break;

        case TRAILS:
            engine_->setTrails(value >= 0.5f);
            break;

        case MEMORY_DRY:
            engine_->setMemoryDry(value >= 0.5f);
            break;

        case WIPE:
            engine_->setWipe(value >= 0.5f);
            break;

        case BYPASSED:
            engine_->setBypassed(value >= 0.5f);
            break;

        case RANDOM_SEED:
            engine_->setRandomSeed(denormalizeRandomSeed(value));
            break;

        case TAPE_MODE:
            engine_->setTapeMode(value >= 0.5f);
            break;

        case TAPE_WINDOW_SECONDS:
            engine_->setTapeWindowSeconds(denormalizeTapeWindow(value));
            break;

        default:
            break;
    }
}

void EchoformAdapter::reset() noexcept
{
    engine_->reset();
}

bool EchoformAdapter::getCapabilities(qa::EffectCapabilities& caps) const
{
    caps.effectTypes = qa::EffectType::DELAY; // Delay/looper hybrid
    caps.behaviors = qa::BehaviorFlag::STATEFUL | qa::BehaviorFlag::TIME_VARYING |
                     qa::BehaviorFlag::FEEDBACK | qa::BehaviorFlag::NONLINEAR;
    caps.description = "Echoform: Stereo Memory Delay with dual playheads, auto-scan, and modifier chains";
    return true;
}

qa::OptionalFeatures EchoformAdapter::getOptionalFeatures() const
{
    qa::OptionalFeatures features;
    features.supportsReset = true;
    features.supportsMidiInput = false;
    features.supportsMidiOutput = false;
    features.supportsTransport = true; // Uses setTransportPosition()
    features.supportsCapabilities = true;
    return features;
}

const char* EchoformAdapter::getParameterName(int index) const
{
    static const char* names[NUM_PARAMETERS] = {
        "Mix",
        "Scan",
        "Auto Scan Rate",
        "Spread",
        "Feedback",
        "Size",
        "Character",
        "Stereo Mode",
        "Feedback Mode",
        "Scan Mode",
        "Routing Mode A",
        "Routing Mode B",
        "Bank A Mod 1 (Wow/Flutter)",
        "Bank A Mod 2 (Dropout)",
        "Bank A Mod 3 (Tone)",
        "Bank B Mod 1 (Wow/Flutter)",
        "Bank B Mod 2 (Dropout)",
        "Bank B Mod 3 (Tone)",
        "Always Record",
        "Dry Kill",
        "Latch",
        "Trails",
        "Memory Dry",
        "Wipe",
        "Bypassed",
        "Random Seed",
        "Tape Mode",
        "Tape Window Seconds"
    };

    if (index < 0 || index >= NUM_PARAMETERS)
        return nullptr;

    return names[index];
}

// Helper functions
float EchoformAdapter::denormalizeSize(float normalized) const
{
    // Size: 0.05–60.0 seconds (logarithmic scale)
    constexpr float minSize = 0.05f;
    constexpr float maxSize = 60.0f;
    const float logMin = std::log(minSize);
    const float logMax = std::log(maxSize);
    return std::exp(logMin + normalized * (logMax - logMin));
}

float EchoformAdapter::denormalizeAutoScanRate(float normalized) const
{
    // Auto scan rate: 0.0–2.0 Hz (linear)
    return normalized * 2.0f;
}

int EchoformAdapter::denormalizeRandomSeed(float normalized) const
{
    // Random seed: 0–65535 (integer)
    return static_cast<int>(normalized * 65535.0f);
}

float EchoformAdapter::denormalizeTapeWindow(float normalized) const
{
    // Tape window: 0.0–30.0 seconds (linear)
    return normalized * 30.0f;
}

} // namespace echoform
