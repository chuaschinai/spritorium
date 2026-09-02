#include "tool_base.hpp"

#include "core/gfx.hpp"
#include "editor/history.hpp"

namespace tool
{

    void Pencil::MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        MouseStart = ctx.Mouse;

        StartPoint(sprite->Size);
    }

    void Pencil::MouseReleased(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        // swap braided area
        if (Area.x > Area.z) { std::swap(Area.x, Area.z); }
        if (Area.y > Area.w) { std::swap(Area.y, Area.w); }

        if (!RectsOverlaps(Area, Vec4(Vec2(), sprite->Size)))
        {
            return;
        }

        const Vec4 rect = CalcAreaWithBrush(*ctx.CurrentBrush);

        Bitmap* const image = &sprite->GetActiveLayer()->Image;
        Bitmap prev_image = image->Crop(rect);

        gfx::MergeLayer(sprite->GetActiveLayer(), sprite->GetScratchLayer(), PixelFilterOp::None);
        gfx::DrawFill(sprite->GetScratchLayer(), C_BLANK);

        Bitmap next_image = image->Crop(rect);
        sprite->History.Commit(std::make_unique<editor::Command_SpriteDraw>(
            sprite.get(),
            sprite->GetActiveLayer()->Id,
            Vec2(rect.x, rect.y),
            prev_image,
            next_image
        ));
    }

    void Pencil::MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        if (key.Shift)
        {
            gfx::DrawFill(sprite->GetScratchLayer(), C_BLANK);
        }

        gfx::DrawLine(sprite->GetScratchLayer(), &ctx.CurrentBrush->Image, MouseStart, ctx.Mouse, color);

        if (!key.Shift)
        {
            MouseStart = ctx.Mouse;
        }

        AddPoint(ctx.Mouse);
    }

}