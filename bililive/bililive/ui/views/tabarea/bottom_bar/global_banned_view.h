#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_GLOBAL_BANNED_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_GLOBAL_BANNED_VIEW_H_

#include "bililive/bililive/livehime/tabarea/global_banned_contract.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/banned_begin_view.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/banned_end_view.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/loading_view.h"

class GlobalBannedView : 
    public BililiveWidgetDelegate,
    public contracts::GlobalBannedView,
    public BannedBeginDelegate,
    public BannedEndDelegate {
public:
    GlobalBannedView();

    ~GlobalBannedView();

    void OnBannedInfo(bool valid_response, int code, int second) override;

    void OnGlobalBanned(bool banned, bool success) override;

private:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }

    // BannedBeginDelegate
    void BannedOn(int minute, const std::string& type, int level) override;

    // BannedEndDelegate
    void BannedOff() override;

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;

    gfx::Size GetPreferredSize() override;

    void Layout() override;
    //presenter

    void InitViews();

private:
    int minute_;

    LoadingView* loading_view_ = nullptr;

    BannedBeginView* banned_begin_view_ = nullptr;

    BannedEndView* banned_end_view_ = nullptr;

    std::unique_ptr<contracts::GlobalBannedPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(GlobalBannedView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_GLOBAL_BANNED_VIEW_H_