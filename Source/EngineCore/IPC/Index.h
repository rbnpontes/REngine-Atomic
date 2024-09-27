#pragma once
#include "./IPC.h"
#include "./IPCBroker.h"
#include "./IPCChannel.h"
#include "./IPCEvents.h"
#include "./IPCMessage.h"
#include "./IPCServer.h"
#include "./IPCTypes.h"
#ifdef ENGINE_PLATFORM_LINUX
    #include "./IPCUnix.h"
#endif
#ifdef ENGINE_PLATFORM_WINDOWS
    #include "./IPCWindows.h"
#endif
#include "./IPCWorker.h"