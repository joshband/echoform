#pragma once

#include <JuceHeader.h>

class TokenLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    struct Spacing
    {
        float xs { 6.0f };
        float sm { 10.0f };
        float md { 16.0f };
        float lg { 24.0f };
    };

    struct Radius
    {
        float sm { 6.0f };
        float md { 10.0f };
        float lg { 16.0f };
    };

    struct Tokens
    {
        juce::Colour background { 0xff0f1218 };
        juce::Colour panel { 0xff1a1f2a };
        juce::Colour accent { 0xfff0c56b };
        juce::Colour text { 0xfff4f4f4 };
        juce::Colour mutedText { 0xff9aa0a6 };
        juce::Colour track { 0xff2a3140 };
        juce::String fontFamily { juce::Font::getDefaultSansSerifFontName() };
        float fontSize { 15.0f };
        float fontSizeSmall { 13.0f };
        float fontSizeLarge { 19.0f };
        Spacing spacing;
        Radius radius;
    };

    TokenLookAndFeel();

    const Tokens& getTokens() const { return tokens; }

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;

private:
    void loadTokensFromFile();
    void applyTokens();

    Tokens tokens;
};
