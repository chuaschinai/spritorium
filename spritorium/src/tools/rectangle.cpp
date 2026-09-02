#include "tool_base.hpp"

#include <algorithm>

#include "core/gfx.hpp"

namespace tool
{

    void Rectangle::MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        MouseStart = ctx.Mouse;

        Area.x = MouseStart.x;
        Area.y = MouseStart.y;
        MaxSize = sprite->Size;
    }

    void Rectangle::MouseReleased(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        // swap braided area
        if (Area.x > Area.z) { std::swap(Area.x, Area.z); }
        if (Area.y > Area.w) { std::swap(Area.y, Area.w); }

        const Vec4 rect = CalcAreaWithBrush(*ctx.CurrentBrush);

        if (!RectsOverlaps(rect, Vec4(Vec2(), sprite->Size)))
        {
            return;
        }

        Bitmap* const image = &sprite->GetActiveLayer()->Image;
        const Bitmap prev_image = image->Crop(rect);

        gfx::MergeLayer(sprite->GetActiveLayer(), sprite->GetScratchLayer(), PixelFilterOp::None);
        gfx::DrawFill(sprite->GetScratchLayer(), C_BLANK);

        const Bitmap next_image = image->Crop(rect);
        sprite->History.Commit(std::make_unique<editor::Command_SpriteDraw>(
            sprite.get(),
            sprite->GetActiveLayer()->Id,
            Vec2(rect.x, rect.y),
            prev_image,
            next_image
        ));
    }

    void Rectangle::MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        gfx::DrawFill(sprite->GetScratchLayer(), C_BLANK);

        const Vec2 d = ctx.Mouse - MouseStart;

        Vec2 pA = Vec2Min(MouseStart, ctx.Mouse);
        Vec2 pB = Vec2Max(MouseStart, ctx.Mouse);

        if (key.Shift)
        {
            const int dx_abs = std::abs(d.x);
            const int dy_abs = std::abs(d.y);

            if (dx_abs < dy_abs)
            {
                if (d.y > 0)
                    pB.y = pA.y + dx_abs;
                else
                    pA.y = pB.y - dx_abs;
            }
            else {
                if (d.x > 0)
                    pB.x = pA.x + dy_abs;
                else
                    pA.x = pB.x - dy_abs;
            }
        }

        pB += Vec2(1, 1);

        gfx::DrawRectangle(sprite->GetScratchLayer(), &ctx.CurrentBrush->Image, pA, pB, color, key.Ctrl);

        Area.z = ctx.Mouse.x;
        Area.w = ctx.Mouse.y;
    }

}