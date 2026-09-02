#pragma once

#include "core/types.hpp"

namespace editor
{
    struct DirtyRect
    {
        DirtyRect()
        {
            Area.x = MaxSize.x;
            Area.y = MaxSize.y;
        }
        
        DirtyRect(const Vec2& max_size) : MaxSize(max_size), IsDirty(false)
        {
            Area.x = MaxSize.x;
            Area.y = MaxSize.y;
        };

        void AddPoint(Vec2 pos)
        {
            if (pos.x > MaxSize.x) { pos.x = MaxSize.x; }
            if (pos.y > MaxSize.y) { pos.y = MaxSize.y; }

            Area.x = std::min(Area.x, pos.x);
            Area.y = std::min(Area.y, pos.y);
            Area.z = std::max(Area.z, pos.x + 1);
            Area.w = std::max(Area.w, pos.y + 1);

            IsDirty = true;
        }

        void AddArea(const Vec4& area)
        {
            AddPoint(Vec2(area.x, area.y));
            AddPoint(Vec2(area.z, area.w));
        }

        void AddFull()
        {
            Area.x = Area.y = 0;
            Area.z = MaxSize.x; Area.w = MaxSize.y;
            IsDirty = true;
        }

        void Clear()
        {
            Area.x = MaxSize.x; Area.y = MaxSize.y;
            Area.z = Area.w = 0;
            IsDirty = false;
        }

        Vec4 Area;
        Vec2 MaxSize;
        bool IsDirty;
    };

}