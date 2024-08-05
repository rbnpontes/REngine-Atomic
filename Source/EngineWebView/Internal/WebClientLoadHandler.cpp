#include "./WebClientLoadHandler.h"
#include "./WebUtils.h"
#include "../WebViewEvents.h"

#include <EngineCore/IO/Log.h>

namespace REngine
{
	using namespace Atomic;
	WebClientLoadHandler::WebClientLoadHandler(ClientLoadHandlerSender sender) :
		sender_(sender)
	{
	}

	void WebClientLoadHandler::OnLoadStart(
		CefRefPtr<CefBrowser> browser, 
		CefRefPtr<CefFrame> frame,
		TransitionType transition_type)
	{
		if (sender_.Null() || !frame->IsMain())
			return;

		String target_url;
		web_utils_convert_cef_str(frame->GetURL(), target_url);

		VariantMap event_data;
		event_data[WebViewLoadStart::P_CLIENT] = sender_;
		event_data[WebViewLoadStart::P_URL] = target_url;

		sender_->SendEvent(E_WEBVIEWLOADSTART, event_data);
	}

	void WebClientLoadHandler::OnLoadEnd(
		CefRefPtr<CefBrowser> browser, 
		CefRefPtr<CefFrame> frame, 
		int httpStatusCode)
	{
		if (sender_.Null() || !frame->IsMain())
			return;

		String target_url;
		web_utils_convert_cef_str(frame->GetURL(), target_url);

		VariantMap event_data;
		event_data[WebViewLoadEnd::P_CLIENT] = sender_;
		event_data[WebViewLoadEnd::P_URL] = target_url;

		sender_->SendEvent(E_WEBVIEWLOADEND, event_data);
	}

	void WebClientLoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
	{
		if (sender_.Null() || !frame->IsMain())
			return;

		String error;
		String failed_url;
		web_utils_convert_cef_str(errorText, error);
		web_utils_convert_cef_str(failedUrl, failed_url);

		ATOMIC_LOGERRORF("Failed to load URL: %s\nCode:%d\nError:%s",
			failed_url.CString(),
			errorCode,
			error.CString()
		);
	}

	void WebClientLoadHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
		bool canGoForward)
	{
		if(sender_.Null())
			return;

		VariantMap event_data;
		event_data[WebViewLoadStateChange::P_CLIENT] = sender_;
		event_data[WebViewLoadStateChange::P_LOADING] = isLoading;
		event_data[WebViewLoadStateChange::P_CANGOBACK] = canGoBack;
		event_data[WebViewLoadStateChange::P_CANGOFORWARD] = canGoForward;

		sender_->SendEvent(E_WEBVIEWLOADSTATECHANGE, event_data);
	}
}
