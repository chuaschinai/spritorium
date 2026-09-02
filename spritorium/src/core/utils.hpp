#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <imgui.h>

#include "core/types.hpp"

namespace util
{

inline std::string FileFromResourcesFolder(const std::string& file) {
    std::filesystem::path path = std::filesystem::current_path();
    return path.string() + "/resources/" + file;
}

namespace fs = std::filesystem;

/** 
 * Return the extension without the dot (.) 
 *
 * If a not extension found, this return the entire path
 */
inline std::string GetExtensionFromPath(const std::string& path) { return path.substr(path.find_last_of('.') + 1); }

inline std::string read_file(const std::string& path) {
    if (!fs::exists(path)) {
        throw fs::filesystem_error(
            "File not found",
            path,
            std::make_error_code(std::errc::no_such_file_or_directory)
        );
    }

    std::ifstream file(path); 
    
    if (!file.is_open()) {
        throw fs::filesystem_error(
            "Failed to open file",
            path,
            std::make_error_code(std::errc::permission_denied)
        );
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (file.bad()) {
        throw fs::filesystem_error(
            "Fatal I/O error while reading file",
            path,
            std::make_error_code(std::errc::io_error)
        );
    }

    return buffer.str();
}

}

// interop with ImGui
inline ImVec2 ToImVec2(const Vec2& v) { return ImVec2(v.x, v.y); }
inline Vec2 ToVec2(const ImVec2& v) { return Vec2(v.x, v.y); }

namespace spto
{

inline int GetID() {
    static int id = 0;
    return id++;
}

}