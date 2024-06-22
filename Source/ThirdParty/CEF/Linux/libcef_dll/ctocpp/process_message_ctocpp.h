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
// $hash=dc73e17f4ab4c0f26126b43d6c2015ab45739376$
//

#ifndef CEF_LIBCEF_DLL_CTOCPP_PROCESS_MESSAGE_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_PROCESS_MESSAGE_CTOCPP_H_
#pragma once

#if !defined(WRAPPING_CEF_SHARED)
#error This file can be included wrapper-side only
#endif

#include "include/capi/cef_process_message_capi.h"
#include "include/cef_process_message.h"
#include "libcef_dll/ctocpp/ctocpp_ref_counted.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefProcessMessageCToCpp
    : public CefCToCppRefCounted<CefProcessMessageCToCpp,
                                 CefProcessMessage,
                                 cef_process_message_t> {
 public:
  CefProcessMessageCToCpp();
  virtual ~CefProcessMessageCToCpp();

  // CefProcessMessage methods.
  bool IsValid() override;
  bool IsReadOnly() override;
  CefRefPtr<CefProcessMessage> Copy() override;
  CefString GetName() override;
  CefRefPtr<CefListValue> GetArgumentList() override;
  CefRefPtr<CefSharedMemoryRegion> GetSharedMemoryRegion() override;
};

#endif  // CEF_LIBCEF_DLL_CTOCPP_PROCESS_MESSAGE_CTOCPP_H_
