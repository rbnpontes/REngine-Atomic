//
// Copyright (c) 2014-2015, THUNDERBEAST GAMES LLC All rights reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <TurboBadger/tb_window.h>

#include "JSUIAPI.h"
#include "JSVM.h"

#include <EngineCore/UI/UI.h>
#include <EngineCore/UI/UISelectItem.h>
#include <EngineCore/UI/UIMenuWindow.h>
#include <EngineCore/UI/UIButton.h>
#include <EngineCore/UI/UIWindow.h>

namespace Atomic
{

static int UIButton_Popup(duk_context* ctx)
{

    if (!duk_is_object(ctx, 0))
    {
        duk_push_string(ctx, "UIButton.popup first argument must be an object");
        duk_throw(ctx);
    }
    if (!duk_is_callable(ctx, 1))
    {
        duk_push_string(ctx, "UIButton.popup second argument must be callable");
        duk_throw(ctx);
    }

    JSVM* vm = JSVM::GetJSVM(ctx);

    duk_enum(ctx, 0, DUK_ENUM_OWN_PROPERTIES_ONLY);

    UISelectItemSource* source = new UISelectItemSource(vm->GetContext());

    while (duk_next(ctx, -1, 0)) {

        String key = duk_get_string(ctx, -1);

        duk_get_prop(ctx, 0);

        if (duk_is_array(ctx, -1))
        {
            // need to support this, for skin image, etc
            assert(0);
        }
        else if (duk_is_string(ctx, -1))
        {
            // id
            String id = duk_get_string(ctx, -1);
            source->AddItem(new UISelectItem(vm->GetContext(), key, id));
        }
        else
        {
            duk_push_string(ctx, "UIButton.popup data object key is not an array or string");
            duk_throw(ctx);
        }

        duk_pop(ctx);  // pop key value
    }

    duk_pop(ctx);  // pop enum object

    duk_push_this(ctx);

    duk_dup(ctx, 1);
    duk_put_prop_string(ctx, -2, "__popup_menu_callback");

    UIButton* button = js_to_class_instance<UIButton>(ctx, -1, 0);
    UIMenuWindow* menuWindow = new UIMenuWindow(vm->GetContext(), button, "__popup-menu");
    menuWindow->Show(source);
    duk_pop(ctx);

    return 0;
}

int UIWindow_GetResizeToFitContentRect(duk_context* ctx)
{
    duk_push_this(ctx);
    UIWindow* window = js_to_class_instance<UIWindow>(ctx, -1, 0);
    duk_pop(ctx);

    tb::TBWindow* tbwindow = (tb::TBWindow*) window->GetInternalWidget();

    tb::TBRect rect = tbwindow->GetResizeToFitContentRect();

    duk_push_object(ctx);
    duk_push_number(ctx, rect.x);
    duk_put_prop_string(ctx, -2, "x");
    duk_push_number(ctx, rect.y);
    duk_put_prop_string(ctx, -2, "y");
    duk_push_number(ctx, rect.w);
    duk_put_prop_string(ctx, -2, "width");
    duk_push_number(ctx, rect.h);
    duk_put_prop_string(ctx, -2, "height");
    return 1;

}

int UI_DebugGetWrappedWidgetCount(duk_context* ctx)
{
    JSVM* vm = JSVM::GetJSVM(ctx);
    UI* ui = vm->GetSubsystem<UI>();

    duk_push_number(ctx, (double) ui->DebugGetWrappedWidgetCount());
    return 1;
}

static int UIWidget_SearchWidgetClass(duk_context* ctx)
{
    const char* clssName = duk_require_string(ctx, 0);
    duk_push_this(ctx);

    UIWidget* uiwidget = js_to_class_instance<UIWidget>(ctx, -1, 0);

    PODVector<UIWidget*> dest;
    uiwidget->SearchWidgetClass( clssName, dest );

    duk_push_array(ctx);

    for (unsigned ii = 0; ii < dest.Size(); ii++)
    {
        js_push_class_object_instance(ctx, dest[ii], "UIWidget");
        duk_put_prop_index(ctx, -2, ii);
    }

    return 1;
}

static int UIWidget_SearchWidgetId(duk_context* ctx)
{
    const char* idName = duk_require_string(ctx, 0);
    duk_push_this(ctx);
 
    UIWidget* uiwidget = js_to_class_instance<UIWidget>(ctx, -1, 0);

    PODVector<UIWidget*> dest;
    uiwidget->SearchWidgetId( idName, dest );

    duk_push_array(ctx);

    for (unsigned i = 0; i < dest.Size(); i++)
    {
        js_push_class_object_instance(ctx, dest[i], "UIWidget");
        duk_put_prop_index(ctx, -2, i);
    }

    return 1;
}

static int UIWidget_SearchWidgetText(duk_context* ctx)
{
    const char* textName = duk_require_string(ctx, 0);
    duk_push_this(ctx);

    UIWidget* uiwidget = js_to_class_instance<UIWidget>(ctx, -1, 0);

    PODVector<UIWidget*> dest;
    uiwidget->SearchWidgetText( textName, dest );

    duk_push_array(ctx);

    for (unsigned i = 0; i < dest.Size(); i++)
    {
        js_push_class_object_instance(ctx, dest[i], "UIWidget");
        duk_put_prop_index(ctx, -2, i);
    }

    return 1;
}


void jsapi_init_ui(JSVM* vm)
{
    duk_context* ctx = vm->GetJSContext();

    // UI class object
    duk_get_global_string(ctx, ENGINE_CORE);
    duk_get_prop_string(ctx, -1, "UI");

    duk_push_c_function(ctx, UI_DebugGetWrappedWidgetCount, 0);
    duk_put_prop_string(ctx, -2, "debugGetWrappedWidgetCount");

    duk_pop_2(ctx);

    js_class_get_prototype(ctx, ENGINE_CORE, "UIButton");
    duk_push_c_function(ctx, UIButton_Popup, 2);
    duk_put_prop_string(ctx, -2, "popup");
    duk_pop(ctx);

    js_class_get_prototype(ctx, ENGINE_CORE, "UIWindow");
    duk_push_c_function(ctx, UIWindow_GetResizeToFitContentRect, 0);
    duk_put_prop_string(ctx, -2, "getResizeToFitContentRect");
    duk_pop(ctx);

    js_class_get_prototype(ctx, ENGINE_CORE, "UIWidget");
    duk_push_c_function(ctx, UIWidget_SearchWidgetClass, 1);
    duk_put_prop_string(ctx, -2, "searchWidgetClass");
    duk_push_c_function(ctx, UIWidget_SearchWidgetId, 1);
    duk_put_prop_string(ctx, -2, "searchWidgetId");
    duk_push_c_function(ctx, UIWidget_SearchWidgetText, 1);
    duk_put_prop_string(ctx, -2, "searchWidgetText");
    duk_pop(ctx);

}

}
