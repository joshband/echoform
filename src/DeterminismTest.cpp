#include <JuceHeader.h>
#include "MemoryDelayEngine.h"
#include <cstring>
#include <cmath>

class DeterminismTest final : public juce::UnitTest
{
public:
    DeterminismTest()
        : juce::UnitTest("EchoformDeterminism")
    {
    }

    void runTest() override
    {
        beginTest("Offline renders are bit-identical with same seed");

        constexpr double sampleRate = 48000.0;
        constexpr int blockSize = 128;
        constexpr float maxDelaySeconds = 10.0f;

        MemoryDelayEngine engineA;
        MemoryDelayEngine engineB;
        engineA.prepare(sampleRate, blockSize, maxDelaySeconds);
        engineB.prepare(sampleRate, blockSize, maxDelaySeconds);

        engineA.setMix(1.0f);
        engineB.setMix(1.0f);
        engineA.setScan(0.25f);
        engineB.setScan(0.25f);
        engineA.setAutoScanRate(0.35f);
        engineB.setAutoScanRate(0.35f);
        engineA.setSpread(0.3f);
        engineB.setSpread(0.3f);
        engineA.setFeedback(0.6f);
        engineB.setFeedback(0.6f);
        engineA.setTime(5.0f);
        engineB.setTime(5.0f);
        engineA.setCharacter(0.7f);
        engineB.setCharacter(0.7f);
        engineA.setStereoMode(static_cast<int>(MemoryDelayEngine::StereoMode::Independent));
        engineB.setStereoMode(static_cast<int>(MemoryDelayEngine::StereoMode::Independent));
        engineA.setMode(static_cast<int>(MemoryDelayEngine::FeedbackMode::Closed));
        engineB.setMode(static_cast<int>(MemoryDelayEngine::FeedbackMode::Closed));
        engineA.setRandomSeed(1234);
        engineB.setRandomSeed(1234);

        juce::AudioBuffer<float> input(2, blockSize);
        juce::AudioBuffer<float> bufferA(2, blockSize);
        juce::AudioBuffer<float> bufferB(2, blockSize);

        for (int block = 0; block < 32; ++block)
        {
            input.clear();
            for (int i = 0; i < blockSize; ++i)
            {
                const float signal = std::sin(0.01f * static_cast<float>(block * blockSize + i));
                input.setSample(0, i, signal);
                input.setSample(1, i, signal * 0.9f);
            }

            bufferA.makeCopyOf(input, true);
            bufferB.makeCopyOf(input, true);

            const int64_t transport = static_cast<int64_t>(block * blockSize);
            engineA.setTransportPosition(transport, true);
            engineB.setTransportPosition(transport, true);

            engineA.processBlock(bufferA);
            engineB.processBlock(bufferB);

            for (int ch = 0; ch < 2; ++ch)
            {
                for (int i = 0; i < blockSize; ++i)
                {
                    const float a = bufferA.getSample(ch, i);
                    const float b = bufferB.getSample(ch, i);
                    uint32_t bitsA = 0;
                    uint32_t bitsB = 0;
                    std::memcpy(&bitsA, &a, sizeof(float));
                    std::memcpy(&bitsB, &b, sizeof(float));
                    if (bitsA != bitsB)
                    {
                        expect(false, "Sample mismatch at block " + juce::String(block) + ", ch " + juce::String(ch)
                                            + ", idx " + juce::String(i));
                        return;
                    }
                }
            }
        }
    }
};

static DeterminismTest determinismTest;
