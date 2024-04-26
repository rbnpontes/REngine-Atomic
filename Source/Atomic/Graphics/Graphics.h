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

#include "../Container/ArrayPtr.h"
#include "../Container/HashSet.h"
#include "../Core/Mutex.h"
#include "../Core/Object.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Graphics/ShaderVariation.h"
#include "../Math/Color.h"
#include "../Math/Plane.h"
#include "../Math/Rect.h"
#include "../Resource/Image.h"
#include "./DrawCommand.h"

struct SDL_Window;

namespace REngine
{
    class DriverInstance;
    class ShaderProgram;
    class VertexDeclaration;
}

namespace Atomic
{

class ConstantBuffer;
class File;
class Image;
class IndexBuffer;
class GPUObject;
class RenderSurface;
class Shader;
class ShaderPrecache;
class ShaderVariation;
class Texture;
class Texture2D;
class Texture2DArray;
class TextureCube;
class Vector3;
class Vector4;
class VertexBuffer;
class RenderTexture;

struct ShaderParameter;
    
/// CPU-side scratch buffer for vertex data updates.
struct ScratchBuffer
{
    ScratchBuffer() :
        size_(0),
        reserved_(false)
    {
    }

    /// Buffer data.
    SharedArrayPtr<unsigned char> data_;
    /// Data size.
    unsigned size_;
    /// Reserved flag.
    bool reserved_;
};

/// %Graphics subsystem. Manages the application window, rendering state and GPU resources.
class ATOMIC_API Graphics : public Object
{
    ATOMIC_OBJECT(Graphics, Object);

public:
    /// Construct.
    Graphics(Context* context);
    /// Destruct. Release the Direct3D11 device and close the window.
    virtual ~Graphics();

    /// Set external window handle. Only effective before setting the initial screen mode.
    void SetExternalWindow(void* window);
    /// Set window title.
    void SetWindowTitle(const String& windowTitle);
    /// Set window icon.
    void SetWindowIcon(Image* windowIcon);
    /// Set window position. Sets initial position if window is not created yet.
    void SetWindowPosition(const IntVector2& position);
    /// Set window position. Sets initial position if window is not created yet.
    void SetWindowPosition(int x, int y);
    /// Set screen mode. Return true if successful.
    bool SetMode
        (int width, int height, bool fullscreen, bool borderless, bool resizable, bool highDPI, bool vsync, bool tripleBuffer,
            int multiSample, int monitor, int refreshRate);
    /// Set screen resolution only. Return true if successful.
    bool SetMode(int width, int height);
    /// Set whether the main window uses sRGB conversion on write.
    void SetSRGB(bool enable);
    /// Set whether rendering output is dithered. Default true on OpenGL. No effect on Direct3D.
    void SetDither(bool enable);
    /// Set whether to flush the GPU command buffer to prevent multiple frames being queued and uneven frame timesteps. Default off, may decrease performance if enabled. Not currently implemented on OpenGL.
    void SetFlushGPU(bool enable);
    /// Set forced use of OpenGL 2 even if OpenGL 3 is available. Must be called before setting the screen mode for the first time. Default false. No effect on Direct3D9 & 11.
    void SetForceGL2(bool enable);
    /// Set allowed screen orientations as a space-separated list of "LandscapeLeft", "LandscapeRight", "Portrait" and "PortraitUpsideDown". Affects currently only iOS platform.
    void SetOrientations(const String& orientations);
    /// Toggle between full screen and windowed mode. Return true if successful.
    bool ToggleFullscreen();
    /// Close the window.
    void Close();
    /// Take a screenshot. Return true if successful.
    bool TakeScreenShot(Image* destImage);  // ATOMIC FIX: Image* must be a pointer type (for script bindings)
    /// Begin frame rendering. Return true if device available and can render.
    bool BeginFrame();
    /// End frame rendering and swap buffers.
    void EndFrame();
    /// Clear any or all of rendertarget, depth buffer and stencil buffer.
    void Clear(unsigned flags, const Color& color = Color(0.0f, 0.0f, 0.0f, 0.0f), float depth = 1.0f, unsigned stencil = 0) const;
    /// Resolve multisampled backbuffer to a texture rendertarget. The texture's size should match the viewport size.
    bool ResolveToTexture(Texture2D* destination, const IntRect& viewport) const;
    /// Resolve a multisampled texture on itself.
    bool ResolveToTexture(Texture2D* texture) const;
    /// Resolve a multisampled cube texture on itself.
    bool ResolveToTexture(TextureCube* texture) const;
    /// Draw non-indexed geometry.
    void Draw(PrimitiveType type, unsigned vertexStart, unsigned vertexCount) const;
    /// Draw indexed geometry.
    void Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex, unsigned vertexCount) const;
    /// Draw indexed geometry with vertex index offset.
    void Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex, unsigned minVertex, unsigned vertexCount) const;
    /// Draw indexed, instanced geometry. An instancing vertex buffer must be set.
    void DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex, unsigned vertexCount,
        unsigned instanceCount) const;
    /// Draw indexed, instanced geometry with vertex index offset.
    void DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex, unsigned minVertex,
        unsigned vertexCount, unsigned instanceCount) const;
    /// Set vertex buffer.
    void SetVertexBuffer(VertexBuffer* buffer) const;
    /// Set multiple vertex buffers.
    bool SetVertexBuffers(const PODVector<VertexBuffer*>& buffers, unsigned instanceOffset = 0) const;
    /// Set multiple vertex buffers.
    bool SetVertexBuffers(const Vector<SharedPtr<VertexBuffer> >& buffers, unsigned instanceOffset = 0) const;
    /// Set index buffer.
    void SetIndexBuffer(IndexBuffer* buffer) const;
    /// Set shaders.
    void SetShaders(ShaderVariation* vs, ShaderVariation* ps) const;
    /// Set shader float constants.
    void SetShaderParameter(StringHash param, const float* data, unsigned count);
    /// Set shader float constant.
    void SetShaderParameter(StringHash param, float value);
    /// Set shader integer constant.
    void SetShaderParameter(StringHash param, int value);
    /// Set shader boolean constant.
    void SetShaderParameter(StringHash param, bool value);
    /// Set shader color constant.
    void SetShaderParameter(StringHash param, const Color& color);
    /// Set shader 2D vector constant.
    void SetShaderParameter(StringHash param, const Vector2& vector);
    /// Set shader 3x3 matrix constant.
    void SetShaderParameter(StringHash param, const Matrix3& matrix);
    /// Set shader 3D vector constant.
    void SetShaderParameter(StringHash param, const Vector3& vector);
    /// Set shader 4x4 matrix constant.
    void SetShaderParameter(StringHash param, const Matrix4& matrix);
    /// Set shader 4D vector constant.
    void SetShaderParameter(StringHash param, const Vector4& vector);
    /// Set shader 3x4 matrix constant.
    void SetShaderParameter(StringHash param, const Matrix3x4& matrix);
    /// Set shader constant from a variant. Supported variant types: bool, float, vector2, vector3, vector4, color.
    void SetShaderParameter(StringHash param, const Variant& value);
    /// Check whether a shader parameter group needs update. Does not actually check whether parameters exist in the shaders.
	bool NeedParameterUpdate(ShaderParameterGroup group, const void* source);
    /// Check whether a shader parameter exists on the currently set shaders.
    bool HasShaderParameter(StringHash param) const;
    /// Check whether the current vertex or pixel shader uses a texture unit.
    bool HasTextureUnit(TextureUnit unit) const;
    /// Clear remembered shader parameter source group.
    void ClearParameterSource(ShaderParameterGroup group);
    /// Clear remembered shader parameter sources.
    void ClearParameterSources();
    /// Clear remembered transform shader parameter sources.
    void ClearTransformSources();
    /// Set texture.
    void SetTexture(u32 index, Texture* texture) const;
    /// Set render texture
    void SetTexture(u32 index, RenderTexture* texture) const;
    /// Bind texture unit 0 for update. Called by Texture. Used only on OpenGL.
    void SetTextureForUpdate(Texture* texture);
    /// Dirty texture parameters of all textures (when global settings change.)
    void SetTextureParametersDirty();
    /// Set default texture filtering mode. Called by Renderer before rendering.
    void SetDefaultTextureFilterMode(TextureFilterMode mode);
    /// Set default texture anisotropy level. Called by Renderer before rendering.
    void SetDefaultTextureAnisotropy(unsigned level);
    /// Reset all rendertargets, depth-stencil surface and viewport.
    void ResetRenderTargets();
    /// Reset specific rendertarget.
    void ResetRenderTarget(unsigned index) const;
    /// Reset depth-stencil surface.
    void ResetDepthStencil() const;
    /// Reset texture
    void ResetTexture(u32 index) const;
    /// Set rendertarget.
    void SetRenderTarget(unsigned index, RenderSurface* renderTarget) const;
    /// Set rendertarget.
    void SetRenderTarget(unsigned index, Texture2D* texture) const;
    /// Set rendertarget.
    void SetRenderTarget(unsigned index, RenderTexture* texture) const;
    /// Set depth-stencil surface.
    void SetDepthStencil(RenderSurface* depthStencil) const;
    /// Set depth-stencil surface.
    void SetDepthStencil(Texture2D* texture) const;
    /// Set viewport.
    void SetViewport(const IntRect& rect) const;
    /// Set blending and alpha-to-coverage modes. Alpha-to-coverage is not supported on Direct3D9.
    void SetBlendMode(BlendMode mode, bool alphaToCoverage = false) const;
    /// Set color write on/off.
    void SetColorWrite(bool enable) const;
    /// Set hardware culling mode.
    void SetCullMode(CullMode mode) const;
    /// Set depth bias.
    void SetDepthBias(float constantBias, float slopeScaledBias) const;
    /// Set depth compare.
    void SetDepthTest(CompareMode mode) const;
    /// Set depth write on/off.
    void SetDepthWrite(bool enable) const;
    /// Set polygon fill mode.
    void SetFillMode(FillMode mode) const;
    /// Set line antialiasing on/off.
    void SetLineAntiAlias(bool enable) const;
    /// Set scissor test.
    void SetScissorTest(bool enable, const Rect& rect = Rect::FULL, bool borderInclusive = true) const;
    /// Set scissor test.
    void SetScissorTest(bool enable, const IntRect& rect) const;
    /// Set stencil test.
    void SetStencilTest
        (bool enable, CompareMode mode = CMP_ALWAYS, StencilOp pass = OP_KEEP, StencilOp fail = OP_KEEP, StencilOp zFail = OP_KEEP,
            unsigned stencilRef = 0, unsigned compareMask = M_MAX_UNSIGNED, unsigned writeMask = M_MAX_UNSIGNED) const;
    /// Set a custom clipping plane. The plane is specified in world space, but is dependent on the view and projection matrices.
    void SetClipPlane(bool enable, const Plane& clipPlane = Plane::UP, const Matrix3x4& view = Matrix3x4::IDENTITY,
        const Matrix4& projection = Matrix4::IDENTITY) const;
    /// Begin dumping shader variation names to an XML file for precaching.
    void BeginDumpShaders(const String& fileName);
    /// End dumping shader variations names.
    void EndDumpShaders();
    /// Precache shader variations from an XML file generated with BeginDumpShaders().
    void PrecacheShaders(Deserializer& source);
    /// Set shader cache directory, Direct3D only. This can either be an absolute path or a path within the resource system.
    void SetShaderCacheDir(const String& path);

    /// Return whether rendering initialized.
    bool IsInitialized() const;

    /// Return graphics implementation, which holds the actual API-specific resources.
    REngine::DriverInstance* GetImpl() const { return impl_; }

    ea::shared_ptr<IDrawCommand> GetDrawCommand() const { return draw_command_; }

    GraphicsBackend GetBackend() const;

    /// Return OS-specific external window handle. Null if not in use.
    void* GetExternalWindow() const { return externalWindow_; }

    /// Return SDL window.
    SDL_Window* GetWindow() const { return window_; }

    /// Return window title.
    const String& GetWindowTitle() const { return windowTitle_; }

    /// Return graphics API name.
    const String& GetApiName() const { return apiName_; }

    /// Return window position.
    IntVector2 GetWindowPosition() const;

    /// Return window width in pixels.
    int GetWidth() const { return width_; }

    /// Return window height in pixels.
    int GetHeight() const { return height_; }

    /// Return multisample mode (1 = no multisampling.)
    int GetMultiSample() const { return multiSample_; }

    /// Return window size in pixels.
    IntVector2 GetSize() const { return IntVector2(width_, height_); }

    /// Return whether window is fullscreen.
    bool GetFullscreen() const { return fullscreen_; }

    /// Return whether window is borderless.
    bool GetBorderless() const { return borderless_; }

    /// Return whether window is resizable.
    bool GetResizable() const { return resizable_; }

    /// Return whether window is high DPI.
    bool GetHighDPI() const { return highDPI_; }

    /// Return whether vertical sync is on.
    bool GetVSync() const { return vsync_; }

    /// Return refresh rate when using vsync in fullscreen
    int GetRefreshRate() const { return refreshRate_; }

    /// Return the current monitor index. Effective on in fullscreen
    int GetMonitor() const { return monitor_; }

    /// Return whether triple buffering is enabled.
    bool GetTripleBuffer() const { return tripleBuffer_; }

    /// Return whether the main window is using sRGB conversion on write.
    bool GetSRGB() const { return sRGB_; }
    
    /// Return whether rendering output is dithered.
    bool GetDither() const;

    /// Return whether the GPU command buffer is flushed each frame.
    bool GetFlushGPU() const { return flushGPU_; }

    /// Return whether OpenGL 2 use is forced. Effective only on OpenGL.
    bool GetForceGL2() const { return forceGL2_; }

    /// Return allowed screen orientations.
    const String& GetOrientations() const { return orientations_; }

    /// Return whether graphics context is lost and can not render or load GPU resources.
    bool IsDeviceLost() const;

    /// Return number of primitives drawn this frame.
    unsigned GetNumPrimitives() const
    {
        if (!draw_command_)
            return 0;
        return draw_command_->GetPrimitiveCount();
    }

    /// Return number of batches drawn this frame.
    unsigned GetNumBatches() const
    {
        if (!draw_command_)
            return 0;
	    return draw_command_->GetNumBatches();
    }

    /// Return dummy color texture format for shadow maps. Is "NULL" (consume no video memory) if supported.
    TextureFormat GetDummyColorFormat() const { return dummyColorFormat_; }

    /// Return shadow map depth texture format, or 0 if not supported.
    TextureFormat GetShadowMapFormat() const { return shadowMapFormat_; }

    /// Return 24-bit shadow map depth texture format, or 0 if not supported.
    TextureFormat GetHiresShadowMapFormat() const { return hiresShadowMapFormat_; }

    /// Return whether hardware instancing is supported.
    bool GetInstancingSupport() const { return instancingSupport_; }

    /// Return whether light pre-pass rendering is supported.
    bool GetLightPrepassSupport() const { return lightPrepassSupport_; }

    /// Return whether deferred rendering is supported.
    bool GetDeferredSupport() const { return deferredSupport_; }

    /// Return whether anisotropic texture filtering is supported.
    bool GetAnisotropySupport() const { return anisotropySupport_; }

    /// Return whether shadow map depth compare is done in hardware.
    bool GetHardwareShadowSupport() const { return hardwareShadowSupport_; }

    /// Return whether a readable hardware depth format is available.
    bool GetReadableDepthSupport() const { return GetReadableDepthFormat() != 0; }

    /// Return whether sRGB conversion on texture sampling is supported.
    bool GetSRGBSupport() const { return sRGBSupport_; }

    /// Return whether sRGB conversion on rendertarget writing is supported.
    bool GetSRGBWriteSupport() const { return sRGBWriteSupport_; }

    /// Return supported fullscreen resolutions (third component is refreshRate). Will be empty if listing the resolutions is not supported on the platform (e.g. Web).
    PODVector<IntVector3> GetResolutions(int monitor) const;
    /// Return supported multisampling levels.
    PODVector<int> GetMultiSampleLevels() const;
    /// Return the desktop resolution.
    IntVector2 GetDesktopResolution(int monitor) const;
    /// Return the number of currently connected monitors.
    int GetMonitorCount() const;
    /// Return hardware format for a compressed image format, or 0 if unsupported.
    TextureFormat GetFormat(CompressedFormat format) const;
    /// Return a shader variation by name and defines.
    ShaderVariation* GetShader(ShaderType type, const String& name, const String& defines = String::EMPTY) const;
    /// Return a shader variation by name and defines.
    ShaderVariation* GetShader(ShaderType type, const char* name, const char* defines) const;
    /// Return current vertex buffer by index.
    VertexBuffer* GetVertexBuffer(unsigned index) const;

    /// Return current index buffer.
    IndexBuffer* GetIndexBuffer() const
    {
        if (!draw_command_)
            return nullptr;
        return draw_command_->GetIndexBuffer();
    }

    /// Return current vertex shader.
    ShaderVariation* GetVertexShader() const
    {
	    if(!draw_command_)
			return nullptr;
		return draw_command_->GetShader(VS);
    }

    /// Return current pixel shader.
    ShaderVariation* GetPixelShader() const
    {
	    if(!draw_command_)
            return nullptr;
        return draw_command_->GetShader(PS);
    }

    /// Return shader program. This is an API-specific class and should not be used by applications.
    REngine::ShaderProgram* GetShaderProgram() const
    {
	    if(!draw_command_)
            return nullptr;
		return draw_command_->GetShaderProgram();
    }
    /// Return texture unit index by name.
    static TextureUnit GetTextureUnit(const String& name);
    /// Return texture unit name by index.
    const String& GetTextureUnitName(TextureUnit unit);
    /// Return current texture by texture unit index.
    Texture* GetTexture(unsigned index) const;

    /// Return default texture filtering mode.
    TextureFilterMode GetDefaultTextureFilterMode() const { return defaultTextureFilterMode_; }

    /// Return default texture max. anisotropy level.
    unsigned GetDefaultTextureAnisotropy() const { return defaultTextureAnisotropy_; }

    /// Return current rendertarget by index.
    RenderSurface* GetRenderTarget(unsigned index) const;

    /// Return current depth-stencil surface.
    RenderSurface* GetDepthStencil() const
    {
        if(!draw_command_)
            return nullptr;
    	return draw_command_->GetDepthStencil();
    }

    /// Return the viewport coordinates.
    IntRect GetViewport() const
    {
	    if(!draw_command_)
			return IntRect::ZERO;
		return draw_command_->GetViewport();
    }

    /// Return blending mode.
    BlendMode GetBlendMode() const
    {
	    if(!draw_command_)
            return BLEND_REPLACE;
        return draw_command_->GetBlendMode();
    }

    /// Return whether alpha-to-coverage is enabled.
    bool GetAlphaToCoverage() const
    {
	    if(!draw_command_)
			return false;
        return draw_command_->GetAlphaToCoverage();
    }

    /// Return whether color write is enabled.
    bool GetColorWrite() const
    {
        if (!draw_command_)
            return false;
        return draw_command_->GetColorWrite();
    }

    /// Return hardware culling mode.
    CullMode GetCullMode() const
    {
	    if(!draw_command_)
			return CULL_NONE;
		return draw_command_->GetCullMode();
    }

    /// Return depth constant bias.
    float GetDepthConstantBias() const
    {
	    if(!draw_command_)
            return 0.0f;
        return draw_command_->GetDepthBias();
    }

    /// Return depth slope scaled bias.
    float GetDepthSlopeScaledBias() const
    {
        if (!draw_command_)
            return 0.0f;
        return draw_command_->GetSlopeScaledDepthBias();
    }

    /// Return depth compare mode.
    CompareMode GetDepthTest() const
    {
	    if(!draw_command_)
			return CMP_ALWAYS;
		return draw_command_->GetDepthTest();
    }

    /// Return whether depth write is enabled.
    bool GetDepthWrite() const
    {
	    if(!draw_command_)
            return false;
        return draw_command_->GetDepthWrite();
    }

    /// Return polygon fill mode.
    FillMode GetFillMode() const
    {
	    if(!draw_command_)
            return FILL_SOLID;
        return draw_command_->GetFillMode();
    }

    /// Return whether line antialiasing is enabled.
    bool GetLineAntiAlias() const
    {
	    if(!draw_command_)
			return false;
		return draw_command_->GetLineAntiAlias();
    }

    /// Return whether stencil test is enabled.
    bool GetStencilTest() const
    {
        if (!draw_command_)
            return false;
        return draw_command_->GetStencilTest();
    }

    /// Return whether scissor test is enabled.
    bool GetScissorTest() const
    {
        if (!draw_command_)
            return false;
        return draw_command_->GetScissorTest();
    }

    /// Return scissor rectangle coordinates.
    const IntRect& GetScissorRect() const
    {
        if (!draw_command_)
            return {};
        return draw_command_->GetScissorRect();
    }

    /// Return stencil compare mode.
    CompareMode GetStencilTestMode() const
    {
	    if(!draw_command_)
            return CMP_ALWAYS;
        return draw_command_->GetStencilTestMode();
    }

    /// Return stencil operation to do if stencil test passes.
    StencilOp GetStencilPass() const
    {
	    if(!draw_command_)
            return OP_KEEP;
		return draw_command_->GetStencilPass();
    }

    /// Return stencil operation to do if stencil test fails.
    StencilOp GetStencilFail() const
    {
	    if(!draw_command_)
			return OP_KEEP;
        return draw_command_->GetStencilFail();
    }

    /// Return stencil operation to do if depth compare fails.
    StencilOp GetStencilZFail() const
    {
	    if(!draw_command_)
            return OP_KEEP;
        return draw_command_->GetStencilZFail();
    }

    /// Return stencil reference value.
    unsigned GetStencilRef() const
    {
	    if(!draw_command_)
			return 0;
		return draw_command_->GetStencilRef();
    }

    /// Return stencil compare bitmask.
    unsigned GetStencilCompareMask() const
    {
        if (!draw_command_)
            return 0;
        return draw_command_->GetStencilCompareMask();
    }

    /// Return stencil write bitmask.
    unsigned GetStencilWriteMask() const
    {
        if(!draw_command_)
            return 0;
    	return draw_command_->GetStencilWriteMask();
    }

    /// Return whether a custom clipping plane is in use.
    bool GetUseClipPlane() const
    {
        if (!draw_command_)
            return false;
        return draw_command_->GetClipPlane();
    }

    /// Return shader cache directory, Direct3D only.
    const String& GetShaderCacheDir() const { return shaderCacheDir_; }

    /// Return current rendertarget width and height.
    IntVector2 GetRenderTargetDimensions() const;

    /// Window was resized through user interaction. Called by Input subsystem.
    void OnWindowResized();
    /// Window was moved through user interaction. Called by Input subsystem.
    void OnWindowMoved();
    /// Restore GPU objects and reinitialize state. Requires an open window. Used only on OpenGL.
    void Restore();
    /// Maximize the window.
    void Maximize();
    /// Minimize the window.
    void Minimize();
    /// Add a GPU object to keep track of. Called by GPUObject.
    void AddGPUObject(GPUObject* object);
    /// Remove a GPU object. Called by GPUObject.
    void RemoveGPUObject(GPUObject* object);
    /// Reserve a CPU-side scratch buffer.
    void* ReserveScratchBuffer(unsigned size);
    /// Free a CPU-side scratch buffer.
    void FreeScratchBuffer(void* buffer);
    /// Clean up too large scratch buffers.
    void CleanupScratchBuffers();
    /// Clean up shader parameters when a shader variation is released or destroyed.
    void CleanupShaderPrograms(ShaderVariation* variation);
    /// Clean up a render surface from all FBOs. Used only on OpenGL.
    void CleanupRenderSurface(RenderSurface* surface);
    void Cleanup(GraphicsClearFlags flags);
    /// Get or create a constant buffer. Will be shared between shaders if possible.
    ConstantBuffer* GetOrCreateConstantBuffer(ShaderType type, unsigned index, unsigned size) const;
    /// Mark the FBO needing an update. Used only on OpenGL.
    void MarkFBODirty();
    /// Bind a VBO, avoiding redundant operation. Used only on OpenGL.
    void SetVBO(unsigned object);
    /// Bind a UBO, avoiding redundant operation. Used only on OpenGL.
    void SetUBO(unsigned object);

    /// Return the API-specific alpha texture format.
    static TextureFormat GetAlphaFormat();
    /// Return the API-specific luminance texture format.
    static TextureFormat GetLuminanceFormat();
    /// Return the API-specific luminance alpha texture format.
    static TextureFormat GetLuminanceAlphaFormat();
    /// Return the API-specific RGB texture format.
    static TextureFormat GetRGBFormat();
    /// Return the API-specific RGBA texture format.
    static TextureFormat GetRGBAFormat();
    /// Return the API-specific RGBA 16-bit texture format.
    static TextureFormat GetRGBA16Format();
    /// Return the API-specific RGBA 16-bit float texture format.
    static TextureFormat GetRGBAFloat16Format();
    /// Return the API-specific RGBA 32-bit float texture format.
    static TextureFormat GetRGBAFloat32Format();
    /// Return the API-specific RG 16-bit texture format.
    static TextureFormat GetRG16Format();
    /// Return the API-specific RG 16-bit float texture format.
    static TextureFormat GetRGFloat16Format();
    /// Return the API-specific RG 32-bit float texture format.
    static TextureFormat GetRGFloat32Format();
    /// Return the API-specific single channel 16-bit float texture format.
    static TextureFormat GetFloat16Format();
    /// Return the API-specific single channel 32-bit float texture format.
    static TextureFormat GetFloat32Format();
    /// Return the API-specific linear depth texture format.
    static TextureFormat GetLinearDepthFormat();
    /// Return the API-specific hardware depth-stencil texture format.
    static TextureFormat GetDepthStencilFormat();
    /// Return the API-specific readable hardware depth format, or 0 if not supported.
    static TextureFormat GetReadableDepthFormat();
    /// Return the API-specific texture format from a textual description, for example "rgb".
    static TextureFormat GetFormat(const String& formatName);

    /// Return UV offset required for pixel perfect rendering.
    static const Vector2& GetPixelUVOffset() { return pixelUVOffset; }

    /// Return maximum number of supported bones for skinning.
    static unsigned GetMaxBones();
    /// Return whether is using an OpenGL 3 context. Return always false on Direct3D9 & Direct3D11.
    static bool GetGL3Support();

    // ATOMIC BEGIN

    /// Get the SDL_Window as a void* to avoid having to include the graphics implementation
    void* GetSDLWindow() { return window_; }

    int GetCurrentMonitor();
    int GetNumMonitors();
    bool GetMaximized();
    IntVector2 GetMonitorResolution(int monitorId) const;
    void RaiseWindow();
    
    /// Return number of passes drawn this frame
    static unsigned GetNumPasses() { return numPasses_; }
    /// Set number of passes drawn this frame
    static void SetNumPasses(unsigned value) { numPasses_ = value; }

    /// Return number of single render pass primitives drawn this frame (D3D9 Only)
    static unsigned GetSinglePassPrimitives() { return numSinglePassPrimitives_; }
    /// Set number of single render pass primitives drawn this frame (D3D9 Only)
    static void SetSinglePassPrimitives(unsigned value) { numSinglePassPrimitives_ = value; }
  
    // ATOMIC END

private:
    /// Create the application window.
    bool OpenWindow(int width, int height, bool resizable, bool borderless);
    /// Create the application window icon.
    void CreateWindowIcon();
    /// Adjust the window for new resolution and fullscreen mode.
    void AdjustWindow(int& newWidth, int& newHeight, bool& newFullscreen, bool& newBorderless, int& monitor);
    /// Create the Direct3D11 device and swap chain. Requires an open window. Can also be called again to recreate swap chain. Return true on success.
    bool CreateDevice(int width, int height, int multiSample);
    /// Update Direct3D11 swap chain state for a new mode and create views for the backbuffer & default depth buffer. Return true on success.
    bool UpdateSwapChain(int width, int height);
    /// Create the Direct3D9 interface.
    bool CreateInterface();
    /// Create the Direct3D9 device.
    bool CreateDevice(unsigned adapter, unsigned deviceType);
    /// Reset the Direct3D9 device.
    void ResetDevice();
    /// Notify all GPU resources so they can release themselves as needed. Used only on Direct3D9.
    void OnDeviceLost();
    /// Notify all GPU resources so they can recreate themselves as needed. Used only on Direct3D9.
    void OnDeviceReset();
    /// Set vertex buffer stream frequency. Used only on Direct3D9.
    void SetStreamFrequency(unsigned index, unsigned frequency);
    /// Reset stream frequencies. Used only on Direct3D9.
    void ResetStreamFrequencies();
    /// Check supported rendering features.
    void CheckFeatureSupport();
    /// Reset cached rendering state.
    void ResetCachedState();
    /// Initialize texture unit mappings.
    void SetTextureUnitMappings();
    /// Process dirtied state before draw.
    void PrepareDraw();
    /// Create intermediate texture for multisampled backbuffer resolve. No-op if already exists.
    void CreateResolveTexture();
    /// Clean up all framebuffers. Called when destroying the context. Used only on OpenGL.
    void CleanupFramebuffers();
    /// Create a framebuffer using either extension or core functionality. Used only on OpenGL.
    unsigned CreateFramebuffer();
    /// Delete a framebuffer using either extension or core functionality. Used only on OpenGL.
    void DeleteFramebuffer(unsigned fbo);
    /// Bind a framebuffer using either extension or core functionality. Used only on OpenGL.
    void BindFramebuffer(unsigned fbo);
    /// Bind a framebuffer color attachment using either extension or core functionality. Used only on OpenGL.
    void BindColorAttachment(unsigned index, unsigned target, unsigned object, bool isRenderBuffer);
    /// Bind a framebuffer depth attachment using either extension or core functionality. Used only on OpenGL.
    void BindDepthAttachment(unsigned object, bool isRenderBuffer);
    /// Bind a framebuffer stencil attachment using either extension or core functionality. Used only on OpenGL.
    void BindStencilAttachment(unsigned object, bool isRenderBuffer);
    /// Check FBO completeness using either extension or core functionality. Used only on OpenGL.
    bool CheckFramebuffer();
    /// Set vertex attrib divisor. No-op if unsupported. Used only on OpenGL.
    void SetVertexAttribDivisor(unsigned location, unsigned divisor);
    /// Release/clear GPU objects and optionally close the window. Used only on OpenGL.
    void Release(bool clearGPUObjects, bool closeWindow);

    /// Mutex for accessing the GPU objects vector from several threads.
    Mutex gpuObjectMutex_;
    /// Implementation.
    REngine::DriverInstance* impl_;
    /// SDL window.
    SDL_Window* window_;
    /// Window title.
    String windowTitle_;
    /// Window icon image.
    WeakPtr<Image> windowIcon_;
    /// External window, null if not in use (default.)
    void* externalWindow_;
    /// Window width in pixels.
    int width_;
    /// Window height in pixels.
    int height_;
    /// Window position.
    IntVector2 position_;
    /// Multisampling mode.
    int multiSample_;
    /// Fullscreen flag.
    bool fullscreen_;
    /// Borderless flag.
    bool borderless_;
    /// Resizable flag.
    bool resizable_;
    /// High DPI flag.
    bool highDPI_;
    /// Vertical sync flag.
    bool vsync_;
    /// Refresh rate in Hz. Only used in fullscreen, 0 when windowed
    int refreshRate_;
    /// Monitor index. Only used in fullscreen, 0 when windowed
    int monitor_;
    /// Triple buffering flag.
    bool tripleBuffer_;
    /// Flush GPU command buffer flag.
    bool flushGPU_;
    /// Force OpenGL 2 flag. Only used on OpenGL.
    bool forceGL2_;
    /// sRGB conversion on write flag for the main window.
    bool sRGB_;
    /// Light pre-pass rendering support flag.
    bool lightPrepassSupport_;
    /// Deferred rendering support flag.
    bool deferredSupport_;
    /// Anisotropic filtering support flag.
    bool anisotropySupport_;
    /// DXT format support flag.
    bool dxtTextureSupport_;
    /// ETC1 format support flag.
    bool etcTextureSupport_;
    /// PVRTC formats support flag.
    bool pvrtcTextureSupport_;
    /// Hardware shadow map depth compare support flag.
    bool hardwareShadowSupport_;
    /// Instancing support flag.
    bool instancingSupport_;
    /// sRGB conversion on read support flag.
    bool sRGBSupport_;
    /// sRGB conversion on write support flag.
    bool sRGBWriteSupport_;
    /// Largest scratch buffer request this frame.
    unsigned maxScratchBufferRequest_;
    /// GPU objects.
    PODVector<GPUObject*> gpuObjects_;
    /// Scratch buffers.
    Vector<ScratchBuffer> scratchBuffers_;
    /// Shadow map dummy color texture format.
    TextureFormat dummyColorFormat_;
    /// Shadow map depth texture format.
    TextureFormat shadowMapFormat_;
    /// Shadow map 24-bit depth texture format.
    TextureFormat hiresShadowMapFormat_;
    /// Default texture filtering mode.
    TextureFilterMode defaultTextureFilterMode_;
    /// Default texture max. anisotropy level.
    unsigned defaultTextureAnisotropy_;
    /// Base directory for shaders.
    String shaderPath_;
    /// Cache directory for Direct3D binary shaders.
    String shaderCacheDir_;
    /// File extension for shaders.
    String shaderExtension_;
    /// Last used shader in shader variation query.
    mutable WeakPtr<Shader> lastShader_;
    /// Last used shader name in shader variation query.
    mutable String lastShaderName_;
    /// Shader precache utility.
    SharedPtr<ShaderPrecache> shaderPrecache_;
    /// Allowed screen orientations.
    String orientations_;
    /// Graphics API name.
    String apiName_;

    ea::shared_ptr<IDrawCommand> draw_command_;

    /// Pixel perfect UV offset.
    static const Vector2 pixelUVOffset;

// ATOMIC BEGIN
    static unsigned numPasses_;
    static unsigned numSinglePassPrimitives_;
// ATOMIC END
};

/// Register Graphics library objects.
void ATOMIC_API RegisterGraphicsLibrary(Context* context);

}
