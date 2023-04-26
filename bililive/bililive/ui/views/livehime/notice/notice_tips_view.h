#pragma once

#include "ui/views/controls/button/button.h"

#include "base/memory/weak_ptr.h"

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"

class LivehimeImageView;

class NoticeTipsView
    : public BililiveWidgetDelegate,
    public views::ButtonListener,
    public BililiveNonTitleBarFrameViewDelegate {
public:
    static void ShowWindow(const std::string& icon_url, const std::string& content);

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget *widget) override;

    // BililiveNonTitleBarFrameViewDelegate
    int NonClientHitTest(const gfx::Point &point) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    NoticeTipsView(const std::string& icon_url, const std::string& content);
    ~NoticeTipsView();

    void InitViews();
    void CloseWindow();

private:
    views::View* info_view_ = nullptr;
    LivehimeImageView* close_view_ = nullptr;
    std::string icon_url_;
    std::string content_;

    base::WeakPtrFactory<NoticeTipsView> weakptr_factory_;


    DISALLOW_COPY_AND_ASSIGN(NoticeTipsView);
};
