#include "./WebUtils.h"

namespace REngine
{
	void web_utils_convert_cef_str(const CefString& input, Atomic::String& output)
	{
		cef_string_utf8_t utf8 = {};

		if(!cef_string_utf16_to_utf8(input.c_str(), input.length(), &utf8))
			return;

		output = utf8.str;
		cef_string_utf8_clear(&utf8);
	}

}