#include "global_settings_view.h"

#include "base/prefs/pref_service.h"

#include "ui/views/layout/grid_layout.h"

#include "bilibase/win/win10_ver.h"

#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/livehime/main_view/livehime_main_close_pref_constants.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/settings/join_slide_tips_view.h"
#include "bililive/bililive/ui/views/login/bilibili_login_control.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/secret/bililive_secret.h"




#include "obs/obs_proxy/public/common/pref_constants.h"

namespace {

    void DonotCarePUpdateLiveReplayConfigResult(bool valid_response, int code, const std::string& err_msg)
    {
        // 产品说不关心设置接口是否成功
    }
}

GlobalSettingsView::GlobalSettingsView()
    : BaseSettingsView(),
    performance_mode_checkbox_(nullptr),
    exclude_from_capture_checkbox_(nullptr),
    drag_full_windows_checkbox_(nullptr),
    min_radio_button_(nullptr),
    exit_radio_botton_(nullptr),
    remind_choice_(nullptr) {
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

bool GlobalSettingsView::SaveNormalSettingsChange() {
    bool is_exclude_from_capture =
        exclude_from_capture_checkbox_ ? exclude_from_capture_checkbox_->checked() : false;

    presenter_->GlobalSaveChange(
        performance_mode_checkbox_->checked(),
        is_exclude_from_capture,
        drag_full_windows_checkbox_->checked(),
        min_radio_button_->checked(),
        remind_choice_->checked());

    if (bilibase::win::IsWindows10V2004OrGreater()) {
        auto widget = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget();
        if (is_exclude_from_capture) {
            BOOL ret = ::SetWindowDisplayAffinity(
                widget->GetNativeView(), WDA_EXCLUDEFROMCAPTURE);
        } else {
            BOOL ret = ::SetWindowDisplayAffinity(
                widget->GetNativeView(), WDA_NONE);
        }
    }

    if (live_replay_view_->visible())
    {
        
    }

    return true;
}

void GlobalSettingsView::InitViews()
{
    InitCmbModel();
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    BililiveLabel* label = nullptr;
    //新版直播间
    new_room_view_ = new HideAwareView();
    {
        views::GridLayout* new_room_layout = new views::GridLayout(new_room_view_);
        new_room_view_->SetLayoutManager(new_room_layout);

        auto column_set = new_room_layout->AddColumnSet(0);
        column_set->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);

        new_room_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
        label = new LivehimeTitleLabel(L"新版直播间");
        new_room_layout->AddView(label);

        new_room_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        new_room_ = new LivehimeCheckbox(L"开启新版直播间");
        new_room_->set_listener(this);
        new_room_layout->AddView(new_room_);

        new_room_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
        label = new LivehimeTipLabel(L"为主播增加曝光机会，让用户更易找到你");
        new_room_layout->AddView(label);

        int join_slide = GetBililiveProcess()->secret_core()->anchor_info().get_join_slide();
        new_room_->SetChecked(!(join_slide == 0));
    }
    layout->StartRowWithPadding(0, 0, 0, 0);

    layout->AddView(new_room_view_);
    //if (AppFunctionController::GetInstance()->app_function_settings().new_live_room_switch) {
    //    layout->AddView(new_room_view_);
    //}


    //new_room_view_->SetVisible(false);

    // 直播回放
    live_replay_view_ = new HideAwareView();
    {
        views::GridLayout* live_replay_layout = new views::GridLayout(live_replay_view_);
        live_replay_view_->SetLayoutManager(live_replay_layout);

        auto column_set = live_replay_layout->AddColumnSet(0);
        column_set->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);

        column_set = live_replay_layout->AddColumnSet(1);
        column_set->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::CENTER,
            0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, GetLengthByDPIScale(60));
        column_set->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::CENTER,
            0, views::GridLayout::USE_PREF, 0, 0);

        live_replay_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
        label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_TOOLBAR_REPLAY_LIVE_REPALY));
        live_replay_layout->AddView(label);

        label = new LivehimeTitleLabel(L"是否每次默认生成直播回放，生成后用户在你的个人空间可见");
        live_replay_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        live_replay_layout->AddView(label);

        ++radio_button_groupid_;

        live_replay_layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        auto_archive_radio_button_ = new LivehimeRadioButton(L"是", radio_button_groupid_);
        auto_archive_radio_button_->set_listener(this);
        live_replay_layout->AddView(auto_archive_radio_button_);

        no_archive_radio_button_ = new LivehimeRadioButton(L"否，每次关播时手动选择", radio_button_groupid_);
        no_archive_radio_button_->set_listener(this);
        live_replay_layout->AddView(no_archive_radio_button_);


        label = new LivehimeTitleLabel(L"发布回放时，是否默认同步至动态");
        live_replay_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
        live_replay_layout->AddView(label);

        ++radio_button_groupid_;

        live_replay_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        ask_replay_radio_button_ = new LivehimeRadioButton(L"仅发布回放，不同步至动态", radio_button_groupid_);
        ask_replay_radio_button_->set_listener(this);
        live_replay_layout->AddView(ask_replay_radio_button_);

        live_replay_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        auto_replay_radio_button_ = new LivehimeRadioButton(L"默认同步至动态", radio_button_groupid_);
        auto_replay_radio_button_->set_listener(this);
        live_replay_layout->AddView(auto_replay_radio_button_);

        live_replay_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
        label = new LivehimeTipLabel(L"* 设置对各开播端均生效，可在直播中心-我的直播间-直播回放中查看回放哦~");
        live_replay_layout->AddView(label);

    }
    layout->StartRowWithPadding(0, 0, 0, 0);
    layout->AddView(live_replay_view_);
    live_replay_view_->SetVisible(false);


    // 窗口捕捉引擎
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_WINDOW_CAPTURE));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
    performance_mode_checkbox_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_WINDOW_CAPTURE_OPTIMIZE_FOR_PERFORMANCE));
    auto performance_help_view = new livehime::HelpSupportView(performance_mode_checkbox_,
        livehime::HelpType::OptimizeForPerformance);
    layout->AddView(performance_help_view);

    // -tips
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_WINDOW_CAPTURE_OPTIMIZE_FOR_PERFORMANCE_TOOLTIP));
    label->SetMultiLine(true);
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    if (bilibase::win::IsWindows10V2004OrGreater()) {
        // 从捕捉中排除
        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        exclude_from_capture_checkbox_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_EXCLUDE_FROM_CAPTURE));
        layout->AddView(exclude_from_capture_checkbox_);

        // -tips
        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
        label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_EXCLUDE_FROM_CAPTURE_TOOLTIP));
        label->SetMultiLine(true);
        label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
        layout->AddView(label);
    }

    // 缩放渲染
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_ZOOM_RENDER));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
    drag_full_windows_checkbox_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_ZOOM_RENDER_DRAG_FULL_WINDOWS));
    layout->AddView(drag_full_windows_checkbox_);

    // -tips
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_ZOOM_RENDER_DRAG_FULL_WINDOWS_TOOLTIP));
    label->SetMultiLine(true);
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    // 退出提示
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_EXIT));
    layout->AddView(label);

    ++radio_button_groupid_;

    layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForGroupCtrls);
    min_radio_button_ = new LivehimeRadioButton(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_EXIT_MIN), radio_button_groupid_);
    layout->AddView(min_radio_button_);

    exit_radio_botton_ = new LivehimeRadioButton(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_EXIT_IMMEDIATELY), radio_button_groupid_);
    layout->AddView(exit_radio_botton_);

    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
    remind_choice_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_CONFIG_GLOBAL_EXIT_REMIND));
    layout->AddView(remind_choice_);

}

void GlobalSettingsView::InitCmbModel()
{}

void GlobalSettingsView::InitData()
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

    performance_mode_checkbox_->SetChecked(
        global_prefs->GetBoolean(prefs::kWindowCaptureOptimizeForPerformance));
    if (exclude_from_capture_checkbox_) {
        exclude_from_capture_checkbox_->SetChecked(
            global_prefs->GetBoolean(prefs::kExcludeMainWindowFromCapture));
    }

    drag_full_windows_checkbox_->SetChecked(!global_prefs->GetBoolean(prefs::kDragFullWindows));

    int exit_style = global_prefs->GetInteger(prefs::kApplicationExitMode);
    if (exit_style == prefs::kMinimizeToTray)
    {
        min_radio_button_->SetChecked(true);
    }
    else if (exit_style == prefs::kExitImmediately)
    {
        exit_radio_botton_->SetChecked(true);
    }

    remind_choice_->SetChecked(!global_prefs->GetBoolean(prefs::kApplicationExitRememberChoice));

    presenter_->RequestGetNewRoomSwitch();
}

void GlobalSettingsView::SetNewRoomViewVisible(bool visible) {

    new_room_view_->SetVisible(visible);
    new_room_view_->InvalidateLayout();
    if (Container()) {
        static_cast<views::View*>(Container())->Layout();
    }
}

void GlobalSettingsView::ShowLiveReplaySetting(bool publish,bool archive)
{
    if (live_replay_view_)
    {
        if (publish)
        {
            auto_replay_radio_button_->SetChecked(true);
            auto_replay_ = true;
        }
        else
        {
            ask_replay_radio_button_->SetChecked(true);
            auto_replay_ = false;
        }
        if (archive)
        {
            auto_archive_radio_button_->SetChecked(true);
            auto_archive_ = true;
        }
        else
        {
            no_archive_radio_button_->SetChecked(true);
            auto_archive_ = false;
        }
        live_replay_view_->SetVisible(true);
        live_replay_view_->InvalidateLayout();
        if (Container()) {
            static_cast<views::View*>(Container())->Layout();
        }
    }
}

void GlobalSettingsView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == auto_replay_radio_button_)
    {
        auto_replay_ = true;
    }
    else if (sender == ask_replay_radio_button_)
    {
        auto_replay_ = false;
    }
    else if (sender == auto_archive_radio_button_)
    {
        auto_archive_ = true;
    }
    else if (sender == no_archive_radio_button_)
    {
        auto_archive_ = false;

    }
    else if (sender == new_room_)
    {
        if (!new_room_->checked()) {
           //取消选中时弹窗阻塞，弹窗确定之后调接口
            EndDialogSignalHandler handler;
            handler.closure = base::Bind(&GlobalSettingsView::ConfirmDialog);
            handler.data = this;
            JoinSlideTipsView::ShowForm(GetWidget(),
                &handler);
        }

        //选中后直接调post接口通知服务器
        UpdateTemporaryConf(1);
    }
}

void GlobalSettingsView::ConfirmDialog(int code, void* data)
{
    if (code == IDCANCEL)
    {
        static_cast<GlobalSettingsView*>(data)->UpdateNewRoomCheckbox(true);
    }
    else {
        static_cast<GlobalSettingsView*>(data)->UpdateTemporaryConf(0);
    }
}

void GlobalSettingsView::UpdateNewRoomCheckbox(bool check)
{
    new_room_->SetChecked(check);
}

void GlobalSettingsView::UpdateTemporaryConf(int join_slide)
{

}