#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include <glad/glad.h>
#include <SDL3/SDL_video.h>

#include "core/io.hpp"
#include "task.hpp"
#include "core/shader.hpp"
#include "editor/view.hpp"
#include "tools/tool_manager.hpp"
#include "manager_application_close_request.hpp"

namespace editor { struct ViewContext; }

inline constexpr const char* AppVersion = "0.1.0";

namespace app
{

    struct AppState
    {
        AppState()
            : GL(nullptr)
            , Window(nullptr)
            , IsRunning(true)
            , DemoWindow(false)
            , SelectedView(-1)
            , UncappedFramerate(false)
            , RequestCloseApplication(false)
            , Modal_SaveBeforeClose(false)
            , CurrentDrawingView(nullptr)
        {};

        spto::TaskManager Task;
        std::unordered_map<std::string, std::unique_ptr<gl::Shader>> Shaders;
        std::vector<editor::RefSprite> Sprites;
        std::vector<std::unique_ptr<editor::ViewContext>> Views;
        std::unique_ptr<ToolManager> Tools;
        SDL_GLContext GL;
        SDL_Window* Window;
        ImGuiWindow* CurrentDrawingView;
        ManagerApplicationCloseRequest AppCloseRequest;
        int SelectedView;
        bool IsRunning;
        bool DemoWindow;
        bool UncappedFramerate;
        bool RequestCloseApplication;
        bool Modal_SaveBeforeClose;

        template <typename T, typename... Args>
        void CreateTask(Args&&... args) { Task.Push(std::make_unique<T>(std::forward<Args>(args)...)); }
        void ProcessTasks() { Task.Process(); }

        gl::Shader& FindShader(const std::string& name);
        void LoadShaderFromData(const std::string& shader_name, std::string_view fragment, std::string_view vertex = gl::vertDefault());

        editor::RefSprite GetCurrentSprite();
        editor::RefSprite CreateSprite(const std::string& name, int width, int height);
        editor::RefSprite CreateSpriteFromBitmaps(const std::string& name, const std::vector<Bitmap>& bitmaps);
        editor::RefSprite CreateSpriteFromDocumentRecord(const io::DocumentRecord& r, const std::string& filepath = "");

        editor::ViewContext* GetViewById(int id);
        editor::ViewContext* GetCurrentView();
        void SelectView(int id);
    };
};

inline app::AppState g;