#include "application.hpp"

#include <chrono>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_timer.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include "core/types.hpp"
#include "core/gl.hpp"
#include "core/shader.hpp"
#include "core/io.hpp"
#include "logger.hpp"
#include "app/task.hpp"
#include "app/app_state.hpp"
#include "app/im_state.hpp"
#include "system/dialog.hpp"
#include "tools/tool_manager.hpp"
#include "editor/view.hpp"

Application::Application()
{
    LoggerInit();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(
            std::string("SDL_Init failed: ") + SDL_GetError()
        );
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    g.Window = SDL_CreateWindow("Spritorium", 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!g.Window)
    {
        SDL_Quit();
        throw std::runtime_error(
            std::string("SDL_CreateWindow failed: ") + SDL_GetError()
        );
    }

    g.GL = SDL_GL_CreateContext(g.Window);
    if (!g.GL)
    {
        SDL_DestroyWindow(g.Window);
        SDL_Quit();
        throw std::runtime_error(
            std::string("SDL_GL_CreateContext failed: ") + SDL_GetError()
        );
    }

    SDL_GL_MakeCurrent(g.Window, g.GL);
    SDL_GL_SetSwapInterval(0);

    SDL_SetWindowPosition(g.Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(g.Window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_GL_DestroyContext(g.GL);
        SDL_DestroyWindow(g.Window);
        SDL_Quit();
        
        throw std::runtime_error(
            std::string("gladLoaderGLLoader failed")
        );
    }

    gl::SetupOpenGLDebugging();
    gl::g_quad.reset(gl::InitQuad());

    gIm = std::make_unique<ImState>(g.Window, g.GL);

    g.Tools = std::make_unique<ToolManager>();

    g.LoadShaderFromData("blit", gl::fragBlit());
    g.LoadShaderFromData("subwhite", gl::fragSubwhite());
    g.LoadShaderFromData("composite", gl::fragComposite());
    g.LoadShaderFromData("canvas", gl::fragCanvas());

#ifdef DEBUG
    const int w = 128;
    const int h = 128;
    const auto canvas = g.CreateSprite("Example", w, h);
    const auto layer = canvas->CreateLayer();
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            uint8_t r = (uint8_t)(x^y);
            uint8_t g = (uint8_t)(y);
            uint8_t b = (uint8_t)(x);
            layer->SetPixel(x, y, { r, g, b });
        }
    }
#endif
}

void Application::Begin()
{
    FpsStart = std::chrono::high_resolution_clock::now();
}

void Application::End()
{
    if (g.UncappedFramerate) { return; }

    const auto fps_end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> elapsed { fps_end - FpsStart };

    if (elapsed < FpsTarget) {
        const auto remaining { FpsTarget - elapsed };
        SDL_DelayPrecise(std::chrono::duration_cast<std::chrono::nanoseconds>(remaining).count());
    }
}

void Application::Run()
{
    Begin();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                g.AppCloseRequest.RequestClose = true;
                if (g.Sprites.empty())
                {
                    g.IsRunning = false;
                }
                else
                {
                    for (const auto& view : g.Views)
                    {
                        g.AppCloseRequest.AddViewIdToClose(view->Id);
                    }
                }
            break;
            case SDL_EVENT_WINDOW_RESIZED:
            {
                int w, h;
                SDL_GetWindowSize(g.Window, &w, &h);
                glViewport(0, 0, w, h);
            } break;
        }
    }
    
    gIm->RenderNewFrame();

    /*
     * ImGui Default Windows
     */
    gIm->RenderMainMenu();
    gIm->RenderSubMenu();
    gIm->RenderFooter();
    gIm->RenderToolMenu();
    gIm->RenderLayers();
    gIm->RenderColorPicker();
    gIm->RenderDebug();
    gIm->RenderModalCreateNewCanvas();
    gIm->RenderModalSaveBeforeClose();
    gIm->RenderModalExportImage();

    if (g.DemoWindow)
    {
        ImGui::ShowDemoWindow(&g.DemoWindow);
    }

    static ImGuiIO& io = ImGui::GetIO();

    // Pending close views
    if (!g.AppCloseRequest.IsEmpty())
    {
        const int view_id = g.AppCloseRequest.GetTopId();

        if (const auto& view = g.GetViewById(view_id))
        {
            if (!view->Sprite->IsUnsavedDocument)
            {
                g.CreateTask<spto::Task_DestroyView>(view_id);
                g.CreateTask<spto::Task_DestroyDocument>(view->Sprite);
                g.AppCloseRequest.PendingViewIdsToClose.pop_back();
                gIm->ModalOpen_SaveBeforeClose = false;
            }
            else
            {
                gIm->ModalOpen_SaveBeforeClose = true;
            }
        }
    }
    else
    {
        gIm->ModalOpen_SaveBeforeClose = false;
        if (g.AppCloseRequest.RequestClose)
        {
            g.IsRunning = false;
        }
    }

    // Render views/documents
    for (const auto& view : g.Views) 
    {
        const auto& dc = view->Sprite;

        dc->RenderProcess();

        const int flags = (dc->IsUnsavedDocument ? ImGuiWindowFlags_UnsavedDocument : 0) | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
        if (!ImGui::Begin(view->GetName(), &view->Open, flags))
        {
            ImGui::End();
            continue;
        }

        if (!view->Open)
        {
            if (dc->IsUnsavedDocument)
            {
                g.AppCloseRequest.AddViewIdToClose(view->Id);
                view->Open = true;
            }
            else
            {
                g.CreateTask<spto::Task_DestroyView>(view->Id);
                g.CreateTask<spto::Task_DestroyDocument>(view->Sprite);
            }
        }
                
        const bool interact = view->Render();
        
        if (interact)
        {
            g.SelectView(view->Id);
        }

        ImGui::End();
    }

    // CTRL + S (quick save)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S) && !g.Sprites.empty())
    {
        auto sprite = g.GetCurrentSprite();

        if (sprite->Filepath.empty())
        {
            auto ptr = std::make_unique<editor::RefSprite>(sprite);

            dl::DialogFileSaveContext ctx
            {
                .Userdata = ptr.release(),
                .Success = [](const std::string& filepath, const std::string&, void* userdata)
                {
                    editor::RefSprite* sprite = static_cast<editor::RefSprite*>(userdata);
                    g.CreateTask<spto::Task_SaveDocument>(*sprite, filepath);
                    delete sprite;
                },
                .Filters = {{"Spritorium Files", "spto"}}
            };

            dl::FileSave(ctx);
        }
        else
        {
            if (io::WriteDocumentOnDisk(sprite->Filepath, sprite))
            {
                sprite->IsUnsavedDocument = false;
            }
        }
    }

    if (const editor::ViewContext* active_view = g.GetCurrentView())
    {
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            active_view->Sprite->History.Undo();
        }
        
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y))
        {
            active_view->Sprite->History.Redo();
        }

        static SpriteContext ctx {};
        ctx.Sprite = active_view->Sprite;
        ctx.Mouse = Vec2(active_view->MouseCanvas.x, active_view->MouseCanvas.y);
        ctx.IsMouseMove = (io.MouseDelta.x != 0 || io.MouseDelta.y);
        ctx.Hover = active_view->HoverCanvas;
        ctx.CurrentBrush = g.Tools->CurrentBrush.get();

        static KeyState key {};
        key.Shift = io.KeyShift;
        key.Ctrl = io.KeyCtrl;

        auto* tool = g.Tools->GetActiveTool();
        assert(tool && "GetActiveTool returning invalid pointer data!");

        ImGuiContext* imctx = ImGui::GetCurrentContext();

        if (imctx->HoveredWindow == active_view->MyImGuiWindow)
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                tool->MousePressed(ctx, key, g.Tools->Color);
                g.CurrentDrawingView = active_view->MyImGuiWindow;
            }
        }
        
        if (g.CurrentDrawingView == active_view->MyImGuiWindow)
        {
            if (imctx->ActiveIdWindow == active_view->MyImGuiWindow)
            {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    tool->MouseDown(ctx, key, g.Tools->Color);
                }
            }
            
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                tool->MouseReleased(ctx, key, g.Tools->Color);
                g.CurrentDrawingView = nullptr;
                active_view->Sprite->IsUnsavedDocument = true;
            }
        }

    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    gIm->RenderFrame();

    g.ProcessTasks();

    SDL_GL_SwapWindow(g.Window);

    End();
}

Application::~Application()
{
    g.Views.clear();
    g.Sprites.clear();
    g.Shaders.clear();
    g.Tools.reset();

    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    if (g.GL)
    {
        SDL_GL_DestroyContext(g.GL);
    }

    if (g.Window)
    {
        SDL_DestroyWindow(g.Window);
    }
    
    SDL_Quit();
}