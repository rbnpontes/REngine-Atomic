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

#pragma once

#include "UIWidget.h"

namespace Atomic
{

class UILayout;

class ATOMIC_API UITabContainer : public UIWidget
{
    ATOMIC_OBJECT(UITabContainer, UIWidget)

public:

    UITabContainer(Context* context, bool createWidget = true);
    virtual ~UITabContainer();

    int GetNumPages();
    void SetCurrentPage(int page);

    UIWidget* GetCurrentPageWidget();

    UILayout* GetTabLayout();
    
    int GetCurrentPage(); /// returns the current page number
    bool DeletePage( int page ); /// deletes a tab + page, returns true if successful
    void AddTabPageFile ( const String &tabname, const String &layoutFile, bool selecttab = true ); /// adds a tab + page from file
    void AddTabPageWidget ( const String &tabname, UIWidget *widget, bool selecttab = true ); /// adds a tab + page widget(s)

    void UndockPage ( int page ); /// undocks the page into a window with the tab name, and removes the tab
    bool DockWindow ( String windowTitle ); /// docks content from a UIDockWindow with specified title

protected:

    virtual bool OnEvent(const tb::TBWidgetEvent &ev);

private:

};

}
