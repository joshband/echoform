#include <JuceHeader.h>
#include "MemoryDelayEngine.h"

#include <cassert>
#include <cmath>

namespace {
void testWraparound()
{
    MemoryDelayEngine engine;
    engine.prepare(10.0, 16, 1.0f);
    engine.setMix(1.0f);
    engine.setFeedback(0.0f);
    engine.setAutoScanRate(0.0f);
    engine.setScan(1.0f);
    engine.setSpread(0.0f);
    engine.setTime(1.0f);
    engine.setCharacter(0.0f);
    engine.setMode(static_cast<int>(MemoryDelayEngine::FeedbackMode::Collect));

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
} // namespace

int main()
{
    testWraparound();
    return 0;
}
