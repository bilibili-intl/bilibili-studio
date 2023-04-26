#include "bililive/bililive/ui/views/login/bililive_login_logining_status_view.h"

#include "base/strings/string16.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_user_related_ctrls.h"
#include "bililive/bililive/ui/views/login/bilibili_login_control.h"
#include "bililive/bililive/ui/views/login/bililive_login_main_view.h"
#include "bililive/bililive/ui/views/login/livehime_login_util.h"

#include "grit/generated_resources.h"
#include "bililive/bililive/ui/views/login/livehime_login_util.h"


namespace {
const int kDelayTime = 3;

};

LoginingStatusView::LoginingStatusView(LoginingStatusDelegate* delegate) :
    delegate_(delegate),
    weakptr_factory_(this) {

}

LoginingStatusView::~LoginingStatusView() {}

void LoginingStatusView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this && details.is_add) {
        InitView();
    }
}

void LoginingStatusView::InitView() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    views::ColumnSet *columnset = layout->AddColumnSet(0);
    columnset->AddPaddingColumn(1.0f, 0);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    columnset = layout->AddColumnSet(1);
    columnset->AddPaddingColumn(0, GetLengthByDPIScale(30));
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, GetLengthByDPIScale(30));

    columnset = layout->AddColumnSet(2);
    columnset->AddPaddingColumn(1.0f, 0);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    // Í·Ïñ
    user_pic_view_ = new HideAwareView();
    user_pic_view_->SetLayoutManager(new views::FillLayout);
    user_pic_ = new LivehimeCircleImageView();
    user_pic_->SetPreferredSize(GetSizeByDPIScale(gfx::Size(55, 55)));
    user_pic_->SetImagePlaceholder(*GetImageSkiaNamed(IDR_LOGIN_DEF_USR_FACE));
    user_pic_view_->AddChildView(user_pic_);

    info_label_ = new views::Label(rb.GetLocalizedString(IDS_LOGINING));
    info_label_->SetHorizontalAlignment(gfx::ALIGN_CENTER);
    info_label_->SetFont(ftPrimary);
    info_label_->SetEnabledColor(clrTextPrimary);
    info_label_->SetAutoColorReadabilityEnabled(false);
    info_label_->SetMultiLine(true);

    cancel_btn_ = new BilibiliLoginLabelButton(this, rb.GetLocalizedString(IDS_BACK));
    cancel_btn_->SetVisible(false);
    cancel_btn_->SetIsDefault(true);

    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
    layout->StartRow(0, 0);
    layout->AddView(user_pic_view_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
    layout->StartRow(0, 1);
    layout->AddView(info_label_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 2);
    layout->AddView(cancel_btn_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
}

void LoginingStatusView::SetUserFace(int64 mid)
{
    if (0 != mid)
    {
        std::string content(std::move(livehime::GetAvatarBinaryDataByMid(mid)));
        user_pic_->SetImageBinary((const unsigned char*)content.data(), content.length());
    }
    else
    {
        user_pic_->SetImage(gfx::ImageSkia());
    }
}

void LoginingStatusView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    if (sender == cancel_btn_) {
        animation_timer.reset();

        delegate_->OnLoginingStatusStop();
    }
}

void LoginingStatusView::ShowBeLogining() {
    base::string16 msg = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_LOGINING);
    info_label_->SetText(msg);
    info_label_->SetEnabledColor(clrLabelText);
    cancel_btn_->SetVisible(false);
    user_pic_view_->SetVisible(true);

    InvalidateLayout();
    Layout();
    delegate_->OnLoginingStatusMsgChanged(msg);
}

void LoginingStatusView::ShowErrorMessage(const base::string16& msg) {
    info_label_->SetText(msg);
    info_label_->SetEnabledColor(clrTextTipWarn);
    cancel_btn_->SetVisible(true);
    user_pic_view_->SetVisible(false);

    InvalidateLayout();
    Layout();
    delegate_->OnLoginingStatusMsgChanged(msg);

    StartReturnTimer();
}

void LoginingStatusView::StartReturnTimer() {
    animation_timer.reset(new base::Timer(FROM_HERE, base::TimeDelta::FromSeconds(kDelayTime),
        base::Bind(&LoginingStatusView::OnTimerArrive, weakptr_factory_.GetWeakPtr()), false));
    animation_timer->Reset();
}

void LoginingStatusView::OnTimerArrive() {
    animation_timer.reset();

    delegate_->OnLoginingStatusStop();
}