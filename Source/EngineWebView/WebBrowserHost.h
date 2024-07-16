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

#include <EngineCore/Core/Object.h>

namespace Atomic
{

class WebBrowserHostPrivate;
class WebClient;

typedef HashMap<String, HashMap<String, Variant>> GlobalPropertyMap;

/// Browser host subsystem, responsible for initializing CEF
class WebBrowserHost : public Object
{
    ATOMIC_OBJECT(WebBrowserHost, Object);

public:
    /// Construct.
    WebBrowserHost(Context* context);
    /// Destruct.
    virtual ~WebBrowserHost();

    void ClearCookies(const String& url = String::EMPTY, const String& cookieName = String::EMPTY);

    /// Set global property object values, available as read only on page
    static void SetGlobalBoolProperty(const String& globalVar, const String& property, bool value);
    static void SetGlobalStringProperty(const String& globalVar, const String& property, const String& value);
    static void SetGlobalNumberProperty(const String& globalVar, const String& property, double value);

    static const GlobalPropertyMap& GetGlobalProperties() { return globalProperties_; }

    // Configuration settings that must be set before creating WebBrowserHost subsystem

    /// Set the absolute root path for the cache, this is combined with the cache name to generate the full cache path
    static void SetRootCacheFolder(const String& rootCacheFolder) { rootCacheFolder_ = rootCacheFolder; }

    /// Set the cache name which is combined with the root cache folder to generate an absolute path to the browser cache
    static void SetCacheName(const String& cacheName) { cacheName_ = cacheName; }

    /// Set value that will be returned as the User-Agent HTTP header.
    static void SetUserAgent(const String& userAgent) { userAgent_ = userAgent; }

    /// Set to a value between 1024 and 65535 to enable remote debugging on the specified port
    static void SetDebugPort(int debugPort) { debugPort_ = debugPort; }

    /// Set the name of the function used for JavaScript message queries
    static void SetJSMessageQueryFunctionName(const String& name) { jsMessageQueryFunctionName_ = name; }

    /// Set the name of the function used to cancel JavaScript message queries
    static void SetJSMessageQueryCancelFunctionName(const String& name) { jsMessageQueryCancelFunctionName_ = name; }

    /// Get the absolute root path for the cache, this is combined with the cache name to generate the full cache path
    static const String& GetRootCacheFolder() { return rootCacheFolder_; }

    /// Get the cache name which is combined with the root cache folder to generate an absolute path to the browser cache
    static const String& GetCacheName() { return cacheName_; }

    /// Get User-Agent of the HTTP header. If empty the default User-Agent string will be used
    static const String& GetUserAgent() { return userAgent_; }

    /// Get value that will be inserted as the product portion of the default User-Agent string.  If empty the Chromium product version will be used      
    static const String& GetProductVersion() { return "REngineBrowser"; }

    /// Get the name of the function used for JavaScript message queries
    static const String& GetJSMessageQueryFunctionName() { return jsMessageQueryFunctionName_; }

    /// Get the name of the function used to cancel JavaScript message queries
    static const String& GetJSMessageQueryCancelFunctionName() { return jsMessageQueryCancelFunctionName_; }


private:

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    static void SetGlobalProperty(const String& globalVar, const String& property, Variant& value);

    WebBrowserHostPrivate* d_;

    static WeakPtr<WebBrowserHost> instance_;

    static GlobalPropertyMap globalProperties_;

    // configuration settings that must be set before WebBrowserHost subsystem is created
    static String userAgent_;

    static String rootCacheFolder_;
    static String cacheName_;

    static String jsMessageQueryFunctionName_;
    static String jsMessageQueryCancelFunctionName_;

    static int debugPort_;


};

}
