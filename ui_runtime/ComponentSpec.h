#pragma once

#include <map>
#include <string>
#include <vector>

namespace ui_runtime {

struct ComponentSpec {
    struct LayerSpec {
        std::string id;
        std::string role;
        std::string transform;
    };

    struct InteractionSpec {
        std::string inputType;
        std::string valueRange;
        std::string gestureCurve;
    };

    struct MotionSpec {
        std::string rotationMin;
        std::string rotationMax;
    };

    std::string id;
    std::string category;
    std::vector<std::string> hierarchy;
    std::map<std::string, std::string> assets;
    LayerSpec layers;
    InteractionSpec interaction;
    MotionSpec motion;
    std::map<std::string, std::string> tokens;

    ComponentSpec() = default;
};

} // namespace ui_runtime
