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

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

// Prefer the high-performance GPU on switchable GPU systems
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Atomic
{
	static HWND GetWindowHandle(SDL_Window* window)
	{
		SDL_SysWMinfo sysInfo;

		SDL_VERSION(&sysInfo.version);
		SDL_GetWindowWMInfo(window, &sysInfo);
		return sysInfo.info.win.window;
	}

	const Vector2 Graphics::pixelUVOffset(0.0f, 0.0f);
	bool Graphics::gl3Support = false;

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
		numPrimitives_(0),
		numBatches_(0),
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

		memset(vertexBuffers_, 0x0, sizeof(VertexBuffer*) * MAX_VERTEX_STREAMS);
		context_->RequireSDL(SDL_INIT_VIDEO);

		// Register Graphics library object factories
		RegisterGraphicsLibrary(context_);
	}

	Graphics::~Graphics()
	{
		{
			MutexLock lock(gpuObjectMutex_);
			// Release all GPU objects that still exist
			for (PODVector<GPUObject*>::Iterator i = gpuObjects_.Begin(); i != gpuObjects_.End(); ++i)
				(*i)->Release();
			gpuObjects_.Clear();
		}

		ResetCachedState();
		Cleanup(GRAPHICS_CLEAR_ALL);
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
		throw std::exception("Not implemented");
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

		// Set default rendertarget and depth buffer
		ResetRenderTargets();
		auto command = REngine::default_render_command_get();
		REngine::render_command_reset(this, command);
		REngine::default_render_command_set(command);

		// Cleanup vertex buffers from previous frame
		memset(vertexBuffers_, 0x0, sizeof(VertexBuffer*) * MAX_VERTEX_STREAMS);
		indexBuffer_ = nullptr;
		// Cleanup textures from previous frame
		for (uint8_t i = 0; i < MAX_TEXTURE_UNITS; ++i)
			SetTexture(i, nullptr);
		vertexShader_ = nullptr;
		pixelShader_ = nullptr;
		numPrimitives_ = 0;
		numBatches_ = 0;

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

	void Graphics::Clear(unsigned flags, const Color& color, float depth, unsigned stencil)
	{
		IntVector2 rtSize = GetRenderTargetDimensions();

		bool oldColorWrite = colorWrite_;
		bool oldDepthWrite = depthWrite_;

		// D3D11 clear always clears the whole target regardless of viewport or scissor test settings
		// Emulate partial clear by rendering a quad
		if (!viewport_.left_ && !viewport_.top_ && viewport_.right_ == rtSize.x_ && viewport_.bottom_ == rtSize.y_)
		{
			// Make sure we use the read-write version of the depth stencil
			SetDepthWrite(true);
			// Skip pipeline build and silence pipeline creation warning
			auto command = REngine::default_render_command_get();
			command.skip_flags |= static_cast<unsigned>(REngine::RenderCommandSkipFlags::pipeline_build);
			REngine::default_render_command_set(command);

			PrepareDraw();
			command = REngine::default_render_command_get();

			REngine::RenderCommandClearDesc clear_desc;
			clear_desc.flags = flags;
			clear_desc.clear_color = color;
			clear_desc.clear_depth = depth;
			clear_desc.driver = impl_;

			if ((flags & CLEAR_COLOR) && command.render_targets[0])
				clear_desc.render_target = command.render_targets[0];

			if ((flags & (CLEAR_DEPTH | CLEAR_STENCIL)) && command.depth_stencil)
			{
				if (flags & CLEAR_DEPTH)
					clear_desc.clear_stencil_flags |= Diligent::CLEAR_DEPTH_FLAG;
				if (flags & CLEAR_STENCIL)
					clear_desc.clear_stencil_flags |= Diligent::CLEAR_STENCIL_FLAG;
				clear_desc.depth_stencil = command.depth_stencil;
			}

			REngine::render_command_clear(clear_desc);
		}
		else
		{
			Renderer* renderer = GetSubsystem<Renderer>();
			if (!renderer)
				return;

			Geometry* geometry = renderer->GetQuadGeometry();

			Matrix3x4 model = Matrix3x4::IDENTITY;
			Matrix4 projection = Matrix4::IDENTITY;
			model.m23_ = Clamp(depth, 0.0f, 1.0f);

			SetBlendMode(BLEND_REPLACE);
			SetColorWrite((flags & CLEAR_COLOR) != 0);
			SetCullMode(CULL_NONE);
			SetDepthTest(CMP_ALWAYS);
			SetDepthWrite((flags & CLEAR_DEPTH) != 0);
			SetFillMode(FILL_SOLID);
			SetScissorTest(false);
			SetStencilTest((flags & CLEAR_STENCIL) != 0, CMP_ALWAYS, OP_REF, OP_KEEP, OP_KEEP, stencil);
			SetShaders(GetShader(VS, "ClearFramebuffer"), GetShader(PS, "ClearFramebuffer"));
			SetShaderParameter(VSP_MODEL, model);
			SetShaderParameter(VSP_VIEWPROJ, projection);
			SetShaderParameter(PSP_MATDIFFCOLOR, color);

			geometry->Draw(this);

			SetStencilTest(false);
			ClearParameterSources();
		}

		// Restore color & depth write state now
		SetColorWrite(oldColorWrite);
		SetDepthWrite(oldDepthWrite);
	}

	bool Graphics::ResolveToTexture(Texture2D* destination, const IntRect& viewport)
	{
		throw std::exception("Not implemented");
		// if (!destination || !destination->GetRenderSurface())
		//     return false;
		//
		// ATOMIC_PROFILE(ResolveToTexture);
		//
		// IntRect vpCopy = viewport;
		// if (vpCopy.right_ <= vpCopy.left_)
		//     vpCopy.right_ = vpCopy.left_ + 1;
		// if (vpCopy.bottom_ <= vpCopy.top_)
		//     vpCopy.bottom_ = vpCopy.top_ + 1;
		//
		// D3D11_BOX srcBox;
		// srcBox.left = Clamp(vpCopy.left_, 0, width_);
		// srcBox.top = Clamp(vpCopy.top_, 0, height_);
		// srcBox.right = Clamp(vpCopy.right_, 0, width_);
		// srcBox.bottom = Clamp(vpCopy.bottom_, 0, height_);
		// srcBox.front = 0;
		// srcBox.back = 1;
		//
		// ID3D11Resource* source = 0;
		// bool resolve = multiSample_ > 1;
		// impl_->defaultRenderTargetView_->GetResource(&source);
		//
		// if (!resolve)
		// {
		//     if (!srcBox.left && !srcBox.top && srcBox.right == width_ && srcBox.bottom == height_)
		//         impl_->deviceContext_->CopyResource((ID3D11Resource*)destination->GetGPUObject(), source);
		//     else
		//         impl_->deviceContext_->CopySubresourceRegion((ID3D11Resource*)destination->GetGPUObject(), 0, 0, 0, 0, source, 0, &srcBox);
		// }
		// else
		// {
		//     if (!srcBox.left && !srcBox.top && srcBox.right == width_ && srcBox.bottom == height_)
		//     {
		//         impl_->deviceContext_->ResolveSubresource((ID3D11Resource*)destination->GetGPUObject(), 0, source, 0, (DXGI_FORMAT)
		//             destination->GetFormat());
		//     }
		//     else
		//     {
		//         CreateResolveTexture();
		//
		//         if (impl_->resolveTexture_)
		//         {
		//             impl_->deviceContext_->ResolveSubresource(impl_->resolveTexture_, 0, source, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		//             impl_->deviceContext_->CopySubresourceRegion((ID3D11Resource*)destination->GetGPUObject(), 0, 0, 0, 0, impl_->resolveTexture_, 0, &srcBox);
		//         }
		//     }
		// }
		//
		// source->Release();
		//
		// return true;
	}

	bool Graphics::ResolveToTexture(Texture2D* texture)
	{
		throw std::exception("Not implemented");
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
	}

	bool Graphics::ResolveToTexture(TextureCube* texture)
	{
		throw std::exception("Not implemented");
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
	}


	void Graphics::Draw(PrimitiveType type, unsigned vertexStart, unsigned vertexCount)
	{
		auto command = REngine::default_render_command_get();

		if (!vertexCount || !command.shader_program)
			return;

		if (command.pipeline_state_info.primitive_type != type)
		{
			command.pipeline_state_info.primitive_type = type;
			command.dirty_state |= static_cast<uint32_t>(REngine::RenderCommandDirtyState::pipeline);
			REngine::default_render_command_set(command);
		}

		PrepareDraw();

		Diligent::DrawAttribs draw_attribs = {};
		draw_attribs.NumVertices = vertexCount;
		draw_attribs.StartVertexLocation = vertexStart;
		draw_attribs.NumInstances = 1;
		draw_attribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;

		impl_->GetDeviceContext()->Draw(draw_attribs);

		uint32_t primitive_count;
		REngine::utils_get_primitive_type(vertexCount, command.pipeline_state_info.primitive_type, &primitive_count);
		numPrimitives_ += primitive_count;
		++numBatches_;
	}

	void Graphics::Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex,
		unsigned vertexCount)
	{
		auto command = REngine::default_render_command_get();
		if (!vertexCount || !command.shader_program)
			return;

		if (fillMode_ == FILL_POINT)
			type = command.pipeline_state_info.primitive_type = POINT_LIST;

		if (command.pipeline_state_info.primitive_type != type)
		{
			command.pipeline_state_info.primitive_type = type;
			command.dirty_state |= static_cast<uint32_t>(REngine::RenderCommandDirtyState::pipeline);
			REngine::default_render_command_set(command);
		}


		PrepareDraw();

		Diligent::DrawIndexedAttribs draw_attribs = {};
		draw_attribs.IndexType = indexBuffer_->GetIndexSize() == sizeof(uint16_t) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		draw_attribs.NumIndices = indexCount;
		draw_attribs.FirstIndexLocation = indexStart;
		draw_attribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;

		impl_->GetDeviceContext()->DrawIndexed(draw_attribs);

		uint32_t primitive_count;
		REngine::utils_get_primitive_type(vertexCount, command.pipeline_state_info.primitive_type, &primitive_count);
		numPrimitives_ += primitive_count;
		++numBatches_;
	}

	void Graphics::Draw(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex,
		unsigned minVertex, unsigned vertexCount)
	{
		auto command = REngine::default_render_command_get();
		if (!vertexCount || !indexCount || !command.shader_program)
			return;

		if (fillMode_ == FILL_POINT)
			type = command.pipeline_state_info.primitive_type = POINT_LIST;

		if (command.pipeline_state_info.primitive_type != type)
		{
			command.pipeline_state_info.primitive_type = type;
			command.dirty_state |= static_cast<uint32_t>(REngine::RenderCommandDirtyState::pipeline);
			REngine::default_render_command_set(command);
		}
		
		PrepareDraw();

		Diligent::DrawIndexedAttribs draw_attribs = {};
		draw_attribs.IndexType = indexBuffer_->GetIndexSize() == sizeof(uint16_t) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		draw_attribs.NumIndices = indexCount;
		draw_attribs.FirstIndexLocation = indexStart;
		draw_attribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
		draw_attribs.BaseVertex = baseVertexIndex;
		impl_->GetDeviceContext()->DrawIndexed(draw_attribs);

		uint32_t primitive_count;
		REngine::utils_get_primitive_type(vertexCount, command.pipeline_state_info.primitive_type, &primitive_count);
		numPrimitives_ += primitive_count;
		++numBatches_;
	}

	void Graphics::DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned minVertex,
		unsigned vertexCount,
		unsigned instanceCount)
	{
		auto command = REngine::default_render_command_get();
		if (!vertexCount || !instanceCount || !command.shader_program)
			return;

		if (fillMode_ == FILL_POINT)
			type = command.pipeline_state_info.primitive_type = POINT_LIST;

		if (command.pipeline_state_info.primitive_type != type)
		{
			command.pipeline_state_info.primitive_type = type;
			command.dirty_state |= static_cast<uint32_t>(REngine::RenderCommandDirtyState::pipeline);
			REngine::default_render_command_set(command);
		}

		PrepareDraw();

		Diligent::DrawIndexedAttribs draw_attribs = {};
		draw_attribs.IndexType = indexBuffer_->GetIndexSize() == sizeof(uint16_t) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		draw_attribs.NumIndices = indexCount;
		draw_attribs.FirstIndexLocation = indexStart;
		draw_attribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
		draw_attribs.NumInstances = instanceCount;

		impl_->GetDeviceContext()->DrawIndexed(draw_attribs);

		uint32_t primitive_count;
		REngine::utils_get_primitive_type(vertexCount, command.pipeline_state_info.primitive_type, &primitive_count);
		numPrimitives_ += primitive_count * instanceCount;
		++numBatches_;
	}

	void Graphics::DrawInstanced(PrimitiveType type, unsigned indexStart, unsigned indexCount, unsigned baseVertexIndex,
		unsigned minVertex, unsigned vertexCount,
		unsigned instanceCount)
	{
		auto command = REngine::default_render_command_get();
		if (!vertexCount || !instanceCount || !command.shader_program)
			return;

		if (fillMode_ == FILL_POINT)
			type = command.pipeline_state_info.primitive_type = POINT_LIST;

		if (command.pipeline_state_info.primitive_type != type)
		{
			command.pipeline_state_info.primitive_type = type;
			command.dirty_state |= static_cast<uint32_t>(REngine::RenderCommandDirtyState::pipeline);
			REngine::default_render_command_set(command);
		}

		PrepareDraw();

		Diligent::DrawIndexedAttribs draw_attribs = {};
		draw_attribs.IndexType = indexBuffer_->GetIndexSize() == sizeof(uint16_t) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		draw_attribs.NumIndices = indexCount;
		draw_attribs.FirstIndexLocation = indexStart;
		draw_attribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
		draw_attribs.NumInstances = instanceCount;
		draw_attribs.BaseVertex = baseVertexIndex;

		uint32_t primitive_count;
		REngine::utils_get_primitive_type(vertexCount, command.pipeline_state_info.primitive_type, &primitive_count);
		numPrimitives_ += primitive_count * instanceCount;
		++numBatches_;
	}

	void Graphics::SetVertexBuffer(VertexBuffer* buffer)
	{
		// Note: this is not multi-instance safe
		static PODVector<VertexBuffer*> vertex_buffers(1);
		vertex_buffers[0] = buffer;
		SetVertexBuffers(vertex_buffers);
	}

	bool Graphics::SetVertexBuffers(const PODVector<VertexBuffer*>& buffers, unsigned instanceOffset)
	{
		if (buffers.Size() > MAX_VERTEX_STREAMS)
		{
			ATOMIC_LOGERROR("Too many vertex buffers");
			return false;
		}

		auto command = REngine::default_render_command_get();
		for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
		{
			bool changed = false;
			VertexBuffer* buffer = i < buffers.Size() ? buffers[i] : nullptr;
			if (buffer)
			{
				const PODVector<VertexElement>& elements = buffer->GetElements();
				// Check if buffer has per-instance data
				const auto has_instance_data = elements.Size() && elements[0].perInstance_;
				const auto offset = has_instance_data ? instanceOffset * buffer->GetVertexSize() : 0;

				if (buffer != vertexBuffers_[i] || offset != command.vertex_offsets[i])
				{
					vertexBuffers_[i] = buffer;
					command.vertex_buffers[i] = buffer->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
					command.vertex_offsets[i] = offset;
					changed = true;
				}
			}
			else if (vertexBuffers_[i])
			{
				vertexBuffers_[i] = nullptr;
				command.vertex_buffers[i] = nullptr;
				command.vertex_offsets[i] = 0;
				changed = true;
			}

			if (changed)
			{
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::vertex_buffer);
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::vertex_decl);
				REngine::default_render_command_set(command);
			}
		}

		return true;
	}

	bool Graphics::SetVertexBuffers(const Vector<SharedPtr<VertexBuffer>>& buffers, unsigned instanceOffset)
	{
		return SetVertexBuffers(reinterpret_cast<const PODVector<VertexBuffer*>&>(buffers), instanceOffset);
	}

	void Graphics::SetIndexBuffer(IndexBuffer* buffer)
	{
		if (buffer == indexBuffer_)
			return;

		auto command = REngine::default_render_command_get();
		if (buffer)
			command.index_buffer = buffer->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
		else
			command.index_buffer = nullptr;
		indexBuffer_ = buffer;
		REngine::default_render_command_set(command);

	}

	void Graphics::SetShaders(ShaderVariation* vs, ShaderVariation* ps)
	{
		// Switch to the clip plane variations if necessary
		if (useClipPlane_)
		{
			if (vs)
				vs = vs->GetOwner()->GetVariation(VS, vs->GetDefinesClipPlane());
			if (ps)
				ps = ps->GetOwner()->GetVariation(PS, ps->GetDefinesClipPlane());
		}

		if (vs == vertexShader_ && ps == pixelShader_)
			return;

		auto command = REngine::default_render_command_get();
		if (vs != vertexShader_)
		{
			// Create the shader now if not yet created. If already attempted, do not retry
			if (vs && !vs->GetGPUObject())
			{
				if (vs->GetCompilerOutput().Empty())
				{
					ATOMIC_PROFILE(CompileVertexShader);

					bool success = vs->Create();
					if (!success)
					{
						ATOMIC_LOGERROR("Failed to compile vertex shader " + vs->GetFullName() + ":\n" + vs->GetCompilerOutput());
						vs = nullptr;
					}
				}
				else
					vs = nullptr;
			}

			command.pipeline_state_info.vs_shader = vs;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::vertex_decl);
			vertexShader_ = vs;
		}

		if (ps != pixelShader_)
		{
			if (ps && !ps->GetGPUObject())
			{
				if (ps->GetCompilerOutput().Empty())
				{
					ATOMIC_PROFILE(CompilePixelShader);

					bool success = ps->Create();
					if (!success)
					{
						ATOMIC_LOGERROR("Failed to compile pixel shader " + ps->GetFullName() + ":\n" + ps->GetCompilerOutput());
						ps = nullptr;
					}
				}
				else
					ps = nullptr;
			}

			command.pipeline_state_info.ps_shader = ps;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			pixelShader_ = ps;
		}

		// Update current shader parameters & constant buffers
		if (vertexShader_ && pixelShader_)
		{
			REngine::ShaderProgramQuery query{ vertexShader_, pixelShader_ };
			SharedPtr<REngine::ShaderProgram> shader_program = REngine::graphics_state_get_shader_program(query);

			if (!shader_program)
			{
				REngine::ShaderProgramCreationDesc creation_desc = {};
				creation_desc.graphics = this;
				creation_desc.vertex_shader = vertexShader_;
				creation_desc.pixel_shader = pixelShader_;
				shader_program = new REngine::ShaderProgram(creation_desc);
				REngine::graphics_state_set_shader_program(shader_program);
			}

			command.shader_program = shader_program;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::shader_program);
		}
		else
		{
			command.shader_program = nullptr;
			command.dirty_state ^= static_cast<unsigned>(REngine::RenderCommandDirtyState::shader_program);
		}

		// Store shader combination if shader dumping in progress
		if (shaderPrecache_)
			shaderPrecache_->StoreShaders(vertexShader_, pixelShader_);

		// Update clip plane parameter if necessary
		if (useClipPlane_)
			SetShaderParameter(VSP_CLIPPLANE, clipPlane_);

		REngine::default_render_command_set(command);
	}

	void Graphics::SetShaderParameter(StringHash param, const float* data, unsigned count)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, count * sizeof(float), data);
	}

	void Graphics::SetShaderParameter(StringHash param, float value)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(float), &value);
	}

	void Graphics::SetShaderParameter(StringHash param, int value)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(int), &value);
	}

	void Graphics::SetShaderParameter(StringHash param, bool value)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(bool), &value);
	}

	void Graphics::SetShaderParameter(StringHash param, const Color& color)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(Color), &color);
	}

	void Graphics::SetShaderParameter(StringHash param, const Vector2& vector)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(Vector2), &vector);
	}

	void Graphics::SetShaderParameter(StringHash param, const Matrix3& matrix)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		const Matrix3x4 matrix3x4(matrix);
		buffer->SetParameter(parameter.offset_, sizeof(Matrix3x4), &matrix3x4);
	}

	void Graphics::SetShaderParameter(StringHash param, const Vector3& vector)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(Vector3), &vector);
	}

	void Graphics::SetShaderParameter(StringHash param, const Matrix4& matrix)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(Matrix4), &matrix);
	}

	void Graphics::SetShaderParameter(StringHash param, const Vector4& vector)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(Vector4), &vector);
	}

	void Graphics::SetShaderParameter(StringHash param, const Matrix3x4& matrix)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return;

		ShaderParameter parameter;
		if (!command.shader_program->GetParameter(param, &parameter))
			return;

		const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
		if (!buffer)
			return;

		buffer->SetParameter(parameter.offset_, sizeof(Matrix3x4), &matrix);
	}

	bool Graphics::NeedParameterUpdate(ShaderParameterGroup group, const void* source)
	{
		if ((unsigned)(size_t)shaderParameterSources_[group] == M_MAX_UNSIGNED || shaderParameterSources_[group] != source)
		{
			shaderParameterSources_[group] = source;
			return true;
		}
		else
			return false;
	}

	bool Graphics::HasShaderParameter(StringHash param)
	{
		const auto& command = REngine::default_render_command_get();
		if (!command.shader_program)
			return false;
		ShaderParameter parameter;
		return command.shader_program->GetParameter(param, &parameter);
	}

	bool Graphics::HasTextureUnit(TextureUnit unit)
	{
		return (vertexShader_ && vertexShader_->HasTextureUnit(unit)) || (pixelShader_ && pixelShader_->HasTextureUnit(unit));
	}

	void Graphics::ClearParameterSource(ShaderParameterGroup group)
	{
		shaderParameterSources_[group] = (const void*)M_MAX_UNSIGNED;
	}

	void Graphics::ClearParameterSources()
	{
		for (unsigned i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i)
			shaderParameterSources_[i] = (const void*)M_MAX_UNSIGNED;
	}

	void Graphics::ClearTransformSources()
	{
		shaderParameterSources_[SP_CAMERA] = (const void*)M_MAX_UNSIGNED;
		shaderParameterSources_[SP_OBJECT] = (const void*)M_MAX_UNSIGNED;
	}

	void Graphics::SetTexture(unsigned index, Texture* texture)
	{
		if (index >= MAX_TEXTURE_UNITS)
			return;

		// Check if texture is currently bound as a rendertarget. In that case, use its backup texture, or blank if not defined
		if (texture)
		{
			if (renderTargets_[0] && renderTargets_[0]->GetParentTexture() == texture)
				texture = texture->GetBackupTexture();
			else
			{
				// Resolve multisampled texture now as necessary
				if (texture->GetMultiSample() > 1 && texture->GetAutoResolve() && texture->IsResolveDirty())
				{
					if (texture->GetType() == Texture2D::GetTypeStatic())
						ResolveToTexture(static_cast<Texture2D*>(texture));  // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
					if (texture->GetType() == TextureCube::GetTypeStatic())
						ResolveToTexture(static_cast<TextureCube*>(texture)); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
				}
			}

			if (texture->GetLevelsDirty())
				texture->RegenerateLevels();
		}

		if (texture && texture->GetParametersDirty())
		{
			texture->UpdateParameters();
			textures_[index] = nullptr; // Force reassign
		}

		if (texture != textures_[index])
		{
			textures_[index] = texture;
			auto command = REngine::default_render_command_get();
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::textures);
			REngine::default_render_command_set(command);
		}
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
		for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
			SetRenderTarget(i, static_cast<RenderSurface*>(nullptr));
		SetDepthStencil(static_cast<RenderSurface*>(nullptr));
		SetViewport(IntRect(0, 0, width_, height_));
	}

	void Graphics::ResetRenderTarget(unsigned index)
	{
		SetRenderTarget(index, static_cast<RenderSurface*>(nullptr));
	}

	void Graphics::ResetDepthStencil()
	{
		SetDepthStencil(static_cast<RenderSurface*>(nullptr));
	}

	void Graphics::SetRenderTarget(unsigned index, RenderSurface* renderTarget)
	{
		if (index >= MAX_RENDERTARGETS)
			return;

		if (renderTarget == renderTargets_[index])
			return;

		renderTargets_[index] = renderTarget;
		auto command = REngine::default_render_command_get();
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::render_targets);
		REngine::default_render_command_set(command);

		if (renderTarget == nullptr)
			return;

		// If the rendertarget is also bound as a texture, replace with backup texture or null
		Texture* parentTexture = renderTarget->GetParentTexture();

		for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
		{
			if (textures_[i] == parentTexture)
				SetTexture(i, textures_[i]->GetBackupTexture());
		}

		// If multisampled, mark the texture & surface needing resolve
		if (parentTexture->GetMultiSample() > 1 && parentTexture->GetAutoResolve())
		{
			parentTexture->SetResolveDirty(true);
			renderTarget->SetResolveDirty(true);
		}

		// If mipmapped, mark the levels needing regeneration
		if (parentTexture->GetLevels() > 1)
			parentTexture->SetLevelsDirty();
	}

	void Graphics::SetRenderTarget(unsigned index, Texture2D* texture)
	{
		RenderSurface* renderTarget = 0;
		if (texture)
			renderTarget = texture->GetRenderSurface();

		SetRenderTarget(index, renderTarget);
	}

	void Graphics::SetDepthStencil(RenderSurface* depthStencil)
	{
		if (depthStencil == depthStencil_)
			return;
		depthStencil_ = depthStencil;
		auto command = REngine::default_render_command_get();
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::render_targets);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetDepthStencil(Texture2D* texture)
	{
		RenderSurface* depthStencil = 0;
		if (texture)
			depthStencil = texture->GetRenderSurface();

		SetDepthStencil(depthStencil);
	}

	void Graphics::SetViewport(const IntRect& rect)
	{
		IntVector2 size = GetRenderTargetDimensions();

		IntRect rectCopy = rect;

		if (rectCopy.right_ <= rectCopy.left_)
			rectCopy.right_ = rectCopy.left_ + 1;
		if (rectCopy.bottom_ <= rectCopy.top_)
			rectCopy.bottom_ = rectCopy.top_ + 1;
		rectCopy.left_ = Clamp(rectCopy.left_, 0, size.x_);
		rectCopy.top_ = Clamp(rectCopy.top_, 0, size.y_);
		rectCopy.right_ = Clamp(rectCopy.right_, 0, size.x_);
		rectCopy.bottom_ = Clamp(rectCopy.bottom_, 0, size.y_);

		auto command = REngine::default_render_command_get();
		command.viewport = rectCopy;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::viewport);
		REngine::default_render_command_set(command);

		viewport_ = rectCopy;

		// Disable scissor test, needs to be re-enabled by the user
		SetScissorTest(false);
	}

	void Graphics::SetBlendMode(BlendMode mode, bool alphaToCoverage)
	{
		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.blend_mode = blendMode_ = mode;
		command.pipeline_state_info.alpha_to_coverage_enabled = alphaToCoverage_ = alphaToCoverage;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetColorWrite(bool enable)
	{
		if (colorWrite_ == enable)
			return;
		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.color_write_enabled = colorWrite_ = enable;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetCullMode(CullMode mode)
	{
		if (cullMode_ == mode)
			return;

		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.cull_mode = cullMode_ = mode;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetDepthBias(float constantBias, float slopeScaledBias)
	{
		if (constantBias == constantDepthBias_ || slopeScaledDepthBias_ == slopeScaledBias)
			return;

		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.constant_depth_bias = constantDepthBias_ = constantBias;
		command.pipeline_state_info.slope_scaled_depth_bias = slopeScaledDepthBias_ = slopeScaledBias;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetDepthTest(CompareMode mode)
	{
		if (mode == depthTestMode_)
			return;

		depthTestMode_ = mode;
		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.depth_cmp_function = mode;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetDepthWrite(bool enable)
	{
		if (enable == depthWrite_)
			return;

		depthWrite_ = enable;
		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.depth_write_enabled = enable;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetFillMode(FillMode mode)
	{
		if (mode == fillMode_)
			return;

		fillMode_ = mode;
		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.fill_mode = mode;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetLineAntiAlias(bool enable)
	{
		if (enable == lineAntiAlias_)
			return;

		lineAntiAlias_ = enable;
		auto command = REngine::default_render_command_get();
		command.pipeline_state_info.line_anti_alias = enable;
		command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		REngine::default_render_command_set(command);
	}

	void Graphics::SetScissorTest(bool enable, const Rect& rect, bool borderInclusive)
	{
		auto command = REngine::default_render_command_get();
		// During some light rendering loops, a full rect is toggled on/off repeatedly.
		// Disable scissor in that case to reduce state changes
		if (rect.min_.x_ <= 0.0f && rect.min_.y_ <= 0.0f && rect.max_.x_ >= 1.0f && rect.max_.y_ >= 1.0f)
			enable = false;

		if (enable)
		{
			IntVector2 rtSize(GetRenderTargetDimensions());
			IntVector2 viewSize(viewport_.Size());
			IntVector2 viewPos(viewport_.left_, viewport_.top_);
			IntRect intRect;
			int expand = borderInclusive ? 1 : 0;

			intRect.left_ = Clamp((int)((rect.min_.x_ + 1.0f) * 0.5f * viewSize.x_) + viewPos.x_, 0, rtSize.x_ - 1);
			intRect.top_ = Clamp((int)((-rect.max_.y_ + 1.0f) * 0.5f * viewSize.y_) + viewPos.y_, 0, rtSize.y_ - 1);
			intRect.right_ = Clamp((int)((rect.max_.x_ + 1.0f) * 0.5f * viewSize.x_) + viewPos.x_ + expand, 0, rtSize.x_);
			intRect.bottom_ = Clamp((int)((-rect.min_.y_ + 1.0f) * 0.5f * viewSize.y_) + viewPos.y_ + expand, 0, rtSize.y_);

			if (intRect.right_ == intRect.left_)
				intRect.right_++;
			if (intRect.bottom_ == intRect.top_)
				intRect.bottom_++;

			if (intRect.right_ < intRect.left_ || intRect.bottom_ < intRect.top_)
				enable = false;

			if (enable && intRect != scissorRect_)
			{
				scissorRect_ = intRect;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::scissor);
			}
		}

		command.scissor = scissorRect_;
		command.pipeline_state_info.scissor_test_enabled = enable;
		if (enable != scissorTest_)
		{
			scissorTest_ = enable;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		}

		REngine::default_render_command_set(command);
	}

	void Graphics::SetScissorTest(bool enable, const IntRect& rect)
	{
		auto command = REngine::default_render_command_get();
		IntVector2 rtSize(GetRenderTargetDimensions());
		IntVector2 viewPos(viewport_.left_, viewport_.top_);

		if (enable)
		{
			IntRect intRect;
			intRect.left_ = Clamp(rect.left_ + viewPos.x_, 0, rtSize.x_ - 1);
			intRect.top_ = Clamp(rect.top_ + viewPos.y_, 0, rtSize.y_ - 1);
			intRect.right_ = Clamp(rect.right_ + viewPos.x_, 0, rtSize.x_);
			intRect.bottom_ = Clamp(rect.bottom_ + viewPos.y_, 0, rtSize.y_);

			if (intRect.right_ == intRect.left_)
				intRect.right_++;
			if (intRect.bottom_ == intRect.top_)
				intRect.bottom_++;

			if (intRect.right_ < intRect.left_ || intRect.bottom_ < intRect.top_)
				enable = false;

			if (enable && intRect != scissorRect_)
			{
				scissorRect_ = intRect;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::scissor);
			}
		}

		command.scissor = scissorRect_;
		command.pipeline_state_info.scissor_test_enabled = enable;
		if (enable != scissorTest_)
		{
			scissorTest_ = enable;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		}

		REngine::default_render_command_set(command);
	}

	void Graphics::SetStencilTest(bool enable, CompareMode mode, StencilOp pass, StencilOp fail, StencilOp zFail,
		unsigned stencilRef,
		unsigned compareMask, unsigned writeMask)
	{
		auto command = REngine::default_render_command_get();
		if (enable != stencilTest_)
		{
			stencilTest_ = enable;
			command.pipeline_state_info.stencil_test_enabled = enable;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		}

		if (enable)
		{
			if (mode != stencilTestMode_)
			{
				stencilTestMode_ = mode;
				command.pipeline_state_info.stencil_cmp_function = mode;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
			if (pass != stencilPass_)
			{
				stencilPass_ = pass;
				command.pipeline_state_info.stencil_op_on_passed = pass;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
			if (fail != stencilFail_)
			{
				stencilFail_ = fail;
				command.pipeline_state_info.stencil_op_on_stencil_failed = fail;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
			if (zFail != stencilZFail_)
			{
				stencilZFail_ = zFail;
				command.pipeline_state_info.stencil_op_depth_failed = zFail;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
			if (compareMask != stencilCompareMask_)
			{
				stencilCompareMask_ = compareMask;
				command.pipeline_state_info.stencil_cmp_mask = static_cast<uint8_t>(compareMask);
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
			if (writeMask != stencilWriteMask_)
			{
				stencilWriteMask_ = writeMask;
				command.pipeline_state_info.stencil_write_mask = static_cast<uint8_t>(writeMask);
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
			if (stencilRef != stencilRef_)
			{
				stencilRef_ = stencilRef;
				command.stencil_ref = static_cast<uint8_t>(stencilRef);
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
			}
		}

		REngine::default_render_command_set(command);
	}

	void Graphics::SetClipPlane(bool enable, const Plane& clipPlane, const Matrix3x4& view, const Matrix4& projection)
	{
		useClipPlane_ = enable;

		if (!enable)
			return;

		Matrix4 viewProj = projection * view;
		clipPlane_ = clipPlane.Transformed(viewProj).ToVector4();
		SetShaderParameter(VSP_CLIPPLANE, clipPlane_);
	}

	bool Graphics::IsInitialized() const
	{
		return window_ != nullptr && impl_->IsInitialized();
	}

	PODVector<int> Graphics::GetMultiSampleLevels() const
	{
		if (!impl_->IsInitialized())
			return {};
		return impl_->GetMultiSampleLevels(impl_->GetSwapChain()->GetDesc().ColorBufferFormat,
			impl_->GetSwapChain()->GetDesc().DepthBufferFormat);
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
		return index < MAX_VERTEX_STREAMS ? vertexBuffers_[index] : nullptr;
	}

	TextureUnit Graphics::GetTextureUnit(const String& name)
	{
		HashMap<String, TextureUnit>::Iterator i = textureUnits_.Find(name);
		if (i != textureUnits_.End())
			return i->second_;
		else
			return MAX_TEXTURE_UNITS;
	}

	const String& Graphics::GetTextureUnitName(TextureUnit unit)
	{
		for (HashMap<String, TextureUnit>::Iterator i = textureUnits_.Begin(); i != textureUnits_.End(); ++i)
		{
			if (i->second_ == unit)
				return i->first_;
		}
		return String::EMPTY;
	}

	Texture* Graphics::GetTexture(unsigned index) const
	{
		return index < MAX_TEXTURE_UNITS ? textures_[index] : nullptr;
	}

	RenderSurface* Graphics::GetRenderTarget(unsigned index) const
	{
		return index < MAX_RENDERTARGETS ? renderTargets_[index] : nullptr;
	}

	IntVector2 Graphics::GetRenderTargetDimensions() const
	{
		int width, height;

		if (renderTargets_[0])
		{
			width = renderTargets_[0]->GetWidth();
			height = renderTargets_[0]->GetHeight();
		}
		else if (depthStencil_) // Depth-only rendering
		{
			width = depthStencil_->GetWidth();
			height = depthStencil_->GetHeight();
		}
		else
		{
			width = width_;
			height = height_;
		}

		return IntVector2(width, height);
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
		if(flags & GRAPHICS_CLEAR_VERTEX_DECLARATIONS)
		{
			const auto cache_count = REngine::graphics_state_vertex_declarations_count();
			REngine::graphics_state_release_vertex_declarations();
			ATOMIC_LOGINFOF("Released (%d) Vertex Declarations", cache_count);
		}
		if(flags & GRAPHICS_CLEAR_CONSTANT_BUFFERS)
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
			window_ = SDL_CreateWindowFrom(externalWindow_, 0);

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
		throw std::exception("Not implemented window acquire");
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
		auto command = REngine::default_render_command_get();
		REngine::render_command_reset(this, command);
		REngine::default_render_command_set(command);

		memset(textures_, 0x0, sizeof(Texture*) * MAX_TEXTURE_UNITS);
	}

	void Graphics::PrepareDraw()
	{
		REngine::RenderCommandProcessDesc process_desc;
		process_desc.driver = impl_;
		process_desc.graphics = this;

		auto command = REngine::default_render_command_get();

		// setup depth stencil
		if (command.dirty_state & static_cast<unsigned>(REngine::RenderCommandDirtyState::depth_stencil))
		{
			auto depth_stencil = (depthStencil_ && depthStencil_->GetUsage() == TEXTURE_DEPTHSTENCIL) ?
				depthStencil_->GetRenderTargetView() : impl_->GetSwapChain()->GetDepthBufferDSV();

			if(!depthWrite_ && depthStencil_ && depthStencil_->GetReadOnlyView())
				depth_stencil = depthStencil_->GetReadOnlyView();
			else if(renderTargets_[0] && !depthStencil_)
			{
				if (renderTargets_[0]->GetWidth() < GetWidth() || renderTargets_[0]->GetHeight() < GetHeight())
					depth_stencil = nullptr;
			}

			if (command.depth_stencil != depth_stencil)
				command.depth_stencil = depth_stencil;
		}

		// setup render targets
		if (command.dirty_state & static_cast<unsigned>(REngine::RenderCommandDirtyState::render_targets))
		{

			unsigned num_rts = 0;
			for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
			{
				if (!renderTargets_[i])
					continue;

				command.render_targets[num_rts] = (renderTargets_[i] && renderTargets_[i]->GetUsage() == TEXTURE_RENDERTARGET)
					? renderTargets_[i]->GetRenderTargetView() : Diligent::RefCntAutoPtr<Diligent::ITextureView>();
				num_rts = i + 1;
			}
			command.num_rts = static_cast<uint8_t>(num_rts);

			if (!renderTargets_[0] &&
				(!depthStencil_ || (depthStencil_ && depthStencil_->GetWidth() == width_ && depthStencil_->GetHeight() == height_)))
			{
				command.num_rts = 1;
				command.render_targets[0] = impl_->GetSwapChain()->GetCurrentBackBufferRTV();
			}
		}

		// setup textures
		if (command.dirty_state & static_cast<unsigned>(REngine::RenderCommandDirtyState::textures) && command.shader_program)
		{
			unsigned next_tex_idx = 0;
			for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
			{
				if (textures_[i] == nullptr)
					continue;

				REngine::SamplerDesc sampler_desc;
				textures_[i]->GetSamplerDesc(sampler_desc);

				const auto tex_names = REngine::utils_get_texture_unit_names(static_cast<TextureUnit>(i));
				for (const auto& tex_name : tex_names)
				{
					if (!command.shader_program->IsInUseTexture(tex_name))
						continue;
					command.textures[tex_name] = textures_[i]->GetShaderResourceView();
					command.pipeline_state_info.immutable_samplers[next_tex_idx].name = tex_name;
					command.pipeline_state_info.immutable_samplers[next_tex_idx].sampler = sampler_desc;
					++next_tex_idx;
				}
			}
			command.pipeline_state_info.num_samplers = next_tex_idx;
			command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);
		}

		auto pipeline_hash = 0u;
		auto vertex_decl = 0u;
		if (command.dirty_state & static_cast<uint32_t>(REngine::RenderCommandDirtyState::vertex_decl) && vertexShader_)
		{
			uint32_t new_vertex_decl_hash = 0;
			for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
			{
				if (vertexBuffers_[i] == nullptr)
					continue;
				CombineHash(new_vertex_decl_hash, static_cast<uint32_t>(vertexBuffers_[i]->GetBufferHash(i)));
			}

			if (new_vertex_decl_hash)
			{
				CombineHash(new_vertex_decl_hash, vertexShader_->ToHash());
				CombineHash(new_vertex_decl_hash, vertexShader_->GetElementHash());
				auto vertex_decl = REngine::graphics_state_get_vertex_declaration(new_vertex_decl_hash);
				if (!vertex_decl)
				{
					REngine::VertexDeclarationCreationDesc creation_desc;
					creation_desc.graphics = this;
					creation_desc.vertex_shader = vertexShader_;
					creation_desc.vertex_buffers = vertexBuffers_;
					vertex_decl = SharedPtr<REngine::VertexDeclaration>(
						new REngine::VertexDeclaration(creation_desc)
					);
					REngine::graphics_state_set_vertex_declaration(new_vertex_decl_hash, vertex_decl);
				}

				command.pipeline_state_info.input_layout = vertex_decl->GetInputLayoutDesc();
				command.vertex_decl_hash = new_vertex_decl_hash;
				command.dirty_state |= static_cast<unsigned>(REngine::RenderCommandDirtyState::pipeline);

				pipeline_hash = pipeline_hash;
			}
			pipeline_hash = command.pipeline_state_info.ToHash();
			vertex_decl = new_vertex_decl_hash;
			command.dirty_state ^= static_cast<unsigned>(REngine::RenderCommandDirtyState::vertex_decl);
		}

		REngine::render_command_process(process_desc, command);
		REngine::default_render_command_set(command);

		impl_->UploadBufferChanges();
	}

	void Graphics::CreateResolveTexture()
	{
		throw new std::exception("Not implemented");
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
