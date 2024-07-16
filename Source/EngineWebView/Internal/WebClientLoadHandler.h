#pragma once
#include <include/cef_client.h>
#include "../WebClient.h"

namespace REngine
{
	typedef Atomic::WeakPtr<Atomic::WebClient> ClientLoadHandlerSender;
	class WebClientLoadHandler : public CefLoadHandler
	{
	public:
		WebClientLoadHandler(
			ClientLoadHandlerSender sender
		);

		void OnLoadStart(
			CefRefPtr<CefBrowser> browser, 
			CefRefPtr<CefFrame> frame, 
			TransitionType transition_type) override;
		void OnLoadEnd(
			CefRefPtr<CefBrowser> browser, 
			CefRefPtr<CefFrame> frame, 
			int httpStatusCode) override;
		void OnLoadError(
			CefRefPtr<CefBrowser> browser, 
			CefRefPtr<CefFrame> frame, 
			ErrorCode errorCode, 
			const CefString& errorText, 
			const CefString& failedUrl) override;
		void OnLoadingStateChange(
			CefRefPtr<CefBrowser> browser, 
			bool isLoading, 
			bool canGoBack, 
			bool canGoForward) override;
		IMPLEMENT_REFCOUNTING(WebClientLoadHandler);

		ClientLoadHandlerSender sender_;
	};
}