#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_LAYOUT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_LAYOUT_H_

#include <vector>

#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/custom_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/flags_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/image_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/medal_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/gift_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_renderer.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"

class Paint;

class TextLayout {
public:
    TextLayout();
    ~TextLayout();

    void Init(const base::string16& text, int max_width);
    void InvalidInlineObjects();

    void DrawWithOutline(
        float x, float y, const base::win::ScopedComPtr<ID2D1RenderTarget>& rt,
        const D2D1_COLOR_F& text_color, const D2D1_COLOR_F& outline_color, float outline_width);

    void SetBold(bool bold);
    void SetBold(bool bold, int start, int size);
    void SetWrap(bool wrap);
    void SetTrim(bool trim);
    void SetFontSize(int size);
    void SetFontSize(int size, int start);
    void SetFontSize(int size, int start, int length);
    void SetMaxWidth(int max_width);
    void SetLineSpacing(bool uniform, float spacing);
    void SetFontCollection(const base::win::ScopedComPtr<IDWriteFontCollection>& c);
    bool SetGiftIncreaseInterval(int64_t interval);

    base::string16 GetText();

    void SetOpacity(float opacity);
    void SetTextColor(const Paint& p, int start, int length);
    void SetFlagsColor(const FlagsInlineObject::FlagColorSetter& f);

    void SetInlineObject(
        CustomInlineObject* obj,
        int start, int length, float multiple);

    void SetInlineObject(
        const base::win::ScopedComPtr<IWICBitmapSource>& source,
        int start, int length, float multiple,
        bool is_cache = false);
    void SetInlineObject(
        const WICBitmaps& source,
        int start, int length, float multiple);

    void SetInlineObject(
        const FlagsInlineObject::Flags& flags,
        int start, int length);
    void SetInlineObject(
        const string16& medal_name,
        int64_t uid,
        int medal_level,
        int guard_level,
        bool has_border, const D2D1_COLOR_F& border_color,
        const D2D1_COLOR_F& start_color, const D2D1_COLOR_F& end_color,
        int start, int length);

    int GetMaxWidth() const;
    int GetMaxHeight() const;
    int GetTextWidth() const;
    int GetTextHeight() const;
    float GetTextWidthFloat() const;
    float GetTextHeightFloat() const;
    DWRITE_TEXT_METRICS GetMetrics();
    DWRITE_OVERHANG_METRICS GetOverHangMetrics();
    void GetLineHeight(float* height, float* baseline) const;

    int GetLineCount();
    std::vector<DWRITE_LINE_METRICS> GetLineMetrics();

    void UpdateInlineObjectMetrics();
    bool UpdateRenderTarget(
        const base::win::ScopedComPtr<ID2D1RenderTarget>& rt, bool hw_acc);
    void ClearInlineObject(bool prune = false);
    void ClearImgInlineObject(int start_pos,int length, bool prune = false);
    void ClearDrawingEffect();

    bool HitTest(int x, int y, uint32_t* pos);

    IDWriteTextLayout* Get() const;
    TextRenderer* GetRenderer() const;

    static string16 ElideText(
        const string16& text, int desired_kanji_length, int font_size, bool has_ellipsis);

private:
    struct InlineObjectInfo {
        float multiple;
        DWRITE_TEXT_RANGE range;
    };

    using CustomInlineObj = std::pair<base::win::ScopedComPtr<CustomInlineObject>, InlineObjectInfo>;
    using ImageInlineObj = std::pair<base::win::ScopedComPtr<ImageInlineObject>, InlineObjectInfo>;
    using FlagsInlineObj = std::pair<base::win::ScopedComPtr<FlagsInlineObject>, InlineObjectInfo>;
    using MedalInlineObj = std::pair<base::win::ScopedComPtr<MedalInlineObject>, InlineObjectInfo>;
    using GiftInlineObj = std::pair<base::win::ScopedComPtr<GiftInlineObject>, InlineObjectInfo>;

    base::string16              text_;

    int text_length_;
    base::win::ScopedComPtr<IDWriteTextLayout> text_layout_;
    base::win::ScopedComPtr<TextRenderer> text_renderer_;

    std::vector<CustomInlineObj> custom_inline_objs_;
    std::vector<ImageInlineObj> img_inline_objs_;
    std::vector<FlagsInlineObj> flags_inline_objs_;
    std::vector<MedalInlineObj> medal_inline_objs_;
    std::vector<GiftInlineObj> gift_inline_objs_;

    DISALLOW_COPY_AND_ASSIGN(TextLayout);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_LAYOUT_H_