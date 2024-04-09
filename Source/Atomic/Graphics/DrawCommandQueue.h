#pragma once
#include "./DrawCommand.h"

namespace Atomic
{
	class ATOMIC_API DrawCommandQueue : public Object
	{
		ATOMIC_OBJECT(DrawCommandQueue, Object);
	public:
		DrawCommandQueue(Context* context);

		static void RegisterObject(Context* context);
	private:
		ea::vector<ea::shared_ptr<IDrawCommand>> commands_in_use_;
		ea::queue<ea::shared_ptr<IDrawCommand>> available_commands_;
	};
}
