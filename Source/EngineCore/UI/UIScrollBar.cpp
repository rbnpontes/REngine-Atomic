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

#include <TurboBadger/tb_widgets.h>
#include <TurboBadger/tb_widgets_common.h>

#include <Atomic/IO/Log.h>

#include "UIEvents.h"
#include "UI.h"
#include "UILayout.h"
#include "UIScrollBar.h"

using namespace tb;

namespace Atomic
{

UIScrollBar::UIScrollBar(Context* context, bool createWidget) : UIWidget(context, false)
{
    if (createWidget)
    {
        widget_ = new TBScrollBar();
        widget_->SetDelegate(this);
        GetSubsystem<UI>()->WrapWidget(this, widget_);
    }
}

UIScrollBar::~UIScrollBar()
{

}

void UIScrollBar::SetLimits(double minimum, double maximum, double visible)
{
    if (!widget_)
        return;
    ((TBScrollBar*) widget_)->SetLimits(minimum, maximum, visible);

}

double UIScrollBar::GetMinValue() const
{
    if (!widget_)
        return 0.0;

   return ((TBScrollBar*) widget_)->GetMinValue();

}

double UIScrollBar::GetMaxValue() const
{
    if (!widget_)
        return 0.0;

   return ((TBScrollBar*) widget_)->GetMaxValue();

}

double UIScrollBar::GetVisible() const
{
    if (!widget_)
        return 0.0;

   return ((TBScrollBar*) widget_)->GetVisible();

}

bool UIScrollBar::OnEvent(const tb::TBWidgetEvent &ev)
{
    if (ev.type == EVENT_TYPE_CUSTOM && ev.ref_id == TBIDC("edit_complete"))
    {
        VariantMap eventData;
        eventData[UIWidgetEditComplete::P_WIDGET] = this;
        SendEvent(E_UIWIDGETEDITCOMPLETE, eventData);

        return true;
    }
    return UIWidget::OnEvent(ev);
}

}
