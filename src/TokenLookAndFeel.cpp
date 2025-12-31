#include "TokenLookAndFeel.h"

namespace {
juce::var findTokenValue(const juce::var& root, const juce::StringArray& path)
{
    const juce::var* current = &root;
    for (const auto& key : path)
    {
        if (auto* obj = current->getDynamicObject())
        {
            if (obj->hasProperty(key))
                current = &obj->getProperty(key);
            else
                return {};
        }
        else
        {
            return {};
        }
    }
    return *current;
}

juce::Colour parseTokenColour(const juce::var& value, juce::Colour fallback)
{
    if (value.isVoid())
        return fallback;

    auto text = value.toString().trim();
    if (text.isEmpty())
        return fallback;

    if (text.startsWithChar('#'))
        text = text.substring(1);

    if (text.length() == 6)
        text = "ff" + text;

    if (text.length() == 8)
        return juce::Colour::fromString(text);

    return fallback;
}

float parseTokenFloat(const juce::var& value, float fallback)
{
    if (value.isVoid())
        return fallback;

    if (value.isDouble() || value.isInt())
        return static_cast<float>(value);

    return value.toString().getFloatValue();
}

juce::File findTokenFile()
{
    const juce::File cwd = juce::File::getCurrentWorkingDirectory();
    juce::File candidate = cwd.getChildFile("resources").getChildFile("visualdna_tokens.json");
    if (candidate.existsAsFile())
        return candidate;

    const juce::File exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                                  .getParentDirectory();
    candidate = exeDir.getChildFile("resources").getChildFile("visualdna_tokens.json");
    if (candidate.existsAsFile())
        return candidate;

    candidate = exeDir.getChildFile("../Resources/visualdna_tokens.json");
    if (candidate.existsAsFile())
        return candidate;

    return {};
}
} // namespace

TokenLookAndFeel::TokenLookAndFeel()
{
    loadTokensFromFile();
    applyTokens();
}

juce::Font TokenLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain));
}

juce::Font TokenLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(juce::FontOptions(tokens.fontFamily, tokens.fontSize, juce::Font::plain));
}

juce::Font TokenLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    const float size = juce::jmin(tokens.fontSizeLarge, static_cast<float>(buttonHeight) * 0.45f);
    return juce::Font(juce::FontOptions(tokens.fontFamily, size, juce::Font::plain));
}

void TokenLookAndFeel::loadTokensFromFile()
{
    const auto tokenFile = findTokenFile();
    if (!tokenFile.existsAsFile())
        return;

    const auto jsonText = tokenFile.loadFileAsString();
    const auto parsed = juce::JSON::parse(jsonText);
    if (parsed.isVoid())
        return;

    tokens.background = parseTokenColour(findTokenValue(parsed, { "colors", "background", "value" }), tokens.background);
    tokens.panel = parseTokenColour(findTokenValue(parsed, { "colors", "panel", "value" }), tokens.panel);
    tokens.accent = parseTokenColour(findTokenValue(parsed, { "colors", "accent", "value" }), tokens.accent);
    tokens.text = parseTokenColour(findTokenValue(parsed, { "colors", "text", "value" }), tokens.text);
    tokens.mutedText = parseTokenColour(findTokenValue(parsed, { "colors", "mutedText", "value" }), tokens.mutedText);
    tokens.track = parseTokenColour(findTokenValue(parsed, { "colors", "track", "value" }), tokens.track);

    const auto familyVar = findTokenValue(parsed, { "font", "family", "value" });
    if (familyVar.isString())
        tokens.fontFamily = familyVar.toString();

    tokens.fontSize = parseTokenFloat(findTokenValue(parsed, { "font", "size", "value" }), tokens.fontSize);
    tokens.fontSizeSmall = parseTokenFloat(findTokenValue(parsed, { "font", "sizeSmall", "value" }), tokens.fontSizeSmall);
    tokens.fontSizeLarge = parseTokenFloat(findTokenValue(parsed, { "font", "sizeLarge", "value" }), tokens.fontSizeLarge);

    tokens.spacing.xs = parseTokenFloat(findTokenValue(parsed, { "spacing", "xs", "value" }), tokens.spacing.xs);
    tokens.spacing.sm = parseTokenFloat(findTokenValue(parsed, { "spacing", "sm", "value" }), tokens.spacing.sm);
    tokens.spacing.md = parseTokenFloat(findTokenValue(parsed, { "spacing", "md", "value" }), tokens.spacing.md);
    tokens.spacing.lg = parseTokenFloat(findTokenValue(parsed, { "spacing", "lg", "value" }), tokens.spacing.lg);

    tokens.radius.sm = parseTokenFloat(findTokenValue(parsed, { "radius", "sm", "value" }), tokens.radius.sm);
    tokens.radius.md = parseTokenFloat(findTokenValue(parsed, { "radius", "md", "value" }), tokens.radius.md);
    tokens.radius.lg = parseTokenFloat(findTokenValue(parsed, { "radius", "lg", "value" }), tokens.radius.lg);
}

void TokenLookAndFeel::applyTokens()
{
    setColour(juce::ResizableWindow::backgroundColourId, tokens.background);
    setColour(juce::Slider::rotarySliderFillColourId, tokens.accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, tokens.track);
    setColour(juce::Slider::thumbColourId, tokens.accent);
    setColour(juce::Slider::textBoxTextColourId, tokens.text);
    setColour(juce::Slider::textBoxBackgroundColourId, tokens.panel);
    setColour(juce::Slider::textBoxOutlineColourId, tokens.track);
    setColour(juce::Slider::textBoxHighlightColourId, tokens.accent.withAlpha(0.2f));
    setColour(juce::Label::textColourId, tokens.text);
    setColour(juce::Label::textWhenEditingColourId, tokens.text);
    setColour(juce::ComboBox::backgroundColourId, tokens.panel);
    setColour(juce::ComboBox::textColourId, tokens.text);
    setColour(juce::ComboBox::outlineColourId, tokens.track);
    setColour(juce::ComboBox::focusedOutlineColourId, tokens.accent);
    setColour(juce::PopupMenu::backgroundColourId, tokens.panel);
    setColour(juce::PopupMenu::textColourId, tokens.text);
    setColour(juce::TextButton::buttonColourId, tokens.panel);
    setColour(juce::TextButton::textColourOffId, tokens.text);
    setColour(juce::TextButton::textColourOnId, tokens.text);
    setColour(juce::TextButton::buttonOnColourId, tokens.accent);
    setColour(juce::TextEditor::focusedOutlineColourId, tokens.accent);
    setColour(juce::TooltipWindow::textColourId, tokens.text);
    setColour(juce::TooltipWindow::backgroundColourId, tokens.panel);
    setColour(juce::TooltipWindow::outlineColourId, tokens.track);
}
