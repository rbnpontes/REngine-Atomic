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
// $hash=0c9aa4c628bb305301c74720a07145c4d2ef7cde$
//

#ifndef CEF_LIBCEF_DLL_CTOCPP_BROWSER_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_BROWSER_CTOCPP_H_
#pragma once

#if !defined(WRAPPING_CEF_SHARED)
#error This file can be included wrapper-side only
#endif

#include <vector>

#include "include/capi/cef_browser_capi.h"
#include "include/capi/cef_client_capi.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "libcef_dll/ctocpp/ctocpp_ref_counted.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefBrowserCToCpp
    : public CefCToCppRefCounted<CefBrowserCToCpp, CefBrowser, cef_browser_t> {
 public:
  CefBrowserCToCpp();
  virtual ~CefBrowserCToCpp();

  // CefBrowser methods.
  bool IsValid() override;
  CefRefPtr<CefBrowserHost> GetHost() override;
  bool CanGoBack() override;
  void GoBack() override;
  bool CanGoForward() override;
  void GoForward() override;
  bool IsLoading() override;
  void Reload() override;
  void ReloadIgnoreCache() override;
  void StopLoad() override;
  int GetIdentifier() override;
  bool IsSame(CefRefPtr<CefBrowser> that) override;
  bool IsPopup() override;
  bool HasDocument() override;
  CefRefPtr<CefFrame> GetMainFrame() override;
  CefRefPtr<CefFrame> GetFocusedFrame() override;
  CefRefPtr<CefFrame> GetFrameByIdentifier(
      const CefString& identifier) override;
  CefRefPtr<CefFrame> GetFrameByName(const CefString& name) override;
  size_t GetFrameCount() override;
  void GetFrameIdentifiers(std::vector<CefString>& identifiers) override;
  void GetFrameNames(std::vector<CefString>& names) override;
};

#endif  // CEF_LIBCEF_DLL_CTOCPP_BROWSER_CTOCPP_H_
