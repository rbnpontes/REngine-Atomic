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
// $hash=fa0d14594aa86c629b8f7a8e25f2472aaac9cb76$
//

#ifndef CEF_LIBCEF_DLL_CPPTOC_JSDIALOG_HANDLER_CPPTOC_H_
#define CEF_LIBCEF_DLL_CPPTOC_JSDIALOG_HANDLER_CPPTOC_H_
#pragma once

#if !defined(WRAPPING_CEF_SHARED)
#error This file can be included wrapper-side only
#endif

#include "include/capi/cef_jsdialog_handler_capi.h"
#include "include/cef_jsdialog_handler.h"
#include "libcef_dll/cpptoc/cpptoc_ref_counted.h"

// Wrap a C++ class with a C structure.
// This class may be instantiated and accessed wrapper-side only.
class CefJSDialogHandlerCppToC
    : public CefCppToCRefCounted<CefJSDialogHandlerCppToC,
                                 CefJSDialogHandler,
                                 cef_jsdialog_handler_t> {
 public:
  CefJSDialogHandlerCppToC();
  virtual ~CefJSDialogHandlerCppToC();
};

#endif  // CEF_LIBCEF_DLL_CPPTOC_JSDIALOG_HANDLER_CPPTOC_H_
