#ifndef BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FRAME_VIEW_BILILIVE_FRAME_VIEW_H
#define BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FRAME_VIEW_BILILIVE_FRAME_VIEW_H

#include "bililive_frame_titlebar.h"

#include "base/memory/scoped_ptr.h"
#include "base/timer/timer.h"

#include "ui/views/window/non_client_view.h"

class BililiveFrameView
    : public views::NonClientFrameView
{
public:
    static const char kDevViewClassName[];

    BililiveFrameView(gfx::ImageSkia *skia, base::string16 caption,
        int TitleBarButtonCombination = TBB_CLOSE);
    explicit BililiveFrameView(int TitleBarButtonCombination = TBB_CLOSE);
    explicit BililiveFrameView(BililiveFrameTitleBar *title_bar);
    virtual ~BililiveFrameView();

    void SetEnableDragSize(bool enable) { enable_left_button_drag_size_ = enable; }
    void SetEnableDragMove(bool enable) { enable_left_button_drag_move_ = enable; }
    void SetBorderColor(SkColor color);
    void SetBackgroundColor(SkColor color);
    void SetTitlebar(BililiveFrameTitleBar *title_bar);
    void SetTitlebarDelegate(BililiveFrameTitleBarDelegate *deleagte);
    void SetCacheMaxminSize(bool cache);
    void RestoreMaxminSize();
    void FlashFrame();
    void SetShadowFrame(bool has_shadow){ has_shadow_ = has_shadow; }

    BililiveFrameTitleBar* GetTitleBar() const { return title_bar_; }

protected:
    // NonClientFrameView
    gfx::Rect GetBoundsForClientView() const override;
    gfx::Rect GetWindowBoundsForClientBounds(const gfx::Rect &client_bounds) const override;
    int NonClientHitTest(const gfx::Point &point) override;
    void GetWindowMask(const gfx::Size &size, gfx::Path *window_mask) override;
    void ResetWindowControls() override;
    void UpdateWindowIcon() override;
    void UpdateWindowTitle() override;
    gfx::Size GetMinimumSize() override;
    gfx::Size GetMaximumSize() override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void Layout() override;
    const char* GetClassName() const override;

private:
    gfx::Rect CalculateClientAreaBounds() const;
    void InitView();
    void OnFlashTimer();
    void DoFlashFrame();

protected:
    bool enable_left_button_drag_move_;
    bool enable_left_button_drag_size_;

private:
    gfx::Rect client_view_bounds_;
    BililiveFrameTitleBar *title_bar_;

    bool cache_maxmin_size_;
    gfx::Size min_size_;
    gfx::Size max_size_;

    bool has_shadow_;
    views::Widget *shadow_widget_;
    int flash_times_;
    base::RepeatingTimer<BililiveFrameView> flash_timer_;

    base::WeakPtrFactory<BililiveFrameView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveFrameView);
};


class BililiveNonTitleBarFrameViewDelegate
{
public:
    virtual int NonClientHitTest(const gfx::Point &point) = 0;

protected:
    virtual ~BililiveNonTitleBarFrameViewDelegate() = default;
};

class BililiveNonTitleBarFrameView : public BililiveFrameView
{
public:
    static const char kDevViewClassName[];

    explicit BililiveNonTitleBarFrameView(BililiveNonTitleBarFrameViewDelegate* delegate);
    virtual ~BililiveNonTitleBarFrameView();

protected:
    // View
    const char* GetClassName() const override;

    // NonClientFrameView
    int NonClientHitTest(const gfx::Point &point) override;

private:
    BililiveNonTitleBarFrameViewDelegate* delegate_ = nullptr;
    DISALLOW_COPY_AND_ASSIGN(BililiveNonTitleBarFrameView);
};

#endif