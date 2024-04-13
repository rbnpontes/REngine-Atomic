//
// Copyright (c) 2008-2017 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Graphics/GPUObject.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Math/Color.h"
#include "../Resource/Resource.h"
#include "../RHI/RHITypes.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

namespace Atomic
{

static const int MAX_TEXTURE_QUALITY_LEVELS = 3;
    
class XMLElement;
class XMLFile;

/// Base class for texture resources.
class ATOMIC_API Texture : public ResourceWithMetadata, public GPUObject
{
    ATOMIC_OBJECT(Texture, ResourceWithMetadata)

public:
    /// Construct.
    Texture(Context* context);
    /// Destruct.
    virtual ~Texture();

    /// Set number of requested mip levels. Needs to be called before setting size.
    /** The default value (0) allocates as many mip levels as necessary to reach 1x1 size. Set value 1 to disable mipmapping.
        Note that rendertargets need to regenerate mips dynamically after rendering, which may cost performance. Screen buffers
        and shadow maps allocated by Renderer will have mipmaps disabled.
     */
    void SetNumLevels(unsigned levels);
    /// Set filtering mode.
    void SetFilterMode(TextureFilterMode filter);
    /// Set addressing mode by texture coordinate.
    void SetAddressMode(TextureCoordinate coord, TextureAddressMode address);
    /// Set texture max. anisotropy level. No effect if not using anisotropic filtering. Value 0 (default) uses the default setting from Renderer.
    void SetAnisotropy(unsigned level);
    /// Set shadow compare mode. Not used on Direct3D9.
    void SetShadowCompare(bool enable);
    /// Set border color for border addressing mode.
    void SetBorderColor(const Color& color);
    /// Set sRGB sampling and writing mode.
    void SetSRGB(bool enable);
    /// Set backup texture to use when rendering to this texture.
    void SetBackupTexture(Texture* texture);
    /// Set mip levels to skip on a quality setting when loading. Ensures higher quality levels do not skip more.
    void SetMipsToSkip(int quality, int toSkip);

    /// Return API-specific texture format.
    TextureFormat GetFormat() const { return format_; }

    /// Return whether the texture format is compressed.
    bool IsCompressed() const;

    /// Return number of mip levels.
    unsigned GetLevels() const { return levels_; }

    /// Return width.
    int GetWidth() const { return width_; }

    /// Return height.
    int GetHeight() const { return height_; }

    /// Return depth.
    int GetDepth() const { return depth_; }

    /// Return filtering mode.
    TextureFilterMode GetFilterMode() const { return filterMode_; }

    /// Return addressing mode by texture coordinate.
    TextureAddressMode GetAddressMode(TextureCoordinate coord) const { return addressMode_[coord]; }

    /// Return texture max. anisotropy level. Value 0 means to use the default value from Renderer.
    unsigned GetAnisotropy() const { return anisotropy_; }

    /// Return whether shadow compare is enabled. Not used on Direct3D9.
    bool GetShadowCompare() const { return shadowCompare_; }

    /// Return border color.
    const Color& GetBorderColor() const { return borderColor_; }

    /// Return whether is using sRGB sampling and writing.
    bool GetSRGB() const { return sRGB_; }

    /// Return texture multisampling level (1 = no multisampling).
    int GetMultiSample() const { return multiSample_; }

    /// Return texture multisampling autoresolve mode. When true, the texture is resolved before being sampled on SetTexture(). When false, the texture will not be resolved and must be read as individual samples in the shader.
    bool GetAutoResolve() const { return autoResolve_; }

    /// Return whether multisampled texture needs resolve.
    bool IsResolveDirty() const { return resolveDirty_; }

    /// Return whether rendertarget mipmap levels need regenration.
    bool GetLevelsDirty() const { return levelsDirty_; }

    /// Return backup texture.
    Texture* GetBackupTexture() const { return backupTexture_; }

    /// Return mip levels to skip on a quality setting when loading.
    int GetMipsToSkip(int quality) const;
    /// Return mip level width, or 0 if level does not exist.
    int GetLevelWidth(unsigned level) const;
    /// Return mip level width, or 0 if level does not exist.
    int GetLevelHeight(unsigned level) const;
    /// Return mip level depth, or 0 if level does not exist.
    int GetLevelDepth(unsigned level) const;

    /// Return texture usage type.
    TextureUsage GetUsage() const { return usage_; }

    /// Return data size in bytes for a rectangular region.
    unsigned GetDataSize(int width, int height) const;
    /// Return data size in bytes for a volume region.
    unsigned GetDataSize(int width, int height, int depth) const;
    /// Return data size in bytes for a pixel or block row.
    unsigned GetRowDataSize(int width) const;
    /// Return number of image components required to receive pixel data from GetData(), or 0 for compressed images.
    unsigned GetComponents() const;

    /// Return whether the parameters are dirty.
    bool GetParametersDirty() const;

    /// Set additional parameters from an XML file.
    void SetParameters(XMLFile* xml);
    /// Set additional parameters from an XML element.
    void SetParameters(const XMLElement& element);
    /// Mark parameters dirty. Called by Graphics.
    void SetParametersDirty();
    /// Update dirty parameters to the texture object. Called by Graphics when assigning the texture.
    void UpdateParameters();

    /// Return shader resource view. Used on Shader Resource Binding
    Diligent::RefCntAutoPtr<Diligent::ITextureView> GetShaderResourceView() const { return view_; }
    /// Return Texture Sampler Description. Can be used on Pipeline State creation
    void GetSamplerDesc(REngine::SamplerDesc& sample_desc) const;

#if RENGINE_DILIGENT
    static TextureFormat GetSRGBFormat(TextureFormat format);
#else
    TextureFormat GetSRGBFormat(TextureFormat format);
#endif
    /// Set or clear the need resolve flag. Called internally by Graphics.
    void SetResolveDirty(bool enable) { resolveDirty_ = enable; }

    /// Set the mipmap levels dirty flag. Called internally by Graphics.
    void SetLevelsDirty();
    /// Regenerate mipmap levels for a rendertarget after rendering and before sampling. Called internally by Graphics. No-op on Direct3D9. On OpenGL the texture must have been bound to work properly.
    void RegenerateLevels();

    /// Check maximum allowed mip levels for a specific texture size.
    static unsigned CheckMaxLevels(int width, int height, unsigned requestedLevels);
    /// Check maximum allowed mip levels for a specific 3D texture size.
    static unsigned CheckMaxLevels(int width, int height, int depth, unsigned requestedLevels);
    /// Return the shader resource view format corresponding to a texture format. Handles conversion of typeless depth texture formats. Only used on Direct3D11.
    static TextureFormat GetSRVFormat(TextureFormat format);
    /// Return the depth-stencil view format corresponding to a texture format. Handles conversion of typeless depth texture formats. Only used on Direct3D11.
    static TextureFormat GetDSVFormat(TextureFormat format);

    static const char** GetTextureAddressModeNames();
    static const char** GetTextureFilterModeNames();
protected:
    /// Check whether texture memory budget has been exceeded. Free unused materials in that case to release the texture references.
    void CheckTextureBudget(StringHash type);
    /// Create the GPU texture. Implemented in subclasses.
    virtual bool Create() { return true; }

#if RENGINE_DILIGENT
    Diligent::RefCntAutoPtr<Diligent::ITextureView> view_;
    Diligent::RefCntAutoPtr<Diligent::ITexture> resolve_texture_;
#else
    union
    {
        /// Direct3D11 shader resource view.
        void* shaderResourceView_;
        /// OpenGL target.
        unsigned target_;
    };
    /// Direct3D11 sampler state object.
    void* sampler_;
    /// Direct3D11 resolve texture object when multisample with autoresolve is used.
    void* resolveTexture_;
#endif
    /// Texture format.
    TextureFormat format_;

    /// Texture usage type.
    TextureUsage usage_;
    /// Current mip levels.
    unsigned levels_;
    /// Requested mip levels.
    unsigned requestedLevels_;
    /// Texture width.
    int width_;
    /// Texture height.
    int height_;
    /// Texture depth.
    int depth_;
    /// Shadow compare mode.
    bool shadowCompare_;
    /// Filtering mode.
    TextureFilterMode filterMode_;
    /// Addressing mode.
    TextureAddressMode addressMode_[MAX_COORDS];
    /// Texture anisotropy level.
    unsigned anisotropy_;
    /// Mip levels to skip when loading per texture quality setting.
    unsigned mipsToSkip_[MAX_TEXTURE_QUALITY_LEVELS];
    /// Border color.
    Color borderColor_;
    /// Multisampling level.
    int multiSample_;
    /// sRGB sampling and writing mode flag.
    bool sRGB_;
    /// Parameters dirty flag.
    bool parametersDirty_;
    /// Multisampling autoresolve flag.
    bool autoResolve_;
    /// Multisampling resolve needed -flag.
    bool resolveDirty_;
    /// Mipmap levels regeneration needed -flag.
    bool levelsDirty_;
    /// Backup texture.
    SharedPtr<Texture> backupTexture_;
};

}
