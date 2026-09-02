#include "view.hpp"

#include <algorithm>
#include <format>

#include <imgui_internal.h>

#include "core/utils.hpp"
#include "core/types.hpp"
#include "app/logger.hpp"
#include "app/app_state.hpp"
#include "app/im_state.hpp"

static int id_increment {};

namespace editor
{

    ViewContext::ViewContext(RefSprite& sprite)
        : Sprite(sprite)
        , ScaleIndex(3)
        , Frame0(1, 1)
        , Id(id_increment++)
        , Open(true)
        , HoverCanvas(false)
        , RequestClose(false)
    {
        Name = std::format("{}##{}", sprite->Name, Id);
    }

    ViewContext::~ViewContext()
    {
        spto::Info("[ViewContext] {} destroyed!", Name);
    }

    bool ViewContext::Render()
    {
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        const ImGuiIO& io = ImGui::GetIO();
        const ImGuiStyle& style = ImGui::GetStyle();

        ImGuiWindow* window = MyImGuiWindow = ImGui::GetCurrentWindow();
        WindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        HoverCanvas = ImGui::IsMouseHoveringRect(window->ContentRegionRect.Min, window->ContentRegionRect.Max);
        Avail = ImGui::GetContentRegionAvail();

        ImVec2 frame(Frame0.Width, Frame0.Height);
        if (Avail != frame && Avail.x >= 1.0f && Avail.y >= 1.0f)
        {
            Frame0.Resize(ToVec2(Avail));
        }

        FrameUpdate();

        const ImVec2 screenpos = ImGui::GetCursorScreenPos();
        const ImVec2 cursor_start = ImGui::GetCursorPos();
        const ImVec2 canvas_size = ToImVec2(Sprite->Size);
        
        ImGui::SetNextItemAllowOverlap();
        const bool interact = ImGui::InvisibleButton("##interact", Avail, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_AllowOverlap);
        const bool hover_view = ImGui::IsItemHovered();

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
        {
            Offset += io.MouseDelta;
        }

        static const ImVec2 padding_bounds(32.0f, 32.0f);
        Offset = ImClamp(Offset, padding_bounds - canvas_size * GetScale(), Avail - padding_bounds);

        ImVec2 texture_pos = screenpos + Offset;
        const ImVec2 mouse_view = io.MousePos - texture_pos;
        MouseCanvas = ImFloor(mouse_view / GetScale());

        if (hover_view) {
            // zoom
            float wheel = io.MouseWheel;
            if (wheel != 0.0f) {
                float prev_scale = GetScale();

                int value = static_cast<int>(ScaleIndex + wheel);
                ScaleIndex = std::clamp(value, 0, static_cast<int>(ScaleList.size()) - 1);
                
                const ImVec2 ratio = mouse_view / (canvas_size * prev_scale);
                Offset += mouse_view - (canvas_size * GetScale()) * ratio;
                Offset = ImFloor(Offset);

                texture_pos = screenpos + Offset;
            }
        }

        const ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        ImGui::PushClipRect(screenpos, screenpos + Avail, false);
        {
            drawlist->AddCallback(platform_io.DrawCallback_SetSamplerNearest);

            drawlist->AddImageRounded(Frame0.TextureID, screenpos, screenpos + ImVec2(Frame0.Width, Frame0.Height), ImVec2(), ImVec2(1.0f, 1.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f), 4.0f);

            drawlist->AddCallback(platform_io.DrawCallback_SetSamplerLinear);
        }
        ImGui::PopClipRect();

        // over view
        ImGui::SetCursorPos(cursor_start + style.FramePadding);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
        ImGui::BeginChild("overview", ImVec2(), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleVar();
        {
            const float max_width = ImGui::GetWindowWidth();

            if (ImGui::Button(ICON_FA_DOWN_LEFT_AND_UP_RIGHT_TO_CENTER, ImVec2(24.0f, 24.0f)))
            {
                CenterCanvas();
            }
            ImGui::SetItemTooltip("Centralize");
        }
        ImGui::EndChild();

        // ImGui::Transform(&TransformController);

#ifdef DEBUG
        char debug_info[128];
        ImFormatString(debug_info, 128, "Offset (%.2f, %0.2f)", Offset.x, Offset.y);
        drawlist->AddText(screenpos + ImVec2(style.WindowPadding.x, Avail.y - style.WindowPadding.y * 2 - style.WindowPadding.y), C_WHITE, debug_info);
#endif

        // perform once
        if (ImGui::IsWindowAppearing())
        {
            CenterCanvas();
            ImGui::SetWindowDock(ImGui::FindWindowByName(GetName()), gIm->DockCentralId, ImGuiCond_Once);
        }

        return interact;
    }

    void ViewContext::CenterCanvas()
    {
        const ImVec2 size = ToImVec2(Sprite->Size);
        int index = 0;
        for (int i = 0; i < ScaleList.size(); ++i) {
            ImVec2 size_zoomed = size * ScaleList[i];
            if (size_zoomed.x > Avail.x || size_zoomed.y > Avail.y) {
                index = ImClamp(i - 1, 0, static_cast<int>(ScaleList.size()) - 1);
                break;
            }
        }
        
        ScaleIndex = index;
        const ImVec2 new_size = size * GetScale();
        Offset = (Avail - new_size) / 2.0f;
        Offset = ImFloor(Offset);
    }

    void ViewContext::FrameUpdate()
    {
        const gl::Shader& s = g.FindShader("canvas");

        auto* brush = g.Tools->CurrentBrush.get();

        gl::FrameBufferBind(Frame0.ID);
        {
            glViewport(0, 0, Frame0.Width, Frame0.Height);

            s.Use();
            s.SetUniform1i("uTex0", 0);
            s.SetUniform1f("uScale", GetScale());
            s.SetUniform2f("uTexSize", Sprite->Size.x, Sprite->Size.y);
            s.SetUniform2f("uResolution", Frame0.Width, Frame0.Height);
            s.SetUniform2f("uPos", Offset.x, Offset.y);

            s.SetUniform1f("uBillboardSize", 16.0f);
            s.SetUniform3f("uBillboardColor", 0.25f, 0.25f, 0.25f);

            s.SetUniform1i("uTexBrush", 1);
            s.SetUniform2f("uBrushSize", brush->Size.x, brush->Size.y);
            s.SetUniform2f("uBrushPos", MouseCanvas.x, MouseCanvas.y);
            s.SetUniform2f("uBrushAlignment", brush->Offset.x, brush->Offset.y);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Sprite->GetRenderer(RenderType::Canvas).TextureID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, brush->GetTexture());

            gl::RenderQuad();
        }
        gl::FrameBufferUnbind();
    }
}
