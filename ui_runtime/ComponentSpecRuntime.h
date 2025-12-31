#pragma once

#include "ComponentSpec.h"
#include "TokenSet.h"

#include <string>

namespace ui_runtime {

bool loadComponentSpec(const std::string& relativePath,
                       const TokenSet& tokens,
                       ComponentSpec& outSpec,
                       std::string& error);

} // namespace ui_runtime
