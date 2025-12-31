#pragma once

#include <string>

#include "TokenSet.h"

namespace ui_runtime {

struct TokenLoadResult {
    TokenSet tokens;
    bool ok { false };
    std::string error;
};

class TokenLoader {
public:
    TokenLoadResult loadFromFile(const std::string& path,
                                 const std::string& pluginName,
                                 const std::string& componentName) const;

    TokenLoadResult loadFromJson(const std::string& json,
                                 const std::string& pluginName,
                                 const std::string& componentName) const;
};

} // namespace ui_runtime
