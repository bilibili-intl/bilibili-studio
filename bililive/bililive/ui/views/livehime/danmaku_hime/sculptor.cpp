#include "sculptor.h"

#include "base/win/windows_version.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_layout.h"


Sculptor::Sculptor(int width, int height, bool dpi_awareness, bool hw_acc)
    : attached_(false),
      hardware_acceleration_(hw_acc)
{
    if (hardware_acceleration_) {
        dxgi_surface_ = D2DManager::GetInstance()->CreateDXGISurface(width, height);
        if (!dxgi_surface_) {
            return;
        }

        render_target_ = D2DManager::GetInstance()->CreateHardwareRenderTarget(
            dxgi_surface_, dpi_awareness);
        if (!render_target_) {
            return;
        }
    } else {
        wic_bitmap_ = D2DManager::GetInstance()->CreateWICBitmap(width, height);
        if (!wic_bitmap_) {
            return;
        }

        render_target_ = D2DManager::GetInstance()->CreateWICRenderTarget(
            wic_bitmap_, dpi_awareness);
        if (!render_target_) {
            return;
        }
    }

    paint_ = std::make_unique<Paint>(render_target_);
}

Sculptor::Sculptor(const base::win::ScopedComPtr<ID2D1RenderTarget>& rt)
    : attached_(true),
      hardware_acceleration_(false)
{
    render_target_ = rt;
    paint_ = std::make_unique<Paint>(render_target_);
}

Sculptor::~Sculptor() {
}

void Sculptor::Begin() {
    if (attached_) {
        return;
    }

    if (!render_target_) {
        return;
    }

    render_target_->BeginDraw();
}

bool Sculptor::End() {
    if (attached_) {
        return true;
    }

    if (!render_target_) {
        return false;
    }

    HRESULT hr = render_target_->EndDraw();
    if (FAILED(hr)) {
        if (hr != D2DERR_RECREATE_TARGET
            && hr != D2DERR_WRONG_RESOURCE_DOMAIN)
        {
            DCHECK(false);
        }
        return false;
    }

    return true;
}

bool Sculptor::IsFailed() const {
    if (attached_) {
        return !render_target_;
    }

    if (hardware_acceleration_) {
        return !dxgi_surface_ || !render_target_;
    } else {
        return !wic_bitmap_ || !render_target_;
    }
}

Paint* Sculptor::GetPaint() const {
    return paint_.get();
}

std::shared_ptr<Paint> Sculptor::CreateNewPaint(const D2D1_COLOR_F& color) {
    auto paint = std::make_shared<Paint>(render_target_);
    paint->SetColor(color);
    return paint;
}

void Sculptor::DrawLine(const D2D1_POINT_2F& start, const D2D1_POINT_2F& end) {
    DrawLine(start, end, paint_.get());
}

void Sculptor::DrawLine(
    const D2D1_POINT_2F& start, const D2D1_POINT_2F& end, const Paint* p)
{
    if (render_target_ && p && p->GetBrush()) {
        render_target_->DrawLine(
            start, end, p->GetBrush(), p->GetStrokeWidth());
    }
}

void Sculptor::DrawBitmap(
    ID2D1Bitmap* bitmap, float x, float y, float opacity)
{
    DCHECK(bitmap != nullptr);

    if (render_target_) {
        int bitmap_width = bitmap->GetPixelSize().width;
        int bitmap_height = bitmap->GetPixelSize().height;

        auto dest = D2D1::RectF(x, y, x + bitmap_width, y + bitmap_height);
        render_target_->DrawBitmap(bitmap, dest, opacity);
    }
}

void Sculptor::DrawBitmap(
    ID2D1Bitmap* bitmap, float x, float y, float width, float height, float opacity)
{
    DCHECK(bitmap != nullptr && width > 0 && height > 0);

    if (render_target_) {
        auto dest = D2D1::RectF(x, y, x + width, y + height);
        render_target_->DrawBitmap(bitmap, dest, opacity);
    }
}

void Sculptor::DrawRect(const D2D1_RECT_F& rect) {
    DrawRect(rect, paint_.get());
}

void Sculptor::DrawRect(const D2D1_RECT_F& rect, const Paint* p) {
    DCHECK(p != nullptr);

    if (render_target_ && p && p->GetBrush()) {
        switch (p->GetStyle()) {
        case Paint::STROKE:
            render_target_->DrawRectangle(
                rect,
                p->GetBrush(),
                p->GetStrokeWidth());
            break;
        case Paint::FILL:
            render_target_->FillRectangle(
                rect,
                p->GetBrush());
            break;
        default:
            NOTREACHED();
            break;
        }
    }
}

void Sculptor::DrawCircle(const D2D1_POINT_2F& center, float radius) {
    DrawCircle(center, radius, paint_.get());
}

void Sculptor::DrawCircle(const D2D1_POINT_2F& center, float radius, const Paint* p) {
    DCHECK(p != nullptr);
    if (render_target_ && p && p->GetBrush()) {
        switch (p->GetStyle()) {
        case Paint::STROKE:
            render_target_->DrawEllipse(
                D2D1::Ellipse(center, radius, radius),
                p->GetBrush(),
                p->GetStrokeWidth());
            break;
        case Paint::FILL:
            render_target_->FillEllipse(
                D2D1::Ellipse(center, radius, radius),
                p->GetBrush());
            break;
        default:
            NOTREACHED();
            break;
        }
    }
}

void Sculptor::DrawRoundRect(const D2D1_RECT_F& rect, float radius) {
    DrawRoundRect(rect, radius, paint_.get());
}

void Sculptor::DrawRoundRect(const D2D1_RECT_F& rect, float radius, const Paint* p) {
    DCHECK(p != nullptr);

    if (render_target_ && p && p->GetBrush()) {
        switch (p->GetStyle()) {
        case Paint::STROKE:
            render_target_->DrawRoundedRectangle(
                D2D1::RoundedRect(rect, radius, radius),
                p->GetBrush(),
                p->GetStrokeWidth());
            break;
        case Paint::FILL:
            render_target_->FillRoundedRectangle(
                D2D1::RoundedRect(rect, radius, radius),
                p->GetBrush());
            break;
        default:
            NOTREACHED();
            break;
        }
    }
}

void Sculptor::DrawRoundRect(
    const D2D1_RECT_F& rect,
    float r_lt, float r_tr, float r_rb, float r_bl)
{
    DrawRoundRect(rect, r_lt, r_tr, r_rb, r_bl, paint_.get());
}

void Sculptor::DrawRoundRect(
    const D2D1_RECT_F& rect,
    float r_lt, float r_tr, float r_rb, float r_bl, const Paint* p)
{
    auto geo = D2DManager::GetInstance()->CreatePathGeometry();
    if (!geo) {
        return;
    }

    base::win::ScopedComPtr<ID2D1GeometrySink> sink;
    HRESULT hr = geo->Open(sink.Receive());
    if (SUCCEEDED(hr)) {
        sink->BeginFigure(
            D2D1::Point2F(rect.left, rect.top + r_lt),
            D2D1_FIGURE_BEGIN_FILLED);

        sink->AddArc(
            D2D1::ArcSegment(
                D2D1::Point2F(rect.left + r_lt, rect.top),
                D2D1::SizeF(r_lt, r_lt), 0.f,
                D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); // left-top corner

        sink->AddLine(D2D1::Point2F(rect.right - r_tr, rect.top));     // top

        sink->AddArc(
            D2D1::ArcSegment(
                D2D1::Point2F(rect.right, rect.top + r_tr),
                D2D1::SizeF(r_tr, r_tr), 0.f,
                D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); // top-right corner

        sink->AddLine(D2D1::Point2F(rect.right, rect.bottom - r_rb));  // right

        sink->AddArc(
            D2D1::ArcSegment(
                D2D1::Point2F(rect.right - r_rb, rect.bottom),
                D2D1::SizeF(r_rb, r_rb), 0.f,
                D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); // right-bottom corner

        sink->AddLine(D2D1::Point2F(rect.left + r_bl, rect.bottom)); // bottom

        sink->AddArc(
            D2D1::ArcSegment(
                D2D1::Point2F(rect.left, rect.bottom - r_bl),
                D2D1::SizeF(r_bl, r_bl), 0.f,
                D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL)); // bottom-left corner

        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        hr = sink->Close();
        DCHECK(SUCCEEDED(hr));
    }

    DrawGeometry(geo, p);
}

void Sculptor::DrawGeometry(ID2D1Geometry* geo) {
    DrawGeometry(geo, paint_.get());
}

void Sculptor::DrawGeometry(ID2D1Geometry* geo, const Paint* p) {
    DCHECK(p != nullptr);

    if (render_target_ && p && p->GetBrush()) {
        switch (p->GetStyle()) {
        case Paint::STROKE:
            render_target_->DrawGeometry(
                geo, p->GetBrush(), p->GetStrokeWidth());
            break;
        case Paint::FILL:
            render_target_->FillGeometry(geo, p->GetBrush());
            break;
        default:
            NOTREACHED();
            break;
        }
    }
}

void Sculptor::DrawTextLayout(TextLayout& layout, float x, float y) {
    DrawTextLayout(layout, x, y, paint_.get());
}

void Sculptor::DrawTextLayout(TextLayout& layout, float x, float y, const Paint* p) {
    DCHECK(layout.Get() != nullptr && p != nullptr);

    if (render_target_ && layout.Get() && p && p->GetBrush()) {
        if (base::win::GetVersion() >= base::win::VERSION_WIN8_1) {
            // D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
            static const int TEXT_OPTIONS_ENABLE_COLOR_FONT = 0x00000004;
            render_target_->DrawTextLayout(
                D2D1::Point2F(x, y), layout.Get(), p->GetBrush(),
                static_cast<D2D1_DRAW_TEXT_OPTIONS>(TEXT_OPTIONS_ENABLE_COLOR_FONT));
        } else {
            render_target_->DrawTextLayout(
                D2D1::Point2F(x, y), layout.Get(), p->GetBrush());
        }
    }
}

void Sculptor::DrawTextLayoutWithOutline(
    TextLayout& layout, float x, float y,
    const D2D1_COLOR_F& text_color, const D2D1_COLOR_F& outline_color, float outline_width)
{
    layout.DrawWithOutline(x, y, render_target_, text_color, outline_color, outline_width);
}

base::win::ScopedComPtr<ID2D1Bitmap> Sculptor::ExtractBitmap() {
    if (attached_) {
        return {};
    }

    if (!render_target_) {
        return {};
    }

    HRESULT hr;
    base::win::ScopedComPtr<ID2D1Bitmap> bitmap;

    if (hardware_acceleration_) {
        auto prop = D2D1::BitmapProperties(D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        hr = render_target_->CreateSharedBitmap(
            __uuidof(IDXGISurface), dxgi_surface_.get(), &prop, bitmap.Receive());
    } else {
        hr = render_target_->CreateBitmapFromWicBitmap(
            wic_bitmap_.get(), nullptr, bitmap.Receive());
    }

    if (FAILED(hr)) {
        DCHECK(false);
        return {};
    }

    return bitmap;
}

base::win::ScopedComPtr<IWICBitmap> Sculptor::GetWicBitmap() {
    return wic_bitmap_;
}

base::win::ScopedComPtr<ID2D1RenderTarget> Sculptor::GetRenderTarget() const {
    return render_target_;
}