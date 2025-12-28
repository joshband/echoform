// MemoryBuffer.h
//
// A simple circular memory buffer for storing stereo audio samples.
// This class allocates a fixed‑size buffer and allows writing
// incoming audio as well as reading past samples at arbitrary
// fractional delays.  It is designed to be thread‑safe when called
// from the audio thread only (no locking).

#pragma once

#include <JuceHeader.h>

/**
    A circular audio buffer that records incoming stereo samples and
    allows random access reads into the past.  The buffer length is
    configured at prepare() time based on the maximum delay time.  A
    single write pointer is maintained; reads compute positions
    relative to this pointer.  Linear interpolation is used for
    fractional sample positions.
*/
class MemoryBuffer
{
public:
    MemoryBuffer() = default;
    ~MemoryBuffer() = default;

    /** Prepares the buffer.  Must be called before use.  @param sampleRate the
        current sample rate; @param maxDelaySeconds the maximum number of
        seconds we need to store. */
    void prepare(double sampleRate, float maxDelaySeconds)
    {
        jassert(sampleRate > 0.0);
        const int maxSamples = static_cast<int>(sampleRate * maxDelaySeconds) + 1;
        buffer.setSize(2, maxSamples);
        buffer.clear();
        writePos = 0;
    }

    /** Writes a block of input samples into the buffer.  The input
        buffer must have at least two channels.  Only the first two
        channels are recorded. */
    void write(const juce::AudioBuffer<float>& input, int numSamples)
    {
        const int bufferSize = buffer.getNumSamples();
        for (int sample = 0; sample < numSamples; ++sample)
        {
            for (int channel = 0; channel < 2; ++channel)
            {
                auto* dest = buffer.getWritePointer(channel);
                auto* src  = input.getReadPointer(channel);
                dest[writePos] = src[sample];
            }
            // advance write position and wrap
            if (++writePos >= bufferSize)
                writePos = 0;
        }
    }

    /** Reads a sample at the given delay in samples for the specified channel.
        Delay values should be positive, where 0 returns the most recently
        written sample.  Fractional delays are linearly interpolated.
        @param channel The channel index (0 = left, 1 = right)
        @param delayInSamples The delay time in samples.
        @return The interpolated sample value from the past. */
    float read(int channel, float delayInSamples) const
    {
        jassert(channel >= 0 && channel < buffer.getNumChannels());
        const int bufferSize = buffer.getNumSamples();
        // compute the read position relative to writePos
        float readPos = static_cast<float>(writePos) - delayInSamples;
        // wrap into buffer range
        while (readPos < 0.0f)
            readPos += bufferSize;
        while (readPos >= bufferSize)
            readPos -= bufferSize;
        // integer part and fractional part
        int index1 = static_cast<int>(readPos);
        int index2 = index1 + 1;
        if (index2 >= bufferSize)
            index2 -= bufferSize;
        float frac = readPos - static_cast<float>(index1);
        const auto* src = buffer.getReadPointer(channel);
        float s1 = src[index1];
        float s2 = src[index2];
        return s1 + frac * (s2 - s1);
    }

    /** Returns the current maximum delay in samples. */
    int getBufferSize() const { return buffer.getNumSamples(); }

    /** Writes a single stereo sample into the buffer.  This avoids
        allocating a temporary AudioBuffer for one sample.  Call this
        from the audio thread only. */
    void writeSample(float left, float right)
    {
        const int bufferSize = buffer.getNumSamples();
        buffer.setSample(0, writePos, left);
        buffer.setSample(1, writePos, right);
        if (++writePos >= bufferSize)
            writePos = 0;
    }

private:
    juce::AudioBuffer<float> buffer;
    int writePos { 0 };
};