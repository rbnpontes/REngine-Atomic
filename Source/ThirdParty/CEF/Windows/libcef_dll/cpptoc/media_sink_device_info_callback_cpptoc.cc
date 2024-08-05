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
// $hash=48d5bd712540c85c83360d4342acedd2412fcf30$
//

#include "libcef_dll/cpptoc/media_sink_device_info_callback_cpptoc.h"

#include "libcef_dll/shutdown_checker.h"
#include "libcef_dll/template_util.h"

namespace {

// MEMBER FUNCTIONS - Body may be edited by hand.

void CEF_CALLBACK media_sink_device_info_callback_on_media_sink_device_info(
    struct _cef_media_sink_device_info_callback_t* self,
    const struct _cef_media_sink_device_info_t* device_info) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: device_info; type: struct_byref_const
  DCHECK(device_info);
  if (!device_info) {
    return;
  }
  if (!template_util::has_valid_size(device_info)) {
    DCHECK(false) << "invalid device_info->[base.]size";
    return;
  }

  // Translate param: device_info; type: struct_byref_const
  CefMediaSinkDeviceInfo device_infoObj;
  if (device_info) {
    device_infoObj.Set(*device_info, false);
  }

  // Execute
  CefMediaSinkDeviceInfoCallbackCppToC::Get(self)->OnMediaSinkDeviceInfo(
      device_infoObj);
}

}  // namespace

// CONSTRUCTOR - Do not edit by hand.

CefMediaSinkDeviceInfoCallbackCppToC::CefMediaSinkDeviceInfoCallbackCppToC() {
  GetStruct()->on_media_sink_device_info =
      media_sink_device_info_callback_on_media_sink_device_info;
}

// DESTRUCTOR - Do not edit by hand.

CefMediaSinkDeviceInfoCallbackCppToC::~CefMediaSinkDeviceInfoCallbackCppToC() {
  shutdown_checker::AssertNotShutdown();
}

template <>
CefRefPtr<CefMediaSinkDeviceInfoCallback>
CefCppToCRefCounted<CefMediaSinkDeviceInfoCallbackCppToC,
                    CefMediaSinkDeviceInfoCallback,
                    cef_media_sink_device_info_callback_t>::
    UnwrapDerived(CefWrapperType type,
                  cef_media_sink_device_info_callback_t* s) {
  DCHECK(false) << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType
    CefCppToCRefCounted<CefMediaSinkDeviceInfoCallbackCppToC,
                        CefMediaSinkDeviceInfoCallback,
                        cef_media_sink_device_info_callback_t>::kWrapperType =
        WT_MEDIA_SINK_DEVICE_INFO_CALLBACK;
