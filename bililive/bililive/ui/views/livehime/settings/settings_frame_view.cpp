#include "settings_frame_view.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/settings/accelerator_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/audio_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/global_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/record_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/language_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/streaming_settings_view.h"
#include "bililive/bililive/ui/views/livehime/settings/video_settings_view.h"
#include "bililive/bililive/ui/views/livehime/volume/bililive_sound_effect_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/public/bililive/bililive_command_ids.h"


namespace {
    class CommonSettingStripView : public LivehimeLeftStripPosStripView {
    public:
        CommonSettingStripView(bool red_point,const std::string& red_key, const base::string16 &text, gfx::ImageSkia* image_skia = nullptr)
            : LivehimeLeftStripPosStripView(text, image_skia)
            , red_point_(red_point)
            , redpoint_setting_key(red_key){
        }

        ~CommonSettingStripView() {}

        // NavigationStripView
        void OnStateChanged() override {
            __super::OnStateChanged();

            if ((state() == NavigationStripView::SS_SELECTED) || (state() == NavigationStripView::SS_PRESSED)) {
                if (red_point_) {
                    red_point_ = false;
                    if (!redpoint_setting_key.empty()) {
                        GetBililiveProcess()->profile()->GetPrefs()->SetBoolean(redpoint_setting_key.c_str(), false);
                    }
                    SchedulePaint();
                }
            }
        }

        void OnPaintBackground(gfx::Canvas* canvas) override {
            __super::OnPaintBackground(canvas);

            if (red_point_) {
                gfx::Size label_size = label_->GetPreferredSize();
                auto bounds = GetContentsBounds();

                static const int kRadius = GetLengthByDPIScale(5);
                static const int kPadding = GetLengthByDPIScale(2);

                int x = bounds.x() + (bounds.width() - label_size.width()) / 2 - kRadius - GetLengthByDPIScale(4);
                int y = bounds.y() + bounds.height() / 2;
                gfx::Point point(x, y);

                SkPaint paint;
                paint.setAntiAlias(true);

                // 外圈白光
                paint.setColor(SK_ColorWHITE);
                canvas->DrawCircle(point, kRadius, paint);
                // 内圈红点
                paint.setColor(SK_ColorRED);
                canvas->DrawCircle(point, kRadius - kPadding, paint);
            }
        }

    private:
        bool red_point_ = false;
        std::string redpoint_setting_key;
    };
}


SettingsFrameView *SettingsFrameView::instance_ = nullptr;

SettingsFrameView::SettingsFrameView(int index)
    : BililiveWidgetDelegate(
        *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_TOOLBAR_SET),
    ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_TOOLBAR_CONFIG)),
    tabbed_pane_(nullptr),
    def_tabindex_(index),
    saved_changes_(false),
    window_closing_(false),
    trigger_close_(false),
    weakptr_factory_(this)
{
    set_focusable(true);
    set_focus_border(nullptr);

    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

SettingsFrameView::~SettingsFrameView()
{
    presenter_->SettingsFrameViewDeleted();
    window_closing_ = true;
    trigger_close_ = true;
    instance_ = nullptr;
}

void SettingsFrameView::ShowForm(views::Widget *parent, int index)
{
    if (!instance_) {
        views::Widget *widget_ = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget_);
        params.parent = parent->GetNativeView();

        instance_ = new SettingsFrameView(index);
        DoModalWidget(instance_, widget_, params);
    }
    else {
        if (instance_->GetWidget()) {
            instance_->GetWidget()->Activate();
        }

        instance_->SelectDefTab(index);
    }
}

void SettingsFrameView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) {
    if (details.is_add && details.child == this) {
        InitViews();
    }
}

gfx::Size SettingsFrameView::GetPreferredSize()
{
    return gfx::Size(GetLengthByDPIScale(900), GetLengthByDPIScale(610));
}

bool SettingsFrameView::OnMousePressed(const ui::MouseEvent& event)
{
    RequestFocus();
    return __super::OnMousePressed(event);
}

void SettingsFrameView::Paint(gfx::Canvas *canvas) {
    __super::Paint(canvas);

    auto strip = tabbed_pane_->GetSelectedTab();
    DCHECK(strip != nullptr);

    gfx::Point start_point(strip->width(), 0);
    gfx::Point end_point(start_point.x(), tabbed_pane_->height());
    canvas->DrawLine(start_point, end_point, GetColor(DropBorder));
}

void SettingsFrameView::WindowClosing()
{
    if (window_closing_)
    {
        LOG(ERROR) << "SettingsFrameView::WindowClosing :windows had been closed!";
        return;
    }
    window_closing_ = true;
    BililiveWidgetDelegate::WindowClosing();

    // 点击窗口右上角关闭按钮时，其行为应与点击取消按钮一致
    if (!saved_changes_) {
        //DiscardSettings();
        presenter_->DiscardSettings();
    }


    LanguageSettingsView* view = (LanguageSettingsView*)sub_views_[language_index_];
    if (view->IsLanguageChanged()) {
        view->ClearLanguageChangeFlag();
        bililive::ExecuteCommandWithParams(
            GetBililiveProcess()->bililive_obs(),
            IDC_LIVEHIME_LANGUAGE_SETTING_CHANGED,
            EmptyCommandParams());
    }
}


void SettingsFrameView::InitViews()
{
    InitViewsForViddup();
}

// 海外的 InitView
void SettingsFrameView::InitViewsForViddup()
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1.0f, views::GridLayout::USE_PREF, 0, 0);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    layout->StartRow(1.0f, 0);
    tabbed_pane_ = new NavigationBar(this, NAVIGATIONBAR_TABSTRIP_LEFT, true);
    // tabbed_pane_->SetTabAreaColor(GetColor(WindowClient));
    tabbed_pane_->SetTabAreaColor(SkColorSetRGB(34, 35, 49)); // 这里是海外的专属构造, 就直接写死了
    layout->AddView(tabbed_pane_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForDiffGroups);
    ok_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_UGC_MSGBOX_SURE), true);
    ok_button_->SetIsDefault(true);
    cancel_button_ = new LivehimeActionLabelButton(
        this, rb.GetLocalizedString(IDS_UGC_MSGBOX_CANCEL), false);
    layout->AddView(ok_button_);
    layout->AddView(cancel_button_);
    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    set_background(views::Background::CreateSolidBackground(SkColorSetRGB(34, 35, 49))); // 这里是海外的专属构造, 就直接写死了

    sub_views_.push_back(new VideoSettingsView());
    auto sound_effect_view = new BililiveSoundEffectView();
    sound_effect_view->SetInsets({ GetLengthByDPIScale(30), GetLengthByDPIScale(30), 0, GetLengthByDPIScale(30) });
    sub_views_.push_back(sound_effect_view);
    // sub_views_.push_back(new AcceleratorSettingsView());
    //sub_views_.push_back(new RecordSettingsView());
    //sub_views_.push_back(new GlobalSettingsView());
    sub_views_.push_back(new LanguageSettingsView());
    //blac_klist_ = new BlacklistView();

    // auto red_point = GetBililiveProcess()->profile()->GetPrefs()->GetBoolean(prefs::kLivehimeGlobalSettingShow);
    // auto voice_red_point = GetBililiveProcess()->profile()->GetPrefs()->GetBoolean(prefs::kLivehimeDanmuSettingShow);
    // auto download_red_point = GetBililiveProcess()->profile()->GetPrefs()->GetBoolean(prefs::kLivehimeDownloadSetShow);

    /// mark index
    auto index = 0;
    sub_views_tabbed_ids_.push_back(tabbed_pane_->AddTab(L"", new LivehimeLeftStripPosStripView(rb.GetLocalizedString(IDS_TOOLBAR_CONFIG_VIDEO)), sub_views_[index++]->Container()));
    sub_views_tabbed_ids_.push_back(tabbed_pane_->AddTab(L"", new LivehimeLeftStripPosStripView(rb.GetLocalizedString(IDS_TOOLBAR_CONFIG_AUDIO)), sub_views_[index++]->Container()));
    // sub_views_tabbed_ids_.push_back(tabbed_pane_->AddTab(L"", new LivehimeLeftStripPosStripView(rb.GetLocalizedString(IDS_TOOLBAR_CONFIG_ACCELERATORS)), sub_views_[index++]->Container()));
    //room_manager_id_ = tabbed_pane_->AddTab(L"",
    //    new LivehimeLeftStripPosStripView(L"房间管理"), blac_klist_->Container());
    //sub_views_tabbed_ids_.push_back(room_manager_id_);
    //download_tap_id_ = tabbed_pane_->AddTab(L"",
    //    new CommonSettingStripView(download_red_point, prefs::kLivehimeDownloadSetShow,
    //        rb.GetLocalizedString(IDS_TOOLBAR_DOWNLOAD_SET)), sub_views_[3]->Container());
    //sub_views_tabbed_ids_.push_back(download_tap_id_);

    // 全局设置下标，修改vector时注意修改(PS:其他地方有用到此下标，历史修改已产生过问题)
    //global_index_ = 4;
    //global_setting_id_ = tabbed_pane_->AddTab(L"",
    //    new CommonSettingStripView(red_point, prefs::kLivehimeGlobalSettingShow,
    //        rb.GetLocalizedString(IDS_TOOLBAR_CONFIG_GLOBAL)), sub_views_[global_index_]->Container());
    //sub_views_tabbed_ids_.push_back(global_setting_id_);

    language_index_ = index;
    // 多语言设置
    sub_views_tabbed_ids_.push_back(tabbed_pane_->AddTab(L"", new LivehimeLeftStripPosStripView(rb.GetLocalizedString(IDS_CONFIG_LANGUAGE_TAB)), sub_views_[index++]->Container()));

    SelectDefTab(def_tabindex_);

    //presenter_->RequestLiveRoomCdnLineList();
    //presenter_->RequestLiveReplaySetting();
}

void SettingsFrameView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (window_closing_)
    {
        LOG(ERROR) << "SettingsFrameView::ButtonPressed :windows had been closed!";
        return;
    }
    if (trigger_close_)
    {
        LOG(ERROR) << "SettingsFrameView::ButtonPressed :trigger twice close!";
        return;
    }
    if (sender == ok_button_) {
        ProcessPositive();
    }
    else if (sender == cancel_button_) {
        ProcessNegative();
    }
}

void SettingsFrameView::SelectDefTab(int index) {
    DCHECK(index >= 0 && index < static_cast<int>(sub_views_tabbed_ids_.size()));
    if (tabbed_pane_) {
        tabbed_pane_->SelectTabAt(sub_views_tabbed_ids_[index]);
    }
}

void SettingsFrameView::NavigationBarSelectedAt(int strip_id)
{
    if (strip_id == room_manager_id_)
    {

    }
    else if (strip_id == download_tap_id_) {
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SettingDowanloadClick, "");
    }
}

void SettingsFrameView::ProcessPositive() {
    for (size_t i = 0; i < sub_views_.size(); ++i) {
        if (!sub_views_[i]->CheckSettingsValid()) {
            tabbed_pane_->SelectTabAt(sub_views_tabbed_ids_[i]);
            return;
        }
    }
    trigger_close_ = true;
    presenter_->SettingChanged();
    presenter_->RequestLivehimeEvent();

    saved_changes_ = true;
    GetWidget()->Close();
}

void SettingsFrameView::ProcessNegative()
{
    for (size_t i = 0; i < sub_views_.size(); ++i)
    {
        sub_views_[i]->OnCancel();
    }

    trigger_close_ = true;
    GetWidget()->Close();
}

bool SettingsFrameView::ShowRestreamingDialog() {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    base::string16 restream = rb.GetLocalizedString(IDS_MESSAGEBOX_BUTTON_RESTREAM);
    base::string16 btns;
    btns.append(restream);
    btns.append(L",");
    btns.append(rb.GetLocalizedString(IDS_CANCEL));

    base::string16 ret = livehime::ShowModalMessageBox(
        GetWidget()->GetNativeView(),
        rb.GetLocalizedString(IDS_MESSAGEBOX_OPER_TITLE),
        rb.GetLocalizedString(IDS_MESSAGEBOX_OPER_CONTENTS),
        btns);
    return ret == restream;
}

bool SettingsFrameView::CheckExclusive() {
    return true;//livehime::exclusive::settings::CheckAtSSChangeConfirm(GetWidget());
}

void SettingsFrameView::OnShowRestreamingDialog(const base::string16& result, void* data)
{
    DCHECK(data);
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    base::string16 restream = rb.GetLocalizedString(IDS_MESSAGEBOX_BUTTON_RESTREAM);
    if (result == restream)
    {
        scoped_ptr<base::Closure> enddialog_closure((base::Closure*)data);
        enddialog_closure->Run();
    }
}

void SettingsFrameView::SaveOrCheckStreamingSettingsChange(
    bool check, BaseSettingsView::ChangeType &result) {
    for (auto& sub_view : sub_views_) {
        sub_view->SaveOrCheckStreamingSettingsChange(check, result);
    }
}

void SettingsFrameView::SaveNormalSettingsChange() {
    for (auto& sub_view : sub_views_) {
        sub_view->SaveNormalSettingsChange();
    }
}

void SettingsFrameView::ShowLiveReplaySetting(bool publish,bool archive)
{
    GlobalSettingsView* global_view = (GlobalSettingsView*)sub_views_[global_index_];
    if (global_view)
    {
        global_view->ShowLiveReplaySetting(publish, archive);
    }

}