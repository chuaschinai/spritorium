#pragma once

#include <deque>

#include <imgui_internal.h>

namespace app
{
    // Manage one or multiple views/documents to close
    struct ManagerApplicationCloseRequest
    {
        std::deque<int> PendingViewIdsToClose;
        ImGuiWindow* CurrentImGuiWindowToClose;
        int CurrentViewIdToClose;
        bool RequestClose;

        ManagerApplicationCloseRequest()
            : CurrentImGuiWindowToClose(nullptr)
            , CurrentViewIdToClose(-1)
            , RequestClose(false)
        {}

        bool IsEmpty() const { return PendingViewIdsToClose.empty(); }
        int GetTopId() const { return PendingViewIdsToClose.back(); }

        void AddViewIdToClose(int id)
        {
            const auto it = std::ranges::find(PendingViewIdsToClose, id);
            assert(it == PendingViewIdsToClose.end() && "Already view with same Id on deque!");
            PendingViewIdsToClose.push_back(id);
        }

        void Cancel()
        {
            RequestClose = false;
            PendingViewIdsToClose.clear();
        }
    };
};