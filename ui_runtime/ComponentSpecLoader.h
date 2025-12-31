#pragma once

#include <string>

namespace ui_runtime {

struct ComponentSpec;

class ComponentSpecLoader {
public:
    static bool loadFromFile(const std::string& path, ComponentSpec& outSpec, std::string& error);
    static bool loadFromJson(const std::string& jsonStr, ComponentSpec& outSpec, std::string& error);
};

} // namespace ui_runtime
