#pragma once

namespace ui_runtime {

class ComponentNode {
public:
    enum class Type {
        Container,
        Control,
        Visual
    };

    struct Bounds {
        float x { 0.0f };
        float y { 0.0f };
        float width { 0.0f };
        float height { 0.0f };
    };

    constexpr ComponentNode(int id = 0, Type type = Type::Container) noexcept
        : id_(id), type_(type)
    {
    }

    void setBounds(const Bounds& bounds) noexcept { bounds_ = bounds; }

    int getId() const noexcept { return id_; }
    Type getType() const noexcept { return type_; }
    Bounds getBounds() const noexcept { return bounds_; }

private:
    int id_ { 0 };
    Type type_ { Type::Container };
    Bounds bounds_ {};
};

} // namespace ui_runtime
