#include "./WebClientRequestHandler.h"

#include <EngineCore/IO/Log.h>

#include "include/wrapper/cef_helpers.h"

namespace REngine
{
	static constexpr const char* s_termination_status_names[] = {
        "TS_ABNORMAL_TERMINATION",
        "TS_PROCESS_WAS_KILLED",
        "TS_PROCESS_CRASHED",
        "TS_PROCESS_OOM",
        "TS_LAUNCH_FAILED",
        "TS_INTEGRITY_FAILURE"
    };
	WebClientRequestHandler::WebClientRequestHandler(CefMessageRouterBrowserSide* browser_router) :
		browser_router_(browser_router)
	{
	}

	void WebClientRequestHandler::OnRenderViewReady(CefRefPtr<CefBrowser> browser)
	{
	}

	bool WebClientRequestHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)
	{
		CEF_REQUIRE_UI_THREAD();
		browser_router_->OnBeforeBrowse(browser, frame);
		return false;
	}

	void WebClientRequestHandler::OnRenderProcessTerminated(
		CefRefPtr<CefBrowser> browser, 
		TerminationStatus status, 
		int error_code, 
		const CefString& error_string)
	{
		CEF_REQUIRE_UI_THREAD();
		ATOMIC_LOGERRORF("Render Process Terminated with error code (%x). Status=%s, Message=%s",
            error_code,
            s_termination_status_names[status],
            error_string.ToString().c_str());
		browser_router_->OnRenderProcessTerminated(browser);
	}
}
