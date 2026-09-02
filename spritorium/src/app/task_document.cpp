#include "task.hpp"

#include "app/logger.hpp"
#include "app/app_state.hpp"

namespace spto
{

    void Task_LoadDocument::Execute()
    {
        g.CreateSpriteFromDocumentRecord(Record, Filepath);
    }

    void Task_SaveDocument::Execute()
    {
        bool success = io::WriteDocumentOnDisk(Filepath, Document);
        if (success)
        {
            Document->IsUnsavedDocument = false;
            Document->Filepath = Filepath;
        }
    }

    void Task_DestroyDocument::Execute()
    {
        const auto it = std::remove(g.Sprites.begin(), g.Sprites.end(), Document);

        if (it == g.Sprites.end())
        {
            spto::Warn("Document not found to be destroyed");
            return;
        }

        g.Sprites.erase(it);
    }

    void Task_LoadImageFromDisk::Execute()
    {
        g.CreateSpriteFromBitmaps(Filename, { Image });
    }

}