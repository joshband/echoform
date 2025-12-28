#include "PluginEditor.h"

namespace
{
constexpr auto scanModeId = "scanMode";
constexpr auto spreadId = "spread";
constexpr auto routingId = "routing";
constexpr auto collectId = "collect";
constexpr auto alwaysId = "always";
constexpr auto wipeId = "wipe";
constexpr auto inspectId = "inspect";
}

MemoryDelayAudioProcessorEditor::MemoryDelayAudioProcessorEditor(MemoryDelayAudioProcessor& processor)
    : AudioProcessorEditor(&processor),
      audioProcessor(processor)
{
    scanModeLabel.setText("Scan Mode", juce::dontSendNotification);
    routingLabel.setText("Routing", juce::dontSendNotification);
    spreadLabel.setText("Spread", juce::dontSendNotification);

    scanModeBox.addItemList(juce::StringArray{"Forward", "PingPong", "Random"}, 1);
    routingBox.addItemList(juce::StringArray{"Stereo", "Swap", "Mid/Side"}, 1);

    spreadSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    spreadSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

    collectButton.setButtonText("Collect");
    alwaysButton.setButtonText("Always");
    wipeButton.setButtonText("Wipe");
    inspectButton.setButtonText("Inspect");

    addAndMakeVisible(scanModeLabel);
    addAndMakeVisible(scanModeBox);
    addAndMakeVisible(routingLabel);
    addAndMakeVisible(routingBox);
    addAndMakeVisible(spreadLabel);
    addAndMakeVisible(spreadSlider);
    addAndMakeVisible(collectButton);
    addAndMakeVisible(alwaysButton);
    addAndMakeVisible(wipeButton);
    addAndMakeVisible(inspectButton);

    scanModeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, scanModeId, scanModeBox);
    routingAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, routingId, routingBox);
    spreadAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, spreadId, spreadSlider);
    collectAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, collectId, collectButton);
    alwaysAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, alwaysId, alwaysButton);
    wipeAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, wipeId, wipeButton);
    inspectAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, inspectId, inspectButton);

    setSize(420, 220);
}

MemoryDelayAudioProcessorEditor::~MemoryDelayAudioProcessorEditor() = default;

void MemoryDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Stereo Memory Delay", getLocalBounds().removeFromTop(24), juce::Justification::centred, 1);
}

void MemoryDelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(24);

    auto topRow = bounds.removeFromTop(100);
    auto leftColumn = topRow.removeFromLeft(topRow.getWidth() / 2);

    scanModeLabel.setBounds(leftColumn.removeFromTop(20));
    scanModeBox.setBounds(leftColumn.removeFromTop(28));
    spreadLabel.setBounds(leftColumn.removeFromTop(20));
    spreadSlider.setBounds(leftColumn);

    routingLabel.setBounds(topRow.removeFromTop(20));
    routingBox.setBounds(topRow.removeFromTop(28));

    auto buttonRow = bounds.removeFromTop(60);
    auto buttonWidth = buttonRow.getWidth() / 4;
    collectButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(4));
    alwaysButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(4));
    wipeButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(4));
    inspectButton.setBounds(buttonRow.removeFromLeft(buttonWidth).reduced(4));
}
