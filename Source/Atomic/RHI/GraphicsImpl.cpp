#include "../Precompiled.h"


#include "../Core/Context.h"
#include "../Core/ProcessUtils.h"
#include "../Core/Profiler.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/Geometry.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ShaderPrecache.h"
#include "../Graphics/Texture2D.h"
#include "../Graphics/TextureCube.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"

// RHI BEGIN
#include "./DriverInstance.h"
#include "./GraphicsState.h"
#include "./RenderCommand.h"
#include "./VertexDeclaration.h"
#include "./DiligentUtils.h"
// RHI END

#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/TextureView.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

// ATOMIC BEGIN
#include <SDL/include/SDL.h>
#include <SDL/include/SDL_syswm.h>
// ATOMIC END


#include "../DebugNew.h"
#include "Graphics/DrawCommandQueue.h"

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

#if WIN32
// Prefer the high-performance GPU on switchable GPU systems
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
namespace Atomic
{
#if WIN32
	static HWND GetWindowHandle(SDL_Window* window)
	{
		SDL_SysWMinfo sysInfo;

		SDL_VERSION(&sysInfo.version);
		SDL_GetWindowWMInfo(window, &sysInfo);
		return sysInfo.info.win.window;
	}
#endif

	const Vector2 Graphics::pixelUVOffset(0.0f, 0.0f);

	Graphics::Graphics(Context* context) :
		Object(context),
		impl_(new REngine::DriverInstance(this)),
		window_(0),
		externalWindow_(0),
		width_(0),
		height_(0),
		position_(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED),
		multiSample_(1),
		fullscreen_(false),
		borderless_(false),
		resizable_(false),
		highDPI_(false),
		vsync_(false),
		monitor_(0),
		refreshRate_(0),
		tripleBuffer_(false),
		flushGPU_(false),
		forceGL2_(false),
		sRGB_(false),
		anisotropySupport_(false),
		dxtTextureSupport_(false),
		etcTextureSupport_(false),
		pvrtcTextureSupport_(false),
		hardwareShadowSupport_(false),
		lightPrepassSupport_(false),
		deferredSupport_(false),
		instancingSupport_(false),
		sRGBSupport_(false),
		sRGBWriteSupport_(false),
		maxScratchBufferRequest_(0),
		defaultTextureFilterMode_(FILTER_TRILINEAR),
		defaultTextureAnisotropy_(4),
		// TODO: use Shaders instead of Shaders/SpirV
		shaderPath_("Shaders/SpirV/"),
		shaderExtension_(".glsl"),
		orientations_("LandscapeLeft LandscapeRight"),
		apiName_("DiligentCore")
	{
		ResetCachedState();

		context_->RequireSDL(SDL_INIT_VIDEO);

		// Register Graphics library object factories
		RegisterGraphicsLibrary(context_);
	}

	Graphics::~Graphics()
	{
		//{
		//	MutexLock lock(gpuObjectMutex_);
		//	// Release all GPU objects that still exist
		//	for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
		//		(*i)->Release();
		//	gpuObjects_.Clear();
		//}

		ResetCachedState();
		Cleanup(GRAPHICS_CLEAR_ALL);

		GetSubsystem<DrawCommandQueue>()->ClearStoredCommands();
		impl_->Release();
		delete impl_;
		impl_ = nullptr;

		if (window_)
		{
			SDL_ShowCursor(SDL_TRUE);
			SDL_DestroyWindow(window_);
			window_ = nullptr;
		}

		context_->ReleaseSDL();
	}

	bool Graphics::SetMode(int width, int height, bool fullscreen, bool borderless, bool resizable, bool highDPI,
		bool vsync, bool tripleBuffer,
		int multiSample, int monitor, int refreshRate)
	{
		ATOMIC_PROFILE(SetScreenMode);

		highDPI = false; // SDL does not support High DPI mode on Windows platform yet, so always disable it for now

		bool maximize = false;

		// Make sure monitor index is not bigger than the currently detected monitors
		int monitors = SDL_GetNumVideoDisplays();
		if (monitor >= monitors || monitor < 0)
			monitor = 0; // this monitor is not present, use first monitor

		// Find out the full screen mode display format (match desktop color depth)
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(monitor, &mode);

		Diligent::TEXTURE_FORMAT fullscreen_format = SDL_BITSPERPIXEL(mode.format) == 16 ? Diligent::TEX_FORMAT_B5G6R5_UNORM : Diligent::TEX_FORMAT_RGBA8_UNORM;

		// If zero dimensions in windowed mode, set windowed mode to maximize and set a predefined default restored window size. If zero in fullscreen, use desktop mode
		if (!width || !height)
		{
			if (fullscreen || borderless)
			{
				width = mode.w;
				height = mode.h;
			}
			else
			{
				maximize = resizable;
				width = 1024;
				height = 768;
			}
		}

		// Fullscreen or Borderless can not be resizable
		if (fullscreen || borderless)
			resizable = false;

		// Borderless cannot be fullscreen, they are mutually exclusive
		if (borderless)
			fullscreen = false;

		// If nothing changes, do not reset the device
		if (width == width_ && height == height_ && fullscreen == fullscreen_ && borderless == borderless_ && resizable == resizable_ &&
			vsync == vsync_ && tripleBuffer == tripleBuffer_ && multiSample == multiSample_)
			return true;

		SDL_SetHint(SDL_HINT_ORIENTATIONS, orientations_.CString());

		if (!window_)
		{
			if (!OpenWindow(width, height, resizable, borderless))
				return false;
		}

		// Check fullscreen mode validity. Use a closest match if not found
		if (fullscreen)
		{
			PODVector<IntVector3> resolutions = GetResolutions(monitor);
			if (resolutions.Size())
			{
				unsigned best = 0;
				unsigned best_error = M_MAX_UNSIGNED;

				for (unsigned i = 0; i < resolutions.Size(); ++i)
				{
					const unsigned error = static_cast<unsigned>(Abs(resolutions[i].x_ - width) + Abs(resolutions[i].y_ - height));
					if (error < best_error)
					{
						best = i;
						best_error = error;
					}
				}

				width = resolutions[best].x_;
				height = resolutions[best].y_;
				refreshRate = resolutions[best].z_;
			}
		}

		AdjustWindow(width, height, fullscreen, borderless, monitor);
		monitor_ = monitor;
		refreshRate_ = refreshRate;

		if (maximize)
		{
			Maximize();
			SDL_GetWindowSize(window_, &width, &height);
		}

		if (!impl_->GetDevice() || multiSample_ != multiSample)
			CreateDevice(width, height, multiSample);
		UpdateSwapChain(width, height);

		fullscreen_ = fullscreen;
		borderless_ = borderless;
		resizable_ = resizable;
		highDPI_ = highDPI;
		vsync_ = vsync;
		tripleBuffer_ = tripleBuffer;

		// Clear the initial window contents to black
		Clear(CLEAR_COLOR);

		impl_->GetSwapChain()->Present(0);

#ifdef ATOMIC_LOGGING
		String msg;
		msg.AppendWithFormat("Set screen mode %dx%d %s monitor %d", width_, height_, (fullscreen_ ? "fullscreen" : "windowed"), monitor_);
		if (borderless_)
			msg.Append(" borderless");
		if (resizable_)
			msg.Append(" resizable");
		if (multiSample > 1)
			msg.AppendWithFormat(" multisample %d", multiSample);
		ATOMIC_LOGINFO(msg);
#endif

		using namespace ScreenMode;

		VariantMap& eventData = GetEventDataMap();
		eventData[P_WIDTH] = width_;
		eventData[P_HEIGHT] = height_;
		eventData[P_FULLSCREEN] = fullscreen_;
		eventData[P_BORDERLESS] = borderless_;
		eventData[P_RESIZABLE] = resizable_;
		eventData[P_HIGHDPI] = highDPI_;
		eventData[P_MONITOR] = monitor_;
		eventData[P_REFRESHRATE] = refreshRate_;
		SendEvent(E_SCREENMODE, eventData);

		return true;
	}

	bool Graphics::SetMode(int width, int height)
	{
		return SetMode(width, height, fullscreen_, borderless_, resizable_, highDPI_, vsync_, tripleBuffer_,
			multiSample_, monitor_, refreshRate_);
	}

	void Graphics::SetSRGB(bool enable)
	{
		bool newEnable = enable && sRGBWriteSupport_;
		if (newEnable != sRGB_)
		{
			sRGB_ = newEnable;
			if (impl_->GetSwapChain())
			{
				// Recreate swap chain for the new backbuffer format
				CreateDevice(width_, height_, multiSample_);
				UpdateSwapChain(width_, height_);
			}
		}
	}

	void Graphics::SetDither(bool enable)
	{
		// No effect on Direct3D11
	}

	void Graphics::SetFlushGPU(bool enable)
	{
		flushGPU_ = enable;
		if (impl_->GetSwapChain())
			impl_->GetSwapChain()->SetMaximumFrameLatency(enable ? 1 : 3);
	}

	void Graphics::SetForceGL2(bool enable)
	{
		// No effect on Direct3D11
	}

	void Graphics::Close()
	{
		if (!window_)
			return;
		SDL_ShowCursor(SDL_TRUE);
		SDL_DestroyWindow(window_);
		window_ = nullptr;
	}

	bool Graphics::TakeScreenShot(Image* destImage_)
	{
		throw std::runtime_error("Not implemented");
		//     ATOMIC_PROFILE(TakeScreenShot);
		//
		//     if (!impl_->device_)
		//         return false;
		//
		// // ATOMIC BEGIN
		//     if (!destImage_)
		//         return false;
		//     Image& destImage = *destImage_;
		// // ATOMIC END
		//
		//     D3D11_TEXTURE2D_DESC textureDesc;
		//     memset(&textureDesc, 0, sizeof textureDesc);
		//     textureDesc.Width = (UINT)width_;
		//     textureDesc.Height = (UINT)height_;
		//     textureDesc.MipLevels = 1;
		//     textureDesc.ArraySize = 1;
		//     textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//     textureDesc.SampleDesc.Count = 1;
		//     textureDesc.SampleDesc.Quality = 0;
		//     textureDesc.Usage = D3D11_USAGE_STAGING;
		//     textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		//
		//     ID3D11Texture2D* stagingTexture = 0;
		//     HRESULT hr = impl_->device_->CreateTexture2D(&textureDesc, 0, &stagingTexture);
		//     if (FAILED(hr))
		//     {
		//         ATOMIC_SAFE_RELEASE(stagingTexture);
		//         ATOMIC_LOGD3DERROR("Could not create staging texture for screenshot", hr);
		//         return false;
		//     }
		//
		//     ID3D11Resource* source = 0;
		//     impl_->defaultRenderTargetView_->GetResource(&source);
		//
		//     if (multiSample_ > 1)
		//     {
		//         // If backbuffer is multisampled, need another DEFAULT usage texture to resolve the data to first
		//         CreateResolveTexture();
		//
		//         if (!impl_->resolveTexture_)
		//         {
		//             stagingTexture->Release();
		//             source->Release();
		//             return false;
		//         }
		//
		//         impl_->deviceContext_->ResolveSubresource(impl_->resolveTexture_, 0, source, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		//         impl_->deviceContext_->CopyResource(stagingTexture, impl_->resolveTexture_);
		//     }
		//     else
		//         impl_->deviceContext_->CopyResource(stagingTexture, source);
		//
		//     source->Release();
		//
		//     D3D11_MAPPED_SUBRESOURCE mappedData;
		//     mappedData.pData = 0;
		//     hr = impl_->deviceContext_->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedData);
		//     if (FAILED(hr) || !mappedData.pData)
		//     {
		//         ATOMIC_LOGD3DERROR("Could not map staging texture for screenshot", hr);
		//         stagingTexture->Release();
		//         return false;
		//     }
		//
		//     destImage.SetSize(width_, height_, 3);
		//     unsigned char* destData = destImage.GetData();
		//     for (int y = 0; y < height_; ++y)
		//     {
		//         unsigned char* src = (unsigned char*)mappedData.pData + y * mappedData.RowPitch;
		//         for (int x = 0; x < width_; ++x)
		//         {
		//             *destData++ = *src++;
		//             *destData++ = *src++;
		//             *destData++ = *src++;
		//             ++src;
		//         }
		//     }
		//
		//     impl_->deviceContext_->Unmap(stagingTexture, 0);
		//     stagingTexture->Release();
		//     return true;
	}

	bool Graphics::BeginFrame()
	{
		if (!IsInitialized())
			return false;

		// If using an external window, check it for size changes, and reset screen mode if necessary
		if (externalWindow_)
		{
			int width, height;

			SDL_GetWindowSize(window_, &width, &height);
			if (width != width_ || height != height_)
				SetMode(width, height);
		}
		else
		{
			// To prevent a loop of endless device loss and flicker, do not attempt to render when in fullscreen
			// and the window is minimized
			if (fullscreen_ && (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED))
				return false;
		}

		if(draw_command_)
			draw_command_->Reset();
		SendEvent(E_BEGINRENDERING);
		return true;
	}

	void Graphics::EndFrame()
	{
		if (!IsInitialized())
			return;

		{
			ATOMIC_PROFILE(Present);

			SendEvent(E_ENDRENDERING);
			impl_->GetSwapChain()->Present(vsync_ ? 1 : 0);
		}

		// Clean up too large scratch buffers
		Cleanup(GRAPHICS_CLEAR_SCRATCH_BUFFERS);
	}

	void Graphics::Clear(unsigned flags, const Color& color, float depth, unsigned stencil) const
	{
		if (!draw_command_)
			return;
		draw_command_->Clear({
			flags,
			color,
			depth,
			stencil
		});
	}

	bool Graphics::ResolveToTexture(Texture2D* destination, const IntRect& viewport) const
	{
		return draw_command_->ResolveTexture(destination, viewport);
	}

	bool Graphics::ResolveToTexture(Texture2D* texture) const
	{
		// if (!texture)
		//     return false;
		// RenderSurface* surface = texture->GetRenderSurface();
		// if (!surface)
		//     return false;
		//
		// texture->SetResolveDirty(false);
		// surface->SetResolveDirty(false);
		// ID3D11Resource* source = (ID3D11Resource*)texture->GetGPUObject();
		// ID3D11Resource* dest = (ID3D11Resource*)texture->GetResolveTexture();
		// if (!source || !dest)
		//     return false;
		//
		// impl_->deviceContext_->ResolveSubresource(dest, 0, source, 0, (DXGI_FORMAT)texture->GetFormat());
		// return true;
		return draw_command_->ResolveTexture(texture);
	}

	bool Graphics::ResolveToTexture(TextureCube* texture) const
	{
		// if (!texture)
		//     return false;
		//
		// texture->SetResolveDirty(false);
		// ID3D11Resource* source = (ID3D11Resource*)texture->GetGPUObject();
		// ID3D11Resource* dest = (ID3D11Resource*)texture->GetResolveTexture();
		// if (!source || !dest)
		//     return false;
		//
		// for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
		// {
		//     // Resolve only the surface(s) that were actually rendered to
		//     RenderSurface* surface = texture->GetRenderSurface((CubeMapFace)i);
		//     if (!surface->IsResolveDirty())
		//         continue;
		//
		//     surface->SetResolveDirty(false);
		//     unsigned subResource = D3D11CalcSubresource(0, i, texture->GetLevels());
		//     impl_->deviceContext_->ResolveSubresource(dest, subResource, source, subResource, (DXGI_FORMAT)texture->GetFormat());
		// }
		//
		// return true;
		return draw_command_->ResolveTexture(texture);
	}


	void Graphics::Draw(PrimitiveType type, unsigned vertexStart, unsigned vertexCount) const
	{
		if (!draw_command_)
			return;

		draw_command_->SetPrimitiveType(type);
		draw_command_->SetIndexBuffer(nullptr);
		DrawCommandDrawDesc desc = {};
		desc.vertex_start = vertexStart;
		desc.vertex_count = vertexCount;
		draw_command_->Draw(desc);
	}

	auto Graphics::Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex,
	                    unsigned vertexCount) const -> void
	{
		if(!draw_command_)
			return;

		draw_command_->SetPrimitiveType(type);
		DrawCommandDrawDesc desc = {};
		desc.index_start = indexStart;
		desc.index_count = indexCount;
		desc.min_vertex = minVertex;
		desc.vertex_count = vertexCount;
		draw_command_->Draw(desc);
	}

	void Graphics::Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex,
		unsigned minVertex, unsigned vertexCount) const
	{
		if(!draw_command_)
			return;

		draw_command_->SetPrimitiveType(type);
		DrawCommandDrawDesc desc = {};
		desc.index_start = indexStart;
		desc.index_count = indexCount;
		desc.base_vertex_index = baseVertexIndex;
		desc.min_vertex = minVertex;
		desc.vertex_count = vertexCount;
		draw_command_->Draw(desc);
	}

	void Graphics::DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex,
		unsigned vertexCount,
		unsigned instanceCount) const
	{
		if (!draw_command_)
			return;

		draw_command_->SetPrimitiveType(type);
		DrawCommandInstancedDrawDesc desc;
		desc.index_count = indexCount;
		desc.instance_count = instanceCount;
		desc.index_start = indexStart;
		desc.min_vertex = desc.base_vertex_index = 0;
		desc.vertex_count = vertexCount;
		draw_command_->Draw(desc);
	}

	void Graphics::DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex,
		unsigned minVertex, unsigned vertexCount,
		unsigned instanceCount) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetPrimitiveType(type);
		DrawCommandInstancedDrawDesc desc = {};
		desc.index_start = indexStart;
		desc.index_count = indexCount;
		desc.base_vertex_index = baseVertexIndex;
		desc.min_vertex = minVertex;
		desc.vertex_count = vertexCount;
		desc.instance_count = instanceCount;
		draw_command_->Draw(desc);
	}

	void Graphics::SetVertexBuffer(VertexBuffer* buffer) const
	{
		if(draw_command_)
			draw_command_->SetVertexBuffer(buffer);
	}

	bool Graphics::SetVertexBuffers(const PODVector<VertexBuffer*>& buffers, unsigned instanceOffset) const
	{
		if (buffers.Size() > MAX_VERTEX_STREAMS)
		{
			ATOMIC_LOGERROR("Too many vertex buffers");
			return false;
		}

		if(!draw_command_)
			return false;

		draw_command_->SetVertexBuffers(buffers, instanceOffset);
		return true;
	}

	bool Graphics::SetVertexBuffers(const Vector<SharedPtr<VertexBuffer>>& buffers, unsigned instanceOffset) const
	{
		return SetVertexBuffers(reinterpret_cast<const PODVector<VertexBuffer*>&>(buffers), instanceOffset);
	}

	void Graphics::SetIndexBuffer(IndexBuffer* buffer) const
	{
		if (!draw_command_)
			return;
		draw_command_->SetIndexBuffer(buffer);
	}

	void Graphics::SetShaders(ShaderVariation* vs, ShaderVariation* ps) const
	{
		if(!draw_command_)
			return;
		const DrawCommandShadersDesc desc = { vs, ps };
		draw_command_->SetShaders(desc);
	}

	void Graphics::SetShaderParameter(StringHash param, const float* data, unsigned count)
	{
		if (!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, data, count);
	}

	void Graphics::SetShaderParameter(StringHash param, float value)
	{
		if (!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, value);
	}

	void Graphics::SetShaderParameter(StringHash param, int value)
	{
		if (!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, value);
	}

	void Graphics::SetShaderParameter(StringHash param, bool value)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, value);
	}

	void Graphics::SetShaderParameter(StringHash param, const Color& color)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, color);
	}

	void Graphics::SetShaderParameter(StringHash param, const Vector2& vector)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, vector);
	}

	void Graphics::SetShaderParameter(StringHash param, const Matrix3& matrix)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, matrix);
	}

	void Graphics::SetShaderParameter(StringHash param, const Vector3& vector)
	{
		if (!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, vector);
	}

	void Graphics::SetShaderParameter(StringHash param, const Matrix4& matrix)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, matrix);
	}

	void Graphics::SetShaderParameter(StringHash param, const Vector4& vector)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, vector);
	}

	void Graphics::SetShaderParameter(StringHash param, const Matrix3x4& matrix)
	{
		if(!draw_command_)
			return;
		draw_command_->SetShaderParameter(param, matrix);
	}

	bool Graphics::NeedParameterUpdate(ShaderParameterGroup group, const void* source)
	{
		if (!draw_command_)
			return false;
		return draw_command_->NeedShaderGroupUpdate(group, source);
	}

	bool Graphics::HasShaderParameter(StringHash param) const
	{
		if(!draw_command_)
			return false;
		return draw_command_->HasShaderParameter(param);
	}

	bool Graphics::HasTextureUnit(TextureUnit unit) const
	{
		return draw_command_->HasTexture(unit);
	}

	void Graphics::ClearParameterSource(ShaderParameterGroup group)
	{
		if(!draw_command_)
			return;
		draw_command_->ClearShaderParameterSource(group);
	}

	void Graphics::ClearParameterSources()
	{
		for(u8 i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i)
			ClearParameterSource(static_cast<ShaderParameterGroup>(i));
	}

	void Graphics::ClearTransformSources()
	{
		ClearParameterSource(SP_CAMERA);
		ClearParameterSource(SP_OBJECT);
	}

	void Graphics::SetTexture(unsigned index, Texture* texture) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetTexture(static_cast<TextureUnit>(index), texture);
	}

	void Graphics::SetTexture(u32 index, RenderTexture* texture) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetTexture(static_cast<TextureUnit>(index), texture);
	}

	void SetTextureForUpdate(Texture* texture)
	{
		// No-op on Direct3D11
	}

	void Graphics::SetDefaultTextureFilterMode(TextureFilterMode mode)
	{
		if (mode == defaultTextureFilterMode_)
			return;
		defaultTextureFilterMode_ = mode;
		SetTextureParametersDirty();
	}

	void Graphics::SetDefaultTextureAnisotropy(unsigned level)
	{
		level = Max(level, 1U);

		if (level == defaultTextureAnisotropy_)
			return;

		defaultTextureAnisotropy_ = level;
		SetTextureParametersDirty();
	}

	void Graphics::Restore()
	{
		// No-op on Direct3D11
	}

	void Graphics::SetTextureParametersDirty()
	{
		MutexLock lock(gpuObjectMutex_);

		for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
		{
			Texture* texture = dynamic_cast<Texture*>(*i);
			if (texture)
				texture->SetParametersDirty();
		}
	}

	void Graphics::ResetRenderTargets()
	{
		if(!draw_command_)
			return;
		draw_command_->ResetRenderTargets();
		draw_command_->ResetDepthStencil();
		draw_command_->SetViewport(IntRect(0, 0, width_, height_));
	}

	void Graphics::ResetRenderTarget(unsigned index) const
	{
		if(!draw_command_)
			return;
		draw_command_->ResetRenderTarget(index);
	}

	void Graphics::ResetDepthStencil() const
	{
		if (!draw_command_)
			return;
		draw_command_->ResetDepthStencil();
	}

	void Graphics::ResetTexture(u32 slot) const
	{
		if (!draw_command_)
			return;
		draw_command_->ResetTexture(static_cast<TextureUnit>(slot));
	}

	void Graphics::SetRenderTarget(unsigned index, RenderSurface* renderTarget) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetRenderTarget(index, renderTarget);
	}

	void Graphics::SetRenderTarget(unsigned index, Texture2D* texture) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetRenderTarget(index, texture);
	}

	void Graphics::SetRenderTarget(u32 index, RenderTexture* texture) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetRenderTarget(index, texture);
	}

	void Graphics::SetDepthStencil(RenderSurface* depthStencil) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetDepthStencil(depthStencil);
	}

	void Graphics::SetDepthStencil(Texture2D* texture) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetDepthStencil(texture);
	}

	void Graphics::SetViewport(const IntRect& rect) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetViewport(rect);
	}

	void Graphics::SetBlendMode(BlendMode mode, bool alphaToCoverage) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetBlendMode(mode, alphaToCoverage);
	}

	void Graphics::SetColorWrite(bool enable) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetColorWrite(enable);
	}

	void Graphics::SetCullMode(CullMode mode) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetCullMode(mode);
	}

	void Graphics::SetDepthBias(float constantBias, float slopeScaledBias) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetDepthBias(constantBias, slopeScaledBias);
	}

	void Graphics::SetDepthTest(CompareMode mode) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetDepthTest(mode);
	}

	void Graphics::SetDepthWrite(bool enable) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetDepthWrite(enable);
	}

	void Graphics::SetFillMode(FillMode mode) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetFillMode(mode);
	}

	void Graphics::SetLineAntiAlias(bool enable) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetLineAntiAlias(enable);
	}

	void Graphics::SetScissorTest(bool enable, const Rect& rect, bool borderInclusive) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetScissorTest(enable, rect, borderInclusive);
	}

	void Graphics::SetScissorTest(bool enable, const IntRect& rect) const
	{
		if(!draw_command_)
			return;
		draw_command_->SetScissorTest(enable, rect);
	}

	void Graphics::SetStencilTest(bool enable, CompareMode mode, StencilOp pass, StencilOp fail, StencilOp zFail,
		unsigned stencilRef,
		unsigned compareMask, unsigned writeMask) const
	{
		if(!draw_command_)
			return;
		DrawCommandStencilTestDesc desc;
		desc.enable = enable;
		desc.mode = mode;
		desc.pass = pass;
		desc.fail = fail;
		desc.depth_fail = zFail;
		desc.stencil_ref = stencilRef;
		desc.compare_mask = compareMask;
		desc.write_mask = writeMask;
		draw_command_->SetStencilTest(desc);
	}

	void Graphics::SetClipPlane(bool enable, const Plane& clipPlane, const Matrix3x4& view, const Matrix4& projection) const
	{
		if(!draw_command_)
			return;

		DrawCommandClipPlaneDesc desc;
		desc.clip_plane = clipPlane;
		desc.view = view;
		desc.projection = projection;
		desc.enable = enable;
		draw_command_->SetClipPlane(desc);
	}

	bool Graphics::IsInitialized() const
	{
		return window_ != nullptr && impl_->IsInitialized();
	}

	GraphicsBackend Graphics::GetBackend() const
	{
		return impl_->GetBackend();
	}

	PODVector<int> Graphics::GetMultiSampleLevels() const
	{
		if (!impl_->IsInitialized())
			return {};
		static u32 s_possible_levels[] = { 2, 4, 8, 16 };
		PODVector<int> levels;
		const auto color_fmt = impl_->GetSwapChain()->GetDesc().ColorBufferFormat;
		for(const auto& level : s_possible_levels)
		{
			if (level == impl_->GetSupportedMultiSample(color_fmt, level))
				levels.Push(level);
		}

		return levels;
	}

	TextureFormat Graphics::GetFormat(CompressedFormat format) const
	{
		using namespace Diligent;
		switch (format)
		{
		case CF_RGBA:
			return TEX_FORMAT_RGBA8_UNORM;

		case CF_DXT1:
			return TEX_FORMAT_BC1_UNORM;

		case CF_DXT3:
			return TEX_FORMAT_BC2_UNORM;

		case CF_DXT5:
			return TEX_FORMAT_BC3_UNORM;

		default:
			return TEX_FORMAT_UNKNOWN;
		}
	}

	ShaderVariation* Graphics::GetShader(ShaderType type, const String& name, const String& defines) const
	{
		return GetShader(type, name.CString(), defines.CString());
	}

	ShaderVariation* Graphics::GetShader(ShaderType type, const char* name, const char* defines) const
	{
		if (lastShaderName_ != name || !lastShader_)
		{
			ResourceCache* cache = GetSubsystem<ResourceCache>();

			String fullShaderName = shaderPath_ + name + shaderExtension_;
			// Try to reduce repeated error log prints because of missing shaders
			if (lastShaderName_ == name && !cache->Exists(fullShaderName))
				return nullptr;

			lastShader_ = cache->GetResource<Shader>(fullShaderName);
			lastShaderName_ = name;
		}

		return lastShader_ ? lastShader_->GetVariation(type, defines) : nullptr;
	}

	VertexBuffer* Graphics::GetVertexBuffer(unsigned index) const
	{
		if(!draw_command_)
			return nullptr;
		return draw_command_->GetVertexBuffer(index);
	}

	TextureUnit Graphics::GetTextureUnit(const String& name)
	{
		return REngine::utils_get_texture_unit(name);
	}

	const String& Graphics::GetTextureUnitName(TextureUnit unit)
	{
		return REngine::utils_get_texture_unit_names(unit)[0];
	}

	Texture* Graphics::GetTexture(unsigned index) const
	{
		return draw_command_->GetTexture(static_cast<TextureUnit>(index));
	}

	RenderSurface* Graphics::GetRenderTarget(unsigned index) const
	{
		return draw_command_->GetRenderTarget(index);
	}

	IntVector2 Graphics::GetRenderTargetDimensions() const
	{
		return draw_command_->GetRenderTargetDimensions();
	}

	bool Graphics::GetDither() const
	{
		return false;
	}

	bool Graphics::IsDeviceLost() const
	{
		// Diligent graphics context is never considered lost
		return false;
	}

	void Graphics::OnWindowResized()
	{
		if (!IsInitialized())
			return;

		int newWidth, newHeight;

		SDL_GetWindowSize(window_, &newWidth, &newHeight);
		if (newWidth == width_ && newHeight == height_)
			return;

		UpdateSwapChain(newWidth, newHeight);

		// Reset rendertargets and viewport for the new screen size
		ResetRenderTargets();

		ATOMIC_LOGDEBUGF("Window was resized to %dx%d", width_, height_);

		using namespace ScreenMode;

		VariantMap& eventData = GetEventDataMap();
		eventData[P_WIDTH] = width_;
		eventData[P_HEIGHT] = height_;
		eventData[P_FULLSCREEN] = fullscreen_;
		eventData[P_RESIZABLE] = resizable_;
		eventData[P_BORDERLESS] = borderless_;
		eventData[P_HIGHDPI] = highDPI_;
		SendEvent(E_SCREENMODE, eventData);
	}

	void Graphics::OnWindowMoved()
	{
		if (!IsInitialized() || fullscreen_)
			return;

		int newX, newY;

		SDL_GetWindowPosition(window_, &newX, &newY);
		if (newX == position_.x_ && newY == position_.y_)
			return;

		position_.x_ = newX;
		position_.y_ = newY;

		ATOMIC_LOGDEBUGF("Window was moved to %d,%d", position_.x_, position_.y_);

		using namespace WindowPos;

		VariantMap& eventData = GetEventDataMap();
		eventData[P_X] = position_.x_;
		eventData[P_Y] = position_.y_;
		SendEvent(E_WINDOWPOS, eventData);
	}

	void Graphics::CleanupShaderPrograms(ShaderVariation* variation)
	{
		// No-op on Diligent
		ATOMIC_LOGWARNING("CleanupShaderPrograms is not support. Please use Cleanup instead.");
	}

	void Graphics::CleanupRenderSurface(RenderSurface* surface)
	{
		// No-op on Diligent
	}

	void Graphics::Cleanup(GraphicsClearFlags flags)
	{
		if (flags & GRAPHICS_CLEAR_SRB)
		{
			const auto cache_count = REngine::srb_cache_items_count();
			REngine::srb_cache_release();
			ATOMIC_LOGINFOF("Released (%d) Shader Resource Bindings", cache_count);
		}
		if (flags & GRAPHICS_CLEAR_PIPELINES)
		{
			const auto cache_count = REngine::pipeline_state_builder_items_count();
			REngine::pipeline_state_builder_release();
			ATOMIC_LOGINFOF("Released (%d) Pipeline States", cache_count);
		}
		if (flags & GRAPHICS_CLEAR_SHADER_PROGRAMS)
		{
			const auto cache_count = REngine::graphics_state_shader_programs_count();
			REngine::graphics_state_release_shader_programs();
			ATOMIC_LOGINFOF("Released (%d) Shader Programs", cache_count);
		}
		if (flags & GRAPHICS_CLEAR_VERTEX_DECLARATIONS)
		{
			const auto cache_count = REngine::graphics_state_vertex_declarations_count();
			REngine::graphics_state_release_vertex_declarations();
			ATOMIC_LOGINFOF("Released (%d) Vertex Declarations", cache_count);
		}
		if (flags & GRAPHICS_CLEAR_CONSTANT_BUFFERS)
		{
			const uint32_t cache_count = static_cast<uint32_t>(MAX_SHADER_PARAMETER_GROUPS) * static_cast<uint32_t>(MAX_SHADER_TYPES) + REngine::graphics_state_constant_buffers_count();
			impl_->ClearConstantBuffers();
			REngine::graphics_state_release_constant_buffers();
			ATOMIC_LOGINFOF("Released (%d) Constant Buffers", cache_count);
		}

		if (flags & GRAPHICS_CLEAR_SCRATCH_BUFFERS)
			CleanupScratchBuffers();
	}

	ConstantBuffer* Graphics::GetOrCreateConstantBuffer(ShaderType type, unsigned index, unsigned size) const
	{
		auto constant_buffer = REngine::graphics_state_get_constant_buffer(type, size);
		if (constant_buffer)
			return constant_buffer;

		constant_buffer = SharedPtr<ConstantBuffer>(new ConstantBuffer(context_));
		REngine::ConstantBufferCacheDesc cache_desc;
		cache_desc.constant_buffer = constant_buffer;
		cache_desc.type = type;
		REngine::graphics_state_set_constant_buffer(cache_desc);
		return constant_buffer;
	}

	TextureFormat Graphics::GetAlphaFormat()
	{
		return Diligent::TEX_FORMAT_A8_UNORM;
	}

	TextureFormat Graphics::GetLuminanceFormat()
	{
		// Note: not same sampling behavior as on D3D9; need to sample the R channel only
		return Diligent::TEX_FORMAT_R8_UNORM;
	}

	TextureFormat Graphics::GetLuminanceAlphaFormat()
	{
		// Note: not same sampling behavior as on D3D9; need to sample the RG channels
		return Diligent::TEX_FORMAT_RG8_UNORM;
	}

	TextureFormat Graphics::GetRGBFormat()
	{
		return Diligent::TEX_FORMAT_RGBA8_UNORM;
	}

	TextureFormat Graphics::GetRGBAFormat()
	{
		return Diligent::TEX_FORMAT_RGBA8_UNORM;
	}

	TextureFormat Graphics::GetRGBA16Format()
	{
		return Diligent::TEX_FORMAT_RGBA16_UNORM;
	}

	TextureFormat Graphics::GetRGBAFloat16Format()
	{
		return Diligent::TEX_FORMAT_RGBA16_FLOAT;
	}

	TextureFormat Graphics::GetRGBAFloat32Format()
	{
		return Diligent::TEX_FORMAT_RGBA32_FLOAT;
	}

	TextureFormat Graphics::GetRG16Format()
	{
		return Diligent::TEX_FORMAT_RG16_UNORM;
	}

	TextureFormat Graphics::GetRGFloat16Format()
	{
		return Diligent::TEX_FORMAT_RG16_FLOAT;
	}

	TextureFormat Graphics::GetRGFloat32Format()
	{
		return Diligent::TEX_FORMAT_RG32_FLOAT;
	}

	TextureFormat Graphics::GetFloat16Format()
	{
		return Diligent::TEX_FORMAT_R16_FLOAT;
	}

	TextureFormat Graphics::GetFloat32Format()
	{
		return Diligent::TEX_FORMAT_R32_FLOAT;
	}

	TextureFormat Graphics::GetLinearDepthFormat()
	{
		return Diligent::TEX_FORMAT_R32_FLOAT;
	}

	TextureFormat Graphics::GetDepthStencilFormat()
	{
		return Diligent::TEX_FORMAT_R24G8_TYPELESS;
	}

	TextureFormat Graphics::GetReadableDepthFormat()
	{
		return Diligent::TEX_FORMAT_R24G8_TYPELESS;
	}

	TextureFormat Graphics::GetFormat(const String& formatName)
	{
		auto nameLower = formatName.ToLower().Trimmed();

		if (nameLower == "a")
			return GetAlphaFormat();
		if (nameLower == "l")
			return GetLuminanceFormat();
		if (nameLower == "la")
			return GetLuminanceAlphaFormat();
		if (nameLower == "rgb")
			return GetRGBFormat();
		if (nameLower == "rgba")
			return GetRGBAFormat();
		if (nameLower == "rgba16")
			return GetRGBA16Format();
		if (nameLower == "rgba16f")
			return GetRGBAFloat16Format();
		if (nameLower == "rgba32f")
			return GetRGBAFloat32Format();
		if (nameLower == "rg16")
			return GetRG16Format();
		if (nameLower == "rg16f")
			return GetRGFloat16Format();
		if (nameLower == "rg32f")
			return GetRGFloat32Format();
		if (nameLower == "r16f")
			return GetFloat16Format();
		if (nameLower == "r32f" || nameLower == "float")
			return GetFloat32Format();
		if (nameLower == "lineardepth" || nameLower == "depth")
			return GetLinearDepthFormat();
		if (nameLower == "d24s8")
			return GetDepthStencilFormat();
		if (nameLower == "readabledepth" || nameLower == "hwdepth")
			return GetReadableDepthFormat();

		return GetRGBFormat();
	}

	unsigned Graphics::GetMaxBones()
	{
		return 128;
	}

	bool Graphics::GetGL3Support()
	{
		return false;
	}

	bool Graphics::OpenWindow(int width, int height, bool resizable, bool borderless)
	{
		if (!externalWindow_)
		{
			unsigned flags = 0;
			if (resizable)
				flags |= SDL_WINDOW_RESIZABLE;
			if (borderless)
				flags |= SDL_WINDOW_BORDERLESS;

			window_ = SDL_CreateWindow(windowTitle_.CString(), position_.x_, position_.y_, width, height, flags);
		}
		else
			window_ = SDL_CreateWindowFrom(externalWindow_);

		if (!window_)
		{
			ATOMIC_LOGERRORF("Could not create window, root cause: '%s'", SDL_GetError());
			return false;
		}

		SDL_GetWindowPosition(window_, &position_.x_, &position_.y_);

		CreateWindowIcon();

		return true;
	}

	void Graphics::AdjustWindow(int& newWidth, int& newHeight, bool& newFullscreen, bool& newBorderless, int& monitor)
	{
		if (!externalWindow_)
		{
			if (!newWidth || !newHeight)
			{
				SDL_MaximizeWindow(window_);
				SDL_GetWindowSize(window_, &newWidth, &newHeight);
			}
			else
			{
				SDL_Rect display_rect;
				SDL_GetDisplayBounds(monitor, &display_rect);

				if (newFullscreen || (newBorderless && newWidth >= display_rect.w && newHeight >= display_rect.h))
				{
					// Reposition the window on the specified monitor if it's supposed to cover the entire monitor
					SDL_SetWindowPosition(window_, display_rect.x, display_rect.y);
				}

				SDL_SetWindowSize(window_, newWidth, newHeight);
			}

			// Hack fix: on SDL 2.0.4 a fullscreen->windowed transition results in a maximized window when the D3D device is reset, so hide before
			SDL_HideWindow(window_);
			SDL_SetWindowFullscreen(window_, newFullscreen ? SDL_WINDOW_FULLSCREEN : 0);
			SDL_SetWindowBordered(window_, newBorderless ? SDL_FALSE : SDL_TRUE);
			SDL_ShowWindow(window_);
		}
		else
		{
			// If external window, must ask its dimensions instead of trying to set them
			SDL_GetWindowSize(window_, &newWidth, &newHeight);
			newFullscreen = false;
		}
	}

	bool Graphics::CreateDevice(int width, int height, int multiSample)
	{
		REngine::DriverInstanceInitDesc desc;
#if WIN32
		desc.window = Diligent::NativeWindow(GetWindowHandle(window_));
#else
		throw std::runtime_error("Not implemented window acquire");
#endif
		desc.window_size = IntVector2(width, height);
		desc.multisample = static_cast<uint8_t>(multiSample);
		desc.color_buffer_format = sRGB_ ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM;
		desc.depth_buffer_format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;

		if (impl_->IsInitialized())
			impl_->Release();

		if (!impl_->InitDevice(desc))
		{
			ATOMIC_LOGERROR("Failed to initialize graphics driver.");
			return false;
		}

		CheckFeatureSupport();
		SetFlushGPU(flushGPU_);
		multiSample_ = impl_->GetMultiSample();
		draw_command_ = ea::shared_ptr<IDrawCommand>(REngine::graphics_create_command(this));
		GetSubsystem<DrawCommandQueue>()->AddCommand(draw_command_);
		return true;
	}

	bool Graphics::UpdateSwapChain(int width, int height)
	{
		if (impl_->GetSwapChain() == nullptr)
			return CreateDevice(width, height, multiSample_);

		impl_->GetSwapChain()->Resize(width, height);

		width_ = width;
		height_ = height;
		ResetRenderTargets();
		return true;
	}

	void Graphics::CheckFeatureSupport() {
		anisotropySupport_ = true;
		dxtTextureSupport_ = true;
		lightPrepassSupport_ = true;
		deferredSupport_ = true;
		hardwareShadowSupport_ = true;
		instancingSupport_ = true;
		shadowMapFormat_ = Diligent::TEX_FORMAT_D16_UNORM;
		hiresShadowMapFormat_ = Diligent::TEX_FORMAT_RG32_FLOAT;
		dummyColorFormat_ = Diligent::TEX_FORMAT_UNKNOWN;
		sRGBSupport_ = true;
		sRGBWriteSupport_ = true;
	}

	void Graphics::ResetCachedState()
	{
		if(!draw_command_)
			return;
		draw_command_->Reset();
	}

	void Graphics::PrepareDraw()
	{
	}

	void Graphics::CreateResolveTexture()
	{
		throw new std::runtime_error("Not implemented");
		// if (impl_->resolveTexture_)
		//     return;
		//
		// D3D11_TEXTURE2D_DESC textureDesc;
		// memset(&textureDesc, 0, sizeof textureDesc);
		// textureDesc.Width = (UINT)width_;
		// textureDesc.Height = (UINT)height_;
		// textureDesc.MipLevels = 1;
		// textureDesc.ArraySize = 1;
		// textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		// textureDesc.SampleDesc.Count = 1;
		// textureDesc.SampleDesc.Quality = 0;
		// textureDesc.Usage = D3D11_USAGE_DEFAULT;
		// textureDesc.CPUAccessFlags = 0;
		//
		// HRESULT hr = impl_->device_->CreateTexture2D(&textureDesc, 0, &impl_->resolveTexture_);
		// if (FAILED(hr))
		// {
		//     ATOMIC_SAFE_RELEASE(impl_->resolveTexture_);
		//     ATOMIC_LOGD3DERROR("Could not create resolve texture", hr);
		// }
	}

	void Graphics::SetTextureUnitMappings()
	{

	}

	// ATOMIC BEGIN

	// To satisfy script binding linking
	void Graphics::SetTextureForUpdate(Texture* texture)
	{
	}

	void Graphics::MarkFBODirty()
	{
	}

	void Graphics::SetVBO(unsigned object)
	{
	}

	void Graphics::SetUBO(unsigned object)
	{
	}


	// ATOMIC END
}
