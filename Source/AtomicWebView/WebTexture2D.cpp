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

#ifdef ATOMIC_PLATFORM_OSX
#include <ThirdParty/GLEW/glew.h>
#endif

#include <include/cef_render_handler.h>

#include <Atomic/Math/Rect.h>
#include <Atomic/IO/Log.h>
#include <Atomic/Resource/ResourceCache.h>
#include <Atomic/Graphics/Graphics.h>
#include <Atomic/Graphics/GraphicsImpl.h>
#include <Atomic/Graphics/Technique.h>

#include "WebClient.h"
#include "WebTexture2D.h"


namespace Atomic
{

class WebTexture2DPrivate : public CefRenderHandler
{
    friend class WebTexture2D;

public:

    IMPLEMENT_REFCOUNTING(WebTexture2DPrivate);

    WebTexture2DPrivate(WebTexture2D* webTexture2D)
    {
        webTexture2D_ = webTexture2D;
        graphics_ = webTexture2D->GetSubsystem<Graphics>();
        ClearPopupRects();
    }

    void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override
    {
        if (!show)
        {
            // Clear the popup rectangle.
            ClearPopupRects();
        }
    }

    void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override
    {
        if (rect.width <= 0 || rect.height <= 0)
        {
            ClearPopupRects();
            return;
        }

        popupRectOriginal_ = rect;
        popupRect_ = GetPopupRectInWebView(popupRectOriginal_);
    }

    CefRect GetPopupRectInWebView(const CefRect& original_rect) {

        CefRect rc(original_rect);

        // if x or y are negative, move them to 0.
        if (rc.x < 0)
            rc.x = 0;
        if (rc.y < 0)
            rc.y = 0;

        // if popup goes outside the view, try to reposition origin
        if (rc.x + rc.width > webTexture2D_->GetWidth())
            rc.x = webTexture2D_->GetWidth() - rc.width;
        if (rc.y + rc.height > webTexture2D_->GetHeight())
            rc.y = webTexture2D_->GetHeight() - rc.height;

        // if x or y became negative, move them to 0 again.
        if (rc.x < 0)
            rc.x = 0;
        if (rc.y < 0)
            rc.y = 0;

        return rc;
    }

    void ClearPopupRects()
    {
        popupRect_.Set(0, 0, 0, 0);
        popupRectOriginal_.Set(0, 0, 0, 0);
    }

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override
    {
        rect = CefRect(0, 0, webTexture2D_->GetWidth(), webTexture2D_->GetHeight());
    }

    // Diligent blitting
    void OnBlit(const IntRect& dstRect, unsigned char* src, unsigned srcStride)
    {
        if (!webTexture2D_ || !webTexture2D_->GetWidth() || !webTexture2D_->GetHeight() || !dstRect.Width() || !dstRect.Height())
        {
            return;
        }

        if ((dstRect.left_ + dstRect.Width() > webTexture2D_->GetWidth()) || (dstRect.top_ + dstRect.Height() > webTexture2D_->GetHeight()))
            return;

        // Call blit directly
        const auto device = graphics_->GetImpl();
        const auto context = device->GetDeviceContext();

        Diligent::Box box;
        box.MinX = dstRect.left_;
        box.MaxX = dstRect.right_;
        box.MinY = dstRect.top_;
        box.MaxY = dstRect.bottom_;
        box.MinZ = 0;
        box.MaxZ = 1;

        Diligent::TextureSubResData data = {};
        data.Stride = srcStride;
        data.pData = src;
        context->UpdateTexture(
            webTexture2D_->GetTexture2D()->GetGPUObject().Cast<Diligent::ITexture>(Diligent::IID_Texture),
            0, 0, box,
            data,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );
    }

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects,
        const void *buffer, int width, int height) override
    {
        if (type == PET_VIEW)
        {
            if (dirtyRects.size() == 1 &&
                dirtyRects[0] == CefRect(0, 0, webTexture2D_->GetWidth(), webTexture2D_->GetHeight()))
            {
                OnBlit(IntRect(0, 0, width, height), (unsigned char*)buffer, width * 4);
                return;
            }

            // Update just the dirty rectangles
            CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();

            for (; i != dirtyRects.end(); ++i)
            {
                const CefRect& rect = *i;
                unsigned char* src = (unsigned char*)buffer;
                src += rect.y * (width * 4) + (rect.x * 4);
                OnBlit(IntRect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height), src, width * 4);
            }

        }
        else if (type == PET_POPUP && popupRect_.width > 0 && popupRect_.height > 0)
        {
            int  x = popupRect_.x;
            int  y = popupRect_.y;
            int w = width;
            int h = height;
            int viewwidth = webTexture2D_->GetWidth();
            int viewheight = webTexture2D_->GetHeight();

            // Adjust the popup to fit inside the view.
            if (x < 0)
            {
                x = 0;
            }
            if (y < 0)
            {
                y = 0;
            }
            if (x + w > viewwidth)
            {
                w -= x + w - viewwidth;
            }
            if (y + h > viewheight)
            {
                h -= y + h - viewheight;
            }

            unsigned char* src = (unsigned char*)buffer;
            OnBlit(IntRect(x, y, x + w, y + h), src, width * 4);
        }
    }

private:

    CefRect popupRect_;
    CefRect popupRectOriginal_;

    WeakPtr<Graphics> graphics_;
    WeakPtr<WebTexture2D> webTexture2D_;

};

WebTexture2D::WebTexture2D(Context* context) : WebRenderHandler(context), clearColor_(Color::WHITE)
{
    d_ = new WebTexture2DPrivate(this);
    d_->AddRef();

    texture_ = new Texture2D(context_);
    texture_->SetNumLevels(1);
    texture_->SetFilterMode(FILTER_NEAREST);

}

WebTexture2D::~WebTexture2D()
{
    //d_->Release();
}

CefRenderHandler* WebTexture2D::GetCEFRenderHandler()
{
    return d_;
}

int WebTexture2D::GetWidth() const
{
    return texture_->GetWidth();
}

int WebTexture2D::GetHeight() const
{
    return texture_->GetHeight();
}

void WebTexture2D::SetSize(int width, int height)
{
    if (width == texture_->GetWidth() && height == texture_->GetHeight())
        return;

    // initialize to white (color should probably be an option)

    TextureUsage textureUsage = TEXTURE_STATIC;
    unsigned format = TextureFormat::TEX_FORMAT_BGRA8_UNORM;

    if (!texture_->SetSize(width, height, static_cast<TextureFormat>(format), textureUsage))
    {
        ATOMIC_LOGERRORF("Unable to set WebTexture2D size to %i x %i", width, height);
        return;
    }
    
    SharedArrayPtr<unsigned> cleardata(new unsigned[width * height]);
    unsigned color = clearColor_.ToUInt();
    unsigned* ptr = cleardata.Get();
    for (unsigned i = 0; i < width * height; i++, ptr++)
    {
        *ptr = color;
    }

    texture_->SetData(0, 0, 0, width, height, cleardata.Get());

}

}