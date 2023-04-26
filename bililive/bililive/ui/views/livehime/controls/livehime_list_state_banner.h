#ifndef LIVEHIME_LIST_STATE_BANNER_H
#define LIVEHIME_LIST_STATE_BANNER_H

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_constants.h"
#include "bililive/secret/public/danmaku_hime_service.h"

#include "base/memory/weak_ptr.h"


namespace livehime
{
    enum class ListState
    {
        ListState_Loading,
        ListState_Faild,
        ListState_Empty,
        ListState_Success
    };

    void DrawListStateImage(gfx::Canvas *canvas, const gfx::Rect &bounds,
        ListState state, const base::string16 &text, SkColor clr = clrLabelDescribe, int offset_x = 0);
}


class LivehimeListStateBannerView : public views::View
{
    struct StateDetails
    {
        gfx::ImageSkia img;
        base::string16 text;
        SkColor clr = clrLabelDescribe;
    };

public:
    LivehimeListStateBannerView(int offset_x = 0);
    virtual ~LivehimeListStateBannerView();

    void SetState(livehime::ListState state, bool repaint = true);
    void SetStateText(livehime::ListState state, const base::string16 &text);
    void SetStateTextColor(livehime::ListState state, SkColor clr);
    void SetTextColor(SkColor clr);
    void SetAutoColorReadabilityEnabled(bool enabled, SkColor background_color);
    void SetStateImage(livehime::ListState state, const gfx::ImageSkia* image);
    void SetNoSubpixelRender(bool no_subpixel);
	void SetNoDataEmptyImage(bool no_image) {no_data_empty_image_ = no_image;}
    void SetTextFont(const gfx::Font& text_font) { text_font_ = text_font; }
    // View
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;

protected:
    // View
    void OnPaintBackground(gfx::Canvas* canvas) override;
    bool HitTestRect(const gfx::Rect& rect) const override;

private:
    int offset_x_;
    std::map<livehime::ListState, StateDetails> state_details_;
    livehime::ListState state_;
    bool auto_color_readability_ = false;
    bool no_subpixel_render_ = false;
    bool no_data_empty_image_ = false;
    SkColor background_color_ = SK_ColorWHITE;
    gfx::Font text_font_ = ftPrimary;
    DISALLOW_COPY_AND_ASSIGN(LivehimeListStateBannerView);
};




#endif
