#include "../Graphics/Graphics.h"
#include "../Graphics/Material.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

namespace Atomic
{

void Texture::SetSRGB(bool enable)
{
    if (graphics_)
        enable &= graphics_->GetSRGBSupport();
    
    if (enable != sRGB_)
    {
        sRGB_ = enable;
        // If texture had already been created, must recreate it to set the sRGB texture format
        if (object_)
            Create();
    }
}

bool Texture::GetParametersDirty() const
{
    return parametersDirty_;
}

bool Texture::IsCompressed() const
{
    return format_ == Diligent::TEX_FORMAT_BC1_UNORM || format_ == Diligent::TEX_FORMAT_BC2_UNORM || format_ == Diligent::TEX_FORMAT_BC3_UNORM;
}

unsigned Texture::GetRowDataSize(int width) const
{
    using namespace Diligent;
    switch (format_)
    {
    case TEX_FORMAT_R8_UNORM:
    case TEX_FORMAT_A8_UNORM:
        return static_cast<unsigned>(width);
    
    case TEX_FORMAT_RG8_UNORM:
    case TEX_FORMAT_R16_UNORM:
    case TEX_FORMAT_R16_FLOAT:
    case TEX_FORMAT_R16_TYPELESS:
        return static_cast<unsigned>(width * 2);
    
    case TEX_FORMAT_RGBA8_UNORM:
    case TEX_FORMAT_RG16_UNORM:
    case TEX_FORMAT_RG16_FLOAT:
    case TEX_FORMAT_R32_FLOAT:
    case TEX_FORMAT_R24G8_TYPELESS:
    case TEX_FORMAT_R32_TYPELESS:
        return static_cast<unsigned>(width * 4);
    
    case TEX_FORMAT_RGBA16_UNORM:
    case TEX_FORMAT_RGBA16_FLOAT:
        return static_cast<unsigned>(width * 8);
    
    case TEX_FORMAT_RGBA32_FLOAT:
        return static_cast<unsigned>(width * 16);
    
    case TEX_FORMAT_BC1_UNORM:
        return static_cast<unsigned>(((width + 3) >> 2) * 8);
    
    case TEX_FORMAT_BC2_UNORM:
    case TEX_FORMAT_BC3_UNORM:
        return static_cast<unsigned>(((width + 3) >> 2) * 16);
    
    default:
        return 0;
    }
}

void Texture::UpdateParameters()
{
    parametersDirty_ = false;
}

TextureFormat Texture::GetSRVFormat(TextureFormat format)
{
    using namespace Diligent;
    if (format == TEX_FORMAT_R24G8_TYPELESS)
        return TEX_FORMAT_R24_UNORM_X8_TYPELESS;
    else if (format == TEX_FORMAT_R16_TYPELESS)
        return TEX_FORMAT_R16_UNORM;
    else if (format == TEX_FORMAT_R32_TYPELESS)
        return TEX_FORMAT_R32_FLOAT;
    return format;
}

TextureFormat Texture::GetDSVFormat(TextureFormat format)
{
    using namespace Diligent;
    if (format == TEX_FORMAT_R24G8_TYPELESS)
        return TEX_FORMAT_D24_UNORM_S8_UINT;
    else if (format == TEX_FORMAT_R16_TYPELESS)
        return TEX_FORMAT_D16_UNORM;
    else if (format == TEX_FORMAT_R32_TYPELESS)
        return TEX_FORMAT_D32_FLOAT;
    else
        return format;
}

TextureFormat Texture::GetSRGBFormat(TextureFormat format)
{
    using namespace Diligent;
    if (format == TEX_FORMAT_RGBA8_UNORM)
        return TEX_FORMAT_RGBA8_UNORM_SRGB;
    else if (format == TEX_FORMAT_BC1_UNORM)
        return TEX_FORMAT_BC1_UNORM_SRGB;
    else if (format == TEX_FORMAT_BC2_UNORM)
        return TEX_FORMAT_BC2_UNORM_SRGB;
    else if (format == TEX_FORMAT_BC3_UNORM)
        return TEX_FORMAT_BC3_UNORM_SRGB;
    return format;
}

void Texture::RegenerateLevels()
{
    if (!view_)
        return;
    graphics_->GetImpl()->GetDeviceContext()->GenerateMips(view_);
    levelsDirty_ = false;
}

}
