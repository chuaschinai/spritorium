#pragma once

#include <string>
#include <memory>
#include <vector>

#include "core/gfx.hpp"
#include "core/types.hpp"
#include "history.hpp"

namespace editor
{

    struct ViewContext;
    struct Sprite;

    using RefSprite = std::shared_ptr<Sprite>;

    enum class RenderType { Canvas, Scratch, Aux0, Aux1 };

    constexpr int MAX_RENDERERS { 5 };

    struct Sprite
    {
        Vec2 Size;
        HistorySystem History;
        std::vector<RefLayer> Layers;
        std::string Name;
        std::string Filepath;
        int ActiveLayerIndex;
        bool IsUnsavedDocument;

        Sprite(const std::string name, int width, int height);
        ~Sprite();

        RefLayer CreateLayer(int pos = 0);
        RefLayer CreateLayerFromBitmap(const std::string& name, const Bitmap& image, bool is_hidden = false);
        void DeleteLayer(int index);
        void SelectLayer(int index);
        RefLayer GetLayer(int index) const { return (index >= 0 && index < Layers.size()) ? Layers[index] : nullptr; }
        RefLayer GetActiveLayer() const { return GetLayer(ActiveLayerIndex); }
        RefLayer GetLayerById(int id) const;

        Bitmap CreateBitmapFromLayers(bool color_black_bg = false);
        // Bitmap* FlattenLayers();
        RefLayer GetScratchLayer() const { return ScratchLayer; }
        gfx::FrameBuffer& GetRenderer(RenderType type) const { return *Renderers[static_cast<int>(type)].get(); }
        void RenderProcess();

    private:
        std::unique_ptr<gfx::FrameBuffer> Renderers[MAX_RENDERERS];
        RefLayer ScratchLayer;
    };

}