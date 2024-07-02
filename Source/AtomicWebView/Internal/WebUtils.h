#pragma once
#include <Atomic/Container/Str.h>
#include <include/cef_client.h>
namespace REngine
{
	void web_utils_convert_cef_str(const CefString& input, Atomic::String& output);
}
