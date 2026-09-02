#pragma once

#include <memory>

#include <imgui.h>

struct SDL_Window;

struct ImState
{
    ImGuiID DockCentralId;
    bool ModalOpen_CreateNewCanvas;
    bool ModalOpen_SaveBeforeClose;
    bool ModalOpen_ExportImage;

    ImState(SDL_Window* window, void* gl);

    void RenderNewFrame();
    void RenderFrame();
    void RenderMainMenu();
    void RenderSubMenu();
    void RenderFooter();
    void RenderToolMenu();
    void RenderLayers();
    void RenderColorPicker();
    void RenderDebug();
    void RenderModalCreateNewCanvas();
    void RenderModalSaveBeforeClose();
    void RenderModalExportImage();

private:
    SDL_Window* Window;
    void* GL;
};

inline std::unique_ptr<ImState> gIm;