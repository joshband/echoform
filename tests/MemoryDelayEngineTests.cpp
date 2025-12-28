#include <JuceHeader.h>
#include "MemoryDelayEngine.h"

#include <cassert>
#include <cmath>
#include <vector>

namespace {
void testWraparoundDsp()
{
    ::MemoryDelayEngine engine;
    engine.prepare(10.0, 16, 1.0f);
    engine.setMix(1.0f);
    engine.setFeedback(0.0f);
    engine.setAutoScanRate(0.0f);
    engine.setScan(1.0f);
    engine.setSpread(0.0f);
    engine.setTime(1.0f);
    engine.setCharacter(0.0f);
    engine.setMode(static_cast<int>(::MemoryDelayEngine::FeedbackMode::Collect));

    constexpr int numSamples = 25;
    juce::AudioBuffer<float> buffer(2, numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        const float base = static_cast<float>(i + 1) * 0.01f;
        buffer.setSample(0, i, base);
        buffer.setSample(1, i, base * 2.0f);
    }

    const float lastInputLeft = static_cast<float>(numSamples) * 0.01f;
    const float lastInputRight = lastInputLeft * 2.0f;

    engine.processBlock(buffer);

    const int maxSamples = engine.getMaxSamples();
    const int writeIndex = engine.getWriteIndex();
    const int lastIndex = (writeIndex - 1 + maxSamples) % maxSamples;

    const float expectedLeft = std::tanh(lastInputLeft);
    const float expectedRight = std::tanh(lastInputRight);

    const float lastLeft = engine.debugGetMemorySample(0, lastIndex);
    const float lastRight = engine.debugGetMemorySample(1, lastIndex);

    assert(std::abs(lastLeft - expectedLeft) < 1.0e-5f);
    assert(std::abs(lastRight - expectedRight) < 1.0e-5f);
}

void testWraparoundEchoform()
{
    echoform::MemoryDelayEngine engine;
    engine.prepare(10.0, 16, 1.0);

    echoform::MemoryDelayParameters params;
    params.sizeMs = 1000.0f;
    params.scanMode = echoform::ScanMode::Manual;
    params.scan = 1.0f;
    engine.setParameters(params);

    std::vector<float> inputL(25, 0.0f);
    std::vector<float> inputR(25, 0.0f);
    for (int i = 0; i < static_cast<int>(inputL.size()); ++i)
    {
        inputL[i] = static_cast<float>(i + 1);
        inputR[i] = static_cast<float>(i + 1) * 2.0f;
    }

    std::vector<float> outputL(inputL.size(), 0.0f);
    std::vector<float> outputR(inputL.size(), 0.0f);

    const float* inputs[2] = { inputL.data(), inputR.data() };
    float* outputs[2] = { outputL.data(), outputR.data() };

    engine.processBlock(inputs, outputs, static_cast<int>(inputL.size()));

    const int writeIndex = engine.getWriteIndex();
    const int lastIndex = (writeIndex - 1 + engine.getMaxSamples()) % engine.getMaxSamples();

    assert(engine.debugGetMemorySample(0, lastIndex) == inputL.back());
    assert(engine.debugGetMemorySample(1, lastIndex) == inputR.back());
}
} // namespace

int main()
{
    testWraparoundDsp();
    testWraparoundEchoform();
    return 0;
}
