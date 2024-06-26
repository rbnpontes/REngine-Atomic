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
// $hash=d2c4416fa5e7ee4f64ffe552a9289c26ebe982c2$
//

#include "libcef_dll/ctocpp/navigation_entry_ctocpp.h"

#include "libcef_dll/ctocpp/sslstatus_ctocpp.h"
#include "libcef_dll/shutdown_checker.h"

// VIRTUAL METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall") bool CefNavigationEntryCToCpp::IsValid() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_valid)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_valid(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall") CefString CefNavigationEntryCToCpp::GetURL() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_url)) {
    return CefString();
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall") CefString CefNavigationEntryCToCpp::GetDisplayURL() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_display_url)) {
    return CefString();
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_display_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall") CefString CefNavigationEntryCToCpp::GetOriginalURL() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_original_url)) {
    return CefString();
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_original_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall") CefString CefNavigationEntryCToCpp::GetTitle() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_title)) {
    return CefString();
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_title(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall")
CefNavigationEntry::TransitionType
CefNavigationEntryCToCpp::GetTransitionType() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_transition_type)) {
    return TT_EXPLICIT;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_transition_type_t _retval = _struct->get_transition_type(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall") bool CefNavigationEntryCToCpp::HasPostData() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, has_post_data)) {
    return false;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->has_post_data(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall")
CefBaseTime CefNavigationEntryCToCpp::GetCompletionTime() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_completion_time)) {
    return CefBaseTime();
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_basetime_t _retval = _struct->get_completion_time(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall") int CefNavigationEntryCToCpp::GetHttpStatusCode() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_http_status_code)) {
    return 0;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->get_http_status_code(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall")
CefRefPtr<CefSSLStatus> CefNavigationEntryCToCpp::GetSSLStatus() {
  shutdown_checker::AssertNotShutdown();

  cef_navigation_entry_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_sslstatus)) {
    return nullptr;
  }

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_sslstatus_t* _retval = _struct->get_sslstatus(_struct);

  // Return type: refptr_same
  return CefSSLStatusCToCpp::Wrap(_retval);
}

// CONSTRUCTOR - Do not edit by hand.

CefNavigationEntryCToCpp::CefNavigationEntryCToCpp() {}

// DESTRUCTOR - Do not edit by hand.

CefNavigationEntryCToCpp::~CefNavigationEntryCToCpp() {
  shutdown_checker::AssertNotShutdown();
}

template <>
cef_navigation_entry_t* CefCToCppRefCounted<
    CefNavigationEntryCToCpp,
    CefNavigationEntry,
    cef_navigation_entry_t>::UnwrapDerived(CefWrapperType type,
                                           CefNavigationEntry* c) {
  DCHECK(false) << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType CefCToCppRefCounted<CefNavigationEntryCToCpp,
                                   CefNavigationEntry,
                                   cef_navigation_entry_t>::kWrapperType =
    WT_NAVIGATION_ENTRY;
