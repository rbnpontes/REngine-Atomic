#pragma once
#include <EngineCore/Container/TypeTraits.h>

namespace REngine
{
	class IJavaScriptSystem
	{
	public:
		virtual bool Eval(const ea::string& js_code) = 0;
		virtual bool EvalFromFilePath(const ea::string& file_path) = 0;
		virtual size_t GetMemoryUsage() const = 0;
		virtual size_t GetMemoryBlocks() const = 0;
	};
}