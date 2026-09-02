#include "im_state.hpp"

#include <filesystem>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_events.h>

#include <stb_image_write.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <IconsFontAwesome6.h>

#include "app/logger.hpp"
#include "app_state.hpp"
#include "core/io.hpp"
#include "core/utils.hpp"
#include "imgui_custom_widgets.hpp"
#include "imgui_styles.hpp"
#include "system/dialog.hpp"
#include "editor/sprite.hpp"
#include "editor/view.hpp"
#include "tools/tool_manager.hpp"

static ImGuiErrorRecoveryState imgui_error_state;

static bool OpenWindowDebug { false };

static void NoWindowMenuButton()
{
    ImGuiWindowClass window_class;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::SetNextWindowClass(&window_class);
}

ImState::ImState(SDL_Window* window, void* gl_context)
    : Window(window)
    , GL(gl_context)
    , ModalOpen_SaveBeforeClose(false)
    , ModalOpen_ExportImage(false)
{
    // imgui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigDockingWithShift = true;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontFromFileTTF(util::FileFromResourcesFolder("Roboto-Medium.ttf").c_str(), 16.0f, nullptr);
    ImFontConfig config;
    config.SizePixels = 14.0f;
    config.MergeMode = true;
    io.Fonts->AddFontFromFileTTF(util::FileFromResourcesFolder("fa-solid-900.ttf").c_str(), 0.0f, &config);

    // ImGui::StyleColorsDark();
    StyleGruvbox();

    ImGuiStyle& style = ImGui::GetStyle();
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    style.DockingNodeHasCloseButton = false;

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    spto::Info("[ImGui] initialized with success");
}

void ImState::RenderNewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

#ifdef DEBUG
    ImGui::ErrorRecoveryStoreState(&imgui_error_state);
#endif
    
    static bool once = false;

    DockCentralId = ImGui::GetID("Dockspace");
    
    ImGui::DockSpaceOverViewport(DockCentralId, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    if (!once) {
        once = true;

        ImGui::DockBuilderRemoveNodeChildNodes(DockCentralId);
        ImGui::DockBuilderAddNode(DockCentralId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(DockCentralId, ImGui::GetMainViewport()->Size);

        ImGuiID dock_top, dock_middle_left, dock_middle_right, dock_right_up;

        // tools
        ImGui::DockBuilderSplitNode(DockCentralId, ImGuiDir_Left, 0.0f, &dock_middle_left, &DockCentralId);
        ImGui::DockBuilderDockWindow("##Tools", dock_middle_left);
        ImGui::DockBuilderSetNodeSize(dock_middle_left, ImVec2(26.0f + ImGui::GetStyle().WindowPadding.x * 2, 1));
        if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_middle_left))
        {
            node->SetLocalFlags(
                node->LocalFlags
                | ImGuiDockNodeFlags_NoDockingSplit
                | ImGuiDockNodeFlags_NoDockingOverMe
                | ImGuiDockNodeFlags_NoResize
                | ImGuiDockNodeFlags_NoTabBar
            );
        }

        // layers
        ImGui::DockBuilderSplitNode(DockCentralId, ImGuiDir_Right, 0.25f, &dock_middle_right, &DockCentralId);
        ImGui::DockBuilderDockWindow("Layers", dock_middle_right);

        // color
        ImGui::DockBuilderSplitNode(dock_middle_right, ImGuiDir_Up, 0.4f, &dock_right_up, &dock_middle_right);
        ImGui::DockBuilderDockWindow("Color", dock_right_up);

        if (ImGuiDockNode* center_node = ImGui::DockBuilderGetNode(DockCentralId)) {
            center_node->SetLocalFlags(center_node->LocalFlags | ImGuiDockNodeFlags_CentralNode | ImGuiDockNodeFlags_NoWindowMenuButton);
        }

        ImGui::DockBuilderFinish(DockCentralId);
    }
}

void ImState::RenderFrame() {
#ifdef DEBUG
    ImGui::ErrorRecoveryTryToRecoverState(&imgui_error_state);
#endif

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static int MDC_Euclidean(int a, int b) { return (b == 0 ? a : MDC_Euclidean(b, a % b)); }

static constexpr float menu_height = 32.0f;
void ImState::RenderMainMenu()
{
    ImGuiStyle& style = ImGui::GetStyle();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Canvas"))
            {
                ModalOpen_CreateNewCanvas = true;
            }

            if (ImGui::MenuItem("Open"))
            {
                dl::DialogFileOpenContext ctx
                {
                    .Success = [](const std::vector<std::string>& filepaths, void* userdata)
                    {
                        for (const auto& path : filepaths)
                        {
                            const auto dc = io::ReadDocumentFromDisk(path);
                            if (std::holds_alternative<io::DocumentRecord>(dc))
                            {
                                g.CreateTask<spto::Task_LoadDocument>(
                                    std::get<io::DocumentRecord>(dc),
                                    path
                                );
                            }
                        }
                    },    
                    .Location = std::filesystem::temp_directory_path().string(),
                    .Filters = {{"Spritorium Files", "spto"}},
                    .AllowMany = true,
                };

                dl::FileOpen(ctx);
            }
            
            if (ImGui::MenuItem("Import"))
            {
                dl::DialogFileOpenContext ctx
                {
                    .Success = [](const std::vector<std::string>& filepaths, void* userdata)
                    {
                        for (const auto& path : filepaths)
                        {
                            std::string filename_width_ext = path.substr(path.find_last_of("\\") + 1);
                            std::string filename = filename_width_ext;
                            if (filename_width_ext != path)
                            {
                                filename = filename_width_ext.substr(0, filename_width_ext.find_last_of("."));
                            }
                            g.CreateTask<spto::Task_LoadImageFromDisk>(path, filename);
                        }
                    },    
                    .Location = std::filesystem::temp_directory_path().string(),
                    .Filters = {
                        { "PNG images", "png" },
                        { "JPEG images", "jpg;jpeg" },
                        { "Bitmap images", "bmp" },
                    },
                    .AllowMany = true,
                };

                dl::FileOpen(ctx);
            }

            if (ImGui::MenuItem("Export", nullptr, false, !g.Views.empty()))
            {
                ModalOpen_ExportImage = true;
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
                SDL_Event e {};
                e.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&e);
            }
            
#if DEBUG

            ImGui::Separator();
            if (ImGui::MenuItem("Demo Window", NULL, g.DemoWindow))
            {
                g.DemoWindow = !g.DemoWindow;
            }

            if (ImGui::MenuItem("Debug", nullptr, OpenWindowDebug))
            {
                OpenWindowDebug = !OpenWindowDebug;
            }

#endif

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    
}

void ImState::RenderSubMenu()
{
    ImGuiStyle& style = ImGui::GetStyle();

    const float height_add = 12.0f;
    const float height = ImGui::GetFrameHeight() + style.DockingSeparatorSize * 2.0f + height_add;
    const int flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.DockingSeparatorSize, style.DockingSeparatorSize));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetColorU32(ImGuiCol_Border));
    const bool is_open = ImGui::BeginViewportSideBar("##SubMenu", nullptr, ImGuiDir_Up, height, flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
   
    if (is_open)
    {
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        
        // fake bg
        const ImVec2 cursor_screen = ImGui::GetCursorScreenPos();
        const ImVec2 p_min = cursor_screen;
        const ImVec2 p_max = p_min + ImVec2(ImGui::GetContentRegionAvail().x, height - style.DockingSeparatorSize * 2.0f);
        drawlist->AddRectFilled(p_min, p_max, ImGui::GetColorU32(ImGuiCol_WindowBg), style.WindowRounding);

        const ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPosX(cursor.x + style.WindowPadding.x);
        ImGui::SetCursorPosY(cursor.y + height_add / 2.0f);

        ImGui::Text("%s", GetNameFromToolType(g.Tools->ActiveTool));
        
        ImGui::SameLine();
        if (g.Tools->ActiveTool != ToolType::Pipette && g.Tools->ActiveTool != ToolType::Bucket)
        {
            
            if (ImGui::Button(g.Tools->ActiveBrushShape == BrushShape::Square ? ICON_FA_SQUARE : ICON_FA_CIRCLE, ImVec2(22.0f, 22.0f)))
            {
                ImGui::OpenPopup("##SelectBrushShape");
            }

            if (ImGui::BeginPopup("##SelectBrushShape", ImGuiWindowFlags_NoMove))
            {
                if (ImGui::Button(ICON_FA_SQUARE))
                {
                    g.Tools->SetBrushShape(BrushShape::Square);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CIRCLE))
                {
                    g.Tools->SetBrushShape(BrushShape::Circle);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(128.0f);
            if (ImGui::SliderInt("Size", &g.Tools->BrushSize, 1, 32, "%dpx"))
            {
                g.Tools->BrushUpdate();
            }
        }
    }

    ImGui::End();
}

void ImState::RenderFooter()
{
    ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();

    float height = ImGui::GetFrameHeight();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    if (!ImGui::BeginViewportSideBar("Footer", viewport, ImGuiDir_Down, height, window_flags))
    {
        ImGui::End();
        return;
    }

    ImGui::BeginMenuBar();

    const ImGuiStyle& style = ImGui::GetStyle();

    if (const auto* view = g.GetCurrentView())
    {
        ImGui::Text(ICON_FA_ARROW_POINTER " (%.0f, %.0f)", view->MouseCanvas.x, view->MouseCanvas.y);
        ImGui::Text(ICON_FA_MAGNIFYING_GLASS " %.2fx", view->GetScale());

        char str_ver[16];
        ImFormatString(str_ver, 16, "v%s", AppVersion);

        const float width = ImGui::GetWindowWidth();
        ImGui::SetCursorPosX(width - ImGui::CalcTextSize(str_ver).x - style.WindowPadding.x);
        ImGui::TextDisabled("%s", str_ver);
    }

    ImGui::EndMenuBar();

    ImGui::End();
}

void ImState::RenderToolMenu() {
    auto& tools = g.Tools;

    const ImGuiStyle& style = ImGui::GetStyle();

    ImGui::Begin("##Tools", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    static const ImVec2 size(24.0f, 24.0f);

    if (ImGui::ButtonIconSelected(ICON_FA_PENCIL, tools->ActiveTool == ToolType::Pencil, "Pencil"))
    {
        tools->SetActive(ToolType::Pencil);
    }

    if (ImGui::ButtonIconSelected(ICON_FA_ERASER, tools->ActiveTool == ToolType::Eraser, "Eraser"))
    {
        tools->SetActive(ToolType::Eraser);
    }

    if (ImGui::ButtonIconSelected(ICON_FA_SQUARE_FULL, tools->ActiveTool == ToolType::Rectangle, "Rectangle"))
    {
        tools->SetActive(ToolType::Rectangle);
    }

    if (ImGui::ButtonIconSelected(ICON_FA_CIRCLE, tools->ActiveTool == ToolType::Ellipse, "Ellipse"))
    {
        tools->SetActive(ToolType::Ellipse);
    }

    if (ImGui::ButtonIconSelected(ICON_FA_EYE_DROPPER, tools->ActiveTool == ToolType::Pipette, "Pipette"))
    {
        tools->SetActive(ToolType::Pipette);
    }

    if (ImGui::ButtonIconSelected(ICON_FA_BUCKET, tools->ActiveTool == ToolType::Bucket, "Bucket"))
    {
        tools->SetActive(ToolType::Bucket);
    }

    ImGui::End();
}

void ImState::RenderLayers()
{
    const auto* view = g.GetCurrentView();

    NoWindowMenuButton();

    ImGui::Begin("Layers");

    if (!view)
    {
        ImGui::Text("No view selected...");
        ImGui::End();
        return;
    }
    
    const auto sprite = view->Sprite;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    static const float title_height = ImGui::GetCursorPosY();
    static const float bottom_menu_height { ImGui::GetFrameHeightWithSpacing() };
    static const float thumb_size { 48.0f };
    const float avail_y = ImGui::GetContentRegionAvail().y;

    static char temp_name[64];

    ImGui::BeginChild("##container", ImVec2(0.0f, avail_y - bottom_menu_height - ImGui::GetCursorPosY() + title_height), ImGuiChildFlags_FrameStyle);

    for (int i = sprite->Layers.size(); i-- > 0;)
    {        
        const auto& layer = sprite->Layers.at(i);

        const ImVec2 prev_cursor = ImGui::GetCursorPos();
        const bool selected = (sprite->ActiveLayerIndex == i);

        ImGui::PushID(i);

        if (ImGui::Selectable("##selectable", selected, ImGuiSelectableFlags_AllowOverlap, ImVec2(0, thumb_size)))
        {
            sprite->SelectLayer(i);
        }

        // Open popup Layer Settings
        if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !ImGui::IsPopupOpen("Layer Settings"))
        {
            ImGui::OpenPopup("Layer Settings");
            ImFormatString(temp_name, 64, "%s", layer->Name.c_str());
        }

        // Layer Settings popup
        if (ImGui::BeginPopup("Layer Settings"))
        {
            ImGui::SetNextItemWidth(192.0f);
            if (ImGui::InputText("Name", temp_name, 64))
            {
                layer->Name = temp_name;
            }
            ImGui::Checkbox("Hidden", &layer->IsHidden);

            ImGui::Separator();
            if (ImGui::Button("Ok", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("DND", &i, sizeof(int));

            ImGui::Text("%s", layer->Name.c_str());

            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND"))
            {
                int payload_n = *(const int*)payload->Data;
                auto layer_payload = sprite->Layers.at(payload_n);

                auto prev_list = sprite->Layers;

                sprite->Layers.erase(sprite->Layers.begin() + payload_n);
                sprite->Layers.insert(sprite->Layers.begin() + i, layer_payload);
                sprite->IsUnsavedDocument = true;
                sprite->SelectLayer(i);

                sprite->History.Commit(std::make_unique<editor::Command_LayerStateList>(sprite.get(), prev_list, sprite->Layers));
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(prev_cursor.x + style.FramePadding.x);
        ImGui::SetCursorPosY(prev_cursor.y + thumb_size / 2.0f - 13.0f);
        
        ImGui::PushID(i);
        if (ImGui::ButtonIconSelected(layer->IsHidden ? ICON_FA_EYE_SLASH : ICON_FA_EYE, layer->IsHidden))
        {
            layer->IsHidden = !layer->IsHidden;
            sprite->IsUnsavedDocument = true;
        }
        ImGui::PopID(); 

        ImGui::SameLine();
        const ImVec2 p_min = ImGui::GetCursorScreenPos();

        ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(2.0f, 2.0f));
        ImGui::Image(layer->GetTexture(), ImVec2(thumb_size - 4.0f, thumb_size - 4.0f));

        ImGui::GetWindowDrawList()->AddRect(p_min, p_min + ImVec2(thumb_size, thumb_size), ImColor(255, 255, 255, 40));
        
        ImGui::SameLine();
        ImGui::Text("%s", layer->Name.c_str());
    }
    
    ImGui::EndChild();

    if (ImGui::Button("Create"))
    {
        sprite->CreateLayer(sprite->ActiveLayerIndex);
        sprite->IsUnsavedDocument = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
    {
        sprite->DeleteLayer(sprite->ActiveLayerIndex);
        sprite->IsUnsavedDocument = true;
    }

    ImGui::End();
}

void ImState::RenderColorPicker()
{ 
    NoWindowMenuButton();

    if (ImGui::Begin("Color"))
    {
        float col[4];
        g.Tools->Color.ToFloat4(col);
        if (ImGui::ColorPicker4("Color picker", col))
        {
            g.Tools->SetBrushColor(col);
        }
    }
    ImGui::End();
}

void ImState::RenderDebug()
{
    if (!OpenWindowDebug) { return; }

    if (!ImGui::Begin("Debug", &OpenWindowDebug))
    {
        ImGui::End();
        return;
    }

    ImGui::Checkbox("Uncapped Framerate", &g.UncappedFramerate);
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

    ImGui::SeparatorText("Application");
    ImGui::BulletText("PendingViewIdsToClose: %llu", g.AppCloseRequest.PendingViewIdsToClose.size());
    ImGui::BulletText("Max History size: (%d)MB", MAX_MB_HISTORY_SIZE);

    ImGui::SeparatorText("Sprite Documents");
    ImGui::BulletText("Count: %llu", g.Sprites.size());

    const auto dc = g.GetCurrentSprite();
    if (dc)
    {
        ImGui::BulletText("Active: %s", dc->Name.c_str());
    }
    else {
        ImGui::BulletText("Active: (No view selected...)");
    }

    for (const auto& sprite : g.Sprites)
    {
        if (ImGui::TreeNode(sprite->Name.c_str()))
        {
            ImGui::BulletText("Filepath: %s", sprite->Filepath.c_str());
            ImGui::BulletText("Size: %d, %d", sprite->Size.x, sprite->Size.y);
            ImGui::BulletText("Count layers: %llu", sprite->Layers.size());
            ImGui::BulletText("History size: (%.2f)kB, (%.2f)MB", sprite->History.TotalBytes / 1000, sprite->History.TotalBytes / 1000000);

            uint32_t tex_canvas_id = sprite->GetRenderer(editor::RenderType::Canvas).TextureID;
            ImGui::Text("Canvas %d", tex_canvas_id);
            ImGui::SameLine();
            ImGui::ImageWithBg(tex_canvas_id, ImVec2(64, 64), {}, {1,1}, {0.3, 0.3, 0.3, 0.3});

            uint32_t tex_layer_scratch_id = sprite->GetScratchLayer()->Texture.ID;
            ImGui::Text("ScratchLayer %d", tex_layer_scratch_id);
            ImGui::SameLine();
            ImGui::ImageWithBg(tex_layer_scratch_id, ImVec2(64, 64), {}, {1,1}, {0.3, 0.3, 0.3, 0.3});

            if (ImGui::TreeNode("Others Renderers"))
            {
                ImGui::Text("Scratch");
                ImGui::SameLine();
                ImGui::ImageWithBg(sprite->GetRenderer(editor::RenderType::Scratch).TextureID, ImVec2(64, 64), {}, {1,1}, {0.3, 0.3, 0.3, 0.3});
                ImGui::Text("Aux0");
                ImGui::SameLine();
                ImGui::ImageWithBg(sprite->GetRenderer(editor::RenderType::Aux0).TextureID, ImVec2(64, 64), {}, {1,1}, {0.3, 0.3, 0.3, 0.3});
                ImGui::Text("Aux1");
                ImGui::SameLine();
                ImGui::ImageWithBg(sprite->GetRenderer(editor::RenderType::Aux1).TextureID, ImVec2(64, 64), {}, {1,1}, {0.3, 0.3, 0.3, 0.3});

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    ImGui::End();
}

void ImState::RenderModalCreateNewCanvas()
{
    if (ModalOpen_CreateNewCanvas)
    {
        ImGui::OpenPopup("Create New Canvas");
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Create New Canvas", &ModalOpen_CreateNewCanvas, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        static char name[256] = "Untitled";
        ImGui::InputText("Name", name, 256);
        
        static int size[2] { 64, 64 };
        int n = MDC_Euclidean(size[0], size[1]);
        ImGui::Text("Aspect ratio (%d:%d)", size[0] / n, size[1] / n);

        if (ImGui::InputInt2("Size", size))
        {
            size[0] = ImClamp(size[0], 1, 2048);
            size[1] = ImClamp(size[1], 1, 2048);
        }

        ImGui::Separator();

        if (ImGui::Button("Create"))
        {
            g.CreateSprite(name, size[0], size[1])->CreateLayer();
            ModalOpen_CreateNewCanvas = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ModalOpen_CreateNewCanvas = false;
        }

        ImGui::EndPopup();
    }
}

void ImState::RenderModalSaveBeforeClose()
{
    if (g.Views.empty() || g.AppCloseRequest.IsEmpty())
    {
        ModalOpen_SaveBeforeClose = false;
    }

    if (ModalOpen_SaveBeforeClose && !ImGui::IsPopupOpen("Save before close?"))
    {
        ImGui::OpenPopup("Save before close?");
    }

    const ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

    int flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    if (!ImGui::BeginPopupModal("Save before close?", &ModalOpen_SaveBeforeClose, flags))
    {
        return;
    }

    const ImVec2 pos = ImVec2(viewport_size - ImGui::GetWindowSize()) / 2.0f;
    ImGui::SetWindowPos(pos);

    auto* const view = g.GetViewById(g.AppCloseRequest.GetTopId());
    const auto& dc = view->Sprite;

    ImGui::Text("Do you want to save before closing?");
    ImGui::BulletText("%s", dc->Name.c_str());
    ImGui::Separator();

    if (!dc->Filepath.empty())
    {
        if (ImGui::Button("Save", ImVec2(128.0f, 0.0f)))
        {
            g.CreateTask<spto::Task_SaveDocument>(view->Sprite, dc->Filepath);
        }
    }
    else
    {
        if (ImGui::Button("Save As", ImVec2(128.0f, 0.0f)))
        {
            dl::DialogFileSaveContext ctx
            {
                .Userdata = view,
                .Success = [](const std::string& filepath, const std::string&, void* userdata)
                {
                    editor::ViewContext* view = static_cast<editor::ViewContext*>(userdata);
                    g.CreateTask<spto::Task_SaveDocument>(view->Sprite, filepath);
                },
                .Filters = {{"Spritorium Files", "spto"}}
            };

            dl::FileSave(ctx);
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Discard"))
    {
        view->RequestClose = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        g.AppCloseRequest.Cancel();
        ImGui::CloseCurrentPopup();
    }

    if (view->RequestClose)
    {
        g.AppCloseRequest.PendingViewIdsToClose.pop_back();
        g.CreateTask<spto::Task_DestroyView>(view->Id);
        g.CreateTask<spto::Task_DestroyDocument>(view->Sprite);
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void ImState::RenderModalExportImage()
{
    auto* sprite = g.GetCurrentSprite().get();

    if (!sprite)
    {
        return;
    }

    static char folder[256];
    static char filename[64];
    static bool invalid_folder = false;

    if (ModalOpen_ExportImage && !ImGui::IsPopupOpen("Export Image"))
    {
        ImGui::OpenPopup("Export Image");
        std::snprintf(folder, 256, "%s", std::filesystem::temp_directory_path().string().c_str());
        std::snprintf(filename, 64, "%s", sprite->Name.c_str());
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (!ImGui::BeginPopupModal("Export Image", &ModalOpen_ExportImage, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        return;
    }

    const ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    ImGui::GetWindowDrawList()->AddCallback(platform_io.DrawCallback_SetSamplerNearest);
    ImGui::Image(sprite->GetRenderer(editor::RenderType::Canvas).TextureID, ImVec2(128, 128));
    ImGui::GetWindowDrawList()->AddCallback(platform_io.DrawCallback_SetSamplerLinear);
    ImGui::Text("Size (%d, %d)", sprite->Size.x, sprite->Size.y);

    static int v = 0;
    
    ImGui::Text("Format");
    ImGui::RadioButton("PNG", &v, 0); ImGui::SameLine();
    ImGui::RadioButton("JPG", &v, 1); ImGui::SameLine();
    ImGui::RadioButton("BMP", &v, 2);

    ImGui::Text("Filename");
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("##InputFilename", filename, 64);

    ImGui::Text("Filepath");
    ImGui::SetNextItemWidth(300.0f);
    if (ImGui::InputText("##InputFilepath", folder, 512))
    {
        invalid_folder = !std::filesystem::is_directory(folder);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ELLIPSIS))
    {
        dl::DialogFolderSelectContext ctx
        {
            .Userdata = folder,
            .Success = [](const std::string& filepath, const std::string&, void* userdata)
            {
                auto strdata = static_cast<char*>(userdata);
                std::snprintf(strdata, 512, "%s", filepath.c_str());
            },
            .Location = folder
        };

        dl::FolderOpen(ctx);
    }

    if (invalid_folder)
    {
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.7f, 1.0f), "Invalid directory");
    }

    ImGui::Separator();
    if (ImGui::Button("Export", ImVec2(128.0f, 0.0f)))
    {
        char path[336];
        char filename_with_ext[80];
        switch (v)
        {
            case 0: std::snprintf(filename_with_ext, 64, "%s.%s", filename, "png"); break;
            case 1: std::snprintf(filename_with_ext, 64, "%s.%s", filename, "jpg"); break;
            case 2: std::snprintf(filename_with_ext, 64, "%s.%s", filename, "bmp"); break;
        }

        std::snprintf(path, 320, "%s\\%s", folder, filename_with_ext);
    
        if (std::filesystem::is_directory(folder))
        {
            bool enabled_black_bg = (v == 1);
            Bitmap image = sprite->CreateBitmapFromLayers(enabled_black_bg);

            switch (v)
            {
                case 0: stbi_write_png(path, image.Size.x, image.Size.y, BPP, image.GetData(), image.Size.x * BPP); break;
                case 1: stbi_write_jpg(path, image.Size.x, image.Size.y, BPP, image.GetData(), image.Size.y * BPP); break;
                case 2: stbi_write_bmp(path, image.Size.x, image.Size.y, BPP, image.GetData()); break;
            }

            spto::Info("[Export Image] Success: {}", path);
            invalid_folder = false;
        }
        else
        {
            spto::Warn("[Export Image] Invalid directory: {}", folder);
            invalid_folder = true;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        ModalOpen_ExportImage = false;
    }

    ImGui::EndPopup();
}