#include "sprite.hpp"

#include "core/shader.hpp"
#include "app/app_state.hpp"
#include "app/logger.hpp"
#include "layer.hpp"

namespace editor
{

    Sprite::Sprite(const std::string name, int width, int height)
        : Name(name)
        , Filepath("")
        , ActiveLayerIndex(0)
        , Size(width, height)
        , IsUnsavedDocument(false)
        , ScratchLayer(std::make_shared<LayerContext>("ScratchLayer", Size))
    {
        for (int i = 0; i < MAX_RENDERERS; ++i)
        {
            Renderers[i] = std::make_unique<gfx::FrameBuffer>(width, height);
        }
    };

    Sprite::~Sprite()
    {
        spto::Info("[Sprite] {} destroyed", Name);
    }

    RefLayer Sprite::CreateLayer(int pos)
    {
        static int layer_count = 0;
        
        std::string layer_name = "Layer " + std::to_string(layer_count++);

        auto new_layer = std::make_shared<LayerContext>(layer_name, Size);
        
        std::vector<RefLayer> prev_list = Layers;

        auto it = Layers.begin();
        Layers.insert(it + pos, new_layer);

        // disable commit for the first layer
        if (Layers.size() > 1)
        {
            History.Commit(std::make_unique<Command_LayerStateList>(this, prev_list, Layers));
        }
        
        return new_layer;
    }

    RefLayer Sprite::CreateLayerFromBitmap(const std::string& name, const Bitmap& image, bool is_hidden)
    {
        const auto layer = std::make_shared<LayerContext>(name, image.Size);

        layer->Image = image;
        layer->Dirty.AddFull();
        layer->IsHidden = is_hidden;

        Layers.push_back(layer);

        return layer;
    }

    void Sprite::DeleteLayer(int index)
    {
        const int size = Layers.size();
        if (size == 1)
        {
            return;
        }

        index = std::clamp(index, 0, size - 1);

        std::vector<RefLayer> prev_list = Layers;

        auto it = Layers.begin();
        Layers.erase(it + index);

        ActiveLayerIndex = std::clamp(ActiveLayerIndex, 0, static_cast<int>(Layers.size()) - 1);

        History.Commit(std::make_unique<Command_LayerStateList>(this, prev_list, Layers));
    }

    void Sprite::SelectLayer(int index)
    {
        History.Commit(std::make_unique<Command_LayerSelection>(this, ActiveLayerIndex, index));
        
        ActiveLayerIndex = index;
    }

    RefLayer Sprite::GetLayerById(int id) const
    {
        const auto it = std::ranges::find_if(Layers,
        [id](const auto& el)
        {
            return el->Id == id;
        });

        if (it == Layers.end())
        {
            return nullptr;
        }

        return *it;
    }

    // / Merge all layers in a Bitmap
    Bitmap Sprite::CreateBitmapFromLayers(bool color_black_bg)
    {
        Bitmap image(Size);
        
        if (color_black_bg)
        {
            pixel::MakeFill(&image, C_BLACK);
        }

        for (const auto& layer : Layers)
        {
            pixel::MakeMerge(&image, &layer->Image, PixelFilterOp::None);
        }

        return image;
    }

    void Sprite::RenderProcess()
    {
        ScratchLayer->UpdateTexture();

        const gl::Shader& blit = g.FindShader("blit");
        const gl::Shader& composite = g.FindShader("composite");
        const gl::Shader& subwhite = g.FindShader("subwhite");

        gfx::FrameBuffer* double_buffer[2]
        {
            Renderers[static_cast<int>(RenderType::Aux0)].get(),
            Renderers[static_cast<int>(RenderType::Aux1)].get()
        };

        const auto& scratch_renderer = GetRenderer(RenderType::Scratch);
        const auto& canvas_renderer = GetRenderer(RenderType::Canvas);

        bool has_visible_layer = false;
        int first_visible_layer = -1;
        
        for (int n = 0; n < Layers.size(); ++n)
        {
            const auto& layer = Layers[n];
            
            if (layer->IsHidden)
            {
                continue;
            }

            layer->UpdateTexture();

            if (!has_visible_layer)
            {
                has_visible_layer = true;
                first_visible_layer = n;
            }
            
            uint32_t curr_tex = layer->GetTexture();

            bool working_layer = (n == ActiveLayerIndex);
            if (working_layer)
            {
                gl::FrameBufferBind(scratch_renderer.ID);
                {
                    glViewport(0, 0, Size.x, Size.y);

                    if (g.Tools->ActiveTool == ToolType::Eraser) {
                        subwhite.Use();
                        subwhite.SetUniform1i("uTex0", 0);
                        subwhite.SetUniform1i("uTex1", 1);
                    }
                    else {
                        composite.Use();
                        composite.SetUniform1i("uTex0", 0);
                        composite.SetUniform1i("uTex1", 1);
                    }

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, layer->GetTexture());
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, GetScratchLayer()->GetTexture());

                    gl::RenderQuad();
                }
                gl::FrameBufferUnbind();

                curr_tex = scratch_renderer.TextureID;
            }

            gl::FrameBufferBind(double_buffer[0]->ID);
            {
                glViewport(0, 0, Size.x, Size.y);

                if (n == first_visible_layer)
                {
                    blit.Use();
                    blit.SetUniform1i("uTex0", 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, curr_tex);
                }
                else {
                    composite.Use();
                    composite.SetUniform1i("uTex0", 0);
                    composite.SetUniform1i("uTex1", 1);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, double_buffer[1]->TextureID);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, curr_tex);
                }

                gl::RenderQuad();
            }
            gl::FrameBufferUnbind();

            std::swap(double_buffer[0], double_buffer[1]);
        }

        gl::FrameBufferBind(canvas_renderer.ID);
        {
            glViewport(0, 0, Size.x, Size.y);

            if (!has_visible_layer)
            {
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            else
            {
                blit.Use();
                blit.SetUniform1i("uTex0", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, double_buffer[1]->TextureID);

                gl::RenderQuad();
            }
        }
        gl::FrameBufferUnbind();
    }

}