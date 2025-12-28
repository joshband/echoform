#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class InspectPanel final : public juce::Component, private juce::Timer
{
public:
    explicit InspectPanel(StereoMemoryDelayAudioProcessor& processor);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    StereoMemoryDelayAudioProcessor& audioProcessor;
    MemoryDelayEngine::VisualSnapshot snapshot;
};
