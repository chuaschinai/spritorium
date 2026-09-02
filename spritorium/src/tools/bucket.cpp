#include "tool_base.hpp"

namespace tool
{

    void Bucket::MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color)
    {
        const auto sprite = ctx.Sprite.lock();

        if (!sprite || !ctx.Hover) { return; }

        editor::RefLayer layer = sprite->GetActiveLayer();

        Bitmap copy = layer->Image;
        
        /// TODO: DrawFlooFill returns the modified area directly
        gfx::DrawFloodFill(layer, ctx.Mouse, color, 0);

        sprite->History.Commit(std::make_unique<editor::Command_SpriteDraw>(
            sprite.get(),
            layer->Id,
            Vec2(layer->Dirty.Area.x, layer->Dirty.Area.y),
            copy.Crop(layer->Dirty.Area),
            layer->Image.Crop(layer->Dirty.Area)
        ));
    }

}