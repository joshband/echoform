#pragma once

#include "ComponentNode.h"
#include "Layer.h"
#include "TokenSet.h"

namespace ui_runtime {

class RendererInterface {
public:
    virtual ~RendererInterface() = default;

    virtual void beginFrame() = 0;
    virtual void render(const ComponentNode& node, const Layer& layer, const TokenSet& tokens) = 0;
    virtual void endFrame() = 0;
};

} // namespace ui_runtime
