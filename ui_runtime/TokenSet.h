#pragma once

namespace ui_runtime {

struct TokenColor {
    float r { 0.0f };
    float g { 0.0f };
    float b { 0.0f };
    float a { 1.0f };
};

struct TokenSpacing {
    float xs { 0.0f };
    float sm { 0.0f };
    float md { 0.0f };
    float lg { 0.0f };
};

struct TokenRadius {
    float sm { 0.0f };
    float md { 0.0f };
    float lg { 0.0f };
};

class TokenSet {
public:
    TokenColor background {};
    TokenColor panel {};
    TokenColor accent {};
    TokenColor text {};
    TokenColor mutedText {};
    TokenColor track {};
    TokenSpacing spacing {};
    TokenRadius radius {};
    float fontSize { 0.0f };
    float fontSizeSmall { 0.0f };
    float fontSizeLarge { 0.0f };
};

} // namespace ui_runtime
