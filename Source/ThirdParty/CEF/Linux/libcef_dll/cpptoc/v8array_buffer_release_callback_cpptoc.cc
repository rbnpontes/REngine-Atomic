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
// $hash=21a1fe189ed3ee668a6896112f00423d2c997f34$
//

#include "libcef_dll/cpptoc/v8array_buffer_release_callback_cpptoc.h"

namespace {

// MEMBER FUNCTIONS - Body may be edited by hand.

void CEF_CALLBACK v8array_buffer_release_callback_release_buffer(
    struct _cef_v8array_buffer_release_callback_t* self,
    void* buffer) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: buffer; type: simple_byaddr
  DCHECK(buffer);
  if (!buffer) {
    return;
  }

  // Execute
  CefV8ArrayBufferReleaseCallbackCppToC::Get(self)->ReleaseBuffer(buffer);
}

}  // namespace

// CONSTRUCTOR - Do not edit by hand.

CefV8ArrayBufferReleaseCallbackCppToC::CefV8ArrayBufferReleaseCallbackCppToC() {
  GetStruct()->release_buffer = v8array_buffer_release_callback_release_buffer;
}

// DESTRUCTOR - Do not edit by hand.

CefV8ArrayBufferReleaseCallbackCppToC::
    ~CefV8ArrayBufferReleaseCallbackCppToC() {}

template <>
CefRefPtr<CefV8ArrayBufferReleaseCallback>
CefCppToCRefCounted<CefV8ArrayBufferReleaseCallbackCppToC,
                    CefV8ArrayBufferReleaseCallback,
                    cef_v8array_buffer_release_callback_t>::
    UnwrapDerived(CefWrapperType type,
                  cef_v8array_buffer_release_callback_t* s) {
  DCHECK(false) << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType
    CefCppToCRefCounted<CefV8ArrayBufferReleaseCallbackCppToC,
                        CefV8ArrayBufferReleaseCallback,
                        cef_v8array_buffer_release_callback_t>::kWrapperType =
        WT_V8ARRAY_BUFFER_RELEASE_CALLBACK;
