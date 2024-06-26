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
// $hash=da9219ed18261db0ce53060f4189a61585b1d4e8$
//

#ifndef CEF_LIBCEF_DLL_CTOCPP_V8STACK_FRAME_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_V8STACK_FRAME_CTOCPP_H_
#pragma once

#if !defined(WRAPPING_CEF_SHARED)
#error This file can be included wrapper-side only
#endif

#include "include/capi/cef_v8_capi.h"
#include "include/cef_v8.h"
#include "libcef_dll/ctocpp/ctocpp_ref_counted.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefV8StackFrameCToCpp : public CefCToCppRefCounted<CefV8StackFrameCToCpp,
                                                         CefV8StackFrame,
                                                         cef_v8stack_frame_t> {
 public:
  CefV8StackFrameCToCpp();
  virtual ~CefV8StackFrameCToCpp();

  // CefV8StackFrame methods.
  bool IsValid() override;
  CefString GetScriptName() override;
  CefString GetScriptNameOrSourceURL() override;
  CefString GetFunctionName() override;
  int GetLineNumber() override;
  int GetColumn() override;
  bool IsEval() override;
  bool IsConstructor() override;
};

#endif  // CEF_LIBCEF_DLL_CTOCPP_V8STACK_FRAME_CTOCPP_H_
