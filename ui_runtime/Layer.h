#pragma once

namespace ui_runtime {

class Layer {
public:
    constexpr Layer(int id = 0, int zIndex = 0) noexcept
        : id_(id), zIndex_(zIndex)
    {
    }

    void setOpacity(float opacity) noexcept { opacity_ = opacity; }
    void setZIndex(int zIndex) noexcept { zIndex_ = zIndex; }

    int getId() const noexcept { return id_; }
    int getZIndex() const noexcept { return zIndex_; }
    float getOpacity() const noexcept { return opacity_; }

private:
    int id_ { 0 };
    int zIndex_ { 0 };
    float opacity_ { 1.0f };
};

} // namespace ui_runtime
