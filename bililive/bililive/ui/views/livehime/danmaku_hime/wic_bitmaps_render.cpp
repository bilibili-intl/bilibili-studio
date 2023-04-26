#include "wic_bitmaps_render.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"
#include <unordered_map>

std::unordered_map<std::string, std::vector<base::win::ScopedComPtr<ID2D1Bitmap>>> g_d2dbitmaps_cache;

WICBitmapsRender::WICBitmapsRender(
    const WICBitmaps& sources, float dpi_x, float dpi_y)
    : sources_(sources),
    dpi_x_(dpi_x),
    dpi_y_(dpi_y),
    loop_count_(0),
    frame_index_(0),
    hw_acc_rendered_(false) {
}

 void WICBitmapsRender::CreateRenderTarget() {
     sculptor_.reset(new Sculptor(sources_.width, sources_.height, false, false));
     SetRenderTarget(sculptor_->GetRenderTarget(), false);
}

 void WICBitmapsRender::ResetRenderTargetSource(const WICBitmaps& sources) {
     bitmaps_.clear();
     sources_ = sources;
     sculptor_.reset(new Sculptor(sources_.width, sources_.height, false, false));
     SetRenderTarget(sculptor_->GetRenderTarget(), false);
 }

 size_t WICBitmapsRender::GetCurrentIndex()
 {
     return frame_index_;
 }

bool WICBitmapsRender::SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc) {
    rt_ = rt;

    if (hw_acc != hw_acc_rendered_) {
        bitmaps_.clear();
        hw_acc_rendered_ = hw_acc;
    }
    HRESULT hr = S_OK;
    if (!sources_.empty() && bitmaps_.empty()) {
        if (!sources_.source_address.empty()) {
            auto itr = g_d2dbitmaps_cache.find(sources_.source_address);
            if (itr != g_d2dbitmaps_cache.end()) {  //有缓存数据
                //LOG(INFO) << "## WICBitmapsRender get dynamic cache image";
                bitmaps_ = itr->second;
                if (bitmaps_.size() > 1) {
                    brt_.Release();
                    frame_index_ = 0;
                    hr = rt->CreateCompatibleRenderTarget(
                        D2D1::SizeF(GetImageWidth(), GetImageHeight()), brt_.Receive());
                    DCHECK(SUCCEEDED(hr));
                    PresentFrame();
                }
                else if (bitmaps_.size() == 1) {
                    cur_bitmap_ = bitmaps_.front();
                }
                return true;
            }
        }

        D2D1_BITMAP_PROPERTIES properties
            = D2D1::BitmapProperties(
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                dpi_x_, dpi_y_);

        for (auto source : sources_.frames) {
            base::win::ScopedComPtr<ID2D1Bitmap> bmp;
            hr = rt->CreateBitmapFromWicBitmap(source.bitmap.get(), properties, bmp.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                LOG(WARNING) << "Failed to create bitmap from wic bitmap: " << hr;
                continue;
            }

            bitmaps_.push_back(bmp);
        }

        if (bitmaps_.size() > 1) {
            brt_.Release();
            frame_index_ = 0;
            hr = rt->CreateCompatibleRenderTarget(
                D2D1::SizeF(GetImageWidth(), GetImageHeight()), brt_.Receive());
            DCHECK(SUCCEEDED(hr));
            PresentFrame();
        } else if (bitmaps_.size() == 1) {
            cur_bitmap_ = bitmaps_.front();
        } else {
            return false;
        }
        if (!sources_.source_address.empty()) {
            //LOG(INFO) << "## WICBitmapsRender add new dynamic image";
            g_d2dbitmaps_cache.insert(std::pair<std::string, std::vector<base::win::ScopedComPtr<ID2D1Bitmap>>>(sources_.source_address, bitmaps_));
        }
    }

    return true;
}

void WICBitmapsRender::ReleaseBitmaps() {
    bitmaps_.clear();
}

bool WICBitmapsRender::PresentFrame() {
    // 少于两帧，不需要一直绘制。
    if (bitmaps_.size() < 2) {
        return false;
    }

    // 有的 gif 图有循环次数的限制，
    // 达到限制之后就不需要再循环了。
    if (sources_.loop_count > 0
        && loop_count_ >= sources_.loop_count) {
        return false;
    }

    // 图片序列的一次循环结束。
    if (bitmaps_.size() <= frame_index_) {
        frame_index_ = 0;

        // 如果需要，记录循环次数。
        if (sources_.loop_count > 0) {
            ++loop_count_;
            if (loop_count_ >= sources_.loop_count) {
                return false;
            }
        }
    }

    cur_bitmap_ = bitmaps_[frame_index_];

    if (brt_) {
        DisposeFrame();
        ComposeFrame();
    }

    ++frame_index_;
    return true;
}

base::win::ScopedComPtr<IWICBitmap> WICBitmapsRender::GetNextFrame() {
    if (!rt_) {
        return {};
    }

    //处理相关的帧数据
    PresentFrame();

    //SaveBitmapToFile(L"C:/Users/rc/Desktop/test.png", cur_bitmap_,brt_);

    //将bitmap绘制到wicbitmap上
    Render(D2D1::RectF(0,0,GetImageWidth(), GetImageHeight()));

    return sculptor_->GetWicBitmap();
}

void WICBitmapsRender::Render(const D2D1_RECT_F& rect) {
    if (bitmaps_.size() > 1 && brt_) {
        base::win::ScopedComPtr<ID2D1Bitmap> bmp;
        HRESULT hr = brt_->GetBitmap(bmp.Receive());
        if (SUCCEEDED(hr)) {
            rt_->BeginDraw();
            rt_->Clear(sources_.bg_color);
            rt_->DrawBitmap(bmp.get(), rect);
            rt_->EndDraw();
        }
    } else {
        if (cur_bitmap_) {
            rt_->BeginDraw();
            rt_->Clear(sources_.bg_color);
            rt_->DrawBitmap(cur_bitmap_.get(), rect);
            rt_->EndDraw();
        }
    }
}

void WICBitmapsRender::RenderProcess(const D2D1_RECT_F& rect)
{
    if (bitmaps_.size() > 1 && brt_) {
        base::win::ScopedComPtr<ID2D1Bitmap> bmp;
        HRESULT hr = brt_->GetBitmap(bmp.Receive());
        if (SUCCEEDED(hr)) {
            rt_->DrawBitmap(bmp.get(), rect);
        }
    }
    else {
        if (cur_bitmap_) {
            rt_->DrawBitmap(cur_bitmap_.get(), rect);
        }
    }
}

void WICBitmapsRender::DisposeFrame() {
    if (frame_index_ == 0) {
        return;
    }

    auto pfd = sources_.frames[frame_index_ - 1];

    // GIF 的 disposal 指示了当要绘制下一帧的时候，
    // 当前画布上的内容要如何处理。
    switch (pfd.disposal) {
    case WICFrame::UNSPECIFIED:
    case WICFrame::NONE: {
        break;
    }
    case WICFrame::BACKGROUND: {
        DisposeToBackground();
        break;
    }
    case WICFrame::PREVIOUS: {
        DisposeToPrevious();
        break;
    }
    default: {
        NOTREACHED();
        break;
    }
    }
}

void WICBitmapsRender::DisposeToBackground() {
    auto pfd = sources_.frames[frame_index_ - 1];
    auto rect = D2D1::RectF(
        pfd.left / (dpi_x_ / 96.f),
        pfd.top / (dpi_y_ / 96.f),
        (pfd.left + pfd.width) / (dpi_x_ / 96.f),
        (pfd.top + pfd.height) / (dpi_y_ / 96.f));

    brt_->BeginDraw();
    brt_->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    brt_->Clear(sources_.bg_color);
    brt_->PopAxisAlignedClip();

    HRESULT hr = brt_->EndDraw();
    if (FAILED(hr)) {
        if (hr != D2DERR_RECREATE_TARGET) {
            DCHECK(false);
        }
    }
}

void WICBitmapsRender::DisposeToPrevious() {
    RestoreFrame();
}

void WICBitmapsRender::SaveFrame() {
    base::win::ScopedComPtr<ID2D1Bitmap> frame_to_be_saved;
    HRESULT hr = brt_->GetBitmap(frame_to_be_saved.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        return;
    }

    if (!saved_bitmap_) {
        auto bitmapSize = frame_to_be_saved->GetPixelSize();
        D2D1_BITMAP_PROPERTIES bitmapProp;
        frame_to_be_saved->GetDpi(&bitmapProp.dpiX, &bitmapProp.dpiY);
        bitmapProp.pixelFormat = frame_to_be_saved->GetPixelFormat();

        hr = brt_->CreateBitmap(bitmapSize, bitmapProp, saved_bitmap_.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            return;
        }
    }

    hr = saved_bitmap_->CopyFromBitmap(nullptr, frame_to_be_saved.get(), nullptr);
    DCHECK(SUCCEEDED(hr));
}

void WICBitmapsRender::RestoreFrame() {
    if (saved_bitmap_) {
        base::win::ScopedComPtr<ID2D1Bitmap> frame;
        HRESULT hr = brt_->GetBitmap(frame.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            return;
        }

        hr = frame->CopyFromBitmap(nullptr, saved_bitmap_.get(), nullptr);
        DCHECK(SUCCEEDED(hr));
    }
}

void WICBitmapsRender::ComposeFrame() {
    auto pfd = sources_.frames[frame_index_];
    auto rect = D2D1::RectF(
        pfd.left / (dpi_x_ / 96.f),
        pfd.top / (dpi_y_ / 96.f),
        (pfd.left + pfd.width) / (dpi_x_ / 96.f),
        (pfd.top + pfd.height) / (dpi_y_ / 96.f));

    if (pfd.disposal == WICFrame::PREVIOUS) {
        SaveFrame();
    }

    brt_->BeginDraw();

    //部分gif有重叠绘制的现象，每次都清空对gif显示无影响
    if (frame_index_ == 0) {
        brt_->Clear(sources_.bg_color);
    }

    brt_->DrawBitmap(cur_bitmap_.get(), rect);

    HRESULT hr = brt_->EndDraw();
    if (FAILED(hr)) {
        if (hr != D2DERR_RECREATE_TARGET) {
            DCHECK(false);
        }
    }
}


float WICBitmapsRender::GetImageWidth() {
    return sources_.width / (dpi_x_ / 96.f);
}

float WICBitmapsRender::GetImageHeight() {
    return sources_.height / (dpi_y_ / 96.f);
}

HRESULT WICBitmapsRender::SaveBitmapToPng(PCWSTR uri,
                                         ID2D1Bitmap* pBitmap,
                                         ID2D1RenderTarget* pRenderTarget){

    HRESULT hr = S_OK;

    ID2D1Factory* d2d_factory = nullptr;
    IWICBitmap* wic_bitmap = nullptr;
    ID2D1RenderTarget* rt = nullptr;
    IWICBitmapEncoder* bit_encoder = nullptr;
    IWICBitmapFrameEncode* frame_encode = nullptr;
    IWICStream* pStream = NULL;

    base::win::ScopedComPtr<IWICImagingFactory> pWICFactory;

    d2d_factory = D2DManager::GetInstance()->GetD2DFactory().get();
    pWICFactory = D2DManager::GetInstance()->GetWicFactory();
    //
    // Create IWICBitmap and RT
    //

    UINT sc_bitmapWidth = pBitmap->GetSize().width;
    UINT sc_bitmapHeight = pBitmap->GetSize().height;

    if (SUCCEEDED(hr)){
        hr = pWICFactory->CreateBitmap(
            sc_bitmapWidth,
            sc_bitmapHeight,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            &wic_bitmap
        );
    }

    if (SUCCEEDED(hr)){
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
        rtProps.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_NONE;

        hr = d2d_factory->CreateWicBitmapRenderTarget(
            wic_bitmap,
            rtProps,
            &rt
        );
    }

    if (SUCCEEDED(hr)){
        //
        // Render into the bitmap
        //
        rt->BeginDraw();
        rt->Clear(D2D1::ColorF(D2D1::ColorF::White));
        rt->DrawBitmap(pBitmap);
        rt->EndDraw();
    }
    if (SUCCEEDED(hr)){
        //
        // Save image to file
        //
        hr = pWICFactory->CreateStream(&pStream);
    }

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppPBGRA;
    if (SUCCEEDED(hr)){

        hr = pStream->InitializeFromFilename(uri, GENERIC_WRITE);
    }
    if (SUCCEEDED(hr)){
        hr = pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &bit_encoder);
    }
    if (SUCCEEDED(hr)){
        hr = bit_encoder->Initialize(pStream, WICBitmapEncoderNoCache);
    }
    if (SUCCEEDED(hr)){
        hr = bit_encoder->CreateNewFrame(&frame_encode, NULL);
    }
    if (SUCCEEDED(hr)){
        hr = frame_encode->Initialize(NULL);
    }
    if (SUCCEEDED(hr)){
        hr = frame_encode->SetSize(sc_bitmapWidth, sc_bitmapHeight);
    }
    if (SUCCEEDED(hr)){
        hr = frame_encode->SetPixelFormat(&format);
    }
    if (SUCCEEDED(hr)){
        hr = frame_encode->WriteSource(wic_bitmap, NULL);
    }
    if (SUCCEEDED(hr)){
        hr = frame_encode->Commit();
    }
    if (SUCCEEDED(hr)){
        hr = bit_encoder->Commit();
    }

    return hr;
}