#include <d3d11.h>

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Core/Profiler.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Texture2D.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"

#include "./DiligentUtils.h"

#include "../DebugNew.h"

namespace Atomic
{
    void Texture2D::OnDeviceLost()
    {
        // No-op on Direct3D11
    }

    void Texture2D::OnDeviceReset()
    {
        // No-op on Direct3D11
    }

    void Texture2D::Release()
    {
        if (graphics_ && object_)
        {
            for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
            {
                if (graphics_->GetTexture(i) == this)
                    graphics_->SetTexture(i, nullptr);
            }
        }

        if (renderSurface_)
            renderSurface_->Release();
    }

    bool Texture2D::SetData(unsigned level, int x, int y, int width, int height, const void* data)
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
        const auto sub_resource = REngine::utils_calc_sub_resource(level, 0, levels_);
        const auto texture = object_.Cast<Diligent::ITexture>(Diligent::IID_Texture);

        if (usage_ == TEXTURE_STATIC)
        {
            Diligent::Box box = {};
            box.MinX = x;
            box.MaxX = x + width;
            box.MinY = y;
            box.MaxY = y + height;
            box.MinZ = 0;
            box.MaxZ = 1;

            Diligent::TextureSubResData sub_res_data = {};
            sub_res_data.pData = data;
            sub_res_data.Stride = row_size;

            graphics_->GetImpl()->GetDeviceContext()->UpdateTexture(
                texture,
                sub_resource,
                0,
                box,
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

        Diligent::MappedTextureSubresource mapped_data = {};
        graphics_->GetImpl()->GetDeviceContext()->MapTextureSubresource(
            texture,
            sub_resource,
            0,
            Diligent::MAP_WRITE,
            Diligent::MAP_FLAG_DISCARD,
            nullptr,
            mapped_data);

        if (!mapped_data.pData)
        {
            ATOMIC_LOGERROR("Failed to map texture for update");
            return false;
        }

        for (int row = 0; row < height; ++row)
            memcpy(static_cast<unsigned char*>(mapped_data.pData) + (row + y) * mapped_data.Stride + row_start,
                   src + row * row_size, row_size);
        graphics_->GetImpl()->GetDeviceContext()->UnmapTextureSubresource(texture, sub_resource, 0);
        return true;
    }

    bool Texture2D::SetData(Image* image, bool useAlpha)
    {
        if (!image)
        {
            ATOMIC_LOGERROR("Null image, can not load texture");
            return false;
        }

        // Use a shared ptr for managing the temporary mip images created during this function
        unsigned memory_use = sizeof(Texture2D);
        int quality = QUALITY_HIGH;

        const auto renderer = GetSubsystem<Renderer>();
        if (renderer)
            quality = renderer->GetTextureQuality();

        if (!image->IsCompressed())
        {
            SharedPtr<Image> mip_image;
            // Convert unsuitable formats to RGBA
            unsigned components = image->GetComponents();
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

            // If image was previously compressed, reset number of requested levels to avoid error if level count is too high for new size
            if (IsCompressed() && requestedLevels_ > 1)
                requestedLevels_ = 0;
            SetSize(level_width, level_height, format);

            for (unsigned i = 0; i < levels_; ++i)
            {
                SetData(i, 0, 0, level_width, level_height, level_data);
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
            int width = image->GetWidth();
            int height = image->GetHeight();
            const unsigned levels = image->GetNumCompressedLevels();
            TextureFormat format = graphics_->GetFormat(image->GetCompressedFormat());
            auto need_decompress = false;

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
            height /= (1 << mips_to_skip);

            SetNumLevels(Max((levels - mips_to_skip), 1U));
            SetSize(width, height, format);

            for (unsigned i = 0; i < levels_ && i < levels - mips_to_skip; ++i)
            {
                CompressedLevel level = image->GetCompressedLevel(i + mips_to_skip);
                if (!need_decompress)
                {
                    SetData(i, 0, 0, level.width_, level.height_, level.data_);
                    memory_use += level.rows_ * level.rowSize_;
                }
                else
                {
                    const auto rgba_data = new unsigned char[level.width_ * level.height_ * 4];
                    level.Decompress(rgba_data);
                    SetData(i, 0, 0, level.width_, level.height_, rgba_data);
                    memory_use += level.width_ * level.height_ * 4;
                    delete[] rgba_data;
                }
            }
        }

        SetMemoryUse(memory_use);
        return true;
    }

    bool Texture2D::GetData(unsigned level, void* dest) const
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
            graphics_->ResolveToTexture(const_cast<Texture2D*>(this));

        const auto level_width = GetLevelWidth(level);
        const auto level_height = GetLevelHeight(level);

        Diligent::TextureDesc texture_desc = {};
        texture_desc.Name = GetName().CString();
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

        const auto src_resource = resolve_texture_ ? resolve_texture_ : object_;
        const auto src_sub_resource = REngine::utils_calc_sub_resource(level, 0, levels_);

        Diligent::Box src_box = {};
        src_box.MinX = 0;
        src_box.MaxX = level_width;
        src_box.MinY = 0;
        src_box.MaxY = level_height;
        src_box.MinZ = 0;
        src_box.MaxZ = 1;

        Diligent::CopyTextureAttribs copy_attribs = {};
        copy_attribs.pDstTexture = staging_texture;
        copy_attribs.DstMipLevel = 0;
        copy_attribs.pSrcBox = &src_box;
        copy_attribs.DstX = 0;
        copy_attribs.DstY = 0;
        copy_attribs.DstZ = 0;
        copy_attribs.SrcSlice = 0;
        copy_attribs.pSrcTexture = src_resource.Cast<Diligent::ITexture>(Diligent::IID_Texture);
        copy_attribs.SrcMipLevel = src_sub_resource;

        graphics_->GetImpl()->GetDeviceContext()->CopyTexture(copy_attribs);

        Diligent::MappedTextureSubresource mapped_data = {};
        const auto row_size = GetRowDataSize(level_width);
        const auto num_rows = static_cast<unsigned>(IsCompressed() ? (level_height + 3) >> 2 : level_height);

        graphics_->GetImpl()->GetDeviceContext()->MapTextureSubresource(
            staging_texture,
            0, 0,
            Diligent::MAP_READ,
            Diligent::MAP_FLAG_NONE,
            nullptr,
            mapped_data);
        if (!mapped_data.pData)
        {
            ATOMIC_LOGERROR("Failed to map staging texture for GetData");
            return false;
        }
        for (unsigned row = 0; row < num_rows; ++row)
            memcpy(static_cast<unsigned char*>(dest) + row * row_size,
                   static_cast<unsigned char*>(mapped_data.pData) + row * mapped_data.Stride, row_size);

        graphics_->GetImpl()->GetDeviceContext()->UnmapTextureSubresource(staging_texture, 0, 0);
        return true;
    }

    bool Texture2D::Create()
    {
        Release();
        
        if (!graphics_ || !width_ || !height_)
            return false;
        
        levels_ = CheckMaxLevels(width_, height_, requestedLevels_);

        Diligent::TextureDesc texture_desc = {};
        texture_desc.Name = GetName().CString();
        texture_desc.Format = sRGB_ ? GetSRGBFormat(format_) : format_;
        texture_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        
        // Disable multisampling if not supported
        if (multiSample_ > 1 && !graphics_->GetImpl()->CheckMultiSampleSupport(multiSample_, texture_desc.Format, TextureFormat::TEX_FORMAT_UNKNOWN))
        {
            multiSample_ = 1;
            autoResolve_ = false;
        }
        
        // Set mipmapping
        if (usage_ == TEXTURE_DEPTHSTENCIL)
            levels_ = 1;
        else if (usage_ == TEXTURE_RENDERTARGET && levels_ != 1 && multiSample_ == 1)
            texture_desc.MiscFlags |= Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;

        texture_desc.Width = width_;
        texture_desc.Height = height_;
        // Disable mip levels from the multisample texture. Rather create them to the resolve texture
        texture_desc.MipLevels = multiSample_ == 1 ? levels_ : 1;
        texture_desc.ArraySize = 1;
        texture_desc.SampleCount = multiSample_;
        
        texture_desc.Usage = usage_ == TEXTURE_DYNAMIC ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_DEFAULT;
        texture_desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        if (usage_ == TEXTURE_RENDERTARGET)
            texture_desc.BindFlags |= Diligent::BIND_RENDER_TARGET;
        else if (usage_ == TEXTURE_DEPTHSTENCIL)
            texture_desc.BindFlags |= Diligent::BIND_DEPTH_STENCIL;
        texture_desc.CPUAccessFlags = usage_ == TEXTURE_DYNAMIC ? Diligent::CPU_ACCESS_WRITE : Diligent::CPU_ACCESS_NONE;
        
        if (usage_ == TEXTURE_DEPTHSTENCIL && multiSample_ > 1)
            texture_desc.BindFlags &= ~Diligent::BIND_SHADER_RESOURCE;

        Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
        graphics_->GetImpl()->GetDevice()->CreateTexture(texture_desc, nullptr, &texture);

        if (!texture)
        {
            ATOMIC_LOGERROR("Failed to create texture");
            return false;
        }
        
        object_ = texture;
        // Create resolve texture for multisampling if necessary
        if (multiSample_ > 1 && autoResolve_)
        {
            texture_desc.MipLevels = levels_;
            texture_desc.SampleCount = 1;
            if (levels_ != 1)
                texture_desc.MiscFlags |= Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;

            graphics_->GetImpl()->GetDevice()->CreateTexture(texture_desc, nullptr, &resolve_texture_);
            if (!resolve_texture_)
            {
                ATOMIC_LOGERROR("Failed to create resolve texture");
                return false;
            }
        }
        
        if (texture_desc.BindFlags & Diligent::BIND_SHADER_RESOURCE)
        {
            view_ = resolve_texture_ ? resolve_texture_->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE) : texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
            if (!view_)
            {
                ATOMIC_LOGERROR("Failed to create shader resource view for texture");
                return false;
            }
        }
        
        if (usage_ == TEXTURE_RENDERTARGET)
        {
            renderSurface_->view_ = texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
            if (!renderSurface_->view_)
            {
                ATOMIC_LOGERROR("Failed to create rendertarget view for texture");
                return false;
            }
        }
        else if (usage_ == TEXTURE_DEPTHSTENCIL)
        {
            Diligent::TextureViewDesc view_desc = {};
            view_desc.ViewType = Diligent::TEXTURE_VIEW_READ_ONLY_DEPTH_STENCIL;
            view_desc.Format = GetDSVFormat(texture_desc.Format);
            view_desc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;

            renderSurface_->view_ = texture->GetDefaultView(view_desc.ViewType);
            if (!renderSurface_->view_)
            {
                ATOMIC_LOGERROR("Failed to create depth-stencil view for texture");
                return false;
            }
        
            // Create also a read-only version of the view for simultaneous depth testing and sampling in shader
            texture->CreateView(view_desc, &renderSurface_->read_only_view_);
            if(!renderSurface_->read_only_view_)
                ATOMIC_LOGERROR("Failed to create read-only depth-stencil view for texture");
        }
        
        return true;
    }
}
