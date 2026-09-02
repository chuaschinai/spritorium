#pragma once

#include <vector>

#include <spdlog/spdlog.h>

#include "core/bitmap.hpp"

#define MAX_MB_HISTORY_SIZE 30

namespace editor
{

struct Sprite;
struct LayerContext;

enum class HistoryCmdType { Undo, Redo };

struct HistoryCommand
{
    virtual ~HistoryCommand() = default;
    virtual void Execute(HistoryCmdType type) = 0;
    virtual float GetDataSize() { return sizeof(*this); }
};

struct Command_SpriteDraw : public HistoryCommand
{
    Command_SpriteDraw(Sprite* document, int layer_id, const Vec2& pos, const Bitmap& prev_image, const Bitmap& next_image)
    : Document(document)
    , LayerId(layer_id)
    , Pos(pos)
    , PrevImage(prev_image)
    , NextImage(next_image)
    {}

    void Execute(HistoryCmdType type) override;

    float GetDataSize() override { return static_cast<float>(PrevImage.GetTotalBytes()) * 2.0f; }

    Sprite* Document;
    Bitmap PrevImage;
    Bitmap NextImage;
    Vec2 Pos;
    int LayerId;
};

struct Command_LayerSelection : public HistoryCommand
{
    Command_LayerSelection(Sprite* document, int prev_index, int next_index)
    : Document(document)
    , PrevIndex(prev_index)
    , NextIndex(next_index)
    {}
    
    void Execute(HistoryCmdType type) override;

    Sprite* Document;
    int PrevIndex;
    int NextIndex;
};

struct Command_LayerStateList : public HistoryCommand
{
    Command_LayerStateList(Sprite* document, std::vector<RefLayer>& prev_list, std::vector<RefLayer>& next_list)
    : Document(document)
    , PrevList(prev_list)
    , NextList(next_list)
    {}

    void Execute(HistoryCmdType type) override;

    Sprite* Document;
    std::vector<RefLayer> PrevList;
    std::vector<RefLayer> NextList;
};

struct HistorySystem
{
    void Undo();
    void Redo();
    void Commit(std::unique_ptr<HistoryCommand> ctx);

    float TotalBytes { 0 };

private:
    std::vector<std::unique_ptr<HistoryCommand>> UndoStack;
    std::vector<std::unique_ptr<HistoryCommand>> RedoStack;
};

}