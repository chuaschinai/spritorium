#pragma once

#include <memory>
#include <queue>

#include "core/io.hpp"
#include "core/bitmap.hpp"

namespace editor { struct Sprite; }

namespace spto
{

    struct ITask
    {
        virtual ~ITask() = default;
        virtual void Execute() = 0;
    };

    struct TaskManager
    {
        void Push(std::unique_ptr<ITask> cmd) { Tasks.push(std::move(cmd)); }
        void Process()
        {
            while (!Tasks.empty())
            {
                Tasks.front()->Execute();
                Tasks.pop();
            }
        }

    private:
        std::queue<std::unique_ptr<ITask>> Tasks;
    };

    /*
    * Defined tasks
    */

    struct Task_DestroyView final : public ITask
    {
        int Id;

        Task_DestroyView(int id) : Id(id) {}
        
        void Execute() override;
    };

    struct Task_LoadDocument final : public ITask
    {
        io::DocumentRecord Record;
        std::string Filepath;

        Task_LoadDocument(io::DocumentRecord record, const std::string& filepath) : Record(record), Filepath(filepath) {}

        void Execute() override;
    };

    struct Task_SaveDocument final : public ITask
    {
        editor::RefSprite Document;
        std::string Filepath;

        Task_SaveDocument(editor::RefSprite document, const std::string& filepath) : Document(document), Filepath(filepath) {}

        void Execute() override;
    };

    struct Task_DestroyDocument final : public ITask
    {
        editor::RefSprite Document;

        Task_DestroyDocument(editor::RefSprite document) : Document(document) {}

        void Execute() override;
    };

    struct Task_LoadImageFromDisk final : public ITask
    {
        Bitmap Image;
        std::string Filename;

        Task_LoadImageFromDisk(const std::string& filepath, const std::string& filename) : Image(filepath), Filename(filename) {}

        void Execute() override;
    };
}