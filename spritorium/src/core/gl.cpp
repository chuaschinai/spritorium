#include "gl.hpp"

#include <cassert>

void gl::SetupOpenGLDebugging()
{
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLDebugCallback, nullptr);
        
        spdlog::info("OpenGL Debug Output initialized successfully");
    } else {
        spdlog::warn("Failed to initialize OpenGL Debug Output, SDL_GL_CONTEXT_DEBUG_FLAG has been set?.");
    }
}

gl::Texture::Texture(int width, int height)
    : Width(width)
    , Height(height)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    const std::vector<uint8_t> zero(width * height * 4, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero.data());
}

gl::Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}

void gl::TextureUpdate(gl::Texture& texture, const std::vector<uint32_t>& pixels, int x, int y, int w, int h)
{
    int index = (y * texture.Width + x);

    assert(index < pixels.size() && "Index out of bounds");

    glBindTexture(GL_TEXTURE_2D, texture.ID);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, texture.Width);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[index]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

gl::Quad* gl::InitQuad()
{
    Quad* q = new Quad;

    static constexpr float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };

    static constexpr uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &q->vao);
    glGenBuffers(1, &q->vbo);
    glGenBuffers(1, &q->ebo);

    glBindVertexArray(q->vao);

    glBindBuffer(GL_ARRAY_BUFFER, q->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, q->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    return q;
}

void gl::FreeQuad(Quad* q)
{
    glDeleteBuffers(1, &q->ebo);
    glDeleteBuffers(1, &q->vbo);
    glDeleteVertexArrays(1, &q->vao);
}

void gl::RenderQuad()
{
    glBindVertexArray(g_quad->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}