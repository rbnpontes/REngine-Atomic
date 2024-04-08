#include "./DrawCommandQueue.h"
#include "../Core/Context.h"

namespace Atomic
{
	DrawCommandQueue::DrawCommandQueue(Context* context) : Object(context),
		commands_({})
	{
	}

	void DrawCommandQueue::RegisterObject(Context* context)
	{
		context->RegisterFactory<DrawCommandQueue>();
	}
}
