#include "JUCEComponentRenderer.h"

#include <algorithm>

namespace ui_runtime {
namespace {

juce::Colour toJuceColour(const TokenColor& color)
{
    return juce::Colour::fromFloatRGBA(color.r, color.g, color.b, color.a);
}

juce::File resolveAssetPath(const std::string& assetName)
{
    return juce::File::getCurrentWorkingDirectory()
        .getChildFile("resources")
        .getChildFile(assetName);
}

} // namespace

void JUCEComponentRenderer::prepare(const ComponentSpec& spec, const TokenSet& tokens)
{
    layers_.clear();
    layers_.reserve(spec.layers.size());

    for (const auto& layerSpec : spec.layers)
    {
        CachedLayer layer;
        auto assetIt = spec.assets.find(layerSpec.id);
        if (assetIt != spec.assets.end())
        {
            const juce::File assetFile = resolveAssetPath(assetIt->second);
            layer.image = juce::ImageCache::getFromFile(assetFile);
        }

        const std::string colorKey = layerSpec.role + ".color";
        if (auto tokenIt = spec.tokens.find(colorKey); tokenIt != spec.tokens.end())
            layer.tint = resolveTokenColour(tokenIt->second, tokens);

        const std::string opacityKey = layerSpec.role + ".opacity";
        if (auto tokenIt = spec.tokens.find(opacityKey); tokenIt != spec.tokens.end())
            layer.opacity = resolveTokenOpacity(tokenIt->second);

        layer.rotates = (layerSpec.transform == "rotate");
        layers_.push_back(std::move(layer));
    }

    if (!spec.motion.rotationMin.empty())
        rotationMinDegrees_ = std::stof(spec.motion.rotationMin);
    if (!spec.motion.rotationMax.empty())
        rotationMaxDegrees_ = std::stof(spec.motion.rotationMax);
}

void JUCEComponentRenderer::beginFrame()
{
    // No-op for now.
}

void JUCEComponentRenderer::render(const ComponentNode& node)
{
    if (graphics_ == nullptr)
        return;

    const auto* spec = node.getSpec();
    const auto* tokens = node.getTokens();
    if (spec == nullptr || tokens == nullptr)
        return;

    const auto bounds = node.getBounds();
    const float normalizedValue = std::clamp(node.getValue(), 0.0f, 1.0f);
    const float rotationDegrees =
        rotationMinDegrees_ + normalizedValue * (rotationMaxDegrees_ - rotationMinDegrees_);
    const float rotationRadians = juce::degreesToRadians(rotationDegrees);

    for (size_t index = 0; index < layers_.size(); ++index)
    {
        const auto& layer = layers_[index];
        if (!layer.image.isValid())
            continue;

        const float imageWidth = static_cast<float>(layer.image.getWidth());
        const float imageHeight = static_cast<float>(layer.image.getHeight());
        if (imageWidth <= 0.0f || imageHeight <= 0.0f)
            continue;

        const float scaleX = bounds.width / imageWidth;
        const float scaleY = bounds.height / imageHeight;

        juce::AffineTransform transform = juce::AffineTransform::scale(scaleX, scaleY)
                                              .translated(bounds.x, bounds.y);

        if (layer.rotates)
        {
            const float centerX = bounds.x + bounds.width * 0.5f;
            const float centerY = bounds.y + bounds.height * 0.5f;
            transform = transform.rotated(rotationRadians, centerX, centerY);
        }

        graphics_->setOpacity(layer.opacity);
        graphics_->drawImageTransformed(layer.image, transform, false);

        if (layer.tint != juce::Colours::white)
        {
            graphics_->setColour(layer.tint.withAlpha(0.35f * layer.opacity));
            graphics_->fillRect(bounds.x, bounds.y, bounds.width, bounds.height);
        }
    }
}

void JUCEComponentRenderer::endFrame()
{
    graphics_ = nullptr;
}

juce::Colour JUCEComponentRenderer::resolveTokenColour(const std::string& tokenName,
                                                      const TokenSet& tokens)
{
    const auto contains = [&tokenName](const char* text) {
        return tokenName.find(text) != std::string::npos;
    };

    if (contains("background"))
        return toJuceColour(tokens.background);
    if (contains("panel"))
        return toJuceColour(tokens.panel);
    if (contains("accent") || contains("indicator") || contains("control"))
        return toJuceColour(tokens.accent);
    if (contains("muted"))
        return toJuceColour(tokens.mutedText);
    if (contains("track"))
        return toJuceColour(tokens.track);
    if (contains("text"))
        return toJuceColour(tokens.text);

    return juce::Colours::white;
}

float JUCEComponentRenderer::resolveTokenOpacity(const std::string& tokenName)
{
    try
    {
        return std::clamp(std::stof(tokenName), 0.0f, 1.0f);
    }
    catch (const std::exception&)
    {
        return 1.0f;
    }
}

} // namespace ui_runtime
