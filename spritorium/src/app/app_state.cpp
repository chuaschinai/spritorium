#include "app_state.hpp"

#include <format>

#include <SDL3/SDL_dialog.h>

#include "editor/view.hpp"
#include "app/logger.hpp"

namespace app
{

    gl::Shader& AppState::FindShader(const std::string& name)
    {
        const auto it = g.Shaders.find(name);
        if (it != g.Shaders.end())
        {
            return *it->second.get();
        }
        
        spto::Warn("[Shader] {} not found!", name);

        return *g.Shaders.find("blit")->second;
    };

    void AppState::LoadShaderFromData(const std::string& shader_name, std::string_view fragment, std::string_view vertex)
    {
        g.Shaders.emplace(
            shader_name,
            std::make_unique<gl::Shader>(vertex, fragment)
        );
    }

    editor::ViewContext* AppState::GetViewById(int id)
    {
        const auto it = std::ranges::find(Views, id, &editor::ViewContext::Id);
        return (it != Views.end() ? it->get() : nullptr);
    }

    editor::ViewContext* AppState::GetCurrentView()
    {
        if (g.Views.empty() || g.SelectedView == -1)
        {
            return nullptr;
        }

        return g.Views[g.SelectedView].get();
    }

    editor::RefSprite AppState::GetCurrentSprite()
    {
        if (g.Sprites.empty() || g.SelectedView == -1)
        {
            return nullptr;
        }

        return g.Sprites[g.SelectedView];
    }

    void AppState::SelectView(int id)
    {
        const auto it_id = std::ranges::find(g.Views, id, &editor::ViewContext::Id);
        const auto it_pos = std::distance(g.Views.begin(), it_id);
        g.SelectedView = it_pos;
    }

    static std::string CreateNumberedName(const std::string& name, const std::vector<editor::RefSprite>& sprites) {
        std::string new_name = name;

        int count = 0;
        for (const auto& sprite : sprites)
        {
            if (sprite->Name == new_name)
                new_name = std::format("{} ({})", name, ++count);
        }

        return new_name;
    }

    editor::RefSprite AppState::CreateSprite(const std::string& name, int width, int height) {
        const std::string new_name = CreateNumberedName(name, g.Sprites);
        
        auto sprite = std::make_shared<editor::Sprite>(new_name, width, height);
        g.Sprites.push_back(sprite);
        g.Views.emplace_back(std::make_unique<editor::ViewContext>(sprite));

        g.SelectedView = g.Views.size() - 1;

        return sprite;
    }

    /// TODO: implement all bitmaps
    editor::RefSprite AppState::CreateSpriteFromBitmaps(const std::string& name, const std::vector<Bitmap>& bitmaps) {
        const Bitmap* b = &bitmaps[0];

        const auto sprite = CreateSprite(name, b->Size.x, b->Size.y);
        const auto layer = sprite->CreateLayerFromBitmap(name, *b);

        return sprite;
    }

    editor::RefSprite AppState::CreateSpriteFromDocumentRecord(const io::DocumentRecord& r, const std::string& filepath)
    {
        const auto dc = CreateSprite(r.Name, r.Size.x, r.Size.y);
        dc->Filepath = filepath;

        for (const auto& l : r.Layers)
        {
            dc->CreateLayerFromBitmap(l.Name, Bitmap(r.Size, l.Pixels), l.IsHidden);
        }

        return dc;
    }

};
