#include "InspectPanel.h"
#include "TokenLookAndFeel.h"

InspectPanel::InspectPanel(StereoMemoryDelayAudioProcessor& processor)
    : audioProcessor(processor)
{
    setInterceptsMouseClicks(false, false);
    startTimerHz(30);
}

void InspectPanel::resized()
{
}

void InspectPanel::timerCallback()
{
    audioProcessor.getVisualSnapshot(snapshot);
    repaint();
}

void InspectPanel::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    const auto* tokenLookAndFeel = dynamic_cast<TokenLookAndFeel*>(&getLookAndFeel());
    const auto tokens = tokenLookAndFeel != nullptr ? tokenLookAndFeel->getTokens() : TokenLookAndFeel::Tokens();

    auto area = getLocalBounds().toFloat();
    area = area.reduced(tokens.spacing.md);

    g.setColour(tokens.panel);
    g.fillRoundedRectangle(area, tokens.radius.md);

    auto plotArea = area.reduced(tokens.spacing.sm);

    const int numBins = static_cast<int>(snapshot.energy.size());
    if (numBins <= 0)
        return;

    const float barWidth = plotArea.getWidth() / static_cast<float>(numBins);
    const int writeIndex = snapshot.writeIndex;

    for (int i = 0; i < numBins; ++i)
    {
        const int index = (writeIndex + i) % numBins;
        const float value = juce::jlimit(0.0f, 1.0f, snapshot.energy[static_cast<size_t>(index)]);
        const float barHeight = plotArea.getHeight() * value;
        juce::Rectangle<float> bar(plotArea.getX() + barWidth * static_cast<float>(i),
                                   plotArea.getBottom() - barHeight,
                                   barWidth * 0.9f,
                                   barHeight);
        g.setColour(tokens.accent.withAlpha(0.2f + 0.6f * value));
        g.fillRect(bar);
    }

    const float primaryX = plotArea.getX() + plotArea.getWidth() * (1.0f - snapshot.primaryPosition);
    const float secondaryX = plotArea.getX() + plotArea.getWidth() * (1.0f - snapshot.secondaryPosition);

    g.setColour(tokens.text);
    g.drawLine(primaryX, plotArea.getY(), primaryX, plotArea.getBottom(), 2.0f);

    g.setColour(tokens.mutedText);
    g.drawLine(secondaryX, plotArea.getY(), secondaryX, plotArea.getBottom(), 1.5f);
}
