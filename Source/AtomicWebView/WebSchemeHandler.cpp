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

#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"

#include <Atomic/Math/MathDefs.h>
#include <Atomic/Resource/ResourceCache.h>
#include <Atomic/IO/FileSystem.h>

#include "WebString.h"
#include "WebBrowserHost.h"
#include "WebSchemeHandler.h"

#include "./Internal/WebInternalStorage.h"
#include "./Internal/WebUtils.h"
namespace Atomic
{


static WeakPtr<WebBrowserHost> webBrowserHost_;

// Implementation of the schema handler for atomic://resources/ requests.
class ResourceSchemeHandler : public CefResourceHandler
{

public:

    ResourceSchemeHandler() : fileLength_(0), offset_(0)
    {

    }

    virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                                CefRefPtr<CefCallback> callback) override
    {
        CEF_REQUIRE_IO_THREAD();

        if (webBrowserHost_.Null())
            return false;

        url_ = String::EMPTY;
        REngine::web_utils_convert_cef_str(request->GetURL(), url_);
        //shave off the atomic:// part
        url_ = url_.SubstringUTF8(9);

        if (IsAbsolutePath(url_))
        {
            file_ = new File(webBrowserHost_->GetContext(), url_, FILE_READ);
        }
        else
        {
            //shave off the resources part so we can grab the value from the resource cache
            url_ = url_.SubstringUTF8(10);
            ResourceCache* cache = webBrowserHost_->GetSubsystem<ResourceCache>();
            file_ = cache->GetFile(url_, false);
        }


        if (!file_->IsOpen())
            return false;

        fileLength_ = file_->GetSize();

        // Indicate the headers are available.
        callback->Continue();
        return true;
    }

    virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                    i64& response_length,
                                    CefString& redirectUrl) override
    {
        CEF_REQUIRE_IO_THREAD();

        String pathName, fileName, ext;
        SplitPath(url_, pathName, fileName, ext);

        if (ext == ".js")
            response->SetMimeType("text/javascript");
        else if (ext == ".ts")
            response->SetMimeType("text/typescript");
        else if (ext == ".html")
            response->SetMimeType("text/html");

        response->SetHeaderByName("Access-Control-Allow-Origin", "*", true);
        response->SetStatus(200);

        // Set the resulting response length
        response_length = fileLength_;
    }

    virtual void Cancel() override
    {
        CEF_REQUIRE_IO_THREAD();
    }

    virtual bool ReadResponse(void* data_out,
                              int bytes_to_read,
                              int& bytes_read,
                              CefRefPtr<CefCallback> callback) override
    {
        bool has_data = false;
        bytes_read = 0;

        if (offset_ < fileLength_)
        {
            // Copy the next block of data into the buffer.
            int transfer_size = Min(bytes_to_read, static_cast<int>(fileLength_ - offset_));

            file_->Read(data_out, transfer_size);
            offset_ += transfer_size;

            bytes_read = transfer_size;
            has_data = true;

            if (offset_ >= fileLength_)
                file_->Close();
        }


        return has_data;
    }
private:
    SharedPtr<File> file_;
    unsigned fileLength_;
    String url_;
    unsigned offset_;

    IMPLEMENT_REFCOUNTING(ResourceSchemeHandler);
};

class MemoryResourceSchemeHandler : public CefResourceHandler
{
public:
    MemoryResourceSchemeHandler() = default;

    bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
    {
        CEF_REQUIRE_IO_THREAD();
        return true;
    }
    void Cancel() override
    {
        CEF_REQUIRE_IO_THREAD();
    }
    void GetResponseHeaders(CefRefPtr<CefResponse> response, int64_t& response_length, CefString& redirectUrl) override
    {
        CEF_REQUIRE_IO_THREAD();

        auto mime_type = REngine::Web::internal_storage_get_item(MEMORY_RESOURCE_KEY_MIME_TYPE);
        if (mime_type.empty())
            mime_type = "text/html";

        response->SetStatus(200);
        response->SetMimeType(CefString(mime_type.c_str()));
        response_length = static_cast<i64>(REngine::Web::internal_storage_get_item(MEMORY_RESOURCE_KEY_DATA).length());
    }
    bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override
    {
        const auto val = REngine::Web::internal_storage_get_item(MEMORY_RESOURCE_KEY_DATA);
        return true;
    }
    IMPLEMENT_REFCOUNTING(MemoryResourceSchemeHandler);
};

// Implementation of the factory for for creating schema handlers.
class ResourceSchemeHandlerFactory : public CefSchemeHandlerFactory
{

public:
    // Return a new scheme handler instance to handle the request.
    virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 const CefString& scheme_name,
                                                 CefRefPtr<CefRequest> request) override
    {
        CEF_REQUIRE_IO_THREAD();
        return new ResourceSchemeHandler();
    }

    IMPLEMENT_REFCOUNTING(ResourceSchemeHandlerFactory);
};
// Required to create MemoryResource Scheme Handler
class MemoryResourceSchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
    virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& scheme_name,
        CefRefPtr<CefRequest> request) override
    {
        CEF_REQUIRE_IO_THREAD();
        return new MemoryResourceSchemeHandler();
    }

    IMPLEMENT_REFCOUNTING(MemoryResourceSchemeHandlerFactory);
};

void RegisterWebSchemeHandlers(WebBrowserHost* host)
{
    webBrowserHost_ = host;

    CefRegisterSchemeHandlerFactory("atomic", "resources",
                                    new ResourceSchemeHandlerFactory());
    CefRegisterSchemeHandlerFactory("rengine", "memres",
                                    new MemoryResourceSchemeHandlerFactory());
}


}
