#include "layer.hpp"

#include <string>

#include "core/utils.hpp"
#include "app/logger.hpp"

namespace editor
{

    LayerContext::LayerContext(const std::string& name, const Vec2& size) 
        : Id(spto::GetID())
        , Name(name)
        , Size(size)
        , Image(size)
        , Texture(size.x, size.y)
        , Dirty(size)
        , IsHidden(false)
    {}

    LayerContext::~LayerContext()
    {
        spto::Info("[Layer] {} destroyed", Name);
    }

    RGBA LayerContext::GetPixel(int x, int y) const
    {
        if (x < 0 || x >= Size.x || y < 0 || y >= Size.y) return C_BLANK;
        return Image.Pixels[y * Size.x + x];
    }

    void LayerContext::SetPixel(int x, int y, RGBA color)
    {
        Image.Pixels[(y * Size.x + x)] = color;

        Dirty.AddPoint(Vec2(x, y));
    }

    void LayerContext::UpdateTexture()
    {
        if (!Dirty.IsDirty) { return; }

        int x1 = std::min(Dirty.Area.x, Dirty.Area.z);
        int y1 = std::min(Dirty.Area.y, Dirty.Area.w);
        int x2 = std::max(Dirty.Area.x, Dirty.Area.z);
        int y2 = std::max(Dirty.Area.y, Dirty.Area.w);

        int w = x2 - x1;
        int h = y2 - y1;

        if (x1 < 0)
        {
            w += x1;
            x1 = 0;
        }

        if (y1 < 0)
        {
            h += y1;
            y1 = 0;
        }

        if (x1 + w > Texture.Width)
            w = Texture.Width - x1;

        if (y1 + h > Texture.Height)
            h = Texture.Height - y1;

        x1 = std::clamp(x1, 0, Texture.Width);
        y1 = std::clamp(y1, 0, Texture.Height);
        w  = std::clamp(w, 0, Texture.Width  - x1);
        h  = std::clamp(h, 0, Texture.Height - y1);

        if (w <= 0 || h <= 0)
        {
            Dirty.Clear();
            return;
        }

        gl::TextureUpdate(Texture, Image.Pixels, x1, y1, w, h);

        Dirty.Clear();
    };

}