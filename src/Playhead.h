// Playhead.h
//
// Represents a read head that scans through a MemoryBuffer at a
// specified delay.  The playhead can be controlled by a parameter
// value (0..1) representing a fraction of the maximum buffer length.

#pragma once

#include <JuceHeader.h>
#include "MemoryBuffer.h"

/**
    A read head that returns samples from a MemoryBuffer.  The head
    takes an offset parameter in the range [0, 1], where 0 means no
    delay (reading the most recently recorded sample) and 1 means the
    maximum delay (furthest point in the buffer).  An additional
    spread parameter allows offsetting the head relative to another
    head.
*/
class Playhead
{
public:
    Playhead() = default;
    ~Playhead() = default;

    /** Assigns the memory buffer to read from. */
    void setMemoryBuffer(const MemoryBuffer* mem) { memory = mem; }

    /** Sets the current offset parameter (0..1). */
    void setOffsetNormalized(float newOffset) { offsetNormalized = juce::jlimit(0.0f, 1.0f, newOffset); }

    /** Sets the spread (additional offset) relative to the other playhead
        in seconds.  This value is applied on top of the base offset.
        Spread can be negative to read ahead of the other head. */
    void setSpread(float newSpreadSeconds) { spreadSeconds = newSpreadSeconds; }

    /** Sets the maximum delay length in seconds.  This should correspond
        to the MemoryBuffer length. */
    void setMaxDelaySeconds(float seconds) { maxDelaySeconds = seconds; }

    /** Reads a single sample from the buffer for the given channel. */
    float readSample(int channel, double sampleRate) const
    {
        if (memory == nullptr)
            return 0.0f;
        // compute delay in samples = base offset * maxDelay + spread
        float totalDelaySeconds = offsetNormalized * maxDelaySeconds + spreadSeconds;
        if (totalDelaySeconds < 0.0f)
            totalDelaySeconds = 0.0f;
        float delaySamples = totalDelaySeconds * static_cast<float>(sampleRate);
        // clamp to buffer length
        float maxSamples = static_cast<float>(memory->getBufferSize());
        if (delaySamples > maxSamples - 1.0f)
            delaySamples = maxSamples - 1.0f;
        return memory->read(channel, delaySamples);
    }

    float readSample(int channel, double sampleRate, float maxDelaySecondsOverride, float spreadSecondsOverride) const
    {
        if (memory == nullptr)
            return 0.0f;

        float totalDelaySeconds = offsetNormalized * maxDelaySecondsOverride + spreadSecondsOverride;
        if (totalDelaySeconds < 0.0f)
            totalDelaySeconds = 0.0f;

        float delaySamples = totalDelaySeconds * static_cast<float>(sampleRate);
        const float maxSamples = static_cast<float>(memory->getBufferSize());
        if (delaySamples > maxSamples - 1.0f)
            delaySamples = maxSamples - 1.0f;

        return memory->read(channel, delaySamples);
    }

private:
    const MemoryBuffer* memory { nullptr };
    float offsetNormalized { 0.0f };
    float spreadSeconds { 0.0f };
    float maxDelaySeconds { 1.0f };
};
