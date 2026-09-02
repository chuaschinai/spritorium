#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "core/gl.hpp"
#include "core/bitmap.hpp"
#include "core/pixel.hpp"

struct Brush
{
    Vec2 Size;
    Vec2 Offset;
    Bitmap Image;
    gl::Texture Texture;

    Brush(int width, int height)
        : Image(Vec2(width, height))
        , Texture(width, height)
        , Size(width, height)
        , Offset(width / 2, height / 2)
    {}

    uint32_t GetTexture() const { return Texture.ID; }
};

inline std::unique_ptr<Brush> CreateBrushSquare(int size, uint32_t color)
{
    std::unique_ptr<Brush> brush = std::make_unique<Brush>(size, size);

    Bitmap pixel(Vec2(1, 1), { 1 });

    pixel::MakeRectangle(&brush->Image, &pixel, Vec2(), Vec2(size, size), color, true);

    gl::TextureUpdate(brush->Texture, brush->Image.Pixels, 0, 0, size, size);

    return brush;
}

inline std::unique_ptr<Brush> CreateBrushCircle(int size, uint32_t color)
{
    if (size == 1)
    {
        return CreateBrushSquare(size, color);
    }

    std::unique_ptr<Brush> brush = std::make_unique<Brush>(size, size);

    Bitmap pixel(Vec2(1, 1), { 1 });

    pixel::MakeEllipse(&brush->Image, &pixel, Vec2(), Vec2(size-1, size-1), color, true);

    gl::TextureUpdate(brush->Texture, brush->Image.Pixels, 0, 0, size, size);

    return brush;
}