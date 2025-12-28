#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "TokenLookAndFeel.h"
#include "InspectPanel.h"

class StereoMemoryDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    StereoMemoryDelayAudioProcessorEditor (StereoMemoryDelayAudioProcessor&);
    ~StereoMemoryDelayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    StereoMemoryDelayAudioProcessor& audioProcessor;

    // UI components
    juce::Component playPanel;
    InspectPanel inspectPanel;
    juce::TextButton inspectButton { "Inspect" };

    juce::Slider mixSlider;
    juce::Slider scanSlider;
    juce::Slider autoScanSlider;
    juce::Slider spreadSlider;
    juce::Slider feedbackSlider;
    juce::Slider timeSlider;
    juce::Slider characterSlider;
    juce::Slider randomSeedSlider;

    juce::ComboBox stereoModeBox;
    juce::ComboBox modeBox;

    juce::Label mixLabel;
    juce::Label scanLabel;
    juce::Label autoScanLabel;
    juce::Label spreadLabel;
    juce::Label feedbackLabel;
    juce::Label timeLabel;
    juce::Label characterLabel;
    juce::Label randomSeedLabel;
    juce::Label stereoModeLabel;
    juce::Label modeLabel;

    // Attachments for binding sliders to parameters
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scanAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> autoScanAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> spreadAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> characterAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> randomSeedAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> stereoModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;

    // LookAndFeel instance (token‑based theming)
    TokenLookAndFeel lookAndFeel;

    bool inspectMode { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoMemoryDelayAudioProcessorEditor)
};
