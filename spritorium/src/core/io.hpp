#pragma once

#include "core/types.hpp"
#include "editor/layer.hpp"
#include "editor/sprite.hpp"

namespace io
{

    struct FileHeader
    {
        FileHeader() = default;
        FileHeader(uint32_t major, uint32_t minor, uint32_t patch)
        : Major(major)
        , Minor(minor)
        , Patch(patch)
        {};

        char Magic[4] {'S','P','T','O'};
        uint32_t Major, Minor, Patch;
    };

    struct LayerRecord
    {
        std::string Name;
        std::vector<uint32_t> Pixels;
        bool IsHidden;
    };

    struct DocumentRecord
    {
        std::string Name;
        Vec2 Size;
        std::vector<LayerRecord> Layers;
    };

    DocumentRecord CreateDocumentRecord(const editor::RefSprite& sprite);

    bool WriteDocumentOnDisk(const std::string& filepath, const editor::RefSprite& sprite);

    std::variant<bool, DocumentRecord> ReadDocumentFromDisk(const std::string& filepath);

}