#pragma once

#include <JuceHeader.h>
#include "RandomGenerator.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <vector>

class Modifier
{
public:
    virtual ~Modifier() = default;
    virtual void prepare(double newSampleRate, int maxBlockSize, int numChannels) = 0;
    virtual void reset() = 0;
    virtual float processSample(float input, int channel, RandomGenerator& random) = 0;

    virtual void setIntensity(float newIntensity)
    {
        intensity = juce::jlimit(0.0f, 1.0f, newIntensity);
    }

protected:
    float intensity { 0.0f };
};

class LowPassModifier final : public Modifier
{
public:
    void prepare(double newSampleRate, int, int numChannels) override
    {
        sampleRate = newSampleRate;
        state.assign(static_cast<size_t>(numChannels), 0.0f);
        updateCoefficient();
    }

    void reset() override
    {
        std::fill(state.begin(), state.end(), 0.0f);
    }

    void setIntensity(float newIntensity) override
    {
        Modifier::setIntensity(newIntensity);
        updateCoefficient();
    }

    float processSample(float input, int channel, RandomGenerator&) override
    {
        if (intensity <= 0.0001f)
            return input;

        float output = (1.0f - coefficient) * input + coefficient * state[static_cast<size_t>(channel)];
        state[static_cast<size_t>(channel)] = output;
        return output;
    }

private:
    void updateCoefficient()
    {
        if (sampleRate <= 0.0)
            return;

        const float maxCutoff = 16000.0f;
        const float minCutoff = 400.0f;
        const float cutoff = juce::jmap(intensity, 0.0f, 1.0f, maxCutoff, minCutoff);
        coefficient = std::exp(-2.0f * juce::MathConstants<float>::pi * cutoff / static_cast<float>(sampleRate));
    }

    double sampleRate { 44100.0 };
    float coefficient { 0.0f };
    std::vector<float> state;
};

class ModulatedDelayLine
{
public:
    void prepare(double newSampleRate, float maxDelayMs, int numChannels)
    {
        sampleRate = newSampleRate;
        const int maxSamples = juce::jmax(1, static_cast<int>(std::ceil(sampleRate * (maxDelayMs / 1000.0f))) + 2);
        buffer.setSize(numChannels, maxSamples);
        buffer.clear();
        writePos = 0;
    }

    void reset()
    {
        buffer.clear();
        writePos = 0;
    }

    float readSample(int channel, float delaySamples) const
    {
        const int bufferSize = buffer.getNumSamples();
        delaySamples = juce::jlimit(0.0f, static_cast<float>(bufferSize - 1), delaySamples);
        float readPos = static_cast<float>(writePos) - delaySamples;
        while (readPos < 0.0f)
            readPos += bufferSize;
        while (readPos >= bufferSize)
            readPos -= bufferSize;
        const int index1 = static_cast<int>(readPos);
        int index2 = index1 + 1;
        if (index2 >= bufferSize)
            index2 -= bufferSize;
        const float frac = readPos - static_cast<float>(index1);
        const float* data = buffer.getReadPointer(channel);
        const float s1 = data[index1];
        const float s2 = data[index2];
        return s1 + frac * (s2 - s1);
    }

    void writeSample(int channel, float input)
    {
        buffer.setSample(channel, writePos, input);
    }

    void advance()
    {
        if (++writePos >= buffer.getNumSamples())
            writePos = 0;
    }

private:
    double sampleRate { 44100.0 };
    juce::AudioBuffer<float> buffer;
    int writePos { 0 };
};

class WowFlutterModifier final : public Modifier
{
public:
    void prepare(double newSampleRate, int, int numChannels) override
    {
        sampleRate = newSampleRate;
        channels = numChannels;
        delayLine.prepare(sampleRate, 12.0f, numChannels);
        updateParameters();
    }

    void reset() override
    {
        delayLine.reset();
        wowPhase = 0.0f;
        flutterPhase = 0.0f;
    }

    void setIntensity(float newIntensity) override
    {
        Modifier::setIntensity(newIntensity);
        updateParameters();
    }

    float processSample(float input, int channel, RandomGenerator&) override
    {
        if (intensity <= 0.0001f)
            return input;

        if (channel == 0)
        {
            const float wow = std::sin(wowPhase);
            const float flutter = std::sin(flutterPhase);
            const float modMs = (wow * 0.7f + flutter * 0.3f) * depthMs;
            currentDelaySamples = (baseDelayMs + modMs) * static_cast<float>(sampleRate) / 1000.0f;
        }

        float delayed = delayLine.readSample(channel, currentDelaySamples);
        delayLine.writeSample(channel, input);

        if (channel == channels - 1)
        {
            delayLine.advance();
            wowPhase += wowPhaseStep;
            flutterPhase += flutterPhaseStep;
            if (wowPhase > juce::MathConstants<float>::twoPi)
                wowPhase -= juce::MathConstants<float>::twoPi;
            if (flutterPhase > juce::MathConstants<float>::twoPi)
                flutterPhase -= juce::MathConstants<float>::twoPi;
        }

        return input + (delayed - input) * intensity;
    }

private:
    void updateParameters()
    {
        const float wowRate = juce::jmap(intensity, 0.05f, 0.6f);
        const float flutterRate = juce::jmap(intensity, 1.8f, 6.5f);
        depthMs = juce::jmap(intensity, 0.0f, 3.5f);
        baseDelayMs = 4.0f + depthMs;
        wowPhaseStep = juce::MathConstants<float>::twoPi * wowRate / static_cast<float>(sampleRate);
        flutterPhaseStep = juce::MathConstants<float>::twoPi * flutterRate / static_cast<float>(sampleRate);
    }

    double sampleRate { 44100.0 };
    int channels { 2 };
    ModulatedDelayLine delayLine;
    float wowPhase { 0.0f };
    float flutterPhase { 0.0f };
    float wowPhaseStep { 0.0f };
    float flutterPhaseStep { 0.0f };
    float depthMs { 0.0f };
    float baseDelayMs { 4.0f };
    float currentDelaySamples { 0.0f };
};

class PitchDriftModifier final : public Modifier
{
public:
    void prepare(double newSampleRate, int, int numChannels) override
    {
        sampleRate = newSampleRate;
        channels = numChannels;
        delayLine.prepare(sampleRate, 8.0f, numChannels);
        reset();
    }

    void reset() override
    {
        delayLine.reset();
        driftCurrentMs = 0.0f;
        driftTargetMs = 0.0f;
        driftStepMs = 0.0f;
        driftSamplesRemaining = 0;
    }

    float processSample(float input, int channel, RandomGenerator& random) override
    {
        if (intensity <= 0.0001f)
            return input;

        if (channel == 0)
        {
            if (driftSamplesRemaining <= 0)
            {
                const float depthMs = juce::jmap(intensity, 0.0f, 2.2f);
                driftTargetMs = random.nextFloatSigned() * depthMs;
                const int rampSamples = juce::jmax(1, static_cast<int>(sampleRate * 0.6f));
                driftStepMs = (driftTargetMs - driftCurrentMs) / static_cast<float>(rampSamples);
                driftSamplesRemaining = rampSamples;
            }

            driftCurrentMs += driftStepMs;
            --driftSamplesRemaining;
            currentDelaySamples = (baseDelayMs + driftCurrentMs) * static_cast<float>(sampleRate) / 1000.0f;
        }

        float delayed = delayLine.readSample(channel, currentDelaySamples);
        delayLine.writeSample(channel, input);

        if (channel == channels - 1)
            delayLine.advance();

        return input + (delayed - input) * intensity;
    }

private:
    double sampleRate { 44100.0 };
    int channels { 2 };
    ModulatedDelayLine delayLine;
    float baseDelayMs { 3.0f };
    float driftCurrentMs { 0.0f };
    float driftTargetMs { 0.0f };
    float driftStepMs { 0.0f };
    int driftSamplesRemaining { 0 };
    float currentDelaySamples { 0.0f };
};

class DropoutModifier final : public Modifier
{
public:
    void prepare(double newSampleRate, int, int numChannels) override
    {
        sampleRate = newSampleRate;
        channels = numChannels;
        reset();
    }

    void reset() override
    {
        dropoutSamplesRemaining = 0;
        dropoutGain = 1.0f;
    }

    float processSample(float input, int channel, RandomGenerator& random) override
    {
        if (intensity <= 0.0001f)
            return input;

        if (channel == 0)
        {
            if (dropoutSamplesRemaining <= 0)
            {
                const float probability = juce::jmap(intensity, 0.0f, 0.0006f);
                if (random.nextFloat01() < probability)
                {
                    dropoutSamplesRemaining = juce::jmax(1, static_cast<int>(sampleRate * random.nextFloatRange(0.01f, 0.08f)));
                    dropoutGain = juce::jmap(intensity, 1.0f, 0.2f);
                }
            }
        }

        const bool applyDropout = dropoutSamplesRemaining > 0;
        const float output = applyDropout ? input * dropoutGain : input;

        if (channel == channels - 1 && dropoutSamplesRemaining > 0)
            --dropoutSamplesRemaining;

        return output;
    }

private:
    double sampleRate { 44100.0 };
    int channels { 2 };
    int dropoutSamplesRemaining { 0 };
    float dropoutGain { 1.0f };
};

class ModifierChain
{
public:
    void prepare(double newSampleRate, int maxBlockSize, int numChannels)
    {
        lowPass.prepare(newSampleRate, maxBlockSize, numChannels);
        pitchDrift.prepare(newSampleRate, maxBlockSize, numChannels);
        wowFlutter.prepare(newSampleRate, maxBlockSize, numChannels);
        dropout.prepare(newSampleRate, maxBlockSize, numChannels);
    }

    void reset()
    {
        lowPass.reset();
        pitchDrift.reset();
        wowFlutter.reset();
        dropout.reset();
    }

    void setCharacter(float character)
    {
        character = juce::jlimit(0.0f, 1.0f, character);
        lowPass.setIntensity(character * 0.9f);
        pitchDrift.setIntensity(character * 0.6f);
        wowFlutter.setIntensity(character);
        dropout.setIntensity(character * character);
    }

    float processSample(float input, int channel, RandomGenerator& random)
    {
        float output = input;
        output = lowPass.processSample(output, channel, random);
        output = pitchDrift.processSample(output, channel, random);
        output = wowFlutter.processSample(output, channel, random);
        output = dropout.processSample(output, channel, random);
        return output;
    }

private:
    LowPassModifier lowPass;
    PitchDriftModifier pitchDrift;
    WowFlutterModifier wowFlutter;
    DropoutModifier dropout;
};
