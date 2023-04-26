#include "text_layout.h"

#include "base/logging.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"


TextLayout::TextLayout()
    :text_length_(0) {
}

TextLayout::~TextLayout() {
}


void TextLayout::Init(const base::string16& text, int max_width) {
    DCHECK(max_width >= 0);

    text_length_ = text.length();
    custom_inline_objs_.clear();
    img_inline_objs_.clear();
    flags_inline_objs_.clear();
    medal_inline_objs_.clear();
    gift_inline_objs_.clear();
    text_layout_.Release();

    text_layout_ = D2DManager::GetInstance()->CreateTextLayout(text, max_width, 0);

    text_ = text;
}

void TextLayout::InvalidInlineObjects() {
    for (auto& pair : custom_inline_objs_) {
        pair.first->ReleaseBitmaps();
    }
    for (auto& pair : img_inline_objs_) {
        pair.first->ReleaseBitmaps();
    }
    for (auto& pair : flags_inline_objs_) {
        pair.first->ReleaseBitmaps();
    }
    for (auto& pair : medal_inline_objs_) {
        pair.first->ReleaseBitmaps();
    }
    for (auto& pair : gift_inline_objs_) {
        pair.first->ReleaseBitmaps();
    }
}

void TextLayout::DrawWithOutline(
    float x, float y,
    const base::win::ScopedComPtr<ID2D1RenderTarget>& rt,
    const D2D1_COLOR_F& text_color, const D2D1_COLOR_F& outline_color, float outline_width)
{
    if (text_layout_ && rt) {
        if (!text_renderer_) {
            text_renderer_.Attach(new TextRenderer());
        }

        text_renderer_->SetRenderTarget(rt);
        text_renderer_->setTextColor(text_color);
        text_renderer_->setOutlineColor(outline_color);
        text_renderer_->setOutlineWidth(outline_width);

        HRESULT hr = text_layout_->Draw(nullptr, text_renderer_, x, y);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetBold(bool bold) {
    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { 0, text_length_ };
        HRESULT hr = text_layout_->SetFontWeight(
            bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR, range);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetBold(bool bold, int start, int size)
{
    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { start, size };
        HRESULT hr = text_layout_->SetFontWeight(
            bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR, range);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetWrap(bool wrap) {
    if (text_layout_) {
        HRESULT hr = text_layout_->SetWordWrapping(
            wrap ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetTrim(bool trim) {
    if (!text_layout_) {
        return;
    }

    if (trim) {
        auto sign = D2DManager::GetInstance()->CreateEllipsisTrimmingSign();

        DWRITE_TRIMMING dt;
        dt.delimiter = 0;
        dt.delimiterCount = 0;
        dt.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
        HRESULT hr = text_layout_->SetTrimming(&dt, sign.get());
        DCHECK(SUCCEEDED(hr));
    } else {
        DWRITE_TRIMMING dt;
        dt.delimiter = 0;
        dt.delimiterCount = 0;
        dt.granularity = DWRITE_TRIMMING_GRANULARITY_NONE;
        HRESULT hr = text_layout_->SetTrimming(&dt, nullptr);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetFontSize(int size) {
    DCHECK(size > 0);

    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { 0, text_length_ };
        HRESULT hr = text_layout_->SetFontSize(size, range);
        DCHECK(SUCCEEDED(hr));

        for (auto& pair : custom_inline_objs_) {
            pair.first->SetFontSize(size);
        }

        for (auto& pair : flags_inline_objs_) {
            pair.first->SetFontSize(size - 2);
        }

        for (auto& pair : medal_inline_objs_) {
            pair.first->SetFontSize(size - 2);
        }
    }
}

void TextLayout::SetFontSize(int size, int start) {
    DCHECK(size > 0 && start >= 0);

    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { start, text_length_ };
        HRESULT hr = text_layout_->SetFontSize(size, range);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetFontSize(int size, int start, int length) {
    DCHECK(size > 0 && start >= 0 && length > 0);

    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { start, length };
        HRESULT hr = text_layout_->SetFontSize(size, range);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetMaxWidth(int max_width) {
    DCHECK(max_width >= 0);

    if (text_layout_) {
        HRESULT hr = text_layout_->SetMaxWidth(max_width);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetLineSpacing(bool uniform, float spacing) {
    if (text_layout_) {
        HRESULT hr = text_layout_->SetLineSpacing(
            uniform ? DWRITE_LINE_SPACING_METHOD_UNIFORM : DWRITE_LINE_SPACING_METHOD_DEFAULT,
            spacing, spacing * 0.8f);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetFontCollection(const base::win::ScopedComPtr<IDWriteFontCollection>& c) {
    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { 0, text_length_ };
        HRESULT hr = text_layout_->SetFontCollection(c, range);
        DCHECK(SUCCEEDED(hr));
    }
}

bool TextLayout::SetGiftIncreaseInterval(int64_t interval)
{
    for (auto& pair : gift_inline_objs_) {
        return pair.first->IncreaseInterval(interval);
    }
    return false;
}

base::string16 TextLayout::GetText()
{
    return text_;
}

void TextLayout::SetOpacity(float opacity) {
    for (auto& pair : custom_inline_objs_) {
        pair.first->SetOpacity(opacity);
    }

    for (auto& pair : img_inline_objs_) {
        pair.first->SetOpacity(opacity);
    }

    for (auto& pair : flags_inline_objs_) {
        pair.first->SetOpacity(opacity);
    }

    for (auto& pair : medal_inline_objs_) {
        pair.first->SetOpacity(opacity);
    }
    for (auto& pair : gift_inline_objs_) {
        pair.first->SetOpacity(opacity);
    }
}

void TextLayout::SetTextColor(const Paint& p, int start, int length) {
    DCHECK(start >= 0 && length > 0);

    if (text_layout_ && p.GetBrush()) {
        DWRITE_TEXT_RANGE range = { start, length };
        HRESULT hr = text_layout_->SetDrawingEffect(p.GetBrush(), range);
        DCHECK(SUCCEEDED(hr));
    }
}

void TextLayout::SetFlagsColor(const FlagsInlineObject::FlagColorSetter& f) {
    for (auto& pair : flags_inline_objs_) {
        pair.first->SetFlagsColor(f);
    }
}

void TextLayout::SetInlineObject(
    CustomInlineObject* obj,
    int start, int length, float multiple)
{
    DCHECK(start >= 0 && length > 0 && obj);
    if (!text_layout_ || !obj) {
        return;
    }

    base::win::ScopedComPtr<CustomInlineObject> inline_obj;
    inline_obj.Attach(obj);

    float baseline = 0.f;
    float line_height = 0.f;
    GetLineHeight(&line_height, &baseline);

    inline_obj->SetMetrics(line_height * multiple, line_height, baseline);

    DWRITE_TEXT_RANGE range = { start, length };
    HRESULT hr = text_layout_->SetInlineObject(inline_obj.get(), range);
    if (FAILED(hr)) {
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << start << ", length: " << length;
    }

    InlineObjectInfo info;
    info.range = range;
    info.multiple = multiple;

    custom_inline_objs_.push_back({ inline_obj, info });
}

void TextLayout::SetInlineObject(
    const base::win::ScopedComPtr<IWICBitmapSource>& source,
    int start, int length, float multiple, bool is_cache) {

    DCHECK(start >= 0 && length > 0 && source);
    if (!text_layout_ || !source) {
        return ;
    }

    base::win::ScopedComPtr<ImageInlineObject> inline_obj;
    inline_obj.Attach(new ImageInlineObject(source,is_cache));

    float baseline = 0.f;
    float line_height = 0.f;
    GetLineHeight(&line_height, &baseline);

    inline_obj->SetMetrics(line_height * multiple, line_height, baseline);
    DWRITE_TEXT_RANGE range = { start, length };
    HRESULT hr = text_layout_->SetInlineObject(inline_obj.get(), range);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << start << ", length: " << length;
    }

    InlineObjectInfo info;
    info.range = range;
    info.multiple = multiple;

    img_inline_objs_.push_back({ inline_obj, info });
}
void TextLayout::SetInlineObject(
    const WICBitmaps& source,
    int start, int length, float multiple) {

    DCHECK(start >= 0 && length > 0);
    if (!text_layout_ ||source.empty()) {
        return;
    }

    base::win::ScopedComPtr<GiftInlineObject> inline_obj;
    inline_obj.Attach(new GiftInlineObject(source));

    float baseline = 0.f;
    float line_height = 0.f;
    GetLineHeight(&line_height, &baseline);

    inline_obj->SetMetrics(line_height * multiple, line_height, baseline);
    DWRITE_TEXT_RANGE range = { start, length };
    HRESULT hr = text_layout_->SetInlineObject(inline_obj.get(), range);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << start << ", length: " << length;
    }

    InlineObjectInfo info;
    info.range = range;
    info.multiple = multiple;

    gift_inline_objs_.push_back({ inline_obj, info });
}
void TextLayout::SetInlineObject(
    const FlagsInlineObject::Flags& flags, int start, int length) {

    DCHECK(start >= 0 && length > 0 && !flags.empty());
    if (!text_layout_ || flags.empty()) {
        return ;
    }

    base::win::ScopedComPtr<FlagsInlineObject> inline_obj;
    inline_obj.Attach(new FlagsInlineObject(flags));

    float baseline = 0.f;
    float line_height = 0.f;
    GetLineHeight(&line_height, &baseline);

    inline_obj->SetMetrics(line_height, line_height, baseline);
    DWRITE_TEXT_RANGE range = { start, length };
    HRESULT hr = text_layout_->SetInlineObject(inline_obj.get(), range);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << start << ", length: " << length;
    }

    InlineObjectInfo info;
    info.range = range;
    info.multiple = 1;

    flags_inline_objs_.push_back({ inline_obj, info });
}

void TextLayout::SetInlineObject(
    const string16& medal_name,
    int64_t uid,
    int medal_level,
    int guard_level,
    bool has_border, const D2D1_COLOR_F& border_color,
    const D2D1_COLOR_F& start_color, const D2D1_COLOR_F& end_color,
    int start, int length)
{
    DCHECK(start >= 0 && length > 0);
    if (!text_layout_) {
        return ;
    }

    base::win::ScopedComPtr<MedalInlineObject> inline_obj;
    inline_obj.Attach(new MedalInlineObject(
        medal_name, uid, medal_level, guard_level,
        has_border, border_color, start_color, end_color));

    float baseline = 0.f;
    float line_height = 0.f;
    GetLineHeight(&line_height, &baseline);

    inline_obj->SetMetrics(line_height, line_height, baseline);
    DWRITE_TEXT_RANGE range = { start, length };
    HRESULT hr = text_layout_->SetInlineObject(inline_obj.get(), range);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << start << ", length: " << length;
    }

    InlineObjectInfo info;
    info.range = range;
    info.multiple = 1;

    medal_inline_objs_.push_back({ inline_obj, info });
}

int TextLayout::GetMaxWidth() const {
    if (text_layout_) {
        return static_cast<int>(text_layout_->GetMaxWidth());
    } else {
        return 0;
    }
}

int TextLayout::GetMaxHeight() const {
    if (text_layout_) {
        return static_cast<int>(text_layout_->GetMaxHeight());
    } else {
        return 0;
    }
}

int TextLayout::GetTextWidth() const {
    return static_cast<int>(std::ceil(GetTextWidthFloat()));
}

int TextLayout::GetTextHeight() const {
    return static_cast<int>(std::ceil(GetTextHeightFloat()));
}

float TextLayout::GetTextWidthFloat() const {
    if (!text_layout_) {
        return 0;
    }

    DWRITE_TEXT_METRICS metrics;
    HRESULT hr = text_layout_->GetMetrics(&metrics);
    if (SUCCEEDED(hr)) {
        return metrics.widthIncludingTrailingWhitespace;
    }

    DCHECK(false);
    return 0;
}

float TextLayout::GetTextHeightFloat() const {
    if (!text_layout_) {
        return 0;
    }

    DWRITE_TEXT_METRICS metrics;
    HRESULT hr = text_layout_->GetMetrics(&metrics);
    if (SUCCEEDED(hr)) {
        return metrics.height;
    }

    DCHECK(false);
    return 0;
}

DWRITE_TEXT_METRICS TextLayout::GetMetrics()
{
    DWRITE_TEXT_METRICS metrics;
    HRESULT hr = text_layout_->GetMetrics(&metrics);
    if (SUCCEEDED(hr)) {
        return metrics;
    }

    DCHECK(false);
    return {};
}

DWRITE_OVERHANG_METRICS TextLayout::GetOverHangMetrics()
{
    return DWRITE_OVERHANG_METRICS();

    DWRITE_OVERHANG_METRICS metrics;
    HRESULT hr = text_layout_->GetOverhangMetrics(&metrics);
    if (SUCCEEDED(hr)) {
        return metrics;
    }

    DCHECK(false);
    return {};
}

void TextLayout::GetLineHeight(float* height, float* baseline) const {
    DCHECK(height && baseline);

    *height = 0.f;
    *baseline = 0.f;
    if (!text_layout_) {
        return;
    }

    UINT32 line_count = 0;
    DWRITE_LINE_METRICS stacked_metrics[4];
    HRESULT hr = text_layout_->GetLineMetrics(stacked_metrics, 4, &line_count);
    if (hr == S_OK) {
        if (line_count == 0) {
            return;
        }
        *height = stacked_metrics[0].height;
        *baseline = stacked_metrics[0].baseline;
    } else if (hr == E_NOT_SUFFICIENT_BUFFER) {
        if (line_count == 0) {
            return;
        }
        DWRITE_LINE_METRICS* metrics = new DWRITE_LINE_METRICS[line_count];
        hr = text_layout_->GetLineMetrics(metrics, line_count, &line_count);
        if (SUCCEEDED(hr)) {
            *height = metrics[0].height;
            *baseline = metrics[0].baseline;
        } else {
            DCHECK(false);
        }
        delete[] metrics;
    } else {
        DCHECK(false);
    }
}

int TextLayout::GetLineCount()
{
    UINT32 line_count{};
    HRESULT hr = text_layout_->GetLineMetrics(nullptr, 0, &line_count);
    if (hr == S_OK || hr == E_NOT_SUFFICIENT_BUFFER)
    {
        return static_cast<int>(line_count);
    }
    else
    {
        return -1;
    }
}

std::vector<DWRITE_LINE_METRICS> TextLayout::GetLineMetrics()
{
    int line_count = GetLineCount();
    std::vector<DWRITE_LINE_METRICS> ret(line_count, DWRITE_LINE_METRICS());
    UINT32 line_param{};
    HRESULT hr = text_layout_->GetLineMetrics(ret.data(), ret.size(), &line_param);

    if (SUCCEEDED(hr))
    {
        return ret;
    }
    else
    {
        return {};
    }
}

void TextLayout::UpdateInlineObjectMetrics() {
    if (!text_layout_) {
        return;
    }

    ClearInlineObject();

    float baseline = 0.f;
    float line_height = 0.f;
    GetLineHeight(&line_height, &baseline);

    for (auto& pair : custom_inline_objs_) {
        auto info = pair.second;
        pair.first->SetMetrics(line_height * info.multiple, line_height, baseline);

        HRESULT hr = text_layout_->SetInlineObject(pair.first.get(), info.range);
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to SetInlineObject: " << hr
                << ", start: " << info.range.startPosition << ", length: " << info.range.length;
        }
    }

    for (auto& pair : img_inline_objs_) {
        auto info = pair.second;
        pair.first->SetMetrics(line_height * info.multiple, line_height, baseline);

        HRESULT hr = text_layout_->SetInlineObject(pair.first.get(), info.range);
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to SetInlineObject: " << hr
                << ", start: " << info.range.startPosition << ", length: " << info.range.length;
        }
    }

    for (auto& pair : flags_inline_objs_) {
        auto info = pair.second;
        pair.first->SetMetrics(line_height, line_height, baseline);

        HRESULT hr = text_layout_->SetInlineObject(pair.first.get(), info.range);
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to SetInlineObject: " << hr
                << ", start: " << info.range.startPosition << ", length: " << info.range.length;
        }
    }

    for (auto& pair : medal_inline_objs_) {
        auto info = pair.second;
        pair.first->SetMetrics(line_height, line_height, baseline);

        HRESULT hr = text_layout_->SetInlineObject(pair.first.get(), info.range);
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to SetInlineObject: " << hr
                << ", start: " << info.range.startPosition << ", length: " << info.range.length;
        }
    }

    for (auto& pair : gift_inline_objs_) {
        auto info = pair.second;
        pair.first->SetMetrics(line_height * info.multiple, line_height, baseline);

        HRESULT hr = text_layout_->SetInlineObject(pair.first.get(), info.range);
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to SetInlineObject: " << hr
                << ", start: " << info.range.startPosition << ", length: " << info.range.length;
        }
    }
}

bool TextLayout::UpdateRenderTarget(
    const base::win::ScopedComPtr<ID2D1RenderTarget>& rt, bool hw_acc)
{
    if (!text_layout_ || !rt) {
        return false;
    }

    /*if (text_renderer_) {
        text_renderer_->SetRenderTarget(rt);
    }*/

    bool need_measure = false;
    for (auto it = custom_inline_objs_.begin();
        it != custom_inline_objs_.end();) {

        // 因某些原因而导致 ID2D1Bitmap 创建失败时，移除该嵌入式对象，
        // 以便作为 placeholder 的文字能够显示。
        bool result = it->first->SetRenderTarget(rt, hw_acc);
        if (!result) {
            text_layout_->SetInlineObject(nullptr, it->second.range);
            it = custom_inline_objs_.erase(it);
            need_measure = true;
        } else {
            ++it;
        }
    }

    for (auto it = img_inline_objs_.begin();
        it != img_inline_objs_.end();) {

        // 因某些原因而导致 ID2D1Bitmap 创建失败时，移除该嵌入式对象，
        // 以便作为 placeholder 的文字能够显示。
        bool result = it->first->SetRenderTarget(rt, hw_acc);
        if (!result) {
            text_layout_->SetInlineObject(nullptr, it->second.range);
            it = img_inline_objs_.erase(it);
            need_measure = true;
        } else {
            ++it;
        }
    }

    for (auto& pair : flags_inline_objs_) {
        pair.first->SetRenderTarget(rt, hw_acc);
    }

    for (auto& pair : medal_inline_objs_) {
        pair.first->SetRenderTarget(rt, hw_acc);
    }
    for (auto& pair : gift_inline_objs_) {
        pair.first->SetRenderTarget(rt, hw_acc);
    }
    return need_measure;
}

void TextLayout::ClearInlineObject(bool prune) {
    if (!text_layout_) {
        return;
    }

    DWRITE_TEXT_RANGE range = { 0, text_length_ };
    HRESULT hr = text_layout_->SetInlineObject(nullptr, range);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << range.startPosition << ", length: " << range.length;
    }

    if (prune) {
        custom_inline_objs_.clear();
        img_inline_objs_.clear();
        gift_inline_objs_.clear();
    }
}

void TextLayout::ClearImgInlineObject(int start_pos, int length, bool prune) {
    if (!text_layout_) {
        return;
    }

    DWRITE_TEXT_RANGE range = { start_pos, length };
    HRESULT hr = text_layout_->SetInlineObject(nullptr, range);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to SetInlineObject: " << hr
            << ", start: " << range.startPosition << ", length: " << range.length;
    }
}

void TextLayout::ClearDrawingEffect() {
    if (text_layout_) {
        DWRITE_TEXT_RANGE range = { 0, text_length_ };
        HRESULT hr = text_layout_->SetDrawingEffect(nullptr, range);
        DCHECK(SUCCEEDED(hr));
    }
}

bool TextLayout::HitTest(int x, int y, uint32_t* pos) {
    if (!text_layout_) {
        return false;
    }

    BOOL is_trailing, is_inside;
    DWRITE_HIT_TEST_METRICS metrics;
    HRESULT hr = text_layout_->HitTestPoint(x, y, &is_trailing, &is_inside, &metrics);
    if (FAILED(hr)) {
        return false;
    }

    if (is_inside == FALSE) {
        return false;
    }

    *pos = metrics.textPosition;
    return true;
}

IDWriteTextLayout* TextLayout::Get() const {
    return text_layout_.get();
}

TextRenderer* TextLayout::GetRenderer() const {
    return text_renderer_.get();
}

// static
string16 TextLayout::ElideText(
    const string16& text, int desired_kanji_length, int font_size, bool has_ellipsis)
{
    if (text.empty()) return text;
    if (desired_kanji_length <= 0 || font_size <= 0) return string16();

    // 组合一下占位符
    string16 placeholder_text;
    for (int i = 0; i < desired_kanji_length; ++i) {
        placeholder_text.append(L"空");
    }
    string16 ellipsis_text = L"...";

    // 获取占位符宽度
    TextLayout layout;
    layout.Init(placeholder_text, 0);
    layout.SetWrap(false);
    layout.SetFontSize(font_size);
    int available_width = layout.GetTextWidth();

    // 设置我们想截取的字符串的文本布局
    layout.Init(text, available_width);
    layout.SetWrap(false);
    layout.SetFontSize(font_size);
    if (layout.GetTextWidth() <= available_width) {
        return text;
    }

    int ellipsis_width = 0;
    if (has_ellipsis) {
        // 获取省略符宽度
        TextLayout ellipsis_layout;
        ellipsis_layout.Init(ellipsis_text, 0);
        ellipsis_layout.SetWrap(false);
        ellipsis_layout.SetFontSize(font_size);
        ellipsis_width = ellipsis_layout.GetTextWidth();
    }

    // 使用 Dwrite 的 HitTestPoint 功能来确定超出文字布局框位置处的文本位置
    BOOL is_trail;
    BOOL is_inside;
    DWRITE_HIT_TEST_METRICS metrics;
    HRESULT hr = layout.text_layout_->HitTestPoint(
        std::max(available_width - ellipsis_width, 0), 0, &is_trail, &is_inside, &metrics);
    if (FAILED(hr)) {
        DCHECK(false);
        return text;
    }

    auto trimmed_text = text.substr(0, metrics.textPosition);
    return has_ellipsis ? trimmed_text + ellipsis_text : trimmed_text;
}