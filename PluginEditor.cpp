#include "PluginEditor.h"
#include "PluginProcessor.h"

StereoMemoryDelayAudioProcessorEditor::StereoMemoryDelayAudioProcessorEditor (StereoMemoryDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

StereoMemoryDelayAudioProcessorEditor::~StereoMemoryDelayAudioProcessorEditor() {}

void StereoMemoryDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Stereo Memory Delay", getLocalBounds(), juce::Justification::centredTop, 1);
}

void StereoMemoryDelayAudioProcessorEditor::resized()
{
    // TODO: add UI components and layout code
}
