#pragma once
#include <EngineCore/Container/TypeTraits.h>

namespace REngine
{
	class IJavaScriptSystem
	{
	public:
		virtual void Eval(const ea::string& js_code) = 0;
		virtual void EvalFromFilePath(const ea::string& file_path) = 0;
	};
}