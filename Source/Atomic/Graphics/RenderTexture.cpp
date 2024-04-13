#include "./RenderTexture.h"
#include "./Graphics.h"
#include "../IO/FileSystem.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../Core/StringUtils.h"

namespace Atomic
{
	RenderTexture::RenderTexture(Context* context)
		: ResourceWithMetadata(context),
		back_buffer_({}),
		depth_stencil_({}),
		render_surface_({}),
		dirty_(true),
		size_(IntVector2::ZERO),
		format_(TextureFormat::TEX_FORMAT_UNKNOWN),
		depth_format_(TextureFormat::TEX_FORMAT_UNKNOWN)
	{
		back_buffer_ = ea::MakeShared<Texture2D>(context);
		depth_stencil_ = ea::MakeShared<Texture2D>(context);
		render_surface_ = ea::MakeShared<RenderSurface>(back_buffer_->GetRenderSurface());
		render_surface_->SetLinkedDepthStencil(depth_stencil_->GetRenderSurface());
	}

	RenderTexture::~RenderTexture()
	= default;

	void RenderTexture::RegisterObject(Context* context)
	{
		context->RegisterFactory<RenderTexture>();
	}

	bool RenderTexture::BeginLoad(Deserializer& source)
	{
		ResourceCache* cache = GetSubsystem<ResourceCache>();
		const auto xml_name = ReplaceExtension(GetName(), ".xml");
		load_parameters_ = cache->GetTempResource<XMLFile>(xml_name);

		return load_parameters_ != nullptr;
	}

	bool RenderTexture::EndLoad()
	{
		back_buffer_->SetName(GetName() + "@backbuffer");
		depth_stencil_->SetName(GetName() + "@depthstencil");

		SetParameters(load_parameters_);

		load_parameters_.Reset();
		return true;
	}

	void RenderTexture::SetNumLevels(u32 levels) const
	{
		back_buffer_->SetNumLevels(levels);
		depth_stencil_->SetNumLevels(levels);
	}
	void RenderTexture::SetFilterMode(TextureFilterMode mode) const
	{
		back_buffer_->SetFilterMode(mode);
		depth_stencil_->SetFilterMode(mode);
	}
	void RenderTexture::SetAddressMode(TextureCoordinate coord, TextureAddressMode mode) const
	{
		back_buffer_->SetAddressMode(coord, mode);
		depth_stencil_->SetAddressMode(coord, mode);
	}
	void RenderTexture::SetAnisotropy(u32 level) const
	{
		back_buffer_->SetAnisotropy(level);
		depth_stencil_->SetAnisotropy(level);
	}
	void RenderTexture::SetShadowCompare(bool enable) const
	{
		back_buffer_->SetShadowCompare(enable);
		depth_stencil_->SetShadowCompare(enable);
	}
	void RenderTexture::SetBorderColor(const Color& color) const
	{
		back_buffer_->SetBorderColor(color);
		depth_stencil_->SetBorderColor(color);
	}
	void RenderTexture::SetSRGB(bool enable)
	{
		back_buffer_->SetSRGB(enable);
		depth_stencil_->SetSRGB(enable);
	}
	void RenderTexture::SetSize(u32 width, u32 height)
	{
		size_ = IntVector2(static_cast<i32>(width), static_cast<i32>(height));
		dirty_ = true;
	}
	void RenderTexture::SetFormat(TextureFormat format, TextureFormat depth_fmt)
	{
		assert(format != TextureFormat::TEX_FORMAT_UNKNOWN);
		format_ = format;
		if (depth_fmt == TextureFormat::TEX_FORMAT_UNKNOWN)
			depth_fmt = Graphics::GetDepthStencilFormat();
		depth_format_ = depth_fmt;
		dirty_ = true;
	}
	void RenderTexture::SetMultiSample(i32 multi_sample)
	{
		multi_sample_ = multi_sample;
		dirty_ = true;
	}
	TextureFormat RenderTexture::GetSurfaceFormat() const
	{
		return format_;
	}
	TextureFormat RenderTexture::GetDepthFormat() const
	{
		return depth_format_;
	}
	u32 RenderTexture::GetWidth() const
	{
		return static_cast<u32>(size_.x_);
	}
	u32 RenderTexture::GetHeight() const
	{
		return static_cast<u32>(size_.y_);
	}
	u32 RenderTexture::GetLevels() const
	{
		return back_buffer_->GetLevels();
	}
	TextureFilterMode RenderTexture::GetFilterMode() const
	{
		return back_buffer_->GetFilterMode();
	}
	TextureAddressMode RenderTexture::GetAddressMode(TextureCoordinate coord) const
	{
		return back_buffer_->GetAddressMode(coord);
	}
	u32 RenderTexture::GetAnisotropy() const
	{
		return back_buffer_->GetAnisotropy();
	}
	bool RenderTexture::GetShadowCompare() const
	{
		return back_buffer_->GetShadowCompare();
	}
	Color RenderTexture::GetBorderColor() const
	{
		return back_buffer_->GetBorderColor();
	}
	bool RenderTexture::GetSRGB() const
	{
		return back_buffer_->GetSRGB();
	}
	int RenderTexture::GetMultiSample() const
	{
		return back_buffer_->GetMultiSample();
	}
	ea::shared_ptr<Texture2D> RenderTexture::GetBackBuffer()
	{
		UpdateBuffers();
		return back_buffer_;
	}
	ea::shared_ptr<Texture2D> RenderTexture::GetDepthStencil()
	{
		UpdateBuffers();
		return depth_stencil_;
	}
	ea::shared_ptr<RenderSurface> RenderTexture::GetRenderSurface()
	{
		UpdateBuffers();
		return render_surface_;
	}

	void RenderTexture::SetParameters(XMLFile* file)
	{
		if (!file)
			return;

		XMLElement root_elem = file->GetRoot();
		// Set File Metadata
		for(XMLElement elem = root_elem.GetChild("metadata"); elem; elem = elem.GetNext("metadata"))
			AddMetadata(elem.GetAttribute("name"), elem.GetVariant());
		for(XMLElement param_elem = root_elem.GetChild(); param_elem; param_elem = param_elem.GetNext())
		{
			const auto name = param_elem.GetName();
			if (name == "address")
			{
				const auto coord = param_elem.GetAttributeLower("coord");
				if (coord.Length() >= 1)
				{
					const auto coord_idx = static_cast<TextureCoordinate>(coord[0] - 'u');
					String mode = param_elem.GetAttributeLower("mode");
					SetAddressMode(
						coord_idx, 
						static_cast<TextureAddressMode>(GetStringListIndex(mode.CString(), Texture::GetTextureAddressModeNames(), ADDRESS_WRAP)));
				}
			}

			if (name == "border")
				SetBorderColor(param_elem.GetColor("color"));

			if(name == "filter")
			{
				const auto mode = param_elem.GetAttributeLower("mode");
				SetFilterMode(static_cast<TextureFilterMode>(GetStringListIndex(mode.CString(), Texture::GetTextureFilterModeNames(), FILTER_NEAREST)));
				if(param_elem.HasAttribute("anisotropy"))
					SetAnisotropy(param_elem.GetUInt("anisotropy"));
			}

			if (name == "mipmap")
				SetNumLevels(param_elem.GetBool("enable") ? 0 : 1);

			if(name == "srgb")
				SetSRGB(param_elem.GetBool("enable"));

			// TODO: Add more parameters
		}
	}

	void RenderTexture::UpdateBuffers()
	{
		if(!dirty_)
			return;
		back_buffer_->SetSize(size_.x_, size_.y_, format_, TEXTURE_RENDERTARGET, multi_sample_, true);
		depth_stencil_->SetSize(size_.x_, size_.y_, depth_format_, TEXTURE_DEPTHSTENCIL);
		dirty_ = false;
	}
}
