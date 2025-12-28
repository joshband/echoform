#include "MemoryDelayEngine.h"

#include <algorithm>
#include <cmath>

namespace echoform {

namespace {
constexpr float kMinSizeMs = 50.0f;
constexpr float kMaxSizeMs = 60000.0f;
constexpr float kMaxRepeats = 0.995f;
constexpr float kScanMinRate = 0.05f;
constexpr float kScanMaxRate = 1.0f;
constexpr float kEnergyDecay = 0.98f;
}

MemoryDelayEngine::MemoryDelayEngine() = default;

void MemoryDelayEngine::prepare(double sampleRate, int maxBlockSize, double maxSeconds) {
    sampleRate_ = sampleRate;
    maxSamples_ = static_cast<int>(std::ceil(sampleRate_ * maxSeconds));
    bufferL_.assign(static_cast<size_t>(maxSamples_), 0.0f);
    bufferR_.assign(static_cast<size_t>(maxSamples_), 0.0f);

    inspect_.energy.assign(128, 0.0f);

    writeIndex_ = 0;
    sizeSamplesCurrent_ = sampleRate_ * (params_.sizeMs / 1000.0);
    sizeSamplesTarget_ = sizeSamplesCurrent_;
    sizeSlewPerSample_ = 0.0;
    scanPhase_ = 0.0;

    filterStateA_ = 0.0f;
    filterStateB_ = 0.0f;
    filterStateASecondary_ = 0.0f;
    filterStateBSecondary_ = 0.0f;

    reset();
}

void MemoryDelayEngine::reset() {
    std::fill(bufferL_.begin(), bufferL_.end(), 0.0f);
    std::fill(bufferR_.begin(), bufferR_.end(), 0.0f);
    std::fill(inspect_.energy.begin(), inspect_.energy.end(), 0.0f);
    writeIndex_ = 0;
    scanPhase_ = 0.0;
}

void MemoryDelayEngine::setParameters(const MemoryDelayParameters& params) {
    params_ = params;
    const float clampedSizeMs = std::clamp(params_.sizeMs, kMinSizeMs, kMaxSizeMs);
    sizeSamplesTarget_ = (clampedSizeMs / 1000.0) * sampleRate_;
    const double slewSeconds = 0.05;
    const double slewSamples = std::max(1.0, slewSeconds * sampleRate_);
    sizeSlewPerSample_ = (sizeSamplesTarget_ - sizeSamplesCurrent_) / slewSamples;
}

void MemoryDelayEngine::processBlock(const float* const* input, float* const* output, int numSamples) {
    if (maxSamples_ == 0) {
        return;
    }

    const float repeats = std::clamp(params_.repeats, 0.0f, kMaxRepeats);
    const float scanAmount = std::clamp(params_.scan, 0.0f, 1.0f);
    const float spreadAmount = std::clamp(params_.spread, 0.0f, 1.0f);
    const bool bypassed = params_.bypassed;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex) {
        sizeSamplesCurrent_ += sizeSlewPerSample_;
        if ((sizeSlewPerSample_ > 0.0 && sizeSamplesCurrent_ > sizeSamplesTarget_) ||
            (sizeSlewPerSample_ < 0.0 && sizeSamplesCurrent_ < sizeSamplesTarget_)) {
            sizeSamplesCurrent_ = sizeSamplesTarget_;
            sizeSlewPerSample_ = 0.0;
        }

        const double sizeSamples = std::clamp(sizeSamplesCurrent_, 1.0, static_cast<double>(maxSamples_ - 1));
        const double sizeSeconds = sizeSamples / sampleRate_;

        double delayOffsetSamples = 0.0;
        if (params_.scanMode == ScanMode::Manual) {
            delayOffsetSamples = scanAmount * sizeSamples;
        } else {
            const double rateHz = kScanMinRate + (kScanMaxRate - kScanMinRate) * scanAmount;
            const double phaseIncrement = rateHz / std::max(1.0, sizeSeconds);
            scanPhase_ += phaseIncrement / sampleRate_;
            if (scanPhase_ >= 1.0) {
                scanPhase_ -= 1.0;
            }
            const double triangle = scanPhase_ < 0.5 ? (scanPhase_ * 2.0) : (2.0 - scanPhase_ * 2.0);
            delayOffsetSamples = triangle * scanAmount * sizeSamples;
        }

        const double primaryReadPos = wrapPosition(static_cast<double>(writeIndex_) - delayOffsetSamples);
        const double secondaryOffset = spreadAmount * sizeSamples;
        const double secondaryReadPos = wrapPosition(primaryReadPos - secondaryOffset);

        const float readL = readInterpolated(bufferL_, primaryReadPos);
        const float readR = readInterpolated(bufferR_, primaryReadPos);
        const float readSecondaryL = readInterpolated(bufferL_, secondaryReadPos);
        const float readSecondaryR = readInterpolated(bufferR_, secondaryReadPos);

        float wetL = 0.5f * (readL + readSecondaryL);
        float wetR = 0.5f * (readR + readSecondaryR);

        if (params_.bankA.routing == RoutingMode::Out) {
            wetL = applyBank(wetL, filterStateA_, params_.bankA);
            wetR = applyBank(wetR, filterStateASecondary_, params_.bankA);
        }
        if (params_.bankB.routing == RoutingMode::Out) {
            wetL = applyBank(wetL, filterStateB_, params_.bankB);
            wetR = applyBank(wetR, filterStateBSecondary_, params_.bankB);
        }

        const float inputL = input ? input[0][sampleIndex] : 0.0f;
        const float inputR = input ? input[1][sampleIndex] : 0.0f;

        float outL = wetL;
        float outR = wetR;

        if (!params_.wipe) {
            if (!bypassed) {
                outL += inputL;
                outR += inputR;
            } else {
                outL = inputL;
                outR = inputR;
            }
        }

        output[0][sampleIndex] = outL;
        output[1][sampleIndex] = outR;

        if (params_.wipe) {
            continue;
        }

        const bool shouldRecord = !bypassed || params_.alwaysRecord;
        float writeL = bufferL_[static_cast<size_t>(writeIndex_)];
        float writeR = bufferR_[static_cast<size_t>(writeIndex_)];

        if (shouldRecord) {
            float overdubL = inputL;
            float overdubR = inputR;
            if (params_.bankA.routing == RoutingMode::In) {
                overdubL = applyBank(overdubL, filterStateA_, params_.bankA);
                overdubR = applyBank(overdubR, filterStateASecondary_, params_.bankA);
            }
            if (params_.bankB.routing == RoutingMode::In) {
                overdubL = applyBank(overdubL, filterStateB_, params_.bankB);
                overdubR = applyBank(overdubR, filterStateBSecondary_, params_.bankB);
            }

            if (params_.collect) {
                writeL = writeL * 0.98f + overdubL;
                writeR = writeR * 0.98f + overdubR;
            } else {
                writeL = overdubL;
                writeR = overdubR;
            }
        }

        float feedbackL = wetL * repeats;
        float feedbackR = wetR * repeats;

        if (params_.bankA.routing == RoutingMode::Feed) {
            feedbackL = applyBank(feedbackL, filterStateA_, params_.bankA);
            feedbackR = applyBank(feedbackR, filterStateASecondary_, params_.bankA);
        }
        if (params_.bankB.routing == RoutingMode::Feed) {
            feedbackL = applyBank(feedbackL, filterStateB_, params_.bankB);
            feedbackR = applyBank(feedbackR, filterStateBSecondary_, params_.bankB);
        }

        writeL += feedbackL;
        writeR += feedbackR;

        bufferL_[static_cast<size_t>(writeIndex_)] = writeL;
        bufferR_[static_cast<size_t>(writeIndex_)] = writeR;

        if (params_.inspectEnabled) {
            const size_t energyIndex = static_cast<size_t>((static_cast<double>(writeIndex_) / maxSamples_) * inspect_.energy.size());
            const float energyValue = std::abs(writeL) + std::abs(writeR);
            inspect_.energy[energyIndex] = inspect_.energy[energyIndex] * kEnergyDecay + energyValue * (1.0f - kEnergyDecay);
            inspect_.primaryPlayhead = static_cast<float>(primaryReadPos / maxSamples_);
            inspect_.secondaryPlayhead = static_cast<float>(secondaryReadPos / maxSamples_);
        }

        ++writeIndex_;
        if (writeIndex_ >= maxSamples_) {
            writeIndex_ = 0;
        }
    }
}

const InspectState& MemoryDelayEngine::getInspectState() const {
    return inspect_;
}

int MemoryDelayEngine::getMaxSamples() const {
    return maxSamples_;
}

int MemoryDelayEngine::getWriteIndex() const {
    return writeIndex_;
}

float MemoryDelayEngine::debugGetMemorySample(int channel, int index) const {
    if (channel == 0) {
        return bufferL_[static_cast<size_t>(index)];
    }
    return bufferR_[static_cast<size_t>(index)];
}

float MemoryDelayEngine::applyBank(float sample, float& filterState, const ModifierBank& bank) const {
    float value = sample;
    const float gain = 1.0f + bank.modifier1 * 0.5f;
    value *= gain;

    const float drive = std::clamp(bank.modifier2, -1.0f, 1.0f);
    const float driveAmount = 1.0f + std::abs(drive) * 2.0f;
    value = std::tanh(value * driveAmount) / std::tanh(driveAmount);

    const float tone = std::clamp(bank.modifier3, -1.0f, 1.0f);
    const float coefficient = 0.02f + 0.48f * (tone + 1.0f) * 0.5f;
    filterState = filterState + coefficient * (value - filterState);
    return filterState;
}

float MemoryDelayEngine::readInterpolated(const std::vector<float>& buffer, double position) const {
    const double wrapped = wrapPosition(position);
    const int indexA = static_cast<int>(wrapped);
    const int indexB = (indexA + 1) % maxSamples_;
    const float frac = static_cast<float>(wrapped - indexA);
    const float a = buffer[static_cast<size_t>(indexA)];
    const float b = buffer[static_cast<size_t>(indexB)];
    return a + (b - a) * frac;
}

double MemoryDelayEngine::wrapPosition(double position) const {
    while (position < 0.0) {
        position += maxSamples_;
    }
    while (position >= maxSamples_) {
        position -= maxSamples_;
    }
    return position;
}

} // namespace echoform
