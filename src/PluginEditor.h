#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "TokenLookAndFeel.h"

#define ECHOFORM_USE_VDNA_UI 0

#if ECHOFORM_USE_VDNA_UI
class FeedbackComponent;
#endif

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
    juce::Label feedbackLabel;
    juce::Label mixLabel;
    juce::Label timeLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;
#if ECHOFORM_USE_VDNA_UI
    std::unique_ptr<FeedbackComponent> feedbackComponent;
#else
    juce::Slider feedbackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
#endif

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
