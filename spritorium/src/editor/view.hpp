#pragma once

#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

#include "core/gfx.hpp"
#include "sprite.hpp"
#include "app/imgui_custom_widgets.hpp"

namespace editor
{

    inline std::vector<float> ScaleList = {
        0.25f, 0.33f, 0.50f, 1.00f, 2.00f, 3.000f,
        4.00f, 5.00f, 6.00f, 8.00f, 12.0f, 18.00f,
        28.0f, 38.0f, 50.0f, 70.0f, 90.0f, 128.0f
    };

    struct ViewContext
    {
        ImVec2 MouseCanvas;
        ImGuiWindow* MyImGuiWindow;
        RefSprite Sprite;
        std::string Name;
        int Id;
        bool Open;
        bool HoverCanvas;
        bool WindowHovered;
        bool RequestClose;

        ImGui::TransformState TransformController;

        ViewContext(RefSprite& sprite_doc);
        ~ViewContext();

        const char* GetName() const { return Name.c_str(); }
        const float GetScale() const { return ScaleList[ScaleIndex]; }

        bool Render();
        void CenterCanvas();

    private:
        ImVec2              Avail;
        ImVec2              LastContentAvail;
        ImVec2              Offset;
        int                 ScaleIndex;
        gfx::FrameBuffer    Frame0;
        
        void FrameUpdate();
    };

}