#include "gfx.hpp"

#include "editor/layer.hpp" // IWYU pragma: keep
#include "app/logger.hpp"

static void InitFrameBuffer(GLuint* id, GLuint* texture, int width, int height)
{
    GLuint fb_id, tex_id;
    glGenFramebuffers(1, &fb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
    *id = fb_id;

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    std::vector<uint8_t> zero(width * height * 4, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
    *texture = tex_id;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error(std::string("glCheckFramebufferStatus failed!"));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

namespace gfx
{
    FrameBuffer::FrameBuffer()
        : Width(1)
        , Height(1)
    {
        InitFrameBuffer(&ID, &TextureID, 1, 1);
    }

    FrameBuffer::FrameBuffer(int width, int height)
        : Width(width)
        , Height(height)
    {
        InitFrameBuffer(&ID, &TextureID, width, height);
    }

    FrameBuffer::~FrameBuffer()
    {
        glDeleteTextures(1, &TextureID);
        glDeleteFramebuffers(1, &ID);
    }

    void FrameBuffer::Resize(const Vec2& size)
    {
        if (Width == size.x && Height == size.y)
        {
            return;
        }

        if (size.x < 1 || size.y < 1)
        {
            spto::Warn("FrameBuffer try resize to negative value!");
            Width = 1;
            Height = 1;
        }
        else
        {
            Width = size.x;
            Height = size.y;
        }

        glBindTexture(GL_TEXTURE_2D, TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static Vec4 CalcDirtyArea(Bitmap* brush, const Vec2& pA, const Vec2& pB)
    {
        const Vec2 offset = brush->Size / 2;

        return {
            std::min(pA.x, pB.x) - offset.x,
            std::min(pA.y, pB.y) - offset.y,
            std::max(pA.x, pB.x) + offset.x,
            std::max(pA.y, pB.y) + offset.y
        };
    }

    void DrawLine(editor::RefLayer layer, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color)
    {
        pixel::MakeLine(&layer->Image, brush, pA, pB, color);

        const Vec4 area = CalcDirtyArea(brush, pA, pB);

        layer->Dirty.AddArea(area);
    }

    void DrawRectangle(editor::RefLayer layer, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color, bool filled)
    {
        pixel::MakeRectangle(&layer->Image, brush, pA, pB, color, false);
        
        if (filled)
        {
            const Vec2 offset = brush->Size / 2;
            pixel::MakeRectangle(&layer->Image, brush, pA + offset, pB - offset, color, true);
        }

        const Vec4 area = CalcDirtyArea(brush, pA, pB);

        layer->Dirty.AddArea(area);
    }

    void DrawEllipse(editor::RefLayer layer, Bitmap* brush, const Vec2& pA, const Vec2& pB, RGBA color, bool filled)
    {
        pixel::MakeEllipse(&layer->Image, brush, pA, pB, color, filled);

        const Vec4 area = CalcDirtyArea(brush, pA, pB);

        layer->Dirty.AddArea(area);
    }

    void DrawFloodFill(editor::RefLayer layer, const Vec2& pos, RGBA color, int tolerance)
    {
        uint32_t oc = layer->GetPixel(pos.x, pos.y);
        if (oc == color){ return; }

        Vec2 op = pos;
        
        std::queue<Vec2> Q;
        Q.push(pos);

        const Vec4 area(0, 0, layer->Size.x - 1, layer->Size.y - 1);

        while (!Q.empty())
        {
            const Vec2 p = Q.front();
            Q.pop();
            op = p;
            uint32_t pixel = layer->GetPixel(p.x, p.y);
            if (PointInsideRect(op, area) && pixel == oc)
            {
                layer->SetPixel(p.x, p.y, color);
                
                Q.push({ p.x - 1, p.y });
                Q.push({ p.x + 1, p.y });
                Q.push({ p.x, p.y - 1 });
                Q.push({ p.x, p.y + 1 });
            }
        }
    }

    void DrawFill(editor::RefLayer layer, RGBA color)
    {
        pixel::MakeFill(&layer->Image, color);
        layer->Dirty.AddFull();
    }

    void MergeLayer(editor::RefLayer layer_target, editor::RefLayer layer_source, PixelFilterOp filter_op)
    {
        pixel::MakeMerge(&layer_target->Image, &layer_source->Image, filter_op);
        layer_target->Dirty.AddFull();
    }

}