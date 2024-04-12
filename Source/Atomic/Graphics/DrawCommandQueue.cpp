#include "./DrawCommandQueue.h"
#include "../Core/Context.h"

namespace Atomic
{
	DrawCommandQueue::DrawCommandQueue(Context* context) : Object(context),
		commands_({})
	{
	}

	void DrawCommandQueue::ClearStoredCommands()
	{
		commands_.clear();
	}

	void DrawCommandQueue::AddCommand(ea::shared_ptr<IDrawCommand> command)
	{
		commands_.push_back(command);
	}

	ea::shared_ptr<IDrawCommand> DrawCommandQueue::CreateImmediateCommand()
	{
		Graphics* graphics = GetSubsystem<Graphics>();
		ea::shared_ptr<IDrawCommand> command(REngine::graphics_create_command(graphics));
		commands_.push_back(command);
		return command;
	}

	void DrawCommandQueue::RegisterObject(Context* context)
	{
		context->RegisterSubsystem(new DrawCommandQueue(context));
	}
}
