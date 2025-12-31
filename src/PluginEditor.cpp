#include "PluginEditor.h"

#include "ui_runtime/ComponentNode.h"
#include "ui_runtime/ComponentSpecRuntime.h"
#include "ui_runtime/JUCEComponentRenderer.h"
#include "ui_runtime/TokenSet.h"

#include <atomic>
#include <utility>

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

ui_runtime::TokenSet toRuntimeTokens(const TokenLookAndFeel::Tokens& tokens)
{
    ui_runtime::TokenSet runtimeTokens;
    runtimeTokens.background = { tokens.background.getFloatRed(),
                                 tokens.background.getFloatGreen(),
                                 tokens.background.getFloatBlue(),
                                 tokens.background.getFloatAlpha() };
    runtimeTokens.panel = { tokens.panel.getFloatRed(),
                            tokens.panel.getFloatGreen(),
                            tokens.panel.getFloatBlue(),
                            tokens.panel.getFloatAlpha() };
    runtimeTokens.accent = { tokens.accent.getFloatRed(),
                             tokens.accent.getFloatGreen(),
                             tokens.accent.getFloatBlue(),
                             tokens.accent.getFloatAlpha() };
    runtimeTokens.text = { tokens.text.getFloatRed(),
                           tokens.text.getFloatGreen(),
                           tokens.text.getFloatBlue(),
                           tokens.text.getFloatAlpha() };
    runtimeTokens.mutedText = { tokens.mutedText.getFloatRed(),
                                tokens.mutedText.getFloatGreen(),
                                tokens.mutedText.getFloatBlue(),
                                tokens.mutedText.getFloatAlpha() };
    runtimeTokens.track = { tokens.track.getFloatRed(),
                            tokens.track.getFloatGreen(),
                            tokens.track.getFloatBlue(),
                            tokens.track.getFloatAlpha() };
    runtimeTokens.fontSize = tokens.fontSize;
    runtimeTokens.fontSizeSmall = tokens.fontSizeSmall;
    runtimeTokens.fontSizeLarge = tokens.fontSizeLarge;
    runtimeTokens.spacing = { tokens.spacing.xs, tokens.spacing.sm, tokens.spacing.md, tokens.spacing.lg };
    runtimeTokens.radius = { tokens.radius.sm, tokens.radius.md, tokens.radius.lg };
    return runtimeTokens;
}

#if ECHOFORM_USE_VDNA_UI
class FeedbackComponent final : public juce::Component, private juce::AudioProcessorValueTreeState::Listener
{
public:
    FeedbackComponent(juce::AudioProcessorValueTreeState& apvts,
                      ui_runtime::ComponentSpec spec,
                      ui_runtime::TokenSet tokens)
        : apvts_(apvts),
          spec_(std::move(spec)),
          tokens_(std::move(tokens))
    {
        parameter_ = apvts_.getParameter("feedback");
        if (parameter_ != nullptr)
        {
            normalizedValue_.store(parameter_->getValue());
            apvts_.addParameterListener("feedback", this);
        }

        node_.setSpec(&spec_);
        node_.setTokens(&tokens_);
        node_.setValue(normalizedValue_.load());

        renderer_.prepare(spec_, tokens_);
    }

    ~FeedbackComponent() override
    {
        if (parameter_ != nullptr)
            apvts_.removeParameterListener("feedback", this);
    }

    void paint(juce::Graphics& g) override
    {
        ui_runtime::ComponentNode::Bounds bounds;
        const auto localBounds = getLocalBounds().toFloat();
        bounds.x = localBounds.getX();
        bounds.y = localBounds.getY();
        bounds.width = localBounds.getWidth();
        bounds.height = localBounds.getHeight();
        node_.setBounds(bounds);
        node_.setValue(normalizedValue_.load());

        renderer_.setGraphics(&g);
        renderer_.beginFrame();
        renderer_.render(node_);
        renderer_.endFrame();
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (parameter_ == nullptr)
            return;
        dragStartValue_ = normalizedValue_.load();
        dragStartY_ = event.getPosition().getY();
        parameter_->beginChangeGesture();
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (parameter_ == nullptr)
            return;
        const float delta = static_cast<float>(dragStartY_ - event.getPosition().getY()) / 150.0f;
        const float nextValue = juce::jlimit(0.0f, 1.0f, dragStartValue_ + delta);
        parameter_->setValueNotifyingHost(nextValue);
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        if (parameter_ != nullptr)
            parameter_->endChangeGesture();
    }

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        if (parameterID != "feedback" || parameter_ == nullptr)
            return;

        const auto range = parameter_->getNormalisableRange();
        const float normalized = range.convertTo0to1(newValue);
        normalizedValue_.store(normalized);

        juce::Component::SafePointer<FeedbackComponent> safeThis(this);
        juce::MessageManager::callAsync([safeThis]() {
            if (safeThis != nullptr)
                safeThis->repaint();
        });
    }

    juce::AudioProcessorValueTreeState& apvts_;
    juce::RangedAudioParameter* parameter_ { nullptr };
    ui_runtime::ComponentSpec spec_;
    ui_runtime::TokenSet tokens_;
    ui_runtime::ComponentNode node_;
    ui_runtime::JUCEComponentRenderer renderer_;
    std::atomic<float> normalizedValue_ { 0.0f };
    float dragStartValue_ { 0.0f };
    int dragStartY_ { 0 };
};
#endif
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
    configureLabel(feedbackLabel, "Feedback");

    addAndMakeVisible(mixLabel);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(timeLabel);
    addAndMakeVisible(timeSlider);
    addAndMakeVisible(feedbackLabel);

#if ECHOFORM_USE_VDNA_UI
    {
        ui_runtime::ComponentSpec spec;
        std::string error;
        const auto runtimeTokens = toRuntimeTokens(getTokens(*this));
        if (ui_runtime::loadComponentSpec("component_specs/feedback_knob.json", runtimeTokens, spec, error))
        {
            feedbackComponent = std::make_unique<FeedbackComponent>(audioProcessor.getParameters(),
                                                                    std::move(spec),
                                                                    runtimeTokens);
            addAndMakeVisible(*feedbackComponent);
        }
        else
        {
            juce::Logger::writeToLog("Feedback component load failed: " + error);
        }
    }
#else
    configureKnob(feedbackSlider);
    addAndMakeVisible(feedbackSlider);
#endif

    auto& apvts = audioProcessor.getParameters();
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "mix", mixSlider);
    timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "time", timeSlider);
#if !ECHOFORM_USE_VDNA_UI
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts,
                                                                                                "feedback",
                                                                                                feedbackSlider);
#endif

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

#if !ECHOFORM_USE_VDNA_UI
    feedbackSlider.setTitle("Feedback");
    feedbackSlider.setHelpText("Sets the feedback amount for the tape loop.");
    feedbackSlider.setNumDecimalPlacesToDisplay(2);
    feedbackSlider.setWantsKeyboardFocus(true);
    feedbackSlider.setHasFocusOutline(true);
    feedbackSlider.setExplicitFocusOrder(3);
#endif
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

    const int columnWidth = bounds.getWidth() / 3;
    auto mixColumn = bounds.removeFromLeft(columnWidth);
    auto feedbackColumn = bounds.removeFromLeft(columnWidth);
    auto timeColumn = bounds;

    auto mixLabelArea = mixColumn.removeFromTop(labelHeight);
    mixLabel.setFont(juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain)));
    mixLabel.setBounds(mixLabelArea);

    auto feedbackLabelArea = feedbackColumn.removeFromTop(labelHeight);
    feedbackLabel.setFont(juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain)));
    feedbackLabel.setBounds(feedbackLabelArea);

    auto timeLabelArea = timeColumn.removeFromTop(labelHeight);
    timeLabel.setFont(juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain)));
    timeLabel.setBounds(timeLabelArea);

    const int knobSize = juce::jmin(mixColumn.getWidth(), mixColumn.getHeight());
    mixSlider.setBounds(mixColumn.withSizeKeepingCentre(knobSize, knobSize));

    const int feedbackKnobSize = juce::jmin(feedbackColumn.getWidth(), feedbackColumn.getHeight());
#if ECHOFORM_USE_VDNA_UI
    if (feedbackComponent != nullptr)
        feedbackComponent->setBounds(feedbackColumn.withSizeKeepingCentre(feedbackKnobSize, feedbackKnobSize));
#else
    feedbackSlider.setBounds(feedbackColumn.withSizeKeepingCentre(feedbackKnobSize, feedbackKnobSize));
#endif

    const int timeKnobSize = juce::jmin(timeColumn.getWidth(), timeColumn.getHeight());
    timeSlider.setBounds(timeColumn.withSizeKeepingCentre(timeKnobSize, timeKnobSize));
}
