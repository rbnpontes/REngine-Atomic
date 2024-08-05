#pragma once
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>

namespace REngine
{
	class WebClientRequestHandler : public CefRequestHandler
	{
	public:
		WebClientRequestHandler(
			CefMessageRouterBrowserSide* browser_router
		);
		void OnRenderViewReady(CefRefPtr<CefBrowser> browser) override;
		bool OnBeforeBrowse(
			CefRefPtr<CefBrowser> browser, 
			CefRefPtr<CefFrame> frame, 
			CefRefPtr<CefRequest> request, 
			bool user_gesture, 
			bool is_redirect) override;
		void OnRenderProcessTerminated(
			CefRefPtr<CefBrowser> browser, 
			TerminationStatus status, 
			int error_code, 
			const CefString& error_string) override;
		IMPLEMENT_REFCOUNTING(WebClientRequestHandler);

		CefMessageRouterBrowserSide* browser_router_;
	};
}
