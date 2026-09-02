#include "shader.hpp"

#include <iostream>
#include <string>

#include "utils.hpp"

namespace gl
{

    static bool GL_ShaderCheckStatus(GLuint shader, int type)
    {
        GLint result;
        
        std::string str_type = (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (!result)
        {
            GLchar info[512];
            glGetShaderInfoLog(shader, 512, NULL, info);
            std::cerr << "[Error: Compile shader " << str_type << "]\n" << info << "\n";
        }

        return result;
    }

    static bool GL_ProgramCheckStatus(GLuint program)
    {
        GLint result;
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        if (!result)
        {
            GLchar info[512];
            glGetProgramInfoLog(program, 512, NULL, info);
            std::cerr << "[Error: Compile program]\n" << info << "\n";
        }

        return result;
    }

    static GLuint CreateShader(std::string_view str, int type)
    {
        const GLchar* rawstr = reinterpret_cast<const GLchar*>(str.data());
        
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &rawstr, nullptr);
        glCompileShader(sh);
        GL_ShaderCheckStatus(sh, type);
        return sh;
    }

    Shader::Shader(std::string_view vertex, std::string_view fragment)
    {
        const GLuint sh_v = CreateShader(vertex, GL_VERTEX_SHADER);
        const GLuint sh_f = CreateShader(fragment, GL_FRAGMENT_SHADER);

        GLuint program = glCreateProgram();
        glAttachShader(program, sh_v);
        glAttachShader(program, sh_f);
        glLinkProgram(program);
        GL_ProgramCheckStatus(program);

        Id = program;

        glDetachShader(program, sh_v);
        glDetachShader(program, sh_f);
        glDeleteShader(sh_v);
        glDeleteShader(sh_f);
    }
}
