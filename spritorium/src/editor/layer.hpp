#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <glad/glad.h>

#include "core/gl.hpp"
#include "core/types.hpp"
#include "core/bitmap.hpp"
#include "dirty_rect.hpp"

namespace editor
{

    struct LayerContext
    {
        std::string Name;
        Vec2 Size;
        Bitmap Image;
        gl::Texture Texture;
        DirtyRect Dirty;
        int Id;
        bool IsHidden;

        LayerContext(const std::string& name, const Vec2& size);
        ~LayerContext();

        uint32_t GetTexture() const { return Texture.ID; }
        uint32_t* GetPixels() { return Image.Pixels.data(); }
        RGBA GetPixel(int x, int y) const;
        void SetPixel(int x, int y, RGBA color);
        void UpdateTexture();
    };

}