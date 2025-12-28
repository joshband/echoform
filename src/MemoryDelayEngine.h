#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "RandomGenerator.h"

class MemoryDelayEngine
{
public:
    void prepare(double sampleRate, int maximumBlockSize)
    {
        juce::ignoreUnused(sampleRate, maximumBlockSize);
        autoScanTarget = 0.0f;
    }

    void updateRandomSeed(double randomSeed, double transportPosition)
    {
        randomGenerator.setSeed(randomSeed, transportPosition);
        autoScanTarget = static_cast<float>(randomGenerator.nextFloat01());
    }

    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        juce::ignoreUnused(buffer);
        autoScanTarget = static_cast<float>(randomGenerator.nextFloat01());
    }

    float getAutoScanTarget() const noexcept
    {
        return autoScanTarget;
    }

    float nextModifierRandom() noexcept
    {
        return static_cast<float>(randomGenerator.nextFloat01());
    }

private:
    RandomGenerator randomGenerator;
    float autoScanTarget = 0.0f;
};
