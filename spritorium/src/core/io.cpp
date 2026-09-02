#include "io.hpp"
#include "app/logger.hpp"

#include <fstream>

#include <zpp_bits.h>

namespace io
{

    DocumentRecord CreateDocumentRecord(const editor::RefSprite& sprite)
    {
        DocumentRecord r;
        r.Name = sprite->Name;
        r.Size = sprite->Size;

        for (const auto& l : sprite->Layers)
        {
            LayerRecord lr;
            lr.Name = l->Name;
            lr.Pixels = l->Image.Pixels;
            lr.IsHidden = l->IsHidden;
            r.Layers.push_back(lr);
        }
        
        return r;
    }

    bool WriteDocumentOnDisk(const std::string& filepath, const editor::RefSprite& sprite)
    {
        auto [data, out] = zpp::bits::data_out();

        FileHeader header(0, 1, 0);

        try
        {
            out(header).or_throw();
            out(CreateDocumentRecord(sprite)).or_throw();
        }
        catch(std::exception& e) {
            spto::Warn("Failed WriteDocumentOnDisk: {}", e.what());
            return false;
        }

        std::ofstream file(filepath, std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            spto::Warn("Failed to open file for writing: {}", filepath);
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();

        return true;
    }

    std::variant<bool, DocumentRecord> ReadDocumentFromDisk(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> data(size);
        file.read(data.data(), size);

        zpp::bits::in in(data);

        FileHeader header;
        DocumentRecord r;

        try
        {
            in(header).or_throw();
            in(r).or_throw();
        }
        catch(std::exception& e) {
            spto::Warn("Failed ReadDocumentFromDisk: {}", e.what());
            return false;
        }

        return r;
    }

}