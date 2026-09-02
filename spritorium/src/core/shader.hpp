#pragma once

#include <string_view>

#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <imgui.h>

namespace gl
{

    inline std::string_view vertDefault()
    {
        static const unsigned char data[] = {
            #include "default.vert.h"
        };
        return std::string_view(reinterpret_cast<const char*>(data), sizeof(data));
    }

    inline std::string_view fragBlit()
    {
        static const unsigned char data[] = {
            #include "blit.frag.h"
        };
        return std::string_view(reinterpret_cast<const char*>(data), sizeof(data));
    }

    inline std::string_view fragSubwhite()
    {
        static const unsigned char data[] = {
            #include "subwhite.frag.h"
        };
        return std::string_view(reinterpret_cast<const char*>(data), sizeof(data));
    }

    inline std::string_view fragComposite()
    {
        static const unsigned char data[] = {
            #include "composite.frag.h"
        };
        return std::string_view(reinterpret_cast<const char*>(data), sizeof(data));
    }

    inline std::string_view fragCanvas()
    {
        static const unsigned char data[] = {
            #include "canvas.frag.h"
        };
        return std::string_view(reinterpret_cast<const char*>(data), sizeof(data));
    }

    class Shader
    {
        GLuint Id;
    public:
        Shader(std::string_view vertex, std::string_view fragment);

        void Use() const { glUseProgram(Id); }
        void SetUniform1f(const char* loc, float v) const { glUniform1f(glGetUniformLocation(Id, loc), v); };
        void SetUniform1i(const char* loc, float v) const { glUniform1i(glGetUniformLocation(Id, loc), v); }
        void SetUniform2f(const char* loc, float x, float y) const { glUniform2f(glGetUniformLocation(Id, loc), x, y); }
        void SetUniform3f(const char* loc, float x, float y, float z) const { glUniform3f(glGetUniformLocation(Id, loc), x, y, z); }
    };

}
