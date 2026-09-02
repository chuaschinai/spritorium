#pragma once

#include <memory>

#include "core/types.hpp"
#include "editor/sprite.hpp"
#include "editor/layer.hpp" // IWYU pragma: keep
#include "tools/brush.hpp"

struct KeyState 
{
    bool Shift;
    bool Ctrl;
};

struct SpriteContext
{
    std::weak_ptr<editor::Sprite> Sprite;
    Brush* CurrentBrush;
    Vec2 Mouse;
    bool Hover;
    bool IsMouseMove;
};

struct ToolBase
{
    ~ToolBase() = default;

    virtual void MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color) {}
    virtual void MouseReleased(const SpriteContext& ctx, const KeyState& key, RGBA& color) {}
    virtual void MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color) {}

protected:
    Vec4 Area;
    Vec2 MaxSize;

    Vec4 CalcAreaWithBrush(const Brush& brush);
    void StartPoint(const Vec2& size);
    void AddPoint(const Vec2& pos);
};

namespace tool
{

struct Pencil : public ToolBase
{
    virtual void MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
    virtual void MouseReleased(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
    virtual void MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;

protected:
    Vec2 MouseStart;
};

struct Eraser final : public Pencil
{
    void MouseReleased(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
    void MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
};

struct Rectangle : public ToolBase
{
    virtual void MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
    virtual void MouseReleased(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
    virtual void MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
protected:
    Vec2 MouseStart;
};

struct Ellipse final : public Rectangle
{
    void MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
};

struct Pipette final : public ToolBase
{
    void MouseDown(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
};

struct Bucket final : public ToolBase
{
    void MousePressed(const SpriteContext& ctx, const KeyState& key, RGBA& color) override;
};

}
