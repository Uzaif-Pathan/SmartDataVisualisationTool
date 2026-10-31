#pragma once
#include <cstdint>

namespace gfx {

// RGBA color, each channel 0-255.  a = 255 is fully opaque, 0 is invisible.
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    constexpr Color() = default;
    constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                    std::uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

constexpr bool operator==(const Color& x, const Color& y) {
    return x.r == y.r && x.g == y.g && x.b == y.b && x.a == y.a;
}
constexpr bool operator!=(const Color& x, const Color& y) { return !(x == y); }

namespace colors {
inline constexpr Color White{255, 255, 255};
inline constexpr Color Black{0, 0, 0};
inline constexpr Color Red{220, 50, 50};
inline constexpr Color Green{60, 170, 80};
inline constexpr Color Blue{50, 100, 210};
inline constexpr Color Orange{240, 150, 40};
inline constexpr Color Purple{140, 80, 190};
inline constexpr Color Gray{128, 128, 128};
inline constexpr Color LightGray{220, 220, 220};
inline constexpr Color Transparent{0, 0, 0, 0};
}  // namespace colors

}  // namespace gfx
