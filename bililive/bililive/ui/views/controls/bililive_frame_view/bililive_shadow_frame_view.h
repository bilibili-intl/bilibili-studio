#ifndef BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FRAME_VIEW_BILILIVE_SHADOW_FRAME_VIEW_H
#define BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FRAME_VIEW_BILILIVE_SHADOW_FRAME_VIEW_H


#include "bililive_frame_titlebar_view.h"

#include "ui/views/bubble/bubble_frame_view.h"

#include "ui/gfx/canvas.h"

class BiliShadowFrameView : public views::BubbleFrameView
{
public:
    explicit BiliShadowFrameView(views::View *title_bar);
    virtual ~BiliShadowFrameView() = default;

    void SetMinMaxSize(const gfx::Size &min_size, const gfx::Size &max_size);
    void set_show_border(bool show){ show_border_ = show; }
    bool show_border() const { return show_border_; }

protected:
    // NonClientFrameView
    int NonClientHitTest(const gfx::Point& point);
    void UpdateWindowTitle();
    void ResetWindowControls() override {}
    gfx::Rect GetBoundsForClientView() const { return CalculateClientAreaBounds(); };
    gfx::Rect GetWindowBoundsForClientBounds(const gfx::Rect& client_bounds) const;

    // View
    gfx::Size GetMaximumSize() override;
    gfx::Size GetMinimumSize() override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;
    gfx::Insets GetInsets() const override;
    void ChildPreferredSizeChanged(View* child){}

protected:
    views::View *title_bar(){ return title_bar_; }
    gfx::Rect CalculateClientAreaBounds() const;
    gfx::Rect CalculateTitleBarAreaBounds() const;
    gfx::Rect CalculateNonClientFrameValidBounds() const;
    gfx::Insets GetShadowInsets() const;

private:
    views::View *title_bar_;
    gfx::Size min_size_;
    gfx::Size max_size_;
    bool show_border_;

    DISALLOW_COPY_AND_ASSIGN(BiliShadowFrameView);
};


class BililiveShadowFrameView : public BiliShadowFrameView
{
public:
    explicit BililiveShadowFrameView(gfx::ImageSkia *skia, base::string16 caption, int buttons = TBB_CLOSE);
    explicit BililiveShadowFrameView(int buttons = TBB_CLOSE);
    explicit BililiveShadowFrameView(views::View *title_bar);
    virtual ~BililiveShadowFrameView() = default;

protected:
private:

    DISALLOW_COPY_AND_ASSIGN(BililiveShadowFrameView);
};

#endif