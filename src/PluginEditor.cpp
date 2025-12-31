#include "PluginEditor.h"

namespace {
TokenLookAndFeel::Tokens getTokens(const juce::Component& component)
{
    const auto* tokenLookAndFeel = dynamic_cast<TokenLookAndFeel*>(&component.getLookAndFeel());
    if (tokenLookAndFeel != nullptr)
        return tokenLookAndFeel->getTokens();
    return TokenLookAndFeel::Tokens();
}

void configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 22);
}

void configureLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setInterceptsMouseClicks(false, false);
}
} // namespace

StereoMemoryDelayAudioProcessorEditor::StereoMemoryDelayAudioProcessorEditor(StereoMemoryDelayAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(520, 320);

    configureKnob(mixSlider);
    configureKnob(timeSlider);
    configureLabel(mixLabel, "Mix");
    configureLabel(timeLabel, "Time (s)");

    addAndMakeVisible(mixLabel);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(timeLabel);
    addAndMakeVisible(timeSlider);

    auto& apvts = audioProcessor.getParameters();
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "mix", mixSlider);
    timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "time", timeSlider);

    mixSlider.setTitle("Mix");
    mixSlider.setHelpText("Blend dry input with the tape loop output.");
    mixSlider.setWantsKeyboardFocus(true);
    mixSlider.setHasFocusOutline(true);
    mixSlider.setExplicitFocusOrder(1);

    timeSlider.setTitle("Time");
    timeSlider.setHelpText("Sets the playback window in seconds (0-30) within the 3-minute loop.");
    timeSlider.setNumDecimalPlacesToDisplay(2);
    timeSlider.setWantsKeyboardFocus(true);
    timeSlider.setHasFocusOutline(true);
    timeSlider.setExplicitFocusOrder(2);
}

StereoMemoryDelayAudioProcessorEditor::~StereoMemoryDelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void StereoMemoryDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto tokens = getTokens(*this);
    g.fillAll(tokens.background);

    g.setColour(tokens.text);
    g.setFont(juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSizeLarge, juce::Font::bold)));

    auto titleArea = getLocalBounds().removeFromTop(static_cast<int>(tokens.spacing.lg + tokens.fontSizeLarge));
    titleArea = titleArea.reduced(static_cast<int>(tokens.spacing.md), 0);
    g.drawFittedText("Echoform", titleArea, juce::Justification::centredLeft, 1);
}

void StereoMemoryDelayAudioProcessorEditor::resized()
{
    const auto tokens = getTokens(*this);
    auto bounds = getLocalBounds().reduced(static_cast<int>(tokens.spacing.lg));

    const int labelHeight = static_cast<int>(tokens.fontSize + tokens.spacing.sm);
    bounds.removeFromTop(static_cast<int>(tokens.spacing.lg + tokens.fontSizeLarge));

    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightColumn = bounds;

    auto mixLabelArea = leftColumn.removeFromTop(labelHeight);
    mixLabel.setFont(juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain)));
    mixLabel.setBounds(mixLabelArea);

    auto timeLabelArea = rightColumn.removeFromTop(labelHeight);
    timeLabel.setFont(juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain)));
    timeLabel.setBounds(timeLabelArea);

    const int knobSize = juce::jmin(leftColumn.getWidth(), leftColumn.getHeight());
    mixSlider.setBounds(leftColumn.withSizeKeepingCentre(knobSize, knobSize));

    const int timeKnobSize = juce::jmin(rightColumn.getWidth(), rightColumn.getHeight());
    timeSlider.setBounds(rightColumn.withSizeKeepingCentre(timeKnobSize, timeKnobSize));
}
