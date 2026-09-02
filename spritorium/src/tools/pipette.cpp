#include "tool_base.hpp"

namespace tool
{

    void Pipette::MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();

        if (!sprite || !ctx.Hover) { return; }
        
        const size_t size = sprite->Layers.size();

        for (int i = size - 1; i >= 0; --i)
        {
            const auto& layer = sprite->GetLayer(i);
            const RGBA new_color = layer->GetPixel(ctx.Mouse.x, ctx.Mouse.y);

            if (new_color.a == 0)
            {
                if (i == 0) { color = C_BLANK; }
                continue;
            }

            color = new_color;
            break;
        }
    }

}