//
// Copyright (c) 2008-2017 the Urho3D project.
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

#include "../Container/List.h"
#include "../Core/Mutex.h"
#include "../Core/Object.h"
#include "../Core/StringUtils.h"

namespace Atomic
{

	/// Fictional message level to indicate a stored raw message.
	static const int LOG_RAW = -1;
	/// Debug message level. By default only shown in debug mode.
	static const int LOG_DEBUG = 0;
	/// Informative message level.
	static const int LOG_INFO = 1;
	/// Warning message level.
	static const int LOG_WARNING = 2;
	/// Error message level.
	static const int LOG_ERROR = 3;
	/// Success message level
	static const int LOG_SUCCESS = 4;

	/// Disable all log messages.
	static const int LOG_NONE = 5;

	class File;

	/// Stored log message from another thread.
	struct StoredLogMessage
	{
		/// Construct undefined.
		StoredLogMessage()
		{
		}

		/// Construct with parameters.
		StoredLogMessage(const String& message, int level, bool error) :
			message_(message),
			level_(level),
			error_(error)
		{
		}

		/// Message text.
		String message_;
		/// Message level. -1 for raw messages.
		int level_;
		/// Error flag for raw messages.
		bool error_;
	};

	/// Logging subsystem.
	class ATOMIC_API Log : public Object
	{
		ATOMIC_OBJECT(Log, Object);

	public:
		/// Construct.
		Log(Context* context);
		/// Destruct. Close the log file if open.
		virtual ~Log();

		/// Open the log file.
		void Open(const String& fileName);
		/// Close the log file.
		void Close();
		/// Set logging level.
		void SetLevel(int level);
		/// Set whether to timestamp log messages.
		void SetTimeStamp(bool enable);
		/// Set quiet mode ie. only print error entries to standard error stream (which is normally redirected to console also). Output to log file is not affected by this mode.
		void SetQuiet(bool quiet);

		/// Return logging level.
		int GetLevel() const { return level_; }

		/// Return whether log messages are timestamped.
		bool GetTimeStamp() const { return timeStamp_; }

		/// Return last log message.
		String GetLastMessage() const { return lastMessage_; }

		/// Return whether log is in quiet mode (only errors printed to standard error stream).
		bool IsQuiet() const { return quiet_; }

		/// Write to the log. If logging level is higher than the level of the message, the message is ignored.
		static void Write(int level, const String& message);
		/// Write raw output to the log.
		static void WriteRaw(const String& message, bool error = false);

		// ATOMIC BEGIN

		const File* GetLogFile() const { return logFile_; }

		// ATOMIC END

	private:
		/// Handle end of frame. Process the threaded log messages.
		void HandleEndFrame(StringHash eventType, VariantMap& eventData);
		/// Execute Stored Log Message.
		void DigestStoredLog(const StoredLogMessage& log_msg);

		/// Mutex for threaded operation.
		Mutex logMutex_;
		/// Log messages from other threads.
		List<StoredLogMessage> threadMessages_;
		/// Log file.
		SharedPtr<File> logFile_;
		/// Last log message.
		String lastMessage_;
		/// Logging level.
		int level_;
		/// Timestamp log messages flag.
		bool timeStamp_;
		/// In write flag to prevent recursion.
		bool inWrite_;
		/// Quiet mode flag.
		bool quiet_;
	};

#ifdef ENGINE_LOGGING
#define ATOMIC_LOGDEBUG(message) Atomic::Log::Write(Atomic::LOG_DEBUG, message)
#define ATOMIC_LOGINFO(message) Atomic::Log::Write(Atomic::LOG_INFO, message)
#define ATOMIC_LOGWARNING(message) Atomic::Log::Write(Atomic::LOG_WARNING, message)
#define ATOMIC_LOGERROR(message) Atomic::Log::Write(Atomic::LOG_ERROR, message)
#define ATOMIC_LOGSUCCESS(message) Atomic::Log::Write(Atomic::LOG_SUCCESS, message)
#define ATOMIC_LOGRAW(message) Atomic::Log::WriteRaw(message)

#define ATOMIC_LOGDEBUGF(format, ...) Atomic::Log::Write(Atomic::LOG_DEBUG, Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_LOGINFOF(format, ...) Atomic::Log::Write(Atomic::LOG_INFO, Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_LOGWARNINGF(format, ...) Atomic::Log::Write(Atomic::LOG_WARNING, Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_LOGERRORF(format, ...) Atomic::Log::Write(Atomic::LOG_ERROR, Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_LOGSUCCESSF(format, ...) Atomic::Log::Write(Atomic::LOG_SUCCESS, Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_LOGRAWF(format, ...) Atomic::Log::WriteRaw(Atomic::ToString(format, ##__VA_ARGS__))

#define ATOMIC_CLASS_LOGDEBUG(klass, message) ATOMIC_LOGDEBUGF("[%s]: %s", #klass, message)
#define ATOMIC_CLASS_LOGINFO(klass, message) ATOMIC_LOGINFOF("[%s]: %s", #klass, message)
#define ATOMIC_CLASS_LOGWARNING(klass, message) ATOMIC_LOGWARNINGF("[%s]: %s", #klass, message)
#define ATOMIC_CLASS_LOGERROR(klass, message) ATOMIC_LOGERRORF("[%s]: %s", #klass, message)
#define ATOMIC_CLASS_LOGRAW(klass, message) ATOMIC_LOGRAWF("[%s]: %s", #klass, message)

#define ATOMIC_CLASS_LOGDEBUGF(klass, format, ...) Atomic::Log::Write(Atomic::LOG_DEBUG, Atomic::ToString("[%s]:", #klass) + Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_CLASS_LOGINFOF(klass, format, ...) Atomic::Log::Write(Atomic::LOG_INFO, Atomic::ToString("[%s]:", #klass) + Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_CLASS_LOGWARNINGF(klass, format, ...) Atomic::Log::Write(Atomic::LOG_WARNING, Atomic::ToString("[%s]:", #klass) + Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_CLASS_LOGERRORF(klass, format, ...) Atomic::Log::Write(Atomic::LOG_ERROR, Atomic::ToString("[%s]:", #klass) + Atomic::ToString(format, ##__VA_ARGS__))
#define ATOMIC_CLASS_LOGRAWF(klass, format, ...) Atomic::Log::WriteRaw(Atomic::ToString("[%s]:", #klass) + Atomic::ToString(format, ##__VA_ARGS__))
#else
#define ATOMIC_LOGDEBUG(message) ((void)0)
#define ATOMIC_LOGINFO(message) ((void)0)
#define ATOMIC_LOGWARNING(message) ((void)0)
#define ATOMIC_LOGERROR(message) ((void)0)
#define ATOMIC_LOGSUCCESS(message) ((void)0)
#define ATOMIC_LOGRAW(message) ((void)0)

#define ATOMIC_LOGDEBUGF(...) ((void)0)
#define ATOMIC_LOGINFOF(...) ((void)0)
#define ATOMIC_LOGWARNINGF(...) ((void)0)
#define ATOMIC_LOGERRORF(...) ((void)0)
#define ATOMIC_LOGSUCCESSF(...) ((void)0)
#define ATOMIC_LOGRAWF(...) ((void)0)

#define ATOMIC_CLASS_LOGDEBUG(klass, message) ((void)0)
#define ATOMIC_CLASS_LOGINFO(klass, message) ((void)0)
#define ATOMIC_CLASS_LOGWARNING(klass, message) ((void)0)
#define ATOMIC_CLASS_LOGERROR(klass, message) ((void)0)
#define ATOMIC_CLASS_LOGRAW(klass, message) ((void)0)

#define ATOMIC_CLASS_LOGDEBUGF(...) ((void)0)
#define ATOMIC_CLASS_LOGINFOF(...) ((void)0)
#define ATOMIC_CLASS_LOGWARNINGF(...) ((void)0)
#define ATOMIC_CLASS_LOGERRORF(...) ((void)0)
#define ATOMIC_CLASS_LOGRAWF(...) ((void)0)
#endif

}
