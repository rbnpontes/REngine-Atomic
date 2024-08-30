#pragma once
#include <EngineCore/Core/Plugin.h>
#include "./IJavaScriptSystem.h"
namespace REngine
{
	using namespace Atomic;
	class IJavaScriptPlugin : public IEnginePlugin
	{
	public:
		virtual IJavaScriptSystem* GetJavaScriptSystem(Context* context) = 0;
	};
}