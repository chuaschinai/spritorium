#pragma once

#include <glad/glad.h>

#include "core/bitmap.hpp"
#include "core/types.hpp"
#include "core/pixel.hpp"

struct Bitmap;

namespace gfx
{

    struct FrameBuffer
    {
        FrameBuffer();
        FrameBuffer(int width, int height);
        ~FrameBuffer();

        void Resize(const Vec2& size);

        GLuint ID;
        GLuint TextureID;
        int Width;
        int Height;
    };

    void DrawLine(editor::RefLayer layer, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color);

    void DrawRectangle(editor::RefLayer layer, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color, bool filled = false);

    void DrawEllipse(editor::RefLayer layer, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color, bool filled = false);

    void DrawFloodFill(editor::RefLayer layer, const Vec2& pos, RGBA color, int tolerance = 0);

    void DrawFill(editor::RefLayer layer, RGBA color);

    void MergeLayer(editor::RefLayer layer_target, editor::RefLayer layer_source, PixelFilterOp filter_op);

}