#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "TokenLookAndFeel.h"

#define ECHOFORM_USE_VDNA_UI 0

class StereoMemoryDelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit StereoMemoryDelayAudioProcessorEditor(StereoMemoryDelayAudioProcessor&);
    ~StereoMemoryDelayAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    StereoMemoryDelayAudioProcessor& audioProcessor;

    juce::Slider mixSlider;
    juce::Slider timeSlider;
    juce::Label mixLabel;
    juce::Label timeLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;

    TokenLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoMemoryDelayAudioProcessorEditor)
};

class VisualDNAEditor final : public juce::AudioProcessorEditor
{
public:
    explicit VisualDNAEditor(juce::AudioProcessor& processor)
        : juce::AudioProcessorEditor(&processor)
    {
        setSize(720, 420);
    }

    void paint(juce::Graphics&) override {}
    void resized() override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualDNAEditor)
};
