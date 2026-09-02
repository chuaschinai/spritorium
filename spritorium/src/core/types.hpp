#pragma once

#include <cstdint>
#include <algorithm>
#include <memory>

#include <spdlog/spdlog.h>
// #include <spdlog/fmt/compile.h>

namespace editor
{
    using RefSprite = std::shared_ptr<struct Sprite>;
    using RefLayer = std::shared_ptr<struct LayerContext>;
}

/*
    Vectors
*/

struct Vec2
{
    int x, y;

    constexpr Vec2() : x(0), y(0) {}
    constexpr Vec2(int x, int y) : x(x), y(y) {}
    constexpr Vec2(const Vec2& v) : x(v.x), y(v.y) {}
    
    int GetArea() const { return x * y; }
};

inline bool operator==(const Vec2& lhs, const Vec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline Vec2& operator-=(Vec2& lhs, const float& rhs) { lhs.x -= rhs; lhs.y -= rhs; return lhs; }
inline Vec2& operator+=(Vec2& lhs, const Vec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline Vec2& operator-=(Vec2& lhs, const Vec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) { return Vec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline Vec2 operator/(const Vec2& lhs, int val) { return Vec2(lhs.x / 2, lhs.y / 2); }

inline Vec2 Vec2Min(const Vec2& a, const Vec2& b) { return Vec2( std::min(a.x, b.x), std::min(a.y, b.y)); }
inline Vec2 Vec2Max(const Vec2& a, const Vec2& b) { return Vec2( std::max(a.x, b.x), std::max(a.y, b.y)); }

struct Vec4
{
    int x, y, z, w;

    constexpr Vec4() : x(0), y(0), z(0), w(0) {}
    constexpr Vec4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
    constexpr Vec4(const Vec2& a, const Vec2& b) : x(a.x), y(a.y), z(b.x), w(b.y) {}
    constexpr Vec4(const Vec4& r) : x(r.x), y(r.y), z(r.z), w(r.w) {}

    int GetWidth() const { return z - x; }
    int GetHeight() const { return w - y; }
};

inline bool operator==(const Vec4& lhs, const Vec4& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w; }

inline bool PointInsideRect(const Vec2& p, const Vec4& v) { return (p.x >= v.x && p.x <= v.z && p.y >= v.y && p.y <= v.w); }

inline bool RectsOverlaps(const Vec4& va, const Vec4& vb) { return (va.z > vb.x && vb.z > va.x) && (va.w > vb.y && vb.w > va.y); }

/*
    RGBA
*/

inline constexpr int COL_R_SHIFT { 0 };
inline constexpr int COL_G_SHIFT { 8 };
inline constexpr int COL_B_SHIFT { 16 };
inline constexpr int COL_A_SHIFT { 24 };

inline uint8_t ColorGetAlpha(uint32_t color) {
    return ((color >> COL_A_SHIFT) & 0xFF);
}

struct RGBA {
    uint8_t r, g, b, a;

    constexpr RGBA() : r(0), g(0), b(0), a(0) {}

    constexpr RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    constexpr RGBA(uint32_t col) : r((col >> COL_R_SHIFT) & 0xFF) , g((col >> COL_G_SHIFT) & 0xFF) , b((col >> COL_B_SHIFT) & 0xFF) , a((col >> COL_A_SHIFT) & 0xFF)  {};

    constexpr RGBA(float* col) : r(col[0] * 255.0f) , g(col[1] * 255.0f) , b(col[2] * 255.0f) , a(col[3] * 255.0f) {}

    bool operator==(RGBA& rhs) { return (rhs.r == r && rhs.g == g && rhs.b == b && rhs.a == a); }

    constexpr void operator=(float* col)
    {
        r = col[0] * 255.0f;
        g = col[1] * 255.0f;
        b = col[2] * 255.0f;
        a = col[3] * 255.0f;
    }

    constexpr operator uint32_t() {
        return (
            (r << COL_R_SHIFT) |
            (g << COL_G_SHIFT) |
            (b << COL_B_SHIFT) |
            (a << COL_A_SHIFT)
        );
    }

    operator uint8_t*() { return reinterpret_cast<uint8_t*>(this); }

    void ToFloat4(float* col) const {
        col[0] = static_cast<float>(r) / 255.0f;
        col[1] = static_cast<float>(g) / 255.0f;
        col[2] = static_cast<float>(b) / 255.0f;
        col[3] = static_cast<float>(a) / 255.0f;
    }
};

inline constexpr uint32_t C_WHITE = RGBA(255, 255, 255, 255);
inline constexpr uint32_t C_BLACK = RGBA(0, 0, 0, 255);
inline constexpr uint32_t C_BLANK = RGBA(0, 0, 0, 0);
inline constexpr uint32_t C_RED = RGBA(255, 0, 0, 255);


/*
    FMT custom formatting types
*/

#define CREATE_FORMATTER_CUSTOM(TYPE, FMT_STR, ...)\
    template <> struct fmt::formatter<TYPE>\
    {\
        constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }\
        template <typename FormatContext>\
        auto format(const TYPE& p, FormatContext& ctx) const { return format_to(ctx.out(), FMT_STR, __VA_ARGS__); }\
    };

CREATE_FORMATTER_CUSTOM(Vec2, "({}, {})", p.x, p.y);
CREATE_FORMATTER_CUSTOM(Vec4, "({}, {}, {}, {})", p.x, p.y, p.z, p.w);