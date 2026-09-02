#include "tool_base.hpp"

Vec4 ToolBase::CalcAreaWithBrush(const Brush& brush)
{
    if (brush.Size == Vec2(1, 1))
    {
        if (Area.x < 0) { Area.x = 0; }
        if (Area.y < 0) { Area.y = 0; }
        return Vec4(Area.x, Area.y, Area.z + 1, Area.w + 1);
    }

    const Vec2 m(brush.Size.x % 2, brush.Size.y % 2);

    Area.x = std::max(std::min(Area.x, Area.x - brush.Offset.x), 0);
    Area.y = std::max(std::min(Area.y, Area.y - brush.Offset.y), 0);
    Area.z = std::min(std::max(Area.z, Area.z + brush.Offset.x + m.x), MaxSize.x);
    Area.w = std::min(std::max(Area.w, Area.w + brush.Offset.y + m.y), MaxSize.y);

    return Area;
}

void ToolBase::StartPoint(const Vec2& size)
{
    Area.x = size.x; Area.y = size.y;
    Area.z = Area.w = 0;
    MaxSize = size;
}

void ToolBase::AddPoint(const Vec2& pos)
{
    Area.x = std::min(Area.x, pos.x);
    Area.y = std::min(Area.y, pos.y);
    Area.z = std::max(Area.z, pos.x);
    Area.w = std::max(Area.w, pos.y);

    if (Area.x < 0) Area.x = 0;
    if (Area.y < 0) Area.y = 0;
    if (Area.z > MaxSize.x) Area.z = MaxSize.x;
    if (Area.w > MaxSize.y) Area.w = MaxSize.y;
}