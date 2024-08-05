#pragma once
#include "./ShaderProgram.h"

namespace REngine
{
	bool shader_parameters_cache_get(const u32& hash, Atomic::ShaderParameter** output);
	void shader_parameters_cache_add(const u32& hash, const Atomic::ShaderParameter& param);
	bool shader_parameters_cache_has(const u32& hash);
	void shader_parameters_cache_clear();
	u32 shader_parameters_cache_count();
}