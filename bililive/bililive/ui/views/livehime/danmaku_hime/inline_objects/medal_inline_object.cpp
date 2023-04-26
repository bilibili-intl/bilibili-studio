#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/medal_inline_object.h"

#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmakuhime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_layout.h"


MedalInlineObject::MedalInlineObject(
    const string16& name, int64_t uid, int level, int guard_level,
    bool has_border, const D2D1_COLOR_F& border_color,
    const D2D1_COLOR_F& start_color, const D2D1_COLOR_F& end_color)
    :
      uid_(uid),
      ref_count_(1),
      baseline_(0),
      line_height_(0),
      desired_height_(0),
      opacity_(1.f),
      level_(level),
      guard_level_(guard_level),
      has_border_(has_border), border_color_(border_color),
      start_color_(start_color), end_color_(end_color),
      level_color_(D2D1::ColorF(D2D1::ColorF::White))
{
    auto name_layout = new TextLayout();
    name_layout->Init(name, 0);
    name_layout->SetWrap(false);

    auto level_layout = new TextLayout();
    level_layout->Init(std::to_wstring(level_), 0);
    level_layout->SetWrap(false);

    user_medal_.first.reset(name_layout);
    user_medal_.second.reset(level_layout);

    //using_honor_ = ImageFetcher::GetInstance()->UsingHonorCrown() && ImageFetcher::GetInstance()->IsHonorAnchor(uid_);

    //if (ImageFetcher::GetInstance()->JuardgeThousandGuard(uid) && uid != 0) {
    //    raw_icon_buffer_ = ImageFetcher::GetInstance()->FetchMedalImage(ImageFetcher::MedalType::GUARD_THOUSAND, guard_level_);
    //}
    //else {
    //    raw_icon_buffer_ = ImageFetcher::GetInstance()->FetchMedalImage(ImageFetcher::MedalType::GUARD, guard_level_);
    //}

    if (!raw_icon_buffer_.empty())
    {
        source_ = D2DManager::GetInstance()->DecodeImageFromData(raw_icon_buffer_);
    }

    if (using_honor_)
    {
        //if (guard_level_ > 0)
        //{
        //    crown_raw_icon_buffer_ = ImageFetcher::GetInstance()->FetchMedalImage(ImageFetcher::MedalType::CROWN_GUARD, guard_level_);
        //}
        //else
        //{
        //    if (level_ > 0)
        //    {
        //        crown_raw_icon_buffer_ = ImageFetcher::GetInstance()->FetchMedalImage(ImageFetcher::MedalType::CROWN_NORMAL, level_);
        //    }
        //    else
        //    {
        //        crown_raw_icon_buffer_ = ImageFetcher::GetInstance()->FetchMedalImage(ImageFetcher::MedalType::CROWN_UNLIT, 0);
        //    }
        //}

        DCHECK(!crown_raw_icon_buffer_.empty());

        if (!crown_raw_icon_buffer_.empty())
        {
            crowwn_source_ = D2DManager::GetInstance()->DecodeImageFromData(crown_raw_icon_buffer_);
        }
    }
}

void MedalInlineObject::SetOpacity(float opacity) {
    opacity_ = opacity;
}

void MedalInlineObject::SetFontSize(float size) {
    user_medal_.first->SetFontSize(size);
    user_medal_.second->SetFontSize(size);
}

void MedalInlineObject::SetMetrics(float desired_height, float line_height, float baseline) {
    baseline_ = baseline;
    line_height_ = line_height;
    desired_height_ = desired_height;
}

bool MedalInlineObject::SetRenderTarget(
    base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc)
{
    DCHECK(rt != nullptr);
    rt_ = rt;

    if (hw_acc != hw_acc_rendered_)
    {
        bitmap_.Release();
        crown_bitmap_.Release();
        hw_acc_rendered_ = hw_acc;
    }

    if (source_)
    {
        float dpi_x, dpi_y;
        rt->GetDpi(&dpi_x, &dpi_y);

        bitmap_.Release();

        D2D1_BITMAP_PROPERTIES properties
            = D2D1::BitmapProperties(
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                dpi_x, dpi_y);

        HRESULT hr = rt->CreateBitmapFromWicBitmap(source_.get(), properties, bitmap_.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create bitmap from wic bitmap: " << hr;
        }
    }

    if (crowwn_source_)
    {
        float dpi_x{}, dpi_y{};
        rt->GetDpi(&dpi_x, &dpi_y);
        crown_bitmap_.Release();
        D2D1_BITMAP_PROPERTIES properties
            = D2D1::BitmapProperties(
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                dpi_x, dpi_y);

        HRESULT hr = rt->CreateBitmapFromWicBitmap(crowwn_source_.get(), properties, crown_bitmap_.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create bitmap from wic bitmap: " << hr;
        }
    }

    return true;
}

void MedalInlineObject::ReleaseBitmaps() {
    bitmap_.Release();
    crown_bitmap_.Release();
}

void MedalInlineObject::DrawUserMedal(float left, float top, float height) {
    if (!rt_) {
        return;
    }

    Sculptor sculptor(rt_);
    sculptor.GetPaint()->SetOpacity(opacity_);

    int guard_offset = 0;
    if (bitmap_) {
        guard_offset = desired_height_ * 1.375f / 2;
    }

    int name_width = user_medal_.first->GetTextWidth();
    int name_height = user_medal_.first->GetTextHeight();
    int level_width = user_medal_.second->GetTextWidth();
    int level_height = user_medal_.second->GetTextHeight();

    int name_rect_width = name_width + dmkhime::kUserMedalPaddingHori * 2 + guard_offset;
    int level_rect_width = level_width + dmkhime::kUserMedalPaddingHori * 2;

    int name_text_offx = left + dmkhime::kUserMedalPaddingHori + guard_offset * 2;
    float name_text_offy = top + (height - name_height) / 2.f;

    int level_text_offx = left + name_rect_width + dmkhime::kUserMedalPaddingHori + guard_offset;
    float level_text_offy = top + (height - level_height) / 2.f;

    D2D1_RECT_F name_rect{};
    D2D1_RECT_F level_rect{};
    D2D1_RECT_F total_rect{};

    float crown_size{};
    float crown_x{};
    float crown_y{};

    if (using_honor_)
    {
        crown_size = height * 1.5f;
        name_rect_width += crown_size / 2.0f;
        level_rect_width = crown_size;

        name_rect = D2D1::RectF(
            left + guard_offset, top,
            left + name_rect_width + guard_offset, top + height);

        level_rect = D2D1::RectF(
            name_rect.right - level_rect_width / 2.0f, top,
            name_rect.right + level_rect_width / 2.0f, top + height);

        total_rect = D2D1::RectF(
            left + guard_offset, top,
            name_rect.right, top + height);

        crown_x = level_rect.left + (level_rect.right - level_rect.left - crown_size) / 2.0f;
        crown_y = top + height - crown_size + crown_size * 0.13636f;

        level_text_offx = crown_x + (level_rect_width - level_width) / 2.0f;
        level_text_offy += crown_size * 0.0909f;
    }
    else
    {
        name_rect = D2D1::RectF(
            left + guard_offset, top,
            left + name_rect_width + guard_offset, top + height);

        level_rect = D2D1::RectF(
            name_rect.right, top,
            name_rect.right + level_rect_width, top + height);

        total_rect = D2D1::RectF(
            left + guard_offset, top,
            name_rect.right + level_rect_width, top + height);

    }

    // 勋章名底色（可能渐变）
    sculptor.GetPaint()->SetStyle(Paint::FILL);
    {
        Paint::Stops stops;
        stops.push_back({ start_color_, 0 });
        stops.push_back({ end_color_, 1 });

        sculptor.GetPaint()->SetGradientEnabled(true);
        sculptor.GetPaint()->SetGradient(
            stops, D2D1::Point2F(name_rect.left, 0), D2D1::Point2F(name_rect.right, 0));
        sculptor.DrawRoundRect(name_rect, 2, 0, 0, 2);
        sculptor.GetPaint()->SetGradientEnabled(false);
    }

    if(!using_honor_)
    {
        // 等级底色
        sculptor.GetPaint()->SetColor(level_color_);
        sculptor.DrawRoundRect(level_rect, 0, 2, 2, 0);
    }

    // 边框
    if (has_border_) {
        sculptor.GetPaint()->SetStyle(Paint::STROKE);
        sculptor.GetPaint()->SetStrokeWidth(1);
        sculptor.GetPaint()->SetColor(border_color_);
        sculptor.DrawRoundRect(total_rect, 2);
    }

    if (using_honor_)
    {
        if (crown_bitmap_)
        {
            sculptor.DrawBitmap(crown_bitmap_, crown_x, crown_y, crown_size, crown_size);
        }
    }

    sculptor.GetPaint()->SetColor(level_color_);
    sculptor.DrawTextLayout(*user_medal_.first, name_text_offx, name_text_offy);

    sculptor.GetPaint()->SetColor(start_color_);
    sculptor.DrawTextLayout(*user_medal_.second, level_text_offx, level_text_offy);

    // 大航海图标
    if (bitmap_) {
        float guard_size = height * 1.375f;
        float y = top - (guard_size - height) / 2.f - 1;
        sculptor.DrawBitmap(bitmap_, left, y, guard_size, guard_size);
    }
}

STDMETHODIMP MedalInlineObject::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect)
{
    DrawUserMedal(originX, originY, desired_height_);
    return S_OK;
}

STDMETHODIMP MedalInlineObject::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics)
{
    if (metrics == nullptr) {
        return E_INVALIDARG;
    }

    int name_width = user_medal_.first->GetTextWidth();
    int level_width = user_medal_.second->GetTextWidth();
    int name_rect_width = name_width + dmkhime::kUserMedalPaddingHori * 2;
    int level_rect_width = level_width + dmkhime::kUserMedalPaddingHori * 2;

    int width{};
    if(using_honor_)
    {
        width = name_rect_width + level_rect_width + dmkhime::kUserFlagMarginHori + desired_height_ * 0.25f;
    }
    else
    {
        width = name_rect_width + level_rect_width + dmkhime::kUserFlagMarginHori;
    }

    metrics->width = width;
    metrics->height = desired_height_;
    metrics->baseline = baseline_ + (desired_height_ - line_height_) / 2;
    metrics->supportsSideways = FALSE;

    if (source_) {
        metrics->width += desired_height_ * 1.375f;
    }

    return S_OK;
}

STDMETHODIMP MedalInlineObject::GetOverhangMetrics(
    DWRITE_OVERHANG_METRICS* overhangs) {

    if (overhangs == nullptr) {
        return E_INVALIDARG;
    }

    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;

    return S_OK;
}

STDMETHODIMP MedalInlineObject::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore,
    DWRITE_BREAK_CONDITION* breakConditionAfter) {

    if (breakConditionBefore == nullptr
        || breakConditionAfter == nullptr) {
        return E_INVALIDARG;
    }

    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;

    return S_OK;
}


STDMETHODIMP_(ULONG) MedalInlineObject::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(ULONG) MedalInlineObject::Release() {
    auto new_count = InterlockedDecrement(&ref_count_);
    if (new_count == 0) {
        delete this;
    }

    return new_count;
}

STDMETHODIMP MedalInlineObject::QueryInterface(
    IID const& riid, void** ppvObject) {

    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(MedalInlineObject) == riid) {
        *ppvObject = this;
    } else if (__uuidof(IDWriteInlineObject) == riid) {
        *ppvObject = this;
    } else if (__uuidof(IUnknown) == riid) {
        *ppvObject = this;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}