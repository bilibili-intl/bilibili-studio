#include "danmaku_interaction_title_viddup_view.h"

#include "bililive/bililive/ui/views/controls/linear_layout.h"
#include "base/prefs/pref_service.h"
#include "ui/base/resource/resource_bundle.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "base/notification/notification_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"

const int DanmakuInteractionTitleViddupView::kViewHeight = GetLengthByDPIScale(34);


DanmakuInteractionTitleViddupView::DanmakuInteractionTitleViddupView()
{
    InitView();
}

DanmakuInteractionTitleViddupView::~DanmakuInteractionTitleViddupView()
{
}

void DanmakuInteractionTitleViddupView::SwitchThemeImpl()
{
    title_label_->SetTextColor(GetDanmakuThemeColor(GetTheme(), DanmakuThemeViewType::kTextHighLight));

    SchedulePaint();
}

void DanmakuInteractionTitleViddupView::EnableEffect(bool enable)
{
}

void DanmakuInteractionTitleViddupView::Lock(bool lock)
{
}

void DanmakuInteractionTitleViddupView::Pin(bool pin)
{
}

void DanmakuInteractionTitleViddupView::InitView()
{
    auto layout = new bililive::LinearLayout(bililive::LinearLayout::kHoirzontal, this);
    this->SetLayoutManager(layout);

    auto title = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_TEMP_SET_DANMU_HUDONG);
    title_label_ = new BililiveLabel(title, ftFourteen);
    title_label_->SetPreferredSize(GetSizeByDPIScale({ 49, 17 }));
    layout->AddView(title_label_, bililive::LinearLayout::kLeading, bililive::LinearLayout::kCenter, GetLengthByDPIScale(10));

    SwitchThemeImpl();
}

void DanmakuInteractionTitleViddupView::SetNoSubpixelRendering(bool no_subpixel_rendering)
{
    title_label_->set_no_subpixel_rendering(no_subpixel_rendering);
}

void DanmakuInteractionTitleViddupView::SetFunctionButtonBright(bool bright)
{
}

void DanmakuInteractionTitleViddupView::UpdateFunctionAreaApplyCount(int apply_count)
{
}

void DanmakuInteractionTitleViddupView::UpdateFunctionButtonVisible()
{
}

void DanmakuInteractionTitleViddupView::UpdateSettingBtRedPointStatus()
{
}

void DanmakuInteractionTitleViddupView::ShowCoreUserDisturbBubble()
{
}