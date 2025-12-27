#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class StereoMemoryDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StereoMemoryDelayAudioProcessorEditor (StereoMemoryDelayAudioProcessor&);
    ~StereoMemoryDelayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    StereoMemoryDelayAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoMemoryDelayAudioProcessorEditor)
};
