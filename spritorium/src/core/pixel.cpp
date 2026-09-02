#include "pixel.hpp"

#include <cmath>
#include <cassert>

#include "composite.hpp"
#include "core/bitmap.hpp"
#include "core/composite.hpp"

static bool PointInRectangle(int px, int py, int width, int height) {
    return (px >= 0 && px < width && py >= 0 && py < height);
}

void pixel::MakePixel(Bitmap* target, const Vec2& pos, RGBA color)
{
    if (!PointInRectangle(pos.x, pos.y, target->Size.x, target->Size.y))
        return;

    uint32_t* const pixels = target->GetData();

    int index = (pos.y * target->Size.x + pos.x);

    assert(index < (target->Size.x * target->Size.y) && "Index pixel out of bound");

    PixelFilter(&pixels[index], color, PixelFilterOp::Passthrough);
}

void pixel::MakePixelBrush(Bitmap* target, const Vec2& pos, Bitmap* brush, RGBA color)
{
    uint32_t* const pixels = target->GetData();

    for (int by = 0; by < brush->Size.y; ++by)
    {
        for (int bx = 0; bx < brush->Size.x; ++bx)
        {
            const Vec2 b_pos(bx, by);
            if (brush->GetColor(b_pos) == C_BLANK)
                continue;

            const Vec2 n_pos = pos + b_pos;
            if (!PointInRectangle(n_pos.x, n_pos.y, target->Size.x, target->Size.y))
                continue;

            const int index = (n_pos.y * target->Size.x + n_pos.x);

            PixelFilter(&pixels[index], color, PixelFilterOp::Passthrough);
        }
    }
}

void pixel::MakeLine(Bitmap* target, Bitmap* brush, Vec2 pA, const Vec2& pB, RGBA color)
{
    int dx = std::abs(pB.x - pA.x);
    int sx = pA.x < pB.x ? 1 : -1;
    int dy = -std::abs(pB.y - pA.y);
    int sy = pA.y < pB.y ? 1 : -1;
    int err = dx + dy;
    int e2;

    const Vec2 offset = brush->Size / 2;

    uint32_t* const pixels = target->Pixels.data();
    while (1)
    {
        MakePixelBrush(target, pA - offset, brush, color);

        if (pA.x == pB.x && pA.y == pB.y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; pA.x += sx; }
        if (e2 <= dx) { err += dx; pA.y += sy; }
    }
}

static void MakeRectangleLined(Bitmap* target, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color)
{
    const Vec2 r_up(pB.x, pA.y);
    const Vec2 l_down(pA.x, pB.y);
    pixel::MakeLine(target, brush, pA, r_up, color);
    pixel::MakeLine(target, brush, r_up, pB, color);
    pixel::MakeLine(target, brush, l_down, pB, color);
    pixel::MakeLine(target, brush, pA, l_down, color);
}

static void MakeRectangleFilled(Bitmap* target, const Vec2& pA, const Vec2& pB, RGBA color)
{
    int w = pB.x - pA.x;
    int h = pB.y - pA.y;

    uint32_t* pixels = target->Pixels.data();
    for (int y = 0; y < h; ++y)
    {
        int py = pA.y + y;
        if (py < 0 || py > target->Size.y - 1)
            continue;

        for (int x = 0; x < w; ++x)
        {
            int px = pA.x + x;
            if (px < 0 || px > target->Size.x - 1)
                continue;

            int index = (py * target->Size.x + px);
            pixel::PixelFilter(&pixels[index], color, PixelFilterOp::Passthrough);
        }
    }
}

void pixel::MakeRectangle(Bitmap* target, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color, bool filled)
{
    const Vec2 nA = Vec2Min(pA, pB);
    Vec2 nB = Vec2Max(pA, pB);

    if (filled)
    {
        MakeRectangleFilled(target, nA, nB, color);
    }

    nB -= 1;
    MakeRectangleLined(target, brush, nA, nB, color);
}

void pixel::MakeEllipse(Bitmap* target, Bitmap* brush, Vec2 pA, Vec2 pB, RGBA color, bool filled)
{   
    int a = abs(pB.x-pA.x), b = abs(pB.y-pA.y), b1 = b&1;
    long dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a;
    long err = dx+dy+b1*a*a, e2;

    if (pA.x > pB.x) { pA.x = pB.x; pB.x += a; }
    if (pA.y > pB.y) pA.y = pB.y; 
    pA.y += (b+1)/2; pB.y = pA.y-b1;   
    a *= 8*a; b1 = 8*b*b;

    auto PlotSpan = [&](int xa, int xb, int y)
    {
        if (filled)
        {
            for (int x = xa; x <= xb; x++)
                MakePixelBrush(target, Vec2(x, y), brush, color);
        } 
        else {
            MakePixelBrush(target, Vec2(xa, y), brush, color); // left quadrant
            MakePixelBrush(target, Vec2(xb, y), brush, color); // right quadrant
        }
    };

    do {
        PlotSpan(pA.x, pB.x, pA.y); // bottom quadrant
        PlotSpan(pA.x, pB.x, pB.y); // top quadrant
        e2 = 2*err;
        if (e2 <= dy) { pA.y++; pB.y--; err += dy += a; }
        if (e2 >= dx || 2*err > dy) { pA.x++; pB.x--; err += dx += b1; }
    } while (pA.x <= pB.x);

    while (pA.y - pB.y < b)
    {
        PlotSpan(pA.x-1, pB.x+1, pA.y);
        PlotSpan(pA.x-1, pB.x+1, pB.y);
        pA.y++; pB.y--;
    }
}

void pixel::MakeFill(Bitmap* target, RGBA color)
{
    uint32_t* const pixels = target->GetData();

    for (int i = 0; i < target->GetArea(); ++i)
        pixels[i] = color;
}

void pixel::MakeMerge(Bitmap* target, Bitmap* source, PixelFilterOp filter_op)
{
    /// TODO: impl area
    /// FIX: if source has different size from target
    for (int y = 0; y < target->Size.y; ++y)
    {
        for (int x = 0; x < target->Size.x; ++x)
        {
            const Vec2 pos(x, y);

            uint32_t source_color = source->GetColor(pos);
            if (source_color == C_BLANK) { continue; }
        
            uint32_t target_color = target->GetColor(pos);
            uint32_t composed = MakeCompose(target_color, source_color, ComposeOp::SrcOver);
            PixelFilter(&source_color, composed, filter_op);
            MakePixel(target, pos, source_color);
        }
    }
}

void pixel::MakeReplace(Bitmap* target, const Vec2& pos, Bitmap* source, PixelFilterOp filter_op)
{
    for (int y = 0; y < source->Size.y; ++y)
    {
        for (int x = 0; x < source->Size.x; ++x)
        {
            const Vec2 ps_pos(x, y);
            const Vec2 pt_pos = pos + ps_pos;
            MakePixel(target, pt_pos, source->GetColor(ps_pos));
        }
    }
}

void pixel::PixelFilter(uint32_t* dst, uint32_t src, PixelFilterOp filter_op) {
    switch (filter_op) {
        case PixelFilterOp::None:
        case PixelFilterOp::Passthrough: *dst = src; break;
        case PixelFilterOp::WhiteErase: *dst = C_BLANK; break;
    }
}