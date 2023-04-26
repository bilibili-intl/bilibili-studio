// The Bilibili Livehime Authors.
//
//

#include "ui/gfx/canvas.h"

#include <map>

#include <d2d1.h>
#include <dwrite.h>

#include "base/containers/mru_cache.h"
#include "base/memory/singleton.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_comptr.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/font.h"
#include "ui/gfx/rect.h"


namespace gfx {

namespace {

// 指定多少长度以内的字符串需要缓存
const unsigned int kTextCacheLengthThreshold = 10;

class CanvasD2DManager {
public:
    static CanvasD2DManager* GetInstance() {
        return Singleton<CanvasD2DManager>::get();
    }

    bool IsSucceeded() const {
        return init_succeeded_;
    }

    string16 GetFontName16(const std::string& name8) {
        string16 name16;
        if (!name8.empty()) {
            auto it = font_name_cache_.Get(name8);
            if (it != font_name_cache_.end()) {
                name16 = it->second;
            } else {
                name16 = UTF8ToUTF16(name8);
                font_name_cache_.Put(name8, name16);
            }
        }

        return name16;
    }

    base::win::ScopedComPtr<ID2D1DCRenderTarget> CreateDCRenderTarget() {
        if (d2d_factory_ && !render_target_) {
            const D2D1_PIXEL_FORMAT format =
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_PREMULTIPLIED);

            const D2D1_RENDER_TARGET_PROPERTIES properties =
                D2D1::RenderTargetProperties(
                    D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                    format, 96, 96);

            base::win::ScopedComPtr<ID2D1DCRenderTarget> render_target;
            HRESULT hr = d2d_factory_->CreateDCRenderTarget(
                &properties,
                render_target.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                LOG(WARNING) << "Failed to create DC RenderTarget: " << hr;
                return {};
            }

            render_target_ = render_target;
        }

        return render_target_;
    }

    base::win::ScopedComPtr<IDWriteInlineObject> CreateEllipsisTrimmingSign() {
        if (!ellipse_ && dwrite_factory_ && text_format_) {
            base::win::ScopedComPtr<IDWriteInlineObject> inline_object;
            HRESULT hr = dwrite_factory_->CreateEllipsisTrimmingSign(
                text_format_.get(), inline_object.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                return {};
            }

            ellipse_ = inline_object;
        }

        return ellipse_;
    }

    base::win::ScopedComPtr<ID2D1SolidColorBrush> CreateBrush(SkColor color) {
        if (!brush_ && render_target_) {
            base::win::ScopedComPtr<ID2D1SolidColorBrush> brush;
            HRESULT hr = render_target_->CreateSolidColorBrush(
                D2D1::ColorF(color, SkColorGetA(color) / 255.f), brush.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                return {};
            }

            brush_ = brush;
        } else {
            brush_->SetColor(D2D1::ColorF(color, SkColorGetA(color) / 255.f));
        }

        return brush_;
    }

    base::win::ScopedComPtr<IDWriteTextLayout> CreateTextLayout(
        const string16& text, int width, int height, const Font& font, int flags, bool is_measure)
    {
        bool is_reused = false;
        base::win::ScopedComPtr<IDWriteTextLayout> text_layout;
        if (text.length() <= kTextCacheLengthThreshold) {
            auto it = text_layout_cache_.Get(text);
            if (it != text_layout_cache_.end()) {
                text_layout = it->second;
                is_reused = true;
            }
        }

        if (!text_layout) {
            HRESULT hr = dwrite_factory_->CreateTextLayout(
                text.c_str(), text.length(), text_format_.get(), width, height, text_layout.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                return {};
            }

            if (text.length() <= kTextCacheLengthThreshold) {
                text_layout_cache_.Put(text, text_layout);
            }
        }

        DWRITE_TEXT_RANGE range;
        range.startPosition = 0;
        range.length = text.length();

        if (is_reused) {
            text_layout->SetMaxWidth(width);
            text_layout->SetMaxHeight(height);
            text_layout->SetFontStyle(DWRITE_FONT_STYLE_NORMAL, range);
            text_layout->SetUnderline(FALSE, range);
        }

        string16 font_name = GetFontName16(font.GetFontName());
        text_layout->SetFontFamilyName(font_name.c_str(), range);
        text_layout->SetFontSize(font.GetFontSize(), range);
        text_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        text_layout->SetReadingDirection(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
        text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

        {
            DWRITE_TRIMMING dt;
            dt.delimiter = 0;
            dt.delimiterCount = 0;
            dt.granularity = DWRITE_TRIMMING_GRANULARITY_NONE;
            text_layout->SetTrimming(&dt, nullptr);
        }

        // 文字样式
        int style = font.GetStyle();
        text_layout->SetFontWeight(DWRITE_FONT_WEIGHT_NORMAL, range);
        if (style & Font::FontStyle::BOLD) {
            text_layout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
        }
        if (style & Font::FontStyle::ITALIC) {
            text_layout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
        }
        if (style & Font::FontStyle::UNDERLINE) {
            text_layout->SetUnderline(TRUE, range);
        }

        // 左右居中对齐
        if (flags & Canvas::TEXT_ALIGN_LEFT) {
            text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        } else if (flags & Canvas::TEXT_ALIGN_CENTER) {
            text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        } else if (flags & Canvas::TEXT_ALIGN_RIGHT) {
            text_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        }

        // 多行处理
        if ((flags & Canvas::MULTI_LINE) && width > 0) {
            // 这里只要指定换行，就必须是 CHARACTER_BREAK，目前对业务无影响
            text_layout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
            if (!(flags & Canvas::NO_ELLIPSIS) && !is_measure) {
                // 在文字超出布局框时，决定是否省略文字.
                // 上层已处理了非多行的情况，这里只需要处理多行，且未指定 CHARACTER_BREAK 的情况。
                // 这里应为 TextLayout 指定省略符，TextLayout 默认无省略符。

                DWRITE_TRIMMING dt;
                dt.delimiter = 0;
                dt.delimiterCount = 0;
                dt.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
                text_layout->SetTrimming(&dt, CreateEllipsisTrimmingSign());
            }
        }

        if (flags & Canvas::SHOW_PREFIX) {
            // 将以 & 开头的字符串渲染为某些样式。上层没这需求，不管
        }
        if (flags & Canvas::HIDE_PREFIX) {
            // 将以 & 开头的字符串渲染为某些样式。上层没这需求，不管
        }

        if (flags & Canvas::FORCE_RTL_DIRECTIONALITY) {
            text_layout->SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
        }
        if (flags & Canvas::FORCE_LTR_DIRECTIONALITY) {
            text_layout->SetReadingDirection(DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
        }
        if (flags & Canvas::NO_SUBPIXEL_RENDERING) {
            // 该标志对 DirectWrite 无意义。
            // 旧的 TextRenderWin 在渲染半透明窗口上的字时会出现问题。指定这个标志
            // 可以防止问题产生，但视觉效果很差。DirectWrite 无此问题。
        }

        return text_layout;
    }

private:
    CanvasD2DManager()
        : init_succeeded_(false),
          font_name_cache_(20),
          text_layout_cache_(100)
    {
        if (!d2d_factory_) {
            HRESULT hr = ::D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                d2d_factory_.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                return;
            }
        }

        if (!dwrite_factory_) {
            HRESULT hr = ::DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(dwrite_factory_.Receive()));
            if (FAILED(hr)) {
                DCHECK(false);
                return;
            }
        }

        if (!text_format_) {
            HRESULT hr = dwrite_factory_->CreateTextFormat(
                L"微软雅黑", nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                14, L"zh-CN", text_format_.Receive());
            if (FAILED(hr)) {
                DCHECK(false);
                return;
            }
        }

        init_succeeded_ = true;
    }

    bool init_succeeded_;

    // 创建一次即可重复使用
    base::win::ScopedComPtr<ID2D1Factory> d2d_factory_;
    base::win::ScopedComPtr<IDWriteFactory> dwrite_factory_;
    base::win::ScopedComPtr<IDWriteTextFormat> text_format_;
    base::win::ScopedComPtr<ID2D1DCRenderTarget> render_target_;
    base::win::ScopedComPtr<ID2D1SolidColorBrush> brush_;
    base::win::ScopedComPtr<IDWriteInlineObject> ellipse_;

    // 字体名字符串缓存，UTF8 转 UTF16 花时间比较多
    base::MRUCache<std::string, string16> font_name_cache_;

    // 文本布局缓存，保存最近使用的 100 个 TextLayout，
    // 只有字符串长度在 kTextCacheLengthThreshold 才缓存
    base::MRUCache<string16, base::win::ScopedComPtr<IDWriteTextLayout>> text_layout_cache_;

    friend struct DefaultSingletonTraits<CanvasD2DManager>;
    DISALLOW_COPY_AND_ASSIGN(CanvasD2DManager);
};

class ScopedCanvasPlatformPaint
{
public:
    explicit ScopedCanvasPlatformPaint(gfx::Canvas* canvas) : canvas_(canvas)
    {
        if (canvas_)
        {
            raw_dc_ = canvas_->BeginPlatformPaint();
        }
    }
    ~ScopedCanvasPlatformPaint()
    {
        if (canvas_)
            canvas_->EndPlatformPaint();
    }

    HDC GetPlatformPaintContext()
    {
        return raw_dc_;
    }

private:
    gfx::Canvas* canvas_;
    HDC raw_dc_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(ScopedCanvasPlatformPaint);
};

}

// static
bool Canvas::using_dwrite_ = true;

void Canvas::SetDWriteEnabled(bool enabled) {
    using_dwrite_ = enabled;
}

bool Canvas::IsDWriteEnabled() {
    return using_dwrite_;
}

bool Canvas::SizeStringIntD2D(
    const base::string16& text,
    const Font& font,
    int* width, int* height,
    int line_height,  // 忽略掉这个参数，以后有需求再搞
    int flags)
{
    if (!using_dwrite_) {
        return false;
    }

    if (!CanvasD2DManager::GetInstance()->IsSucceeded()) {
        return false;
    }

    auto text_layout = CanvasD2DManager::GetInstance()->
        CreateTextLayout(text, *width, *height, font, flags, true);
    if (!text_layout) {
        return false;
    }

    DWRITE_TEXT_METRICS metrics;
    HRESULT hr = text_layout->GetMetrics(&metrics);
    if (FAILED(hr)) {
        return false;
    }

    *width = std::ceil(metrics.width);
    *height = std::ceil(metrics.height);
    return true;
}

bool Canvas::DrawStringWithShadowsD2D(
    const base::string16& text,
    const Font& font,
    SkColor color,
    const Rect& text_bounds,
    int line_height,  // 忽略掉这个参数，以后有需求再搞
    int flags,
    const ShadowValues& shadows)  // 不渲染阴影
{
    if (!using_dwrite_) {
        return false;
    }

    if (!CanvasD2DManager::GetInstance()->IsSucceeded()) {
        return false;
    }

    auto text_layout = CanvasD2DManager::GetInstance()->CreateTextLayout(
        text, text_bounds.width(), text_bounds.height(), font, flags, false);
    if (!text_layout) {
        return false;
    }

    auto render_target = CanvasD2DManager::GetInstance()->CreateDCRenderTarget();
    if (!render_target) {
        return false;
    }

    auto brush = CanvasD2DManager::GetInstance()->CreateBrush(color);
    if (!brush) {
        return false;
    }

    // 要自动释放引用
    ScopedCanvasPlatformPaint scoped_paint(this);
    HDC hDC = scoped_paint.GetPlatformPaintContext();

    float offset_y = 0;
    if (flags & MULTI_LINE) {
        // 多行情况不需特别处理
    } else {
        DWRITE_TEXT_METRICS metrics;
        HRESULT hr = text_layout->GetMetrics(&metrics);
        if (FAILED(hr)) {
            return false;
        }
        offset_y = (text_bounds.height() - metrics.height) / 2.f;
    }

    RECT rect = text_bounds.ToRECT();
    HRESULT hr = render_target->BindDC(hDC, &rect);
    if (FAILED(hr)) {
        return false;
    }

    render_target->BeginDraw();
    render_target->DrawTextLayout(D2D1::Point2F(0, offset_y), text_layout, brush);
    hr = render_target->EndDraw();
    if (FAILED(hr)) {
        return false;
    }

    //EndPlatformPaint();
    return true;
}

}
