#pragma once

#include "ComponentSpec.h"
#include "RendererInterface.h"
#include "TokenSet.h"

#include <JuceHeader.h>

#include <vector>

namespace ui_runtime {

class JUCEComponentRenderer final : public RendererInterface {
public:
    JUCEComponentRenderer() = default;

    void prepare(const ComponentSpec& spec, const TokenSet& tokens);
    void setGraphics(juce::Graphics* graphics) noexcept { graphics_ = graphics; }

    void beginFrame() override;
    void render(const ComponentNode& node) override;
    void endFrame() override;

private:
    struct CachedLayer {
        juce::Image image;
        juce::Colour tint { juce::Colours::white };
        float opacity { 1.0f };
        bool rotates { false };
    };

    static juce::Colour resolveTokenColour(const std::string& tokenName, const TokenSet& tokens);
    static float resolveTokenOpacity(const std::string& tokenName);

    std::vector<CachedLayer> layers_;
    float rotationMinDegrees_ { 0.0f };
    float rotationMaxDegrees_ { 0.0f };
    juce::Graphics* graphics_ { nullptr };
};

} // namespace ui_runtime
