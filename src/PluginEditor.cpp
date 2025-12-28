#include "PluginEditor.h"

StereoMemoryDelayAudioProcessorEditor::StereoMemoryDelayAudioProcessorEditor(StereoMemoryDelayAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
{
    setSize(300, 200);
}

void StereoMemoryDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void StereoMemoryDelayAudioProcessorEditor::resized()
{
}
