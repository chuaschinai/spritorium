#pragma once

#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{

    inline bool ButtonIconSelected(const char* icon, bool selected, const char* tooltip = nullptr)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImVec4()));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_HeaderActive));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_HeaderHovered));

        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_HeaderActive));
        }

        const bool pressed = ImGui::Button(icon, ImVec2(26, 26));
        if (tooltip)
        {
            ImGui::SetItemTooltip("%s", tooltip);
        }

        if (selected)
        {
            ImGui::PopStyleColor();
        }
        
        ImGui::PopStyleColor(3);

        return pressed;
    }

    struct DotState
    {
        ImVec2 Pos;
        ImVec2 Offset;
        float Radius { 16.0f };
        ImU32 Color;
    };

    struct TransformState
    {
        ImVec2 Pos;
        float Scale;

        DotState Dot;
    };

    inline void Dot(const char* str_id, DotState* state)
    {
        IM_ASSERT(state != nullptr && "Invalid dot state");

        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;

        ImU32 color = state->Color;
        
        SetCursorPos(state->Pos + state->Offset);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        SetNextItemAllowOverlap();
        Button(str_id, ImVec2(state->Radius * 2, state->Radius * 2));

        pos += GetItemRectSize() / 2.0f;

        if (IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left))
        {
            state->Offset += g.IO.MouseDelta;
            color = ImColor(255, 255, 0, 255);
        }

        window->DrawList->AddCircle(pos, state->Radius, color);
    }

    inline void Transform(TransformState* s)
    {
        IM_ASSERT(s != nullptr && "Invalid state");

        s->Dot.Pos = s->Pos;

        Dot("*", &s->Dot);
    }

}