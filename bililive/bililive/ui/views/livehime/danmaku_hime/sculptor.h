#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_SCULPTOR_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_SCULPTOR_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"


class Paint;
class TextLayout;

class Sculptor {
public:
    Sculptor(int width, int height, bool dpi_awareness, bool hw_acc);
    explicit Sculptor(const base::win::ScopedComPtr<ID2D1RenderTarget>& rt);
    ~Sculptor();

    void Begin();
    bool End();
    bool IsFailed() const;
    Paint* GetPaint() const;

    std::shared_ptr<Paint> CreateNewPaint(const D2D1_COLOR_F& color);

    void DrawLine(const D2D1_POINT_2F& start, const D2D1_POINT_2F& end);
    void DrawLine(const D2D1_POINT_2F& start, const D2D1_POINT_2F& end, const Paint* p);
    void DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float opacity = 1.f);
    void DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float width, float height, float opacity = 1.f);
    void DrawRect(const D2D1_RECT_F& rect);
    void DrawRect(const D2D1_RECT_F& rect, const Paint* p);
    void DrawCircle(const D2D1_POINT_2F& center, float radius);
    void DrawCircle(const D2D1_POINT_2F& center, float radius, const Paint* p);
    void DrawRoundRect(const D2D1_RECT_F& rect, float radius);
    void DrawRoundRect(const D2D1_RECT_F& rect, float radius, const Paint* p);
    void DrawRoundRect(
        const D2D1_RECT_F& rect,
        float r_lt, float r_tr, float r_rb, float r_bl);
    void DrawRoundRect(
        const D2D1_RECT_F& rect,
        float r_lt, float r_tr, float r_rb, float r_bl, const Paint* p);
    void DrawGeometry(ID2D1Geometry* geo);
    void DrawGeometry(ID2D1Geometry* geo, const Paint* p);
    void DrawTextLayout(TextLayout& layout, float x, float y);
    void DrawTextLayout(TextLayout& layout, float x, float y, const Paint* p);

    void DrawTextLayoutWithOutline(
        TextLayout& layout, float x, float y,
        const D2D1_COLOR_F& text_color, const D2D1_COLOR_F& outline_color, float outline_width);

    base::win::ScopedComPtr<ID2D1Bitmap> ExtractBitmap();
    base::win::ScopedComPtr<IWICBitmap> GetWicBitmap();
    base::win::ScopedComPtr<ID2D1RenderTarget> GetRenderTarget() const;

private:
    bool attached_;
    bool hardware_acceleration_;

    std::unique_ptr<Paint> paint_;
    base::win::ScopedComPtr<IWICBitmap> wic_bitmap_;
    base::win::ScopedComPtr<IDXGISurface> dxgi_surface_;
    base::win::ScopedComPtr<ID2D1RenderTarget> render_target_;

    DISALLOW_COPY_AND_ASSIGN(Sculptor);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_SCULPTOR_H_