//
// Copyright (c) 2017 the Atomic project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Container/Str.h"
#include "../Core/Thread.h"
#include "../Core/Timer.h"

#if ATOMIC_PROFILING

#if RENGINE_PROFILER_EASYPROFILER
#   include <easy/profiler.h>
#elif RENGINE_PROFILER_TRACY
#include <tracy/Tracy.hpp>
namespace profiler { class BaseBlockDescriptor {}; };
#endif

#else
namespace profiler { class BaseBlockDescriptor {}; };
#endif

namespace Atomic
{
#if RENGINE_PROFILER_TRACY
static const int PROFILER_DEFAULT_PORT = 8086;
#else
static const int PROFILER_DEFAULT_PORT = 28077;
#endif
static const uint32_t PROFILER_COLOR_DEFAULT = 0xffffecb3;
static const uint32_t PROFILER_COLOR_EVENTS = 0xffff9800;
static const uint32_t PROFILER_COLOR_RESOURCES = 0xff00bcd4;

// Copied from easy_profiler
enum ProfilerBlockStatus
{
    OFF = 0,
    ON = 1,
    FORCE_ON = ON | 2,
    OFF_RECURSIVE = 4,
    ON_WITHOUT_CHILDREN = ON | OFF_RECURSIVE,
    FORCE_ON_WITHOUT_CHILDREN = FORCE_ON | OFF_RECURSIVE,
};


/// Hierarchical performance profiler subsystem.
class ATOMIC_API Profiler : public Object
{
    ATOMIC_OBJECT(Profiler, Object);

public:
    /// Construct.
    Profiler(Context* context);
    /// Destruct.
    virtual ~Profiler();

    /// Enables or disables profiler.
    void SetEnabled(bool enabled);
    /// Returns true if profiler is enabled, false otherwise.
    bool GetEnabled() const;
    /// Enables or disables event profiling.
    void SetEventProfilingEnabled(bool enabled);
    /// Returns true if event profiling is enabled, false otherwise.
    bool GetEventProfilingEnabled() const;
    /// Starts listening for incoming profiler tool connections.
    void StartListen(unsigned short port=PROFILER_DEFAULT_PORT);
    /// Stops listening for incoming profiler tool connections.
    void StopListen();
    /// Returns true if profiler is currently listening for incoming connections.
    bool GetListening() const;
    /// Enables or disables event tracing. This is windows-specific, does nothing on other OS.
    void SetEventTracingEnabled(bool enable);
    /// Returns true if event tracing is enabled, false otherwise.
    bool GetEventTracingEnabled();
    /// Enables or disables low priority event tracing. This is windows-specific, does nothing on other OS.
    void SetLowPriorityEventTracing(bool isLowPriority);
    /// Returns true if low priority event tracing is enabled, false otherwise.
    bool GetLowPriorityEventTracing();
    /// Save profiler data to a file.
    void SaveProfilerData(const String& filePath);
    /// Begin non-scoped profiled block. Block has to be terminated with call to EndBlock(). This is slow and is for
    /// integration with scripting lnaguages. Use ATOMIC_PROFILE* macros when writing c++ code instead.
    void BeginBlock(const char* name, const char* file, int line, unsigned int argb=PROFILER_COLOR_DEFAULT,
                    unsigned char status=ProfilerBlockStatus::ON);
    /// End block started with BeginBlock().
    void EndBlock();

private:

    bool enableEventProfiling_ = true;
    HashMap<unsigned, ::profiler::BaseBlockDescriptor*> blockDescriptorCache_;
};

#if ATOMIC_PROFILING
#if RENGINE_PROFILER_EASYPROFILER
#   define ATOMIC_PROFILE_FRAME()
#   define ATOMIC_PROFILE(name, ...) EASY_BLOCK(#name, __VA_ARGS__)
#   define ATOMIC_PROFILE_SCOPED(name, ...) EASY_BLOCK(name, __VA_ARGS__)
#   define ATOMIC_PROFILE_NONSCOPED(name, ...) EASY_NONSCOPED_BLOCK(name, __VA_ARGS__)
#   define ATOMIC_PROFILE_END(...) EASY_END_BLOCK
#   define ATOMIC_PROFILE_THREAD(name) EASY_THREAD(name)
#elif RENGINE_PROFILER_TRACY
#   define ATOMIC_PROFILE_FRAME() FrameMark
//#   define ATOMIC_PROFILE_FIXED_ZONE(varname, name) ZoneNamedN(TracyConcat(__tracy_scoped_zone, TracyLine), #name, true)
#   define ATOMIC_PROFILE(name, ...) ZoneNamedN(TracyConcat(__tracy_scoped_zone, TracyLine), #name, true)
#   define ATOMIC_PROFILE_SCOPED_NON_CONST_ZONE(varname, name, active) \
    	static tracy::SourceLocationData TracyConcat(__tracy_source_location, TracyLine) { name, TracyFunction, TracyFile, (uint32_t)TracyLine, 0 }; \
        tracy::ScopedZone varname(&TracyConcat(__tracy_source_location, TracyLine), active)
//#   define ATOMIC_PROFILE_SCOPED(name, ...) ATOMIC_PROFILE_SCOPED_NON_CONST_ZONE(___tracy_scoped_zone, name, true)
#   define ATOMIC_PROFILE_SCOPED(name, ...) \
	ZoneNamed(TracyConcat(profiler_scoped, TracyLine), true); \
    ZoneTextV(TracyConcat(profiler_scoped, TracyLine), name, strlen(name))
#   define ATOMIC_PROFILE_NONSCOPED(name, ...)
#   define ATOMIC_PROFILE_END(...)
#   define ATOMIC_PROFILE_THREAD(name)
#   define ATOMIC_PROFILE_MSG(message) TracyMessageL(message)
#endif

#else
#   define ATOMIC_PROFILE_FRAME()
#   define ATOMIC_PROFILE(name, ...)
#   define ATOMIC_PROFILE_NONSCOPED(name, ...)
#   define ATOMIC_PROFILE_SCOPED(name, ...)
#   define ATOMIC_PROFILE_END(...)
#   define ATOMIC_PROFILE_THREAD(name)
#   define ATOMIC_PROFILE_MSG(message)
#endif

}
