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
// $hash=a69369fe8fadea409955f130afb3d158e9e66c79$
//

#include "libcef_dll/cpptoc/menu_model_delegate_cpptoc.h"

#include "libcef_dll/ctocpp/menu_model_ctocpp.h"
#include "libcef_dll/shutdown_checker.h"

namespace {

// MEMBER FUNCTIONS - Body may be edited by hand.

void CEF_CALLBACK
menu_model_delegate_execute_command(struct _cef_menu_model_delegate_t* self,
                                    cef_menu_model_t* menu_model,
                                    int command_id,
                                    cef_event_flags_t event_flags) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return;
  }

  // Execute
  CefMenuModelDelegateCppToC::Get(self)->ExecuteCommand(
      CefMenuModelCToCpp::Wrap(menu_model), command_id, event_flags);
}

void CEF_CALLBACK
menu_model_delegate_mouse_outside_menu(struct _cef_menu_model_delegate_t* self,
                                       cef_menu_model_t* menu_model,
                                       const cef_point_t* screen_point) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return;
  }
  // Verify param: screen_point; type: simple_byref_const
  DCHECK(screen_point);
  if (!screen_point) {
    return;
  }

  // Translate param: screen_point; type: simple_byref_const
  CefPoint screen_pointVal = screen_point ? *screen_point : CefPoint();

  // Execute
  CefMenuModelDelegateCppToC::Get(self)->MouseOutsideMenu(
      CefMenuModelCToCpp::Wrap(menu_model), screen_pointVal);
}

void CEF_CALLBACK menu_model_delegate_unhandled_open_submenu(
    struct _cef_menu_model_delegate_t* self,
    cef_menu_model_t* menu_model,
    int is_rtl) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return;
  }

  // Execute
  CefMenuModelDelegateCppToC::Get(self)->UnhandledOpenSubmenu(
      CefMenuModelCToCpp::Wrap(menu_model), is_rtl ? true : false);
}

void CEF_CALLBACK menu_model_delegate_unhandled_close_submenu(
    struct _cef_menu_model_delegate_t* self,
    cef_menu_model_t* menu_model,
    int is_rtl) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return;
  }

  // Execute
  CefMenuModelDelegateCppToC::Get(self)->UnhandledCloseSubmenu(
      CefMenuModelCToCpp::Wrap(menu_model), is_rtl ? true : false);
}

void CEF_CALLBACK
menu_model_delegate_menu_will_show(struct _cef_menu_model_delegate_t* self,
                                   cef_menu_model_t* menu_model) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return;
  }

  // Execute
  CefMenuModelDelegateCppToC::Get(self)->MenuWillShow(
      CefMenuModelCToCpp::Wrap(menu_model));
}

void CEF_CALLBACK
menu_model_delegate_menu_closed(struct _cef_menu_model_delegate_t* self,
                                cef_menu_model_t* menu_model) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return;
  }

  // Execute
  CefMenuModelDelegateCppToC::Get(self)->MenuClosed(
      CefMenuModelCToCpp::Wrap(menu_model));
}

int CEF_CALLBACK
menu_model_delegate_format_label(struct _cef_menu_model_delegate_t* self,
                                 cef_menu_model_t* menu_model,
                                 cef_string_t* label) {
  shutdown_checker::AssertNotShutdown();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self) {
    return 0;
  }
  // Verify param: menu_model; type: refptr_diff
  DCHECK(menu_model);
  if (!menu_model) {
    return 0;
  }
  // Verify param: label; type: string_byref
  DCHECK(label);
  if (!label) {
    return 0;
  }

  // Translate param: label; type: string_byref
  CefString labelStr(label);

  // Execute
  bool _retval = CefMenuModelDelegateCppToC::Get(self)->FormatLabel(
      CefMenuModelCToCpp::Wrap(menu_model), labelStr);

  // Return type: bool
  return _retval;
}

}  // namespace

// CONSTRUCTOR - Do not edit by hand.

CefMenuModelDelegateCppToC::CefMenuModelDelegateCppToC() {
  GetStruct()->execute_command = menu_model_delegate_execute_command;
  GetStruct()->mouse_outside_menu = menu_model_delegate_mouse_outside_menu;
  GetStruct()->unhandled_open_submenu =
      menu_model_delegate_unhandled_open_submenu;
  GetStruct()->unhandled_close_submenu =
      menu_model_delegate_unhandled_close_submenu;
  GetStruct()->menu_will_show = menu_model_delegate_menu_will_show;
  GetStruct()->menu_closed = menu_model_delegate_menu_closed;
  GetStruct()->format_label = menu_model_delegate_format_label;
}

// DESTRUCTOR - Do not edit by hand.

CefMenuModelDelegateCppToC::~CefMenuModelDelegateCppToC() {
  shutdown_checker::AssertNotShutdown();
}

template <>
CefRefPtr<CefMenuModelDelegate> CefCppToCRefCounted<
    CefMenuModelDelegateCppToC,
    CefMenuModelDelegate,
    cef_menu_model_delegate_t>::UnwrapDerived(CefWrapperType type,
                                              cef_menu_model_delegate_t* s) {
  DCHECK(false) << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType CefCppToCRefCounted<CefMenuModelDelegateCppToC,
                                   CefMenuModelDelegate,
                                   cef_menu_model_delegate_t>::kWrapperType =
    WT_MENU_MODEL_DELEGATE;
