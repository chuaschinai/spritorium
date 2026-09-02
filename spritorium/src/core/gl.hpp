#pragma once

#include <memory>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace gl
{

    static void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

        spdlog::critical("[gl] [{}]: {}", id, message);

        switch (source) {
            case GL_DEBUG_SOURCE_API:             spdlog::info("Source: API"); break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   spdlog::info("Source: Window System"); break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: spdlog::info("Source: Shader Compiler"); break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     spdlog::info("Source: Third Party"); break;
            case GL_DEBUG_SOURCE_APPLICATION:     spdlog::info("Source: Application"); break;
            default:                              spdlog::info("Source: Unknown"); break;
        }

        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               spdlog::info("Type: Error"); break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: spdlog::info("Type: Deprecated Behaviour"); break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  spdlog::info("Type: Undefined Behaviour"); break;
            case GL_DEBUG_TYPE_PORTABILITY:         spdlog::info("Type: Portability"); break;
            case GL_DEBUG_TYPE_PERFORMANCE:         spdlog::info("Type: Performance"); break;
            default:                                spdlog::info("Type: Other"); break;
        }

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:   spdlog::critical("Severity: HIGH"); break;
            case GL_DEBUG_SEVERITY_MEDIUM: spdlog::warn("Severity: MEDIUM"); break;
            case GL_DEBUG_SEVERITY_LOW:    spdlog::info("Severity: LOW"); break;
        }

        // Trigger an IDE breakpoint immediately if a hard error occurs
        if (type == GL_DEBUG_TYPE_ERROR)
        {
            #if defined(_MSC_VER)
                __debugbreak(); 
            #elif defined(__GNUC__) || defined(__clang__)
                __builtin_trap();
            #endif
        }
    }

    void SetupOpenGLDebugging();

    struct Texture
    {
        Texture(int width, int height);
        ~Texture();

        GLuint ID;
        int Width;
        int Height;
    };

    struct Quad
    {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
    };

    inline void FrameBufferBind(GLuint id) { glBindFramebuffer(GL_FRAMEBUFFER, id); }
    inline void FrameBufferUnbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void TextureUpdate(gl::Texture& texture, const std::vector<uint32_t>& pixels, int x, int y, int w, int h);

    void FreeQuad(Quad* q);
    Quad* InitQuad();
    void RenderQuad();

    using QuadPtr = std::unique_ptr<Quad, decltype(&FreeQuad)>;
    inline QuadPtr g_quad(nullptr, FreeQuad);

}