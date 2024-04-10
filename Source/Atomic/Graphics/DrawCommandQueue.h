#pragma once
#include "./DrawCommand.h"

namespace Atomic
{
	class ATOMIC_API DrawCommandQueue : public Object
	{
		ATOMIC_OBJECT(DrawCommandQueue, Object);
	public:
		DrawCommandQueue(Context* context);
		void ClearStoredCommands();
		ea::shared_ptr<IDrawCommand> CreateImmediateCommand();
		static void RegisterObject(Context* context);
	private:
		ea::vector<ea::shared_ptr<IDrawCommand>> commands_;
	};
}
