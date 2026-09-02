#include "bitmap.hpp"

#include <iostream>

#include <stb_image.h>
#include <stb_image_write.h>

#include "core/utils.hpp"

Bitmap::Bitmap(const Vec2& size, const std::vector<uint32_t>& pixels)
    : Size(size)
    , Pixels(std::move(pixels))
{}

Bitmap::Bitmap(const Vec2& size)
    : Bitmap(size, std::vector<uint32_t>(size.GetArea(), 0))
{}

Bitmap::Bitmap(const std::string& filepath)
{
    Vec2 new_size;
    void* data = stbi_load(filepath.c_str(), &new_size.x, &new_size.y, nullptr, 4);
    if (!data) {
        std::cerr << "(Bitmap)[stbi_load] failed load image: " << stbi_failure_reason() << "\n";
        return;
    }

    Size = new_size;

    int index = new_size.x * new_size.y;
    uint32_t* ptr = static_cast<uint32_t*>(data);
    Pixels.assign(ptr, ptr + index);
    
    stbi_image_free(data);
}

void Bitmap::Load(const std::string& path)
{

}

void Bitmap::Save(const std::string& path)
{
    std::string extension = util::GetExtensionFromPath(path);

    // if (extension == ".bmp")
    // {
    //     stbi_write_bmp(filepath.c_str(), Size.x, Size.y, 4, GetData());
    // }
    // else if (extension == ".jpg" || extension == ".jpeg")
    // {
    //     stbi_write_jpg(filepath.c_str(), Size.x, Size.y, 4, GetData(), 100);
    // }
    // else if (extension == ".png")
    // {
    //     stbi_write_png(filepath.c_str(), Size.x, Size.y, 4, GetData(), Size.x * 4);
    // }

    // if (const char* failure = stbi_failure_reason())
    // {
    //     spdlog::error("[Bitmap] (::Save) failed save image: {}", failure);
    //     return;
    // }

    // spdlog::info("[Bitmap] (::Save) saved with success: {}", filepath);
}

Bitmap Bitmap::Crop(Vec4 r) const
{
    if (r.x < 0) r.x = 0;
    if (r.y < 0) r.y = 0;
    if (r.z > Size.x) r.z = Size.x;
    if (r.w > Size.y) r.w = Size.y;

    int w = r.GetWidth();
    int h = r.GetHeight();

    Bitmap part(Vec2(w, h));

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int index = (y * w + x);
            part.Pixels[index] = Pixels[(r.y + y) * Size.x + (r.x + x)];
        }
    }
    
    return part;
}