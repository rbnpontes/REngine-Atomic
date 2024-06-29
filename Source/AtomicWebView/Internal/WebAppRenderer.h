//
// Copyright (c) 2014-2016, THUNDERBEAST GAMES LLC All rights reserved
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

#include <Atomic/Container/List.h>
#include "WebApp.h"

namespace Atomic
{

// Web app implementation for the renderer process.
class WebAppRenderer :
	public WebApp,
	public CefRenderProcessHandler,
	public CefBrowserProcessHandler
{

public:
    // Interface for renderer delegates. All Delegates must be returned via
    // CreateDelegates. Do not perform work in the Delegate
    // constructor. See CefRenderProcessHandler for documentation.
    class Delegate : public virtual CefBaseRefCounted
    {
    public:
        virtual void OnWebKitInitialized(CefRefPtr<WebAppRenderer> app) {}

        virtual void OnBrowserCreated(CefRefPtr<WebAppRenderer> app,
                                      CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefDictionaryValue> extra_info) {}

        virtual void OnBrowserDestroyed(CefRefPtr<WebAppRenderer> app,
                                        CefRefPtr<CefBrowser> browser) {}

        virtual CefRefPtr<CefLoadHandler> GetLoadHandler(CefRefPtr<WebAppRenderer> app)
        {
            return nullptr;
        }

        virtual void OnContextCreated(CefRefPtr<WebAppRenderer> app,
                                      CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context) {}

        virtual void OnContextReleased(CefRefPtr<WebAppRenderer> app,
                                       CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefV8Context> context) {}

        virtual void OnUncaughtException(CefRefPtr<WebAppRenderer> app,
                                         CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefV8Context> context,
                                         CefRefPtr<CefV8Exception> exception,
                                         CefRefPtr<CefV8StackTrace> stackTrace) {}

        virtual void OnFocusedNodeChanged(CefRefPtr<WebAppRenderer> app,
                                          CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefDOMNode> node) {}

        // Called when a process message is received. Return true if the message was
        // handled and should not be passed on to other handlers. Delegates
        // should check for unique message names to avoid interfering with each
        // other.
        virtual bool OnProcessMessageReceived(
                CefRefPtr<WebAppRenderer> app,
                CefRefPtr<CefBrowser> browser,
                CefProcessId source_process,
                CefRefPtr<CefProcessMessage> message)
        {
            return false;
        }
    };

    typedef List<CefRefPtr<Delegate>> DelegateSet;

    WebAppRenderer();

private:
    // Creates all of the Delegate objects. Implemented by cefclient in
    // client_app_delegates_renderer.cc
    static void CreateDelegates(DelegateSet& delegates);

    // CefApp methods.
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }

    // CefRenderProcessHandler methods.

    void OnBrowserCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDictionaryValue> extra_info) override;

    void OnWebKitInitialized() override;

    void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;

    CefRefPtr<CefLoadHandler> GetLoadHandler() override;

    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;

    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override;

    void OnUncaughtException(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefV8Context> context,
                             CefRefPtr<CefV8Exception> exception,
                             CefRefPtr<CefV8StackTrace> stackTrace) override;

    void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefDOMNode> node) override;

    bool OnProcessMessageReceived(
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message) override;

private:
    // Set of supported Delegates.
    DelegateSet delegates_;

    IMPLEMENT_REFCOUNTING(WebAppRenderer);
    DISALLOW_COPY_AND_ASSIGN(WebAppRenderer);
};
}
