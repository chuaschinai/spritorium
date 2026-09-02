#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "core/types.hpp"

inline constexpr int BPP { 4 };

struct Bitmap
{
    Bitmap() {};
    Bitmap(const Vec2& size, const std::vector<uint32_t>& pixels);
    Bitmap(const Vec2& size);
    Bitmap(const std::string& filepath);

    void Load(const std::string& path);
    void Save(const std::string& path);

    uint32_t GetColor(const Vec2& pos) const
    {
        if (pos.x < 0 || pos.x >= Size.x || pos.y < 0 || pos.y >= Size.y) return 0;
        return Pixels[pos.y * Size.x + pos.x];
    }
    void SetPixel(const Vec2& pos, uint32_t color) { Pixels[(pos.y * Size.x + pos.x)] = color; }
    uint32_t* GetData() { return Pixels.data(); }
    const int GetTotalBytes() const { return Size.x * Size.y * sizeof(int); }
    const int GetArea() { return Size.GetArea(); }
    Bitmap Crop(Vec4 r) const;

    std::vector<uint32_t> Pixels;
    Vec2 Size;
};