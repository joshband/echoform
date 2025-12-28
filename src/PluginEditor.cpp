#include "PluginEditor.h"

namespace {
void configureKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
}

void configureLabel(juce::Label& label, juce::Component& target, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&target, false);
}

void configureCombo(juce::ComboBox& box)
{
    box.setJustificationType(juce::Justification::centred);
}
} // namespace

StereoMemoryDelayAudioProcessorEditor::StereoMemoryDelayAudioProcessorEditor (StereoMemoryDelayAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      inspectPanel(p)
{
    setSize(720, 420);
    setLookAndFeel(&lookAndFeel);

    configureKnob(mixSlider);
    configureKnob(scanSlider);
    configureKnob(autoScanSlider);
    configureKnob(spreadSlider);
    configureKnob(feedbackSlider);
    configureKnob(timeSlider);
    configureKnob(characterSlider);
    configureKnob(randomSeedSlider);

    randomSeedSlider.setNumDecimalPlacesToDisplay(0);

    configureCombo(stereoModeBox);
    configureCombo(modeBox);

    stereoModeBox.addItem("Independent", 1);
    stereoModeBox.addItem("Linked", 2);
    stereoModeBox.addItem("Cross", 3);

    modeBox.addItem("Collect", 1);
    modeBox.addItem("Feed", 2);
    modeBox.addItem("Closed", 3);

    addAndMakeVisible(playPanel);
    addAndMakeVisible(inspectPanel);
    addAndMakeVisible(inspectButton);

    playPanel.addAndMakeVisible(mixSlider);
    playPanel.addAndMakeVisible(scanSlider);
    playPanel.addAndMakeVisible(autoScanSlider);
    playPanel.addAndMakeVisible(spreadSlider);
    playPanel.addAndMakeVisible(feedbackSlider);
    playPanel.addAndMakeVisible(timeSlider);
    playPanel.addAndMakeVisible(characterSlider);
    playPanel.addAndMakeVisible(randomSeedSlider);
    playPanel.addAndMakeVisible(stereoModeBox);
    playPanel.addAndMakeVisible(modeBox);

    playPanel.addAndMakeVisible(mixLabel);
    playPanel.addAndMakeVisible(scanLabel);
    playPanel.addAndMakeVisible(autoScanLabel);
    playPanel.addAndMakeVisible(spreadLabel);
    playPanel.addAndMakeVisible(feedbackLabel);
    playPanel.addAndMakeVisible(timeLabel);
    playPanel.addAndMakeVisible(characterLabel);
    playPanel.addAndMakeVisible(randomSeedLabel);
    playPanel.addAndMakeVisible(stereoModeLabel);
    playPanel.addAndMakeVisible(modeLabel);

    configureLabel(mixLabel, mixSlider, "Mix");
    configureLabel(scanLabel, scanSlider, "Scan");
    configureLabel(autoScanLabel, autoScanSlider, "Auto Scan");
    configureLabel(spreadLabel, spreadSlider, "Spread");
    configureLabel(feedbackLabel, feedbackSlider, "Feedback");
    configureLabel(timeLabel, timeSlider, "Time");
    configureLabel(characterLabel, characterSlider, "Character");
    configureLabel(randomSeedLabel, randomSeedSlider, "Seed");
    configureLabel(stereoModeLabel, stereoModeBox, "Stereo");
    configureLabel(modeLabel, modeBox, "Mode");

    auto& apvts = audioProcessor.getParameters();
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "mix", mixSlider);
    scanAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "scan", scanSlider);
    autoScanAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "autoScanRate", autoScanSlider);
    spreadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "spread", spreadSlider);
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "feedback", feedbackSlider);
    timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "time", timeSlider);
    characterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "character", characterSlider);
    randomSeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "randomSeed", randomSeedSlider);

    stereoModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "stereoMode", stereoModeBox);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "mode", modeBox);

    inspectButton.setClickingTogglesState(true);
    inspectButton.onClick = [this]()
    {
        inspectMode = inspectButton.getToggleState();
        playPanel.setVisible(!inspectMode);
        inspectPanel.setVisible(inspectMode);
        repaint();
    };

    inspectPanel.setVisible(false);
}

StereoMemoryDelayAudioProcessorEditor::~StereoMemoryDelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void StereoMemoryDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    const auto tokens = lookAndFeel.getTokens();
    g.setColour(tokens.text);
    g.setFont(juce::Font(tokens.fontFamily, tokens.fontSizeLarge, juce::Font::bold));
    g.drawFittedText("Echoform", getLocalBounds().removeFromTop(static_cast<int>(tokens.spacing.lg + tokens.fontSizeLarge)),
                     juce::Justification::centredLeft, 1);
}

void StereoMemoryDelayAudioProcessorEditor::resized()
{
    const auto tokens = lookAndFeel.getTokens();
    auto bounds = getLocalBounds();

    const int headerHeight = static_cast<int>(tokens.spacing.lg + tokens.fontSizeLarge);
    auto headerArea = bounds.removeFromTop(headerHeight);
    const int buttonWidth = 100;
    const int buttonHeight = static_cast<int>(tokens.spacing.lg);
    inspectButton.setBounds(headerArea.removeFromRight(buttonWidth).reduced(static_cast<int>(tokens.spacing.xs),
                                                                           static_cast<int>(tokens.spacing.xs)));

    auto contentArea = bounds.reduced(static_cast<int>(tokens.spacing.md));
    playPanel.setBounds(contentArea);
    inspectPanel.setBounds(contentArea);

    auto gridArea = playPanel.getLocalBounds().reduced(static_cast<int>(tokens.spacing.sm));
    juce::Grid grid;
    grid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                          juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    grid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    grid.rowGap = juce::Grid::Px(tokens.spacing.sm);
    grid.columnGap = juce::Grid::Px(tokens.spacing.sm);

    grid.items = {
        juce::GridItem(mixSlider),
        juce::GridItem(scanSlider),
        juce::GridItem(autoScanSlider),
        juce::GridItem(spreadSlider),
        juce::GridItem(feedbackSlider),
        juce::GridItem(timeSlider),
        juce::GridItem(characterSlider),
        juce::GridItem(stereoModeBox),
        juce::GridItem(modeBox),
        juce::GridItem(randomSeedSlider)
    };

    grid.performLayout(gridArea);
}
