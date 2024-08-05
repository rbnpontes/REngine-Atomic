#include "./ShaderParametersCache.h"

namespace REngine
{
	static ea::hash_map<u32, ea::shared_ptr<Atomic::ShaderParameter>> s_params_map = {};

	bool shader_parameters_cache_get(const u32& hash, Atomic::ShaderParameter** output)
	{
		const auto it = s_params_map.find_as(hash);
		if (it == s_params_map.end())
			return false;
		*output = it->second.get();
		return true;
	}

	void shader_parameters_cache_add(const u32& hash, const Atomic::ShaderParameter& param)
	{
		const auto target = ea::make_shared<Atomic::ShaderParameter>();
		*target = param;
		s_params_map[hash] = target;
	}

	bool shader_parameters_cache_has(const u32& hash)
	{
		return s_params_map.find_as(hash) != s_params_map.end();
	}

	void shader_parameters_cache_clear()
	{
		s_params_map.clear();
	}

	u32 shader_parameters_cache_count()
	{
		return s_params_map.size();
	}
}
