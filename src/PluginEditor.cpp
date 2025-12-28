#include "PluginEditor.h"

StereoMemoryDelayAudioProcessorEditor::StereoMemoryDelayAudioProcessorEditor(StereoMemoryDelayAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
{
    randomSeedSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    randomSeedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randomSeedSlider);

    randomSeedLabel.setText("Random Seed", juce::dontSendNotification);
    randomSeedLabel.setJustificationType(juce::Justification::centred);
    randomSeedLabel.attachToComponent(&randomSeedSlider, false);

    randomSeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getValueTreeState(), "randomSeed", randomSeedSlider);

    setSize(300, 200);
}

void StereoMemoryDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void StereoMemoryDelayAudioProcessorEditor::resized()
{
    randomSeedSlider.setBounds(getLocalBounds().reduced(40));
}
