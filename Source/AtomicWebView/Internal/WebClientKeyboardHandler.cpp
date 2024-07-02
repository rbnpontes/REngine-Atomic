#include "WebClientKeyboardHandler.h"

namespace REngine
{
	bool WebClientKeyboardHandler::OnPreKeyEvent(
		CefRefPtr<CefBrowser> browser,
		const CefKeyEvent& event, 
		CefEventHandle os_event, 
		bool* is_keyboard_shortcut)
	{
		return false;
	}

}
