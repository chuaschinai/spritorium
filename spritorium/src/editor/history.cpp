#include "history.hpp"

#include "core/pixel.hpp"
#include "editor/sprite.hpp"
#include "editor/layer.hpp" // IWYU pragma: keep

namespace editor
{

    /*
        HistoryCommands
    */

    void Command_SpriteDraw::Execute(HistoryCmdType type)
    {
        auto const layer = Document->GetLayerById(LayerId);

        pixel::MakeReplace(&layer->Image, Pos, (type == HistoryCmdType::Undo ? &PrevImage : &NextImage), PixelFilterOp::Passthrough);

        layer->Dirty.AddFull();
    }

    void Command_LayerSelection::Execute(HistoryCmdType type)
    {
        Document->ActiveLayerIndex = (type == HistoryCmdType::Undo ? PrevIndex : NextIndex);
    }

    void Command_LayerStateList::Execute(HistoryCmdType type)
    {
        Document->Layers = (type == HistoryCmdType::Undo ? PrevList : NextList);
    }

    /*
        HistorySystem
    */
    
    void HistorySystem::Undo()
    {
        if (UndoStack.empty()) { return; }

        auto history = std::move(UndoStack.back());
        UndoStack.pop_back();
        
        history->Execute(HistoryCmdType::Undo);

        RedoStack.push_back(std::move(history));
    }

    void HistorySystem::Redo()
    {
        if (RedoStack.empty()) { return; }

        auto history = std::move(RedoStack.back());
        RedoStack.pop_back();
        
        history->Execute(HistoryCmdType::Redo);

        UndoStack.push_back(std::move(history));
    }

    void HistorySystem::Commit(std::unique_ptr<HistoryCommand> ctx)
    {
        const float data_size = ctx->GetDataSize();

        UndoStack.push_back(std::move(ctx));

        for (const auto& redo : RedoStack)
        {
            TotalBytes -= redo->GetDataSize();
        }

        RedoStack.clear();

        TotalBytes += data_size;
        while (TotalBytes > MAX_MB_HISTORY_SIZE * 1000000.0f)
        {
            TotalBytes -= UndoStack.front()->GetDataSize();
            UndoStack.erase(UndoStack.begin());
        }
    }

}