#pragma once

namespace ui_runtime {

struct ComponentSpec;
class TokenSet;

} // namespace ui_runtime

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
    void setSpec(const ComponentSpec* spec) noexcept { spec_ = spec; }
    void setTokens(const TokenSet* tokens) noexcept { tokens_ = tokens; }
    void setValue(float value) noexcept { value_ = value; }

    int getId() const noexcept { return id_; }
    Type getType() const noexcept { return type_; }
    Bounds getBounds() const noexcept { return bounds_; }
    const ComponentSpec* getSpec() const noexcept { return spec_; }
    const TokenSet* getTokens() const noexcept { return tokens_; }
    float getValue() const noexcept { return value_; }

private:
    int id_ { 0 };
    Type type_ { Type::Container };
    Bounds bounds_ {};
    const ComponentSpec* spec_ { nullptr };
    const TokenSet* tokens_ { nullptr };
    float value_ { 0.0f };
};

} // namespace ui_runtime
