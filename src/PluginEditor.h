#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MemoryDelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MemoryDelayAudioProcessorEditor(MemoryDelayAudioProcessor& processor);
    ~MemoryDelayAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    MemoryDelayAudioProcessor& audioProcessor;

    juce::ComboBox scanModeBox;
    juce::ComboBox routingBox;
    juce::Slider spreadSlider;
    juce::ToggleButton collectButton;
    juce::ToggleButton alwaysButton;
    juce::ToggleButton wipeButton;
    juce::ToggleButton inspectButton;

    juce::Label scanModeLabel;
    juce::Label routingLabel;
    juce::Label spreadLabel;

    std::unique_ptr<ComboBoxAttachment> scanModeAttachment;
    std::unique_ptr<ComboBoxAttachment> routingAttachment;
    std::unique_ptr<SliderAttachment> spreadAttachment;
    std::unique_ptr<ButtonAttachment> collectAttachment;
    std::unique_ptr<ButtonAttachment> alwaysAttachment;
    std::unique_ptr<ButtonAttachment> wipeAttachment;
    std::unique_ptr<ButtonAttachment> inspectAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemoryDelayAudioProcessorEditor)
};
