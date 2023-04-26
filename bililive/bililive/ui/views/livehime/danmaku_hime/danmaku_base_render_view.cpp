#include "danmaku_base_render_view.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"

DanmakuBaseRenderView::DanmakuBaseRenderView()
{
}

DanmakuBaseRenderView::~DanmakuBaseRenderView()
{
}

void DanmakuBaseRenderView::OnPaint(gfx::Canvas* canvas)
{
    __super::OnPaint(canvas);

    if (using_hardware_acceleration_) {
        auto result = HardwareRender(canvas);
        switch (result) {
        case HWRenderResult::NeedRetry: {
            LOG(WARNING) << "An error occurred when using hardware-rendering, retrying...";
            // 只重试一次
            hw_rt_.Release();
            result = HardwareRender(canvas);
            if (result == HWRenderResult::Succeeded) {
                LOG(INFO) << "The error have been repaired.";
            }
            else {
                LOG(WARNING) << "Failed to hardware-rendering, fall back to software-rendering.";
                // fallback
                FallbackToSoftwareRender();
            }
            break;
        }
        case HWRenderResult::Failed: {
            LOG(WARNING) << "Failed to hardware-rendering, fall back to software-rendering.";
            // fallback
            FallbackToSoftwareRender();
            break;
        }
        default:
            break;
        }
    }
    else {
        SoftwareRender(canvas);
    }
}

void DanmakuBaseRenderView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    if (!using_hardware_acceleration_) {
        return;
    }

    auto bounds = GetContentsBounds();
    if (!bounds.IsEmpty()) {
        auto surface = D2DManager::GetInstance()->CreateDXGISurface(bounds.width(), bounds.height());
        if (surface) {
            hw_rt_ = D2DManager::GetInstance()->CreateHardwareRenderTarget(surface, false);
        }
        else {
            hw_rt_ = nullptr;
        }
    }
}

void DanmakuBaseRenderView::OnDraw(Sculptor& s, bool use_hardware)
{
    
}

bool DanmakuBaseRenderView::UsingHardware()
{
    return using_hardware_acceleration_;
}

ID2D1RenderTarget* DanmakuBaseRenderView::GetRenderTarget()
{
    return hw_rt_.get();
}

void DanmakuBaseRenderView::CreateHardwareSurface()
{
    if (!using_hardware_acceleration_) {
        return;
    }

    auto bounds = GetContentsBounds();
    if (!bounds.IsEmpty()) {
        auto surface = D2DManager::GetInstance()->CreateDXGISurface(bounds.width(), bounds.height());
        if (surface) {
            hw_rt_ = D2DManager::GetInstance()->CreateHardwareRenderTarget(surface, false);
        }
        else {
            hw_rt_ = nullptr;
        }
    }
}

DanmakuBaseRenderView::HWRenderResult DanmakuBaseRenderView::HardwareRender(gfx::Canvas* canvas)
{
    if (!hw_rt_) {
        CreateHardwareSurface();
        if (!hw_rt_) {
            return HWRenderResult::NeedRetry;
        }
    }

    HWRenderResult result_code = HWRenderResult::Succeeded;
    gfx::Canvas offc(GetContentsBounds().size(), ui::SCALE_FACTOR_100P, false);

    HDC hDC = offc.BeginPlatformPaint();

    hw_rt_->BeginDraw();
    hw_rt_->Clear();

    Sculptor sp(hw_rt_);
    OnDraw(sp, true);

    base::win::ScopedComPtr<ID2D1GdiInteropRenderTarget> gdi_rt;
    HRESULT hr = hw_rt_.QueryInterface(gdi_rt.Receive());
    if (SUCCEEDED(hr)) {
        HDC hdc = nullptr;
        hr = gdi_rt->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdc);
        if (SUCCEEDED(hr)) {
            auto bounds = GetContentsBounds();
            BOOL result = ::BitBlt(
                hDC, 0, 0, bounds.width(), bounds.height(),
                hdc, 0, 0, SRCPAINT);
            DCHECK(result != 0);

            RECT rect = {};
            hr = gdi_rt->ReleaseDC(&rect);
            DCHECK(SUCCEEDED(hr));
        }
        else {
            if (hr == D2DERR_WRONG_RESOURCE_DOMAIN) {
                result_code = HWRenderResult::NeedRetry;
            }
            else if (D2DManager::GetInstance()->ProcessDeviceError(hr)) {
                result_code = HWRenderResult::NeedRetry;
            }
            else {
                result_code = HWRenderResult::Failed;
                DCHECK(false);
            }
        }
    }
    else {
        NOTREACHED();
    }

    if (hw_rt_) {
        hr = hw_rt_->EndDraw();
        if (FAILED(hr)) {
            if (hr == D2DERR_RECREATE_TARGET ||
                hr == D2DERR_WRONG_RESOURCE_DOMAIN)
            {
                if (result_code == HWRenderResult::Succeeded) {
                    result_code = HWRenderResult::NeedRetry;
                }
            }
            else {
                if (result_code == HWRenderResult::Succeeded) {
                    result_code = HWRenderResult::Failed;
                    DCHECK(false);
                }
            }
        }
    }

    offc.EndPlatformPaint();

    canvas->DrawImageInt(gfx::ImageSkia(offc.ExtractImageRep()), 0, 0);

    return result_code;
}

void DanmakuBaseRenderView::SoftwareRender(gfx::Canvas* canvas)
{
    auto render_target = D2DManager::GetInstance()->CreateDCRenderTarget();
    if (render_target) {
        HDC hDC = canvas->BeginPlatformPaint();

        auto bounds = GetContentsBounds();
        RECT rect = { 0, 0, bounds.width(), bounds.height() };
        HRESULT hr = render_target->BindDC(hDC, &rect);

        if (SUCCEEDED(hr)) {
            base::win::ScopedComPtr<ID2D1RenderTarget> rt;
            hr = render_target.QueryInterface(rt.Receive());
            DCHECK(SUCCEEDED(hr));

            rt->BeginDraw();

            Sculptor sp(rt);
            OnDraw(sp, false);

            hr = rt->EndDraw();
            if (FAILED(hr)) {
                DCHECK(false);
            }
        }
        else {
            DCHECK(false);
        }

        canvas->EndPlatformPaint();
    }
}

void DanmakuBaseRenderView::FallbackToSoftwareRender()
{
    hw_rt_.Release();
    using_hardware_acceleration_ = false;
    SchedulePaint();
}


