#ifndef LIVEHIME_HOVER_TIP_BUTTON_H
#define LIVEHIME_HOVER_TIP_BUTTON_H

#include "ui/views/controls/button/image_button.h"

#include "bililive/bililive/ui/views/controls/bililive_bubble.h"


class LivehimeTipsBubble
    : public BililiveBubbleView
{
public:
    static void Show(views::View *anchor, const base::string16 &title,
        const base::string16 &tips, views::BubbleBorder::Arrow arrow);
    static void Close(views::View *anchor);
    static void OnAnchorViewVisibleBoundsChanged();

protected:
    // WidgetDelegate overrides:
    bool CanActivate() const override { return false; }

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;

protected:
    LivehimeTipsBubble(views::View *anchor, const base::string16 &title,
        const base::string16 &tips, views::BubbleBorder::Arrow arrow);
    virtual ~LivehimeTipsBubble();

    void OnAnchorViewBoundsChangedEx();

private:
    static LivehimeTipsBubble *instance_;
    BililiveLabel *title_label_;
    BililiveLabel *tips_label_;
    base::string16 title_;
    base::string16 tips_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeTipsBubble);
};


// hover文本提示按钮
class LivehimeHoverTipButton : public views::ImageButton
{
public:
    LivehimeHoverTipButton(const base::string16 &title, const base::string16 &tips,
        views::BubbleBorder::Arrow arrow = views::BubbleBorder::LEFT_TOP,
        views::ButtonListener* listener = nullptr);
    virtual ~LivehimeHoverTipButton();

    void SetTooltipText(const base::string16 &title, const base::string16 &tips);

    void SetButtonImage(const gfx::ImageSkia* nor,const gfx::ImageSkia* hv);

protected:
    // view
    const char* GetClassName() const override { return "LivehimeHoverTipButton"; }
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void StateChanged() override;

private:
    LivehimeTipsBubble *bubble_;
    bool is_mouse_enter_;
    base::string16 title_;
    base::string16 tips_;
    views::BubbleBorder::Arrow arrow_;
};

#endif
