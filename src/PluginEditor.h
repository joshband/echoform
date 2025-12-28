#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class StereoMemoryDelayAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit StereoMemoryDelayAudioProcessorEditor(StereoMemoryDelayAudioProcessor&);
    ~StereoMemoryDelayAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    StereoMemoryDelayAudioProcessor& processor;
    juce::Slider randomSeedSlider;
    juce::Label randomSeedLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randomSeedAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoMemoryDelayAudioProcessorEditor)
};
