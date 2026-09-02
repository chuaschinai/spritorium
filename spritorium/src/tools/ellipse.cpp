#include "tool_base.hpp"

#include "core/gfx.hpp"

namespace tool
{

    void Ellipse::MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();
        if (!sprite) { return; }

        gfx::DrawFill(sprite->GetScratchLayer(), C_BLANK);

        const Vec2 d = ctx.Mouse - MouseStart;
        
        const Vec2 offset = ctx.CurrentBrush->Size / 2;

        Vec2 pA = Vec2Min(MouseStart, ctx.Mouse) - offset;
        Vec2 pB = Vec2Max(MouseStart, ctx.Mouse) - offset;

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

        gfx::DrawEllipse(sprite->GetScratchLayer(), &ctx.CurrentBrush->Image, pA, pB, color, key.Ctrl);

        Area.z = ctx.Mouse.x;
        Area.w = ctx.Mouse.y;
    }

}