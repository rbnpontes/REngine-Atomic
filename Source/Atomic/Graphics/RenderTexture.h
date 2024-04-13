#pragma once
#include "./Texture2D.h"

namespace Atomic
{
	class RENGINE_API RenderTexture : public ResourceWithMetadata
	{
		ATOMIC_OBJECT(RenderTexture, ResourceWithMetadata)
	public:
		RenderTexture(Context* context);
		~RenderTexture() override;
		static void RegisterObject(Context* context);
		
		bool BeginLoad(Deserializer& source) override;
		bool EndLoad() override;
		void SetNumLevels(u32 levels) const;
		void SetFilterMode(TextureFilterMode mode) const;
		void SetAddressMode(TextureCoordinate coord, TextureAddressMode mode) const;
		void SetAnisotropy(u32 level) const;
		void SetShadowCompare(bool enable) const;
		void SetBorderColor(const Color& color) const;
		void SetSRGB(bool enable);
		void SetSize(u32 width, u32 height);
		void SetSize(const IntVector2& size) { SetSize(size.x_, size.y_); }
		void SetFormat(TextureFormat format, TextureFormat depth_fmt = TextureFormat::TEX_FORMAT_UNKNOWN);
		void SetMultiSample(i32 multi_sample);

		TextureFormat GetSurfaceFormat() const;
		TextureFormat GetDepthFormat() const;
		u32 GetWidth() const;
		u32 GetHeight() const;
		IntVector2 GetSize() const { return IntVector2(
			static_cast<i32>(GetWidth()), 
			static_cast<i32>(GetHeight()));
		}
		u32 GetLevels() const;
		TextureFilterMode GetFilterMode() const;
		TextureAddressMode GetAddressMode(TextureCoordinate coord) const;
		u32 GetAnisotropy() const;
		bool GetShadowCompare() const;
		Color GetBorderColor() const;
		bool GetSRGB() const;
		int GetMultiSample() const;

		/// Get back buffer texture. Don't change internal texture data.
		ea::shared_ptr<Texture2D> GetBackBuffer();
		/// Get depth stencil texture. Don't change internal texture data.
		ea::shared_ptr<Texture2D> GetDepthStencil();
		ea::shared_ptr<RenderSurface> GetRenderSurface();
	private:
		void SetParameters(XMLFile* file);
		void UpdateBuffers();
		ea::shared_ptr<Texture2D> back_buffer_;
		ea::shared_ptr<Texture2D> depth_stencil_;
		ea::shared_ptr<RenderSurface> render_surface_;

		bool dirty_;
		IntVector2 size_;
		i32 multi_sample_;
		TextureFormat format_;
		TextureFormat depth_format_;

		SharedPtr<XMLFile> load_parameters_;
	};
}