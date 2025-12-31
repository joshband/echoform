#include "ComponentSpecRuntime.h"

#include "ComponentSpecLoader.h"

#include <iostream>

namespace ui_runtime {

bool loadComponentSpec(const std::string& relativePath,
                       const TokenSet& tokens,
                       ComponentSpec& outSpec,
                       std::string& error)
{
    (void)tokens;

    if (relativePath.empty())
    {
        error = "Component spec path cannot be empty.";
        return false;
    }

    const std::string fullPath = "resources/" + relativePath;
    if (!ComponentSpecLoader::loadFromFile(fullPath, outSpec, error))
    {
        std::cerr << "Component spec load failed: " << error << std::endl;
        return false;
    }

    return true;
}

} // namespace ui_runtime
