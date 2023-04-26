#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_BUBBLE_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_BUBBLE_H


#include "bililive_theme_common.h"
#include "bililive_label.h"

#include "base/memory/weak_ptr.h"

#include "ui/gfx/canvas.h"
#include "ui/gfx/platform_font.h"
#include "ui/gfx/font.h"
#include "ui/views/bubble/bubble_delegate.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/controls/label.h"
#include "ui/views/widget/widget.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/bubble/bubble_frame_view.h"

class BililiveBubble : public views::BubbleDelegateView
{
public:
    // 单纯显示文本
    static void ShowBubble(views::View* anchor, views::BubbleBorder::Arrow arrow, const base::string16 &tooltip,
        bool close_on_deactivate = true)
    {
        views::Widget* widget = BubbleDelegateView::CreateBubble(new BililiveBubble(anchor, arrow, tooltip, close_on_deactivate));
        widget->Show();
    }

    explicit BililiveBubble(views::View* anchor, views::BubbleBorder::Arrow arrow, const base::string16 &tooltip,
        bool close_on_deactivate = true)
        : BubbleDelegateView(anchor, arrow)
        , tooltip_(tooltip)
        , close_on_deactivate_(close_on_deactivate)
    {
        init_style();
    }

    explicit BililiveBubble(views::View* anchor, views::BubbleBorder::Arrow arrow,
        bool close_on_deactivate = true)
        : BubbleDelegateView(anchor, arrow)
        , close_on_deactivate_(close_on_deactivate)
    {
        init_style();
    }

    views::Widget* Show()
    {
        views::Widget* widget = BubbleDelegateView::CreateBubble(this);
        widget->Show();
        return widget;
    }

    void Close()
    {
        GetWidget()->Close();
    }

protected:
    virtual void Init() OVERRIDE
    {
    }

private:
    void init_style()
    {
        set_color(SK_ColorWHITE);
        set_close_on_esc(true);
        set_close_on_deactivate(close_on_deactivate_);
        set_shadow(views::BubbleBorder::SMALL_SHADOW);
        //set_use_focusless(true);
        //set_move_with_anchor(true);
        if (anchor_view())
        {
            set_parent_window(anchor_view()->GetWidget()->GetNativeWindow());
        }

        InitView();
    }

    void InitView()
    {
        set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));
        SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, 0));
        BililiveLabel *label = new BililiveLabel(tooltip_);
        //label->SetMultiLine(true);
        //label->SetAutoColorReadabilityEnabled(false);
        //label->SetFont(ftLabel);
        label->SetTextColor(SkColorSetRGB(48, 48, 48));
        label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
        AddChildView(label);
    }

private:
    base::string16 tooltip_;
    bool close_on_deactivate_;

    DISALLOW_COPY_AND_ASSIGN(BililiveBubble);
};


class BililiveBubbleView : public views::BubbleDelegateView
{
public:
    BililiveBubbleView(views::View* anchor_view, views::BubbleBorder::Arrow arrow,
        const base::string16 &tooltip);
    BililiveBubbleView(views::View* anchor_view, views::BubbleBorder::Arrow arrow,
        views::View* content_view);
    BililiveBubbleView(views::View* anchor_view, views::BubbleBorder::Arrow arrow);

    virtual ~BililiveBubbleView();

    void set_background_color(SkColor clr);
    void set_text_color(SkColor clr);
    void SetFont(const gfx::Font& font);
    void SetRadius(const int radius);
    void SetShadowBorderThickness(const int borderThickness);

    void StartFade(bool fade_in, int after_timespan_ms, int fade_duration_ms);
    void ResetFade();

    views::View* content_view() { return content_view_; }
    //更新位置
    void UpdatePosition();

protected:
    // WidgetDelegate
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // BubbleDelegateView
    void OnNativeThemeChanged(const ui::NativeTheme* theme) override;
    int GetFadeDuration() override;

private:
    void InitStyle();

private:
    base::string16 tooltip_;
    int fade_duration_ms_ = 200;
    SkColor text_color_;
    gfx::Font text_font_;

    BililiveLabel* def_label_ = nullptr;
    views::View* content_view_ = nullptr;

    base::WeakPtrFactory<BililiveBubbleView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveBubbleView);
};

#endif