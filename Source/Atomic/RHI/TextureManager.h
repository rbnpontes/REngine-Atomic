#pragma once
#include "./DriverInstance.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Container/Hash.h"

namespace REngine
{
	struct TextureManagerCreateInfo
	{
		ea::string name;
		u16 width;
		u16 height;
		Atomic::TextureFormat format;
		DriverInstance* driver;
	};

	Diligent::RefCntAutoPtr<Diligent::ITexture> texture_manager_tex2d_get_resolve(const TextureManagerCreateInfo& create_info);
	Diligent::RefCntAutoPtr<Diligent::ITexture> texture_manager_tex2d_get_stg(const TextureManagerCreateInfo& create_info);

	u32 texture_manager_to_hash(const Diligent::TextureDesc desc);
	u32 texture_manager_tmp_textures_count();
	void texture_manager_clear_tmp_textures();
}
