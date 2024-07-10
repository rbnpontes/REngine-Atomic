#include "./TextureManager.h"
#include "../IO/Log.h"

typedef Diligent::RefCntAutoPtr<Diligent::ITexture> TextureRef;
namespace REngine
{
	static ea::hash_map<u32, TextureRef> s_tmp_textures;
	static ea::hash_map<Atomic::TextureFormat, TextureRef> s_stg_textures;
	static u32 s_textures_alive = 0;

	ea::string texture_manager_internal_build_name(const ea::string& name, const ea::string& identifier)
	{
		ea::string result = identifier;
		if(!name.empty())
		{
			result += " | ";
			result += name;
		}

		return result;
	}
	TextureRef texture_manager_internal_create(const Diligent::TextureDesc desc, Diligent::IRenderDevice* device)
	{
		const auto hash = texture_manager_to_hash(desc);
		const auto tex_it = s_tmp_textures.find_as(hash);
		if (tex_it != s_tmp_textures.end() && tex_it->second)
			return tex_it->second;

		TextureRef tex;
		device->CreateTexture(desc, nullptr, &tex);
		if(!tex)
		{
			ATOMIC_LOGERRORF("Failed to create texture %s", desc.Name);
			return tex;
		}

		++s_textures_alive;
		s_tmp_textures[hash] = tex;
		return tex;
	}

	u32 texture_manager_to_hash(const Diligent::TextureDesc desc)
	{
		u32 hash = desc.Type;
		Atomic::CombineHash(hash, desc.Width);
		Atomic::CombineHash(hash, desc.Height);
		Atomic::CombineHash(hash, desc.ArraySizeOrDepth());
		Atomic::CombineHash(hash, desc.Format);
		Atomic::CombineHash(hash, desc.MipLevels);
		Atomic::CombineHash(hash, desc.SampleCount);
		Atomic::CombineHash(hash, desc.BindFlags);
		Atomic::CombineHash(hash, desc.Usage);
		Atomic::CombineHash(hash, desc.CPUAccessFlags);
		Atomic::CombineHash(hash, desc.MiscFlags);
		Atomic::CombineHash(hash, desc.ClearValue.Format);
		Atomic::CombineHash(hash, desc.ClearValue.DepthStencil.Depth);
		Atomic::CombineHash(hash, desc.ClearValue.DepthStencil.Stencil);
		for (u8 i = 0; i < 4; ++i)
			Atomic::CombineHash(hash, desc.ClearValue.Color[i]);
		Atomic::CombineHash(hash, desc.ImmediateContextMask);

		return hash;
	}

	TextureRef texture_manager_tex2d_get_resolve(const TextureManagerCreateInfo& create_info)
	{
		const auto driver = create_info.driver;
		const auto device = driver->GetDevice();
		const auto dbg_name = texture_manager_internal_build_name(create_info.name, "ResolveTexture");

		Diligent::TextureDesc tex_desc;
		tex_desc.Name = dbg_name.c_str();
		tex_desc.Width = create_info.width;
		tex_desc.Height = create_info.height;
		tex_desc.MipLevels = 1;
		tex_desc.ArraySize = 1;
		tex_desc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		tex_desc.SampleCount = 1;
		tex_desc.Usage = Diligent::USAGE_DEFAULT;
		tex_desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		tex_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;

		return texture_manager_internal_create(tex_desc, device);
	}

	TextureRef texture_manager_tex2d_get_stg(const TextureManagerStgCreateInfo& create_info)
	{
		const auto driver = create_info.driver;
		const auto device = driver->GetDevice();

		const auto tex_it = s_stg_textures.find_as(create_info.format);
		if(tex_it != s_stg_textures.end() && tex_it->second)
		{
			const auto desc = tex_it->second->GetDesc();
			if (desc.Width < create_info.width && desc.Height < create_info.height)
				return tex_it->second;
		}

		Diligent::TextureDesc tex_desc;
		tex_desc.Name = "StagingTexture";
		tex_desc.Width = create_info.width;
		tex_desc.Height = create_info.height;
		tex_desc.MipLevels = 1;
		tex_desc.ArraySize = 1;
		tex_desc.SampleCount = 1;
		tex_desc.Format = create_info.format;
		tex_desc.Usage = Diligent::USAGE_STAGING;
		tex_desc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
		tex_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;

		TextureRef result;
		device->CreateTexture(tex_desc, nullptr, &result);

		s_stg_textures[create_info.format] = result;
		return result;
	}

	u32 texture_manager_tmp_textures_count()
	{
		return s_textures_alive;
	}

	u32 texture_manager_stg_textures_count()
	{
		u32 count = 0;
		for(const auto& tex : s_stg_textures)
		{
			if (tex.second)
				++count;
		}
		return count;
	}

	void texture_manager_clear_tmp_textures()
	{
		if (s_textures_alive == 0)
			return;
		for(auto& it : s_tmp_textures)
		{
			if (!it.second)
				return;
			if (it.second->GetReferenceCounters()->GetNumStrongRefs() > 1)
				return;
			it.second = nullptr;
			--s_textures_alive;
		}
	}

	void texture_manager_clear_stg_textures()
	{
		s_stg_textures.clear(true);
	}

}