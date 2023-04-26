#include "bililive/bililive/ui/views/tabarea/bottom_bar/global_banned_view.h"

#include "bililive/bililive/livehime/tabarea/global_banned_presenter_impl.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_toast_view.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

GlobalBannedView::GlobalBannedView() : minute_(0) {
    presenter_ = std::make_unique<GlobalBannedPresenterImpl>(this);
}

GlobalBannedView::~GlobalBannedView() {}

void GlobalBannedView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.is_add && details.child == this) {
        InitViews();
    }
}

gfx::Size GlobalBannedView::GetPreferredSize() {
    gfx::Size loading_size = loading_view_->GetPreferredSize();
    gfx::Size banned_begin_size = banned_begin_view_->GetPreferredSize();

    int cx = kMainWndTabAreaWidth;
    int cy = std::max(loading_size.height(), banned_begin_size.height());

    return gfx::Size(cx, cy);
}

void GlobalBannedView::Layout() {
    gfx::Size banned_begin_size = banned_begin_view_->GetPreferredSize();

    loading_view_->SetBounds(0, height() - banned_begin_size.height(), width(), banned_begin_size.height());
    banned_begin_view_->SetBounds(0, height() - banned_begin_size.height(), width(), banned_begin_size.height());
    banned_end_view_->SetBounds(0, height() - banned_begin_size.height(), width(), banned_begin_size.height());
}

void GlobalBannedView::InitViews() {
    loading_view_ = new LoadingView();
    AddChildView(loading_view_);

    banned_begin_view_ = new BannedBeginView(this);
    AddChildView(banned_begin_view_);
    banned_begin_view_->SetVisible(false);

    banned_end_view_ = new BannedEndView(this);
    AddChildView(banned_end_view_);
    banned_end_view_->SetVisible(false);

    presenter_->GetBannedInfo();
}

void GlobalBannedView::BannedOn(int minute, const std::string& type, int level) {
    minute_ = minute;
    presenter_->GlobalBanned(true, minute, type, level);
}

void GlobalBannedView::BannedOff() {
    presenter_->GlobalBanned(false, kDefaultMinute, kOff, kDefaultLevel);
}

void GlobalBannedView::OnBannedInfo(bool valid_response, int code, int second) {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    if (!(valid_response && code == 0)) {
        loading_view_->set_label_text(rb.GetLocalizedString(IDS_DANMUKU_BANNED_ERROR));
    } else {
        if (second == 0) {
            loading_view_->SetVisible(false);
            banned_begin_view_->SetVisible(true);
            banned_end_view_->SetVisible(false);
        } else {
            if (second == -1) {
                banned_end_view_->set_label_text(rb.GetLocalizedString(IDS_DANMUKU_BANNED_THIS_LIVE));
            } else {
                std::string text =
                    base::StringPrintf("%d:%02d", second / 60, second % 60);
                banned_end_view_->set_label_text(
                    base::UTF8ToUTF16(text).append(rb.GetLocalizedString(IDS_DANMUKU_BANNED_REMOVE)));
            }

            loading_view_->SetVisible(false);
            banned_begin_view_->SetVisible(false);
            banned_end_view_->SetVisible(true);
        }
    }

    InvalidateLayout();
    Layout();
}

void GlobalBannedView::OnGlobalBanned(bool banned, bool success) {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    if (!success) {
        ShowTabAreaToast(TabAreaToastType_Warning,
            rb.GetLocalizedString(IDS_DANMUKU_BANNED_TOAST_ERROR));
        return;
    }

    if (banned) {
        string16 text;
        if (minute_ == 0) text = rb.GetLocalizedString(IDS_DANMUKU_BANNED_THIS_LIVE);
        else {
            if (minute_ == 3) text = L"3:00";
            if (minute_ == 10) text = L"10:00";
            if (minute_ == 30) text = L"30:00";
            text.append(rb.GetLocalizedString(IDS_DANMUKU_BANNED_REMOVE));
        }

        banned_end_view_->set_label_text(text);

        loading_view_->SetVisible(false);
        banned_begin_view_->SetVisible(false);
        banned_end_view_->SetVisible(true);

    } else {
        banned_begin_view_->ResetCombobox();

        loading_view_->SetVisible(false);
        banned_begin_view_->SetVisible(true);
        banned_end_view_->SetVisible(false);
    }

    InvalidateLayout();
    Layout();
}

