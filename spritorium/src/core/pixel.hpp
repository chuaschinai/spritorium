#pragma once

#include "types.hpp"

/* Bresenham algorithms
 * Reference: http://members.chello.at/easyfilter/bresenham.html
 */

struct Bitmap;

enum class PixelFilterOp {
    None,
    Passthrough,
    WhiteErase
};

namespace pixel
{

void MakePixel(Bitmap* target, const Vec2& pos, RGBA color);

void MakePixelBrush(Bitmap* target, const Vec2& pos, Bitmap* brush, RGBA color);

void MakeLine(Bitmap* target, Bitmap* brush, Vec2 pA, const Vec2& pB, RGBA color = C_WHITE);

void MakeRectangle(Bitmap* target, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color = C_WHITE, bool filled = true);

void MakeEllipse(Bitmap* target, Bitmap* brush, Vec2 pA, Vec2 pB, RGBA color, bool filled);

void MakeFill(Bitmap* target, RGBA color);

void MakeMerge(Bitmap* target, Bitmap* source, PixelFilterOp filter_op);

void MakeReplace(Bitmap* target, const Vec2& pos, Bitmap* source, PixelFilterOp filter_op);

void PixelFilter(uint32_t* dst, uint32_t src, PixelFilterOp filter_op);

}