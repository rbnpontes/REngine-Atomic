#pragma once
#include <EngineCore/Container/TypeTraits.h>
#include <EngineCore/IO/File.h>
#include <Duktape/duktape.h>

namespace REngine
{
	class IJavaScriptSystem
	{
	public:
		/**
		 * \brief Execute an arbitrary javascript code.
		 * \param js_code JavaScript Code
		 * \return success state
		 */
		virtual bool Eval(const ea::string& js_code) = 0;
		/**
		 * \brief Execute an arbitrary javascript file.
		 * \param script_file JavaScript File Code
		 * \return success state
		 */
		virtual bool Eval(Atomic::SharedPtr<Atomic::File> script_file) const = 0;
		/**
		 * \brief Execute JavaScript from a given path
		 * \param file_path Path to JavaScript file
		 * \return success state
		 */
		virtual bool EvalFromFilePath(const ea::string& file_path) = 0;
		/**
		 * \brief Execute pending timers like setImmediate, setTimeout and setInterval.
		 */
		virtual void ExecutePendingTimers() = 0;
		/**
		 * \brief Clear all pending timers, all timers like setImmediate, setTimeout and setInterval
		 * will be cleared.
		 */
		virtual void ClearAllTimers() = 0;
		/**
		 * \brief Check if system has any pending timers.
		 * Ex: if setTimeout is not yet executed
		 * \return 
		 */
		virtual bool HasPendingTimers() = 0;
		/**
		 * \brief Get Duktape heap context
		 * \return duktape context
		 */
		virtual duk_context* GetHeap() const = 0;
		/**
		 * \brief get current javascript allocated memory
		 * \return 
		 */
		virtual size_t GetMemoryUsage() const = 0;
		/**
		 * \brief get current memory blocks, each time js vm executes an malloc
		 * the a new block is created.
		 * \return 
		 */
		virtual size_t GetMemoryBlocks() const = 0;
	};
}