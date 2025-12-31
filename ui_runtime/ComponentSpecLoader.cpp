#include "ComponentSpecLoader.h"
#include "ComponentSpec.h"

namespace ui_runtime {

bool ComponentSpecLoader::loadFromFile(const std::string&, ComponentSpec&, std::string& error)
{
    error = "ComponentSpecLoader::loadFromFile not implemented";
    return false;
}

bool ComponentSpecLoader::loadFromJson(const std::string&, ComponentSpec&, std::string& error)
{
    error = "ComponentSpecLoader::loadFromJson not implemented";
    return false;
}

} // namespace ui_runtime
