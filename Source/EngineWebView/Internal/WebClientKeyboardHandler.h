#pragma once
#include <include/cef_client.h>

namespace REngine
{
	class WebClientKeyboardHandler : public CefKeyboardHandler
	{
	public:
		bool OnPreKeyEvent(
			CefRefPtr<CefBrowser> browser, 
			const CefKeyEvent& event, 
			CefEventHandle os_event, 
			bool* is_keyboard_shortcut) override;
		IMPLEMENT_REFCOUNTING(WebClientKeyboardHandler);
	};
}
