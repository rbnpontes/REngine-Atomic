// Copyright (c) 2024 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//
// $hash=8db751f9d33c0279c801c0820d8a15d4c14fede6$
//

#include "libcef_dll/ctocpp/browser_host_ctocpp.h"

#include "libcef_dll/cpptoc/client_cpptoc.h"
#include "libcef_dll/cpptoc/dev_tools_message_observer_cpptoc.h"
#include "libcef_dll/cpptoc/download_image_callback_cpptoc.h"
#include "libcef_dll/cpptoc/navigation_entry_visitor_cpptoc.h"
#include "libcef_dll/cpptoc/pdf_print_callback_cpptoc.h"
#include "libcef_dll/cpptoc/run_file_dialog_callback_cpptoc.h"
#include "libcef_dll/ctocpp/browser_ctocpp.h"
#include "libcef_dll/ctocpp/dictionary_value_ctocpp.h"
#include "libcef_dll/ctocpp/drag_data_ctocpp.h"
#include "libcef_dll/ctocpp/extension_ctocpp.h"
#include "libcef_dll/ctocpp/navigation_entry_ctocpp.h"
#include "libcef_dll/ctocpp/registration_ctocpp.h"
#include "libcef_dll/ctocpp/request_context_ctocpp.h"
#include "libcef_dll/shutdown_checker.h"
#include "libcef_dll/transfer_util.h"

// STATIC METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall")
bool CefBrowserHost::CreateBrowser(
    const CefWindowInfo& windowInfo,
    CefRefPtr<CefClient> client,
    const CefString& url,
    const CefBrowserSettings& settings,
    CefRefPtr<CefDictionaryValue> extra_info,
    CefRefPtr<CefRequestContext> request_context) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: client, url, extra_info, request_context

  // Execute
  int _retval = cef_browser_host_create_browser(
      &windowInfo, CefClientCppToC::Wrap(client), url.GetStruct(), &settings,
      CefDictionaryValueCToCpp::Unwrap(extra_info),
      CefRequestContextCToCpp::Unwrap(request_context));

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefBrowser> CefBrowserHost::CreateBrowserSync(
    const CefWindowInfo& windowInfo,
    CefRefPtr<CefClient> client,
    const CefString& url,
    const CefBrowserSettings& settings,
    CefRefPtr<CefDictionaryValue> extra_info,
    CefRefPtr<CefRequestContext> request_context) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: client, url, extra_info, request_context

  // Execute
  cef_browser_t* _retval = cef_browser_host_create_browser_sync(
      &windowInfo, CefClientCppToC::Wrap(client), url.GetStruct(), &settings,
      CefDictionaryValueCToCpp::Unwrap(extra_info),
      CefRequestContextCToCpp::Unwrap(request_context));

  // Return type: refptr_same
  return CefBrowserCToCpp::Wrap(_retval);
}

// VIRTUAL METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall")
CefRefPtr<CefBrowser> CefBrowserHostCToCpp::GetBrowser() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_browser)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_browser_t* _retval = _struct->get_browser(_struct);

  // Return type: refptr_same
  return CefBrowserCToCpp::Wrap(_retval);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::CloseBrowser(bool force_close) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, close_browser)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->close_browser(_struct, force_close);
}

NO_SANITIZE("cfi-icall") bool CefBrowserHostCToCpp::TryCloseBrowser() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, try_close_browser)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->try_close_browser(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::SetFocus(bool focus) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_focus)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_focus(_struct, focus);
}

NO_SANITIZE("cfi-icall")
CefWindowHandle CefBrowserHostCToCpp::GetWindowHandle() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_window_handle)) {
    return kNullWindowHandle;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_window_handle_t _retval = _struct->get_window_handle(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall")
CefWindowHandle CefBrowserHostCToCpp::GetOpenerWindowHandle() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_opener_window_handle)) {
    return kNullWindowHandle;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_window_handle_t _retval = _struct->get_opener_window_handle(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall") bool CefBrowserHostCToCpp::HasView() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, has_view)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->has_view(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefClient> CefBrowserHostCToCpp::GetClient() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_client)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_client_t* _retval = _struct->get_client(_struct);

  // Return type: refptr_diff
  return CefClientCppToC::Unwrap(_retval);
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefRequestContext> CefBrowserHostCToCpp::GetRequestContext() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_request_context)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_request_context_t* _retval = _struct->get_request_context(_struct);

  // Return type: refptr_same
  return CefRequestContextCToCpp::Wrap(_retval);
}

NO_SANITIZE("cfi-icall")
bool CefBrowserHostCToCpp::CanZoom(cef_zoom_command_t command) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, can_zoom)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->can_zoom(_struct, command);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::Zoom(cef_zoom_command_t command) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, zoom)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->zoom(_struct, command);
}

NO_SANITIZE("cfi-icall") double CefBrowserHostCToCpp::GetDefaultZoomLevel() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_default_zoom_level)) {
    return 0;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  double _retval = _struct->get_default_zoom_level(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall") double CefBrowserHostCToCpp::GetZoomLevel() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_zoom_level)) {
    return 0;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  double _retval = _struct->get_zoom_level(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SetZoomLevel(double zoomLevel) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_zoom_level)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_zoom_level(_struct, zoomLevel);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::RunFileDialog(
    FileDialogMode mode,
    const CefString& title,
    const CefString& default_file_path,
    const std::vector<CefString>& accept_filters,
    CefRefPtr<CefRunFileDialogCallback> callback) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, run_file_dialog)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: callback; type: refptr_diff
  DCHECK(callback.get());
  if (!callback.get()) {
    return;
  }
  // Unverified params: title, default_file_path, accept_filters

  // Translate param: accept_filters; type: string_vec_byref_const
  cef_string_list_t accept_filtersList = cef_string_list_alloc();
  DCHECK(accept_filtersList);
  if (accept_filtersList) {
    transfer_string_list_contents(accept_filters, accept_filtersList);
  }

  // Execute
  _struct->run_file_dialog(_struct, mode, title.GetStruct(),
                           default_file_path.GetStruct(), accept_filtersList,
                           CefRunFileDialogCallbackCppToC::Wrap(callback));

  // Restore param:accept_filters; type: string_vec_byref_const
  if (accept_filtersList) {
    cef_string_list_free(accept_filtersList);
  }
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::StartDownload(const CefString& url) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, start_download)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: url; type: string_byref_const
  DCHECK(!url.empty());
  if (url.empty()) {
    return;
  }

  // Execute
  _struct->start_download(_struct, url.GetStruct());
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::DownloadImage(
    const CefString& image_url,
    bool is_favicon,
    uint32_t max_image_size,
    bool bypass_cache,
    CefRefPtr<CefDownloadImageCallback> callback) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, download_image)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: image_url; type: string_byref_const
  DCHECK(!image_url.empty());
  if (image_url.empty()) {
    return;
  }
  // Verify param: callback; type: refptr_diff
  DCHECK(callback.get());
  if (!callback.get()) {
    return;
  }

  // Execute
  _struct->download_image(_struct, image_url.GetStruct(), is_favicon,
                          max_image_size, bypass_cache,
                          CefDownloadImageCallbackCppToC::Wrap(callback));
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::Print() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, print)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->print(_struct);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::PrintToPDF(const CefString& path,
                                      const CefPdfPrintSettings& settings,
                                      CefRefPtr<CefPdfPrintCallback> callback) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, print_to_pdf)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: path; type: string_byref_const
  DCHECK(!path.empty());
  if (path.empty()) {
    return;
  }
  // Unverified params: callback

  // Execute
  _struct->print_to_pdf(_struct, path.GetStruct(), &settings,
                        CefPdfPrintCallbackCppToC::Wrap(callback));
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::Find(const CefString& searchText,
                                bool forward,
                                bool matchCase,
                                bool findNext) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, find)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: searchText; type: string_byref_const
  DCHECK(!searchText.empty());
  if (searchText.empty()) {
    return;
  }

  // Execute
  _struct->find(_struct, searchText.GetStruct(), forward, matchCase, findNext);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::StopFinding(bool clearSelection) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, stop_finding)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->stop_finding(_struct, clearSelection);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ShowDevTools(const CefWindowInfo& windowInfo,
                                        CefRefPtr<CefClient> client,
                                        const CefBrowserSettings& settings,
                                        const CefPoint& inspect_element_at) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, show_dev_tools)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: windowInfo, client, settings, inspect_element_at

  // Execute
  _struct->show_dev_tools(_struct, &windowInfo, CefClientCppToC::Wrap(client),
                          &settings, &inspect_element_at);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::CloseDevTools() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, close_dev_tools)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->close_dev_tools(_struct);
}

NO_SANITIZE("cfi-icall") bool CefBrowserHostCToCpp::HasDevTools() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, has_dev_tools)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->has_dev_tools(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
bool CefBrowserHostCToCpp::SendDevToolsMessage(const void* message,
                                               size_t message_size) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_dev_tools_message)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: message; type: simple_byaddr
  DCHECK(message);
  if (!message) {
    return false;
  }

  // Execute
  int _retval = _struct->send_dev_tools_message(_struct, message, message_size);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
int CefBrowserHostCToCpp::ExecuteDevToolsMethod(
    int message_id,
    const CefString& method,
    CefRefPtr<CefDictionaryValue> params) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, execute_dev_tools_method)) {
    return 0;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: method; type: string_byref_const
  DCHECK(!method.empty());
  if (method.empty()) {
    return 0;
  }
  // Unverified params: params

  // Execute
  int _retval = _struct->execute_dev_tools_method(
      _struct, message_id, method.GetStruct(),
      CefDictionaryValueCToCpp::Unwrap(params));

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefRegistration> CefBrowserHostCToCpp::AddDevToolsMessageObserver(
    CefRefPtr<CefDevToolsMessageObserver> observer) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, add_dev_tools_message_observer)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: observer; type: refptr_diff
  DCHECK(observer.get());
  if (!observer.get()) {
    return nullptr;
  }

  // Execute
  cef_registration_t* _retval = _struct->add_dev_tools_message_observer(
      _struct, CefDevToolsMessageObserverCppToC::Wrap(observer));

  // Return type: refptr_same
  return CefRegistrationCToCpp::Wrap(_retval);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::GetNavigationEntries(
    CefRefPtr<CefNavigationEntryVisitor> visitor,
    bool current_only) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_navigation_entries)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: visitor; type: refptr_diff
  DCHECK(visitor.get());
  if (!visitor.get()) {
    return;
  }

  // Execute
  _struct->get_navigation_entries(
      _struct, CefNavigationEntryVisitorCppToC::Wrap(visitor), current_only);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ReplaceMisspelling(const CefString& word) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, replace_misspelling)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: word; type: string_byref_const
  DCHECK(!word.empty());
  if (word.empty()) {
    return;
  }

  // Execute
  _struct->replace_misspelling(_struct, word.GetStruct());
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::AddWordToDictionary(const CefString& word) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, add_word_to_dictionary)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: word; type: string_byref_const
  DCHECK(!word.empty());
  if (word.empty()) {
    return;
  }

  // Execute
  _struct->add_word_to_dictionary(_struct, word.GetStruct());
}

NO_SANITIZE("cfi-icall")
bool CefBrowserHostCToCpp::IsWindowRenderingDisabled() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_window_rendering_disabled)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_window_rendering_disabled(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::WasResized() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, was_resized)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->was_resized(_struct);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::WasHidden(bool hidden) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, was_hidden)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->was_hidden(_struct, hidden);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::NotifyScreenInfoChanged() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, notify_screen_info_changed)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->notify_screen_info_changed(_struct);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::Invalidate(PaintElementType type) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, invalidate)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->invalidate(_struct, type);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::SendExternalBeginFrame() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_external_begin_frame)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_external_begin_frame(_struct);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SendKeyEvent(const CefKeyEvent& event) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_key_event)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_key_event(_struct, &event);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SendMouseClickEvent(const CefMouseEvent& event,
                                               MouseButtonType type,
                                               bool mouseUp,
                                               int clickCount) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_mouse_click_event)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_mouse_click_event(_struct, &event, type, mouseUp, clickCount);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SendMouseMoveEvent(const CefMouseEvent& event,
                                              bool mouseLeave) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_mouse_move_event)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_mouse_move_event(_struct, &event, mouseLeave);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SendMouseWheelEvent(const CefMouseEvent& event,
                                               int deltaX,
                                               int deltaY) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_mouse_wheel_event)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_mouse_wheel_event(_struct, &event, deltaX, deltaY);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SendTouchEvent(const CefTouchEvent& event) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_touch_event)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_touch_event(_struct, &event);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::SendCaptureLostEvent() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, send_capture_lost_event)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->send_capture_lost_event(_struct);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::NotifyMoveOrResizeStarted() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, notify_move_or_resize_started)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->notify_move_or_resize_started(_struct);
}

NO_SANITIZE("cfi-icall") int CefBrowserHostCToCpp::GetWindowlessFrameRate() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_windowless_frame_rate)) {
    return 0;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->get_windowless_frame_rate(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SetWindowlessFrameRate(int frame_rate) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_windowless_frame_rate)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_windowless_frame_rate(_struct, frame_rate);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ImeSetComposition(
    const CefString& text,
    const std::vector<CefCompositionUnderline>& underlines,
    const CefRange& replacement_range,
    const CefRange& selection_range) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, ime_set_composition)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: text, underlines

  // Translate param: underlines; type: simple_vec_byref_const
  const size_t underlinesCount = underlines.size();
  cef_composition_underline_t* underlinesList = NULL;
  if (underlinesCount > 0) {
    underlinesList = new cef_composition_underline_t[underlinesCount];
    DCHECK(underlinesList);
    if (underlinesList) {
      for (size_t i = 0; i < underlinesCount; ++i) {
        underlinesList[i] = underlines[i];
      }
    }
  }

  // Execute
  _struct->ime_set_composition(_struct, text.GetStruct(), underlinesCount,
                               underlinesList, &replacement_range,
                               &selection_range);

  // Restore param:underlines; type: simple_vec_byref_const
  if (underlinesList) {
    delete[] underlinesList;
  }
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ImeCommitText(const CefString& text,
                                         const CefRange& replacement_range,
                                         int relative_cursor_pos) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, ime_commit_text)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: text

  // Execute
  _struct->ime_commit_text(_struct, text.GetStruct(), &replacement_range,
                           relative_cursor_pos);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ImeFinishComposingText(bool keep_selection) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, ime_finish_composing_text)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->ime_finish_composing_text(_struct, keep_selection);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::ImeCancelComposition() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, ime_cancel_composition)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->ime_cancel_composition(_struct);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::DragTargetDragEnter(CefRefPtr<CefDragData> drag_data,
                                               const CefMouseEvent& event,
                                               DragOperationsMask allowed_ops) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, drag_target_drag_enter)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: drag_data; type: refptr_same
  DCHECK(drag_data.get());
  if (!drag_data.get()) {
    return;
  }

  // Execute
  _struct->drag_target_drag_enter(_struct, CefDragDataCToCpp::Unwrap(drag_data),
                                  &event, allowed_ops);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::DragTargetDragOver(const CefMouseEvent& event,
                                              DragOperationsMask allowed_ops) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, drag_target_drag_over)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->drag_target_drag_over(_struct, &event, allowed_ops);
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::DragTargetDragLeave() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, drag_target_drag_leave)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->drag_target_drag_leave(_struct);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::DragTargetDrop(const CefMouseEvent& event) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, drag_target_drop)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->drag_target_drop(_struct, &event);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::DragSourceEndedAt(int x,
                                             int y,
                                             DragOperationsMask op) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, drag_source_ended_at)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->drag_source_ended_at(_struct, x, y, op);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::DragSourceSystemDragEnded() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, drag_source_system_drag_ended)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->drag_source_system_drag_ended(_struct);
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefNavigationEntry>
CefBrowserHostCToCpp::GetVisibleNavigationEntry() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_visible_navigation_entry)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_navigation_entry_t* _retval =
      _struct->get_visible_navigation_entry(_struct);

  // Return type: refptr_same
  return CefNavigationEntryCToCpp::Wrap(_retval);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SetAccessibilityState(
    cef_state_t accessibility_state) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_accessibility_state)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_accessibility_state(_struct, accessibility_state);
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::SetAutoResizeEnabled(bool enabled,
                                                const CefSize& min_size,
                                                const CefSize& max_size) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_auto_resize_enabled)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_auto_resize_enabled(_struct, enabled, &min_size, &max_size);
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefExtension> CefBrowserHostCToCpp::GetExtension() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_extension)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_extension_t* _retval = _struct->get_extension(_struct);

  // Return type: refptr_same
  return CefExtensionCToCpp::Wrap(_retval);
}

NO_SANITIZE("cfi-icall") bool CefBrowserHostCToCpp::IsBackgroundHost() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_background_host)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_background_host(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall") void CefBrowserHostCToCpp::SetAudioMuted(bool mute) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_audio_muted)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_audio_muted(_struct, mute);
}

NO_SANITIZE("cfi-icall") bool CefBrowserHostCToCpp::IsAudioMuted() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_audio_muted)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_audio_muted(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall") bool CefBrowserHostCToCpp::IsFullscreen() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_fullscreen)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_fullscreen(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ExitFullscreen(bool will_cause_resize) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, exit_fullscreen)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->exit_fullscreen(_struct, will_cause_resize);
}

NO_SANITIZE("cfi-icall")
bool CefBrowserHostCToCpp::CanExecuteChromeCommand(int command_id) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, can_execute_chrome_command)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->can_execute_chrome_command(_struct, command_id);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
void CefBrowserHostCToCpp::ExecuteChromeCommand(
    int command_id,
    cef_window_open_disposition_t disposition) {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, execute_chrome_command)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->execute_chrome_command(_struct, command_id, disposition);
}

NO_SANITIZE("cfi-icall")
bool CefBrowserHostCToCpp::IsRenderProcessUnresponsive() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_render_process_unresponsive)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_render_process_unresponsive(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
cef_runtime_style_t CefBrowserHostCToCpp::GetRuntimeStyle() {
  shutdown_checker::AssertNotShutdown();

  cef_browser_host_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_runtime_style)) {
    return CEF_RUNTIME_STYLE_DEFAULT;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_runtime_style_t _retval = _struct->get_runtime_style(_struct);

  // Return type: simple
  return _retval;
}

// CONSTRUCTOR - Do not edit by hand.

CefBrowserHostCToCpp::CefBrowserHostCToCpp() {}

// DESTRUCTOR - Do not edit by hand.

CefBrowserHostCToCpp::~CefBrowserHostCToCpp() {
  shutdown_checker::AssertNotShutdown();
}

template <>
cef_browser_host_t*
CefCToCppRefCounted<CefBrowserHostCToCpp, CefBrowserHost, cef_browser_host_t>::
    UnwrapDerived(CefWrapperType type, CefBrowserHost* c) {
  DCHECK(false) << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType CefCToCppRefCounted<CefBrowserHostCToCpp,
                                   CefBrowserHost,
                                   cef_browser_host_t>::kWrapperType =
    WT_BROWSER_HOST;
