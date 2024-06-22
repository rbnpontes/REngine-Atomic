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
// $hash=2a9862aaf02e64b32d50e31fd0e61e17007f53de$
//

#include "libcef_dll/ctocpp/permission_prompt_callback_ctocpp.h"

#include "libcef_dll/shutdown_checker.h"

// VIRTUAL METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall")
void CefPermissionPromptCallbackCToCpp::Continue(
    cef_permission_request_result_t result) {
  shutdown_checker::AssertNotShutdown();

  cef_permission_prompt_callback_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, cont)) {
    return;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->cont(_struct, result);
}

// CONSTRUCTOR - Do not edit by hand.

CefPermissionPromptCallbackCToCpp::CefPermissionPromptCallbackCToCpp() {}

// DESTRUCTOR - Do not edit by hand.

CefPermissionPromptCallbackCToCpp::~CefPermissionPromptCallbackCToCpp() {
  shutdown_checker::AssertNotShutdown();
}

template <>
cef_permission_prompt_callback_t*
CefCToCppRefCounted<CefPermissionPromptCallbackCToCpp,
                    CefPermissionPromptCallback,
                    cef_permission_prompt_callback_t>::
    UnwrapDerived(CefWrapperType type, CefPermissionPromptCallback* c) {
  DCHECK(false) << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType
    CefCToCppRefCounted<CefPermissionPromptCallbackCToCpp,
                        CefPermissionPromptCallback,
                        cef_permission_prompt_callback_t>::kWrapperType =
        WT_PERMISSION_PROMPT_CALLBACK;
