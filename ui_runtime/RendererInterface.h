#pragma once

#include "ComponentNode.h"
#include "TokenSet.h"

namespace ui_runtime {

class RendererInterface {
public:
    virtual ~RendererInterface() = default;

    virtual void beginFrame() = 0;
    virtual void render(const ComponentNode& node) = 0;
    virtual void endFrame() = 0;
};

} // namespace ui_runtime
