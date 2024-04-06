#include "../Precompiled.h"

#include "../Core/Profiler.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/TextureCube.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "./DiligentUtils.h"

#include "../DebugNew.h"

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

namespace Atomic
{
    void TextureCube::OnDeviceLost()
    {
        // No-op on Direct3D11
    }

    void TextureCube::OnDeviceReset()
    {
        // No-op on Direct3D11
    }

    void TextureCube::Release()
    {
        if (graphics_)
        {
            for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
            {
                if (graphics_->GetTexture(i) == this)
                    graphics_->SetTexture(i, nullptr);
            }
        }

        for (const auto& renderSurface : renderSurfaces_)
        {
            if (renderSurface)
                renderSurface->Release();
        }

        object_ = nullptr;
    }

    bool TextureCube::SetData(CubeMapFace face, unsigned level, int x, int y, int width, int height, const void* data)
    {
        ATOMIC_PROFILE(SetTextureData);

        if (!object_)
        {
            ATOMIC_LOGERROR("No texture created, can not set data");
            return false;
        }

        if (!data)
        {
            ATOMIC_LOGERROR("Null source for setting data");
            return false;
        }

        if (level >= levels_)
        {
            ATOMIC_LOGERROR("Illegal mip level for setting data");
            return false;
        }

        const auto level_width = GetLevelWidth(level);
        const auto level_height = GetLevelHeight(level);
        if (x < 0 || x + width > level_width || y < 0 || y + height > level_height || width <= 0 || height <= 0)
        {
            ATOMIC_LOGERROR("Illegal dimensions for setting data");
            return false;
        }

        // If compressed, align the update region on a block
        if (IsCompressed())
        {
            x &= ~3;
            y &= ~3;
            width += 3;
            width &= 0xfffffffc;
            height += 3;
            height &= 0xfffffffc;
        }

        const auto src = static_cast<const unsigned char*>(data);
        const auto row_size = GetRowDataSize(width);
        const auto row_start = GetRowDataSize(x);

        const auto texture = object_.Cast<Diligent::ITexture>(Diligent::IID_Texture);

        Diligent::Box dest_box;
        dest_box.MinX = x;
        dest_box.MaxX = x + width;
        dest_box.MinY = y;
        dest_box.MaxY = y + height;
        dest_box.MinZ = 0;
        dest_box.MaxZ = 1;

        if (usage_ != TEXTURE_DYNAMIC)
        {
            Diligent::TextureSubResData sub_res_data = {};
            sub_res_data.pData = src;
            sub_res_data.Stride = row_size;

            graphics_
                ->GetImpl()
                ->GetDeviceContext()
                ->UpdateTexture(texture,
                                level,
                                face,
                                dest_box,
                                sub_res_data,
                                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            return true;
        }

        if (IsCompressed())
        {
            height = (height + 3) >> 2;
            y >>= 2;
        }

        Diligent::MappedTextureSubresource mapped_data;
        graphics_
            ->GetImpl()
            ->GetDeviceContext()
            ->MapTextureSubresource(texture,
                                    level, face, Diligent::MAP_WRITE,
                                    Diligent::MAP_FLAG_DISCARD, &dest_box,
                                    mapped_data);

        if (!mapped_data.pData)
        {
            ATOMIC_LOGERROR("Failed to map texture for update");
            return false;
        }

        for (int row = 0; row < height; ++row)
            memcpy(static_cast<unsigned char*>(mapped_data.pData) + (row + y) * mapped_data.Stride + row_start,
                   src + row * row_size, row_size);
        graphics_
            ->GetImpl()
            ->GetDeviceContext()
            ->UnmapTextureSubresource(texture, level, face);
        return true;
    }

    bool TextureCube::SetData(const CubeMapFace face, Deserializer& source)
    {
        SharedPtr<Image> image(new Image(context_));
        if (!image->Load(source))
            return false;

        return SetData(face, image);
    }

    bool TextureCube::SetData(const CubeMapFace face, const Image* image, bool useAlpha)
    {
        if (!image)
        {
            ATOMIC_LOGERROR("Null image, can not load texture");
            return false;
        }

        // Use a shared ptr for managing the temporary mip images created during this function
        auto memory_use = 0u;
        auto quality = QUALITY_HIGH;
        const auto renderer = GetSubsystem<Renderer>();
        if (renderer)
            quality = renderer->GetTextureQuality();

        if (!image->IsCompressed())
        {
            SharedPtr<Image> mip_image;
            // Convert unsuitable formats to RGBA
            auto components = image->GetComponents();
            if ((components == 1 && !useAlpha) || components == 2 || components == 3)
            {
                mip_image = image->ConvertToRGBA();
                image = mip_image;
                if (!image)
                    return false;
                components = image->GetComponents();
            }

            auto level_data = image->GetData();
            auto level_width = image->GetWidth();
            auto level_height = image->GetHeight();
            auto format = TextureFormat::TEX_FORMAT_UNKNOWN;

            if (level_width != level_height)
            {
                ATOMIC_LOGERROR("Cube texture width not equal to height");
                return false;
            }

            // Discard unnecessary mip levels
            for (unsigned i = 0; i < mipsToSkip_[quality]; ++i)
            {
                mip_image = image->GetNextLevel();
                image = mip_image;
                level_data = image->GetData();
                level_width = image->GetWidth();
                level_height = image->GetHeight();
            }

            switch (components)
            {
            case 1:
                format = Graphics::GetAlphaFormat();
                break;

            case 4:
                format = Graphics::GetRGBAFormat();
                break;

            default: break;
            }

            // Create the texture when face 0 is being loaded, check that rest of the faces are same size & format
            if (!face)
            {
                // If image was previously compressed, reset number of requested levels to avoid error if level count is too high for new size
                if (IsCompressed() && requestedLevels_ > 1)
                    requestedLevels_ = 0;
                SetSize(level_width, format);
            }
            else
            {
                if (!object_)
                {
                    ATOMIC_LOGERROR("Cube texture face 0 must be loaded first");
                    return false;
                }
                if (level_width != width_ || format != format_)
                {
                    ATOMIC_LOGERROR("Cube texture face does not match size or format of face 0");
                    return false;
                }
            }

            for (unsigned i = 0; i < levels_; ++i)
            {
                SetData(face, i, 0, 0, level_width, level_height, level_data);
                memory_use += level_width * level_height * components;

                if (i < levels_ - 1)
                {
                    mip_image = image->GetNextLevel();
                    image = mip_image;
                    level_data = image->GetData();
                    level_width = image->GetWidth();
                    level_height = image->GetHeight();
                }
            }
        }
        else
        {
            auto width = image->GetWidth();
            const auto height = image->GetHeight();
            const auto levels = image->GetNumCompressedLevels();
            auto format = graphics_->GetFormat(image->GetCompressedFormat());
            auto need_decompress = false;

            if (width != height)
            {
                ATOMIC_LOGERROR("Cube texture width not equal to height");
                return false;
            }

            if (!format)
            {
                format = Graphics::GetRGBAFormat();
                need_decompress = true;
            }

            auto mips_to_skip = mipsToSkip_[quality];
            if (mips_to_skip >= levels)
                mips_to_skip = levels - 1;
            while (mips_to_skip && (width / (1 << mips_to_skip) < 4 || height / (1 << mips_to_skip) < 4))
                --mips_to_skip;
            width /= (1 << mips_to_skip);

            // Create the texture when face 0 is being loaded, assume rest of the faces are same size & format
            if (!face)
            {
                SetNumLevels(Max((levels - mips_to_skip), 1U));
                SetSize(width, format);
            }
            else
            {
                if (!object_)
                {
                    ATOMIC_LOGERROR("Cube texture face 0 must be loaded first");
                    return false;
                }
                if (width != width_ || format != format_)
                {
                    ATOMIC_LOGERROR("Cube texture face does not match size or format of face 0");
                    return false;
                }
            }

            for (unsigned i = 0; i < levels_ && i < levels - mips_to_skip; ++i)
            {
                CompressedLevel level = image->GetCompressedLevel(i + mips_to_skip);
                if (!need_decompress)
                {
                    SetData(face, i, 0, 0, level.width_, level.height_, level.data_);
                    memory_use += level.rows_ * level.rowSize_;
                }
                else
                {
                    const auto rgba_data = new unsigned char[static_cast<unsigned>(level.width_ * level.height_ * 4)];
                    level.Decompress(rgba_data);
                    SetData(face, i, 0, 0, level.width_, level.height_, rgba_data);
                    memory_use += level.width_ * level.height_ * 4;
                    delete[] rgba_data;
                }
            }
        }

        faceMemoryUse_[face] = memory_use;
        unsigned total_memory_use = sizeof(TextureCube);
        for (const auto i : faceMemoryUse_)
            total_memory_use += i;
        SetMemoryUse(total_memory_use);

        return true;
    }

    bool TextureCube::GetData(const CubeMapFace face, unsigned level, void* dest) const
    {
        if (!object_)
        {
            ATOMIC_LOGERROR("No texture created, can not get data");
            return false;
        }
        
        if (!dest)
        {
            ATOMIC_LOGERROR("Null destination for getting data");
            return false;
        }
        
        if (level >= levels_)
        {
            ATOMIC_LOGERROR("Illegal mip level for getting data");
            return false;
        }
        
        if (multiSample_ > 1 && !autoResolve_)
        {
            ATOMIC_LOGERROR("Can not get data from multisampled texture without autoresolve");
            return false;
        }
        
        if (resolveDirty_)
            graphics_->ResolveToTexture(const_cast<TextureCube*>(this));
        
        auto level_width = GetLevelWidth(level);
        auto level_height = GetLevelHeight(level);

        Diligent::TextureDesc texture_desc ={};
        texture_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        texture_desc.Width = level_width;
        texture_desc.Height = level_height;
        texture_desc.MipLevels = 1;
        texture_desc.ArraySize = 1;
        texture_desc.Format = format_;
        texture_desc.SampleCount = 1;
        texture_desc.Usage = Diligent::USAGE_STAGING;
        texture_desc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
        
        Diligent::RefCntAutoPtr<Diligent::ITexture> staging_texture;
        graphics_->GetImpl()->GetDevice()->CreateTexture(texture_desc, nullptr, &staging_texture);
        
        if (!staging_texture)
        {
            ATOMIC_LOGERROR("Failed to create staging texture for GetData");
            return false;
        }

        const auto src_resource = resolve_texture_ ? resolve_texture_ : object_.Cast<Diligent::ITexture>(Diligent::IID_Texture);

        Diligent::Box src_box;
        src_box.MinX = 0;
        src_box.MaxX = level_width;
        src_box.MinY = 0;
        src_box.MaxY = level_height;
        src_box.MinZ = 0;
        src_box.MaxZ = 1;

        Diligent::CopyTextureAttribs copy_attribs = {};
        copy_attribs.pSrcTexture = src_resource;
        copy_attribs.SrcMipLevel = level;
        copy_attribs.SrcSlice = face;
        copy_attribs.pSrcBox = &src_box;
        copy_attribs.pDstTexture = staging_texture;
        copy_attribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copy_attribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        graphics_->GetImpl()->GetDeviceContext()->CopyTexture(copy_attribs);

        Diligent::MappedTextureSubresource mapped_data = {};
        graphics_->GetImpl()->GetDeviceContext()->MapTextureSubresource(staging_texture,
            0, 0, Diligent::MAP_READ, Diligent::MAP_FLAG_NONE, nullptr, mapped_data);

        if(!mapped_data.pData)
        {
            ATOMIC_LOGERROR("Failed to map staging texture for GetData");
            return false;
        }

        const auto row_size = GetRowDataSize(level_width);
        const auto num_rows = static_cast<unsigned>(IsCompressed() ? (level_height + 3) >> 2 : level_height);
        for (unsigned row = 0; row < num_rows; ++row)
            memcpy(static_cast<unsigned char*>(dest) + row * row_size, static_cast<unsigned char*>(mapped_data.pData) + row * mapped_data.Stride, row_size);

        return true;
    }

    bool TextureCube::Create()
    {
        Release();
        
        if (!graphics_ || !width_ || !height_)
            return false;
        
        levels_ = CheckMaxLevels(width_, height_, requestedLevels_);

        Diligent::TextureDesc texture_desc = {};
        texture_desc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;
        texture_desc.Format = sRGB_ ? GetSRGBFormat(format_) : format_;
        
        // Disable multisampling if not supported
        if (multiSample_ > 1 && !graphics_->GetImpl()->CheckMultiSampleSupport(multiSample_, texture_desc.Format, TextureFormat::TEX_FORMAT_UNKNOWN))
        {
            multiSample_ = 1;
            autoResolve_ = false;
        }
        
        // Set mipmapping
        if (usage_ == TEXTURE_RENDERTARGET && levels_ != 1 && multiSample_ == 1)
            texture_desc.MiscFlags |= Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;
        
        texture_desc.Width = width_;
        texture_desc.Height = height_;
        // Disable mip levels from the multisample texture. Rather create them to the resolve texture
        texture_desc.MipLevels = multiSample_ == 1 ? levels_ : 1;
        texture_desc.ArraySize = MAX_CUBEMAP_FACES;
        texture_desc.SampleCount = multiSample_;
        texture_desc.Usage = usage_ == TEXTURE_DYNAMIC ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_DEFAULT;
        texture_desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        if (usage_ == TEXTURE_RENDERTARGET)
            texture_desc.BindFlags |= Diligent::BIND_RENDER_TARGET;
        else if (usage_ == TEXTURE_DEPTHSTENCIL)
            texture_desc.BindFlags |= Diligent::BIND_DEPTH_STENCIL;
        texture_desc.CPUAccessFlags = usage_ == TEXTURE_DYNAMIC ? Diligent::CPU_ACCESS_WRITE : Diligent::CPU_ACCESS_NONE;
        // When multisample is specified, creating an actual cube texture will fail. Rather create as a 2D texture array
        // whose faces will be rendered to; only the resolve texture will be an actual cube texture
        if (multiSample_ < 2)
        {
	        if(!REngine::utils_is_compressed_texture_format(texture_desc.Format))
                texture_desc.MiscFlags |= Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;
        }

        Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
        graphics_->GetImpl()->GetDevice()->CreateTexture(texture_desc, nullptr, &texture);
        if (!texture)
        {
            ATOMIC_LOGERROR("Failed to create texture");
            return false;
        }

        object_ = texture;
        
        // Create resolve texture for multisampling
        if (multiSample_ > 1)
        {
            texture_desc.SampleCount = 1;
            if (levels_ != 1)
                texture_desc.MiscFlags |= Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;

            graphics_->GetImpl()->GetDevice()->CreateTexture(texture_desc,
                    nullptr,
                    &resolve_texture_);
            
            if(!resolve_texture_)
            {
                ATOMIC_LOGERROR("Failed to create resolve texture");
                return false;
            }
        }

        view_ = REngine::utils_create_texture_view(resolve_texture_ ? resolve_texture_ : texture, Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
        if(!view_)
        {
            ATOMIC_LOGERROR("Failed to create shader resource view for texture");
            return false;
        }


        if(usage_ != TEXTURE_RENDERTARGET)
            return true;

        texture = object_.Cast<Diligent::ITexture>(Diligent::IID_Texture);
        for (unsigned i =0; i < MAX_CUBEMAP_FACES; ++i)
        {
            Diligent::TextureViewDesc render_target_view_desc = {};
            render_target_view_desc.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
            render_target_view_desc.Format = texture_desc.Format;
            render_target_view_desc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
            render_target_view_desc.NumArraySlices = 1;
            render_target_view_desc.FirstArraySlice = i;

            texture->CreateView(render_target_view_desc, &renderSurfaces_[i]->view_);

            if(renderSurfaces_[i]->view_)
            {
                ATOMIC_LOGERROR("Failed to create render target view for texture");
                return false;
            }
        }
        
        return true;
    }
}
