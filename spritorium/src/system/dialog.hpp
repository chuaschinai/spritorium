#pragma once

#include <string>
#include <functional>

#include <SDL3/SDL_dialog.h>

#include "app/app_state.hpp"

namespace dl
{

#define DIALOG_SET_DEFAULT_FIELDS(__TYPE__, __CALLBACK_CONTEXT__)   \
    __TYPE__* Userdata { nullptr };                                 \
    __CALLBACK_CONTEXT__ Success;                                   \
    std::string Location;

    using CallbackOpen = std::function<void(const std::vector<std::string>& filenames, void* userdata)>;
    template <typename Data = void>
    struct DialogFileOpenContext
    {
        DIALOG_SET_DEFAULT_FIELDS(Data, CallbackOpen);
        std::vector<SDL_DialogFileFilter> Filters;
        bool AllowMany { false };
    };

    using CallbackSDLDialog = std::function<void(const std::string& filepath, const std::string& filename, void* userdata)>;
    template <typename Data = void>
    struct DialogFolderSelectContext
    {
        DIALOG_SET_DEFAULT_FIELDS(Data, CallbackSDLDialog);
    };

    template <typename Data = void>
    struct DialogFileSaveContext
    {
        DIALOG_SET_DEFAULT_FIELDS(Data, CallbackSDLDialog);
        std::vector<SDL_DialogFileFilter> Filters;
    };

    template <typename Data = void>
    void FileOpen(const DialogFileOpenContext<Data>& ctx)
    {
        std::unique_ptr<DialogFileOpenContext<Data>> ptr_ctx = std::make_unique<DialogFileOpenContext<Data>>(ctx);

        const auto func = [](void* userdata, const char* const* filelist, int filter) -> void
        {
            if (!filelist || !filelist[0])
            {
                return;
            }
            
            const DialogFileOpenContext<>* const ctx = static_cast<DialogFileOpenContext<>*>(userdata);

            std::vector<std::string> filenames;

            while (*filelist)
            {
                filenames.push_back(*filelist++);
            }
            
            ctx->Success(filenames, ctx->Userdata);

            delete ctx;
        };

        SDL_ShowOpenFileDialog(func, ptr_ctx.release(), g.Window, ctx.Filters.data(), ctx.Filters.size(), ctx.Location.c_str(), true);
    }

    template <typename Data = void>
    void FileSave(const DialogFileSaveContext<Data>& ctx)
    {
        std::unique_ptr<DialogFileSaveContext<Data>> ptr_ctx = std::make_unique<DialogFileSaveContext<Data>>(ctx);

        const auto func = [](void* userdata, const char* const* filelist, int filter) -> void
        {
            if (!filelist || !filelist[0])
            {
                return;
            }

            const DialogFileSaveContext<>* const ctx = static_cast<DialogFileSaveContext<>*>(userdata);
            
            std::string filepath(filelist[0]);

            const std::string pattern = ctx->Filters[filter].pattern;
            const std::string extension = "." + pattern.substr(pattern.find_first_of(";") + 1);
            if (filepath.rfind(extension) == std::string::npos)
            {
                filepath += extension;
            }

            std::string filename = filepath.substr(filepath.find_last_of("\\") + 1);
            if (filename == filepath)
            {
                filename = "";
            }
            
            ctx->Success(filepath, filename, ctx->Userdata);

            delete ctx;
        };

        SDL_ShowSaveFileDialog(func, ptr_ctx.release(), g.Window, ctx.Filters.data(), ctx.Filters.size(), nullptr);
    }

    template <typename Data = void>
    void FolderOpen(const DialogFolderSelectContext<Data>& ctx)
    {
        std::unique_ptr<DialogFolderSelectContext<Data>> ptr_ctx = std::make_unique<DialogFolderSelectContext<Data>>(ctx);

        const auto func = [](void* userdata, const char* const* filelist, int filter) -> void
        {
            if (!filelist || !filelist[0])
            {
                return;
            }

            const DialogFolderSelectContext<>* const ctx = static_cast<DialogFolderSelectContext<>*>(userdata);

            const std::string filepath(filelist[0]);            
            ctx->Success(filepath, "", ctx->Userdata);

            delete ctx;
        };

        SDL_ShowOpenFolderDialog(func, ptr_ctx.release(), g.Window, ctx.Location.c_str(), false);
    }
    
}