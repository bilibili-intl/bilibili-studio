#include "bililive/bililive/ui/views/viddup/toolbar/toolbar_viddup_view.h"

#include <regex>
#include <shellapi.h>
#include <tlhelp32.h>

#include "base/ext/callable_callback.h"
#include "base/rand_util.h"
#include "base/ext/bind_lambda.h"
#include "base/json/json_writer.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/view.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"

#include "bilibase/basic_types.h"
#include "bilibase/scope_guard.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/kv/vanish_ul_controller.h"
#include "bililive/bililive/livehime/notify_ui_control/notify_ui_controller.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/livehime/toolbar/toolbar_presenter_impl.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/notice/notice_tips_view.h"
#include "bililive/bililive/ui/views/livehime/settings/settings_frame_view.h"
#include "bililive/bililive/ui/views/livehime/tips_dialog/fleet_achievement_dialog.h"
#include "bililive/bililive/ui/views/toolbar/toolbar_volume_ctrl.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/log_ext/log_constants.h"
#include <bililive/public/bililive/bililive_thread.h>

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/bililive/ui/views/livehime/sing_identify_notice/sing_identify_notice_dialog.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"
#include "bililive/common/bililive_context.h"

#include "bililive/bililive/ui/views/viddup/toolbar/open_live_viddup_view.h"

namespace {

    // The sei inserts the frame tag, which is currently fixed and spliced directly in front of the service key
    const char kLiveSeiChannel[] = "LIVE_SEI_CHANNEL";

    // Sei insert frame key, used for cargo video stream optimization
    const char kLiveSeiInsertKey[] = "LIVE_SHOPPING.unique_id";

    using bililive::OBSStatusDelegate;

    enum ToolbarCtrlID
    {
        Button_Preview,
        Button_SoundEffect,
        Button_Record,
        Button_Live,
    };

    const int kLiveTrackingTimeSpanInMin = 5;
    const int kSeparatorLineThickness = GetLengthByDPIScale(2);

    void LivehimeSourceEventTracking() {
        auto obs_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
        obs_proxy::Scene *scene = obs_proxy->GetCurrentSceneCollection()->current_scene();

        std::vector<std::string> vec_source;

        for (auto item : scene->GetItems()) {
            auto type = (*item).type();
            if (type == obs_proxy::SceneItemType::VideoCaptureDevice) vec_source.push_back("a");
            if (type == obs_proxy::SceneItemType::GameCapture) vec_source.push_back("b");
            if (type == obs_proxy::SceneItemType::WindowCapture) vec_source.push_back("c");
            if (type == obs_proxy::SceneItemType::DisplayCapture) vec_source.push_back("d");
            if (type == obs_proxy::SceneItemType::MediaSource) vec_source.push_back("e");
            if (type == obs_proxy::SceneItemType::Image) vec_source.push_back("f");
            if (type == obs_proxy::SceneItemType::Text) vec_source.push_back("g");
            if (type == obs_proxy::SceneItemType::Slider) vec_source.push_back("i");
            if (type == obs_proxy::SceneItemType::ColorSource) vec_source.push_back("colorsource");
            if (type == obs_proxy::SceneItemType::ReceiverSource) vec_source.push_back("receiversource");
        }

        std::string event_msg;

        if (vec_source.empty()) {
            event_msg.append("h");
        } else {
            std::string fields = JoinString(vec_source, ";");
            event_msg.append(fields);
        }

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeSource, secret_core->account_info().mid(), event_msg).Call();
    }

    int GetResolutionLevel(const std::string &resolution)
    {
        int ret = 0, width = 0, height = 0, useless = 0;
        char c = 0;

        if (sscanf(resolution.c_str(), "%d%c%d%c", &width, &c, &height, (char*)&useless) == 3)
        {
            DCHECK(c == 'x');

            ret = 1;
            std::vector<int> range = { 480, 720, 1080 };
            for (auto range_item : range)
            {
                if (height < range_item)
                {
                    return ret;
                }
                ++ret;
            }

            return ret;
        }

        return ret;
    }

    void LivehimeCameraEventTracking(bool just_log = false)
    {
        auto obs_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
        obs_proxy::Scene *scene = obs_proxy->GetCurrentSceneCollection()->current_scene();
        auto scene_items = scene->GetItems();

        auto current_item = std::find_if(scene_items.begin(), scene_items.end(), [](obs_proxy::SceneItem* item)
        {
            return item->type() == obs_proxy::SceneItemType::VideoCaptureDevice ? true : false;
        });

        if (current_item == scene_items.end())
        {
            return;
        }

        CameraSceneItemHelper camera_scene(*current_item);

        if (just_log)
        {
            LOG(INFO) << app_log::kLogBeautyStart << camera_scene.to_string();
            return;
        }

        std::string event_msg;
        std::vector<std::string> vec_source;

        int resolution_level = GetResolutionLevel(camera_scene.SelectedResolution());
        vec_source.push_back(base::StringPrintf("resolution:%d", resolution_level));

        std::string fields = JoinString(vec_source, ";");

        event_msg.append(fields);

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeCameraProperties, secret_core->account_info().mid(), event_msg).Call();

        std::vector<std::string> makeup_type;
        livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::CameraBeauty,
            ("makeup_type=" + JoinString(makeup_type, "x")));
    }

    void StreamingStartedEvent() {
        LivehimeSourceEventTracking();
        LivehimeCameraEventTracking();
    }

    void RecordingEventTracking(const base::FilePath& video_path,
        obs_proxy::RecordingErrorCode error_code) {

        auto code = bilibase::enum_cast(error_code);
        std::string msg = base::StringPrintf("code:%d", code);

        auto secret_core = GetBililiveProcess()->secret_core();
        auto mid = secret_core->account_info().mid();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeRecordError, mid, msg).Call();

        LOG(INFO) << app_log::kLogRecordScreenFail << "invoker:error; code:" << code;
    }

    void RecordBehaviorEventReport(bool open, int64 ts)
    {
        int fmt = secret::kRequireValueDummyInt;
        std::string format = GetBililiveProcess()->profile()->GetPrefs()->GetString(prefs::kOutputRecordingFormat);
        if (format == prefs::kRecordingFormatFLV)
        {
            fmt = 1;
        }
        else if (format == prefs::kRecordingFormatMP4)
        {
            fmt = 3;
        }
        else if (format == prefs::kRecordingFormatFMP4)
        {
            fmt = 2;
        }

        std::vector<std::string> fields;
        fields.push_back(base::StringPrintf("result:%s", open ? "open" : "close"));
        fields.push_back(base::StringPrintf("time:%lld", ts));
        fields.push_back(base::StringPrintf("format:%d", fmt));
        livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::LiveRecord, JoinString(fields, ";"));

        LOG(INFO) << (open ? app_log::kLogRecordScreenStart : app_log::kLogRecordScreenStop) << "invoker:user";
    }

    void LiveBehaviorEventReport(bool open,const WebLinkInfo& report_info)
    {
        auto pf = GetBililiveProcess()->profile()->GetPrefs();
        bool auto_br = pf->GetBoolean(prefs::kAutoVideoSwitch);
        int code = pf->GetInteger(prefs::kVideoBitRate);
        std::string rate = pf->GetString(prefs::kVideoBitRateControl);
        std::string fps = pf->GetString(prefs::kVideoFPSCommon);
        int ratio = secret::kRequireValueDummyInt;
        LONG ratio_xy = MAKELONG(pf->GetInteger(prefs::kVideoOutputCX), pf->GetInteger(prefs::kVideoOutputCY));
        switch (ratio_xy)
        {
        case MAKELONG(712, 400):
            ratio = 1;
            break;
        case MAKELONG(1280, 720):
            ratio = 2;
            break;
        case MAKELONG(1920, 1080):
            ratio = 3;
            break;
        default:
            break;
        }
    }

    base::string16 LocalStr(int res_id) {
        return ui::ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
    }

    void OnVerifyFailDialog(const string16& btn, void* data) {
        DCHECK(data);
        scoped_ptr<std::wstring> button_url(static_cast<std::wstring*>(data));

        if (btn != LocalStr(IDS_LIVE_ROOM_MSGBOX_IKNOW)) {
            ShellExecuteW(nullptr, L"open", bililive::FastForwardChangeEnv(*button_url).c_str(), nullptr, nullptr, SW_SHOW);
        }
    }

    // Start/Record button
    class ToolBarRecordLiveButton : public views::View
    {
        class RecordLiveButton : public LivehimeLabelButton
        {
        public:
            RecordLiveButton(views::ButtonListener* listener, const string16& text, bool positive)
                : LivehimeLabelButton(listener, text,
                    positive ? LivehimeButtonStyle_V4_LiveButton : LivehimeButtonStyle_V4_RecordButton)
                , is_on_hover_(false)
                , is_starting_(false)
            {
                record_time_ = L"  00:00:00";
                if (positive)
                {
                    mouse_entered_image_ = new gfx::ImageSkia();
                    mouse_entered_text_ = LocalStr(IDS_TOOLBAR_STOP_LIVE);
                    mouse_exited_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_LIVE_END);
                    not_starting_text_ = LocalStr(IDS_TOOLBAR_START_LIVE);
                    not_starting_image_ = new gfx::ImageSkia();
                }
                else
                {
                    mouse_entered_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_RECORD_RED);
                    mouse_entered_text_ = LocalStr(IDS_TOOLBAR_STOP_RECORDING);
                    mouse_exited_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_RECORD_TIME);
                    not_starting_text_ = LocalStr(IDS_TOOLBAR_START_RECORDING);
                    not_starting_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_RECORD);
                }
            }

            void OnPaint(gfx::Canvas* canvas) override
            {
                __super::OnPaint(canvas);

                if (is_starting_ && !is_on_hover_)
                {
                    gfx::Rect rect = this->bounds();
                    canvas->DrawLine(gfx::Point(rect.width()*0.32, rect.height()*0.3),
                        gfx::Point(rect.width()*0.32, rect.height()*0.7), SkColorSetRGB(0x0e, 0xbe, 0xff));
                }
            }

            void OnMouseEntered(const ui::MouseEvent& event) override
            {
                is_on_hover_ = true;
                if (is_starting_)
                {
                    SetAllStateImage(*mouse_entered_image_);
                    SetText(mouse_entered_text_);
                }

                __super::OnMouseEntered(event);
            }

            void OnMouseExited(const ui::MouseEvent& event) override
            {
                is_on_hover_ = false;
                if (is_starting_)
                {
                    SetAllStateImage(*mouse_exited_image_);
                    SetText(record_time_);
                }

                __super::OnMouseExited(event);
            }

            void SetRecordOrLiveTime(const base::string16 &record_time)
            {
                record_time_ = record_time;
                if (!is_starting_)
                    return;
                if (!is_on_hover_)
                {
                    SetText(record_time_);
                }
            }

            void SetStartingStatus(bool running)
            {
                is_starting_ = running;
            }

            void SetFirstButtonStatus()
            {
                if (this->IsMouseHovered())
                {
                    is_on_hover_ = true;
                }
                else
                {
                    is_on_hover_ = false;
                }
            }

            void SetButtonStatus()
            {
                if (is_starting_)
                {
                    if (is_on_hover_)
                    {
                        SetAllStateImage(*mouse_entered_image_);
                        SetText(mouse_entered_text_);
                    }
                    else
                    {
                        SetAllStateImage(*mouse_exited_image_);
                        SetText(record_time_);
                    }
                }
                else
                {
                    SetAllStateImage(*not_starting_image_);
                    SetText(not_starting_text_);
                    record_time_ = L"  00:00:00";
                }
            }

        private:
            bool is_on_hover_;
            base::string16 record_time_;
            bool is_starting_;
            gfx::ImageSkia*  mouse_entered_image_;
            gfx::ImageSkia*  mouse_exited_image_;
            gfx::ImageSkia*  not_starting_image_;
            base::string16   mouse_entered_text_;
            base::string16   not_starting_text_;
        };

    public:
        ToolBarRecordLiveButton(views::ButtonListener *listen, const base::string16 &str, bool record)
            : start_event_(false)
            , inter_call_(false)
            , is_record_bt_(record)
        {
            button_ = new RecordLiveButton(listen, str, !record);
            button_->SetPreferredSize(gfx::Size(kRecordButtonWidth, kRecordButtonHeight));
            SetRecordStatus();

            button_->set_focusable(false);

            SetLayoutManager(new views::FillLayout());
            AddChildView(button_);
        }

        virtual ~ToolBarRecordLiveButton() = default;

        void StartTimer()
        {
            if (!timer_.IsRunning())
            {
                InitAnchorTaskSchedule();
                start_event_ = true;
                inter_call_ = false;
                time_ = base::Time::Now();
                time_.LocalExplode(&across_the_day_time_);
                timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(1), this, &ToolBarRecordLiveButton::OnTimer);
            }

            button_->SetFirstButtonStatus();
            SetRecordStatus();
        };

        void StopTimer()
        {
            if (timer_.IsRunning())
            {
                timer_.Stop();
            }

            SetRecordStatus();
        };

        void SetTimeCallBack(base::Callback<void(bool)> time_num_notify)
        {
            time_num_notify_ = time_num_notify;
        };

        void SetText(const base::string16 &text)
        {
            button_->SetText(text);
        }

        const base::string16& GetText() const
        {
            return button_->GetText();
        }

        void set_id(int id)
        {
            button_->set_id(id);
        };

        views::View* button() { return button_; }

        bililive::TimeSpan time_delta() const {
            bililive::TimeSpan delta(base::Time::Now() - time_);
            return delta;
        }

    protected:
        // View
        void OnEnabledChanged() override
        {
            button_->SetEnabled(enabled());
            __super::OnEnabledChanged();
        }

    private:
        void OnTimer()
        {
            base::Time cur_time = base::Time::Now();
            bililive::TimeSpan delta(cur_time - time_);

            if (start_event_) {
                if (delta.GetMinutes() >= kLiveTrackingTimeSpanInMin) {
                    StreamingStartedEvent();
                    start_event_ = false;
                }
            }

            base::string16 record_time = base::StringPrintf(L"  %02d:%02d:%02d",
                static_cast<LONG>(delta.GetTotalHours()), delta.GetMinutes(), delta.GetSeconds());
            button_->SetRecordOrLiveTime(record_time);
            button_->SetStartingStatus(timer_.IsRunning());
            button_->SetButtonStatus();

            if (!time_num_notify_.is_null())
            {
                if (300 <= delta.GetTotalSeconds() && !inter_call_)
                {
                    inter_call_ = true;
                    time_num_notify_.Run(true);
                }
            }

            if (!is_record_bt_)
            {
                ProcessAnchorTaskSchedule(delta.GetTotalSeconds());
                base::Time::Exploded cur_epl;
                cur_time.LocalExplode(&cur_epl);

                if ((cur_epl.month != across_the_day_time_.month) || (cur_epl.day_of_month != across_the_day_time_.day_of_month))
                {
                    across_the_day_time_ = cur_epl;
                    base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                        base::Bind(livehime::BehaviorEventReportViaServer, secret::LivehimeViaServerBehaviorEvent::LiveAcrossTheDay, ""),
                        base::TimeDelta::FromSeconds(base::RandInt(1, 20)));
                }
            }
        }

        void SetRecordStatus()
        {
            button_->SetStartingStatus(timer_.IsRunning());
            button_->SetButtonStatus();
        }

        // To improve the efficiency, improve the query efficiency
        void InitAnchorTaskSchedule()
        {
            schedule_live_time_sec_list_.clear();
            const std::vector<secret::AnchorInfo::TaskScheduleInfo>& anchor_task_schedule_list =
                GetBililiveProcess()->secret_core()->anchor_info().get_anchor_task_schedule_list();
            for (auto& it : anchor_task_schedule_list)
            {
                if (it.touch_type == static_cast<int>(secret::AnchorInfo::TaskScheduleInfo::TouchType::Begin_Live))
                {
                    schedule_live_time_sec_list_.push_back(it.live_time_sec);
                }
            }
        }

        void ProcessAnchorTaskSchedule(long long pass_second)
        {
            if (pass_second == 5)
            {
                InitAnchorTaskSchedule();
            }

            if (schedule_live_time_sec_list_.empty())
            {
                return;
            }

            std::vector<int>::iterator it = std::find(schedule_live_time_sec_list_.begin(), schedule_live_time_sec_list_.end(), pass_second);
            if (it == schedule_live_time_sec_list_.end())
            {
                return;
            }
            else
            {
                schedule_live_time_sec_list_.erase(it);
            }

            const std::vector<secret::AnchorInfo::TaskScheduleInfo>& anchor_task_schedule_list =
                GetBililiveProcess()->secret_core()->anchor_info().get_anchor_task_schedule_list();
            for (auto &it : anchor_task_schedule_list)
            {
                if (it.touch_type == static_cast<int>(secret::AnchorInfo::TaskScheduleInfo::TouchType::Begin_Live)
                    && it.live_time_sec == pass_second)
                {
                        break;
                }
            }
        }

    private:
        RecordLiveButton *button_ = nullptr;
        base::RepeatingTimer<ToolBarRecordLiveButton> timer_;
        base::Time time_;
        base::Time::Exploded across_the_day_time_; 

        bool start_event_ = false;

        base::Callback<void(bool)> time_num_notify_;
        bool inter_call_ = false;

        bool is_record_bt_;
        std::vector<int> schedule_live_time_sec_list_;//
        DISALLOW_COPY_AND_ASSIGN(ToolBarRecordLiveButton);
    };

    class ToolBarEntrancesManagerButton
        : public BililiveLabelButton
    {
    public:
        ToolBarEntrancesManagerButton(views::ButtonListener* listener, const string16& text)
            : BililiveLabelButton(listener, text)
        {
            SetStyle(LivehimeButtonStyle::LivehimeButtonStyle_FunctionButton);
            set_round_corner(false);
            SetFont(ftPrimary);
        }

        void ShowRedpoint(bool show)
        {
            if (show)
            {

            }

            if (show_redpoint_ != show)
            {
                show_redpoint_ = show;
                SchedulePaint();
            }
        }

    protected:
        // View
        void PaintChildren(gfx::Canvas* canvas) override
        {
            __super::PaintChildren(canvas);

            if (show_redpoint_)
            {
                static const int kRadius = GetLengthByDPIScale(5);
                static const int kPadding = GetLengthByDPIScale(2);
                gfx::Point point(image_view()->bounds().x() + kRadius, image_view()->bounds().y() + kRadius);

                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(SK_ColorWHITE);
                canvas->DrawCircle(point, kRadius, paint);
                paint.setColor(SK_ColorRED);
                canvas->DrawCircle(point, kRadius - kPadding, paint);
            }
        }

        // Button
        void NotifyClick(const ui::Event& event) override
        {
            __super::NotifyClick(event);
            ShowRedpoint(false);
        }

    private:
        bool show_redpoint_ = false;
    };

    class EncoderErrorBubbleContentView
        : public views::View,
        views::ButtonListener
    {
        enum
        {
            HardwareBtn = 1,
            SoftwareBtn,
        };

    public:
        EncoderErrorBubbleContentView(const base::string16& text)
        {
            SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, kPaddingRowHeightForCtrlTips));

            LivehimeContentLabel* label = new LivehimeContentLabel(text);
            label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
            label->SetMultiLine(true);
            AddChildView(label);

            views::View* help_view = new views::View();
            {
                help_view->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal, 0, 0, kPaddingColWidthForCtrlTips));

                LivehimeContentLabel* label = new LivehimeContentLabel(GetLocalizedString(IDS_HELP_VIEW_HELP) + L": ");

                LivehimeLinkButton* hardware_btn = new LivehimeLinkButton(this, GetLocalizedString(IDS_STREAMING_SETTING_HARDWARE_BTN_LABEL));
                hardware_btn->SetAllStateTextColor(clrLinkButtonPressed);
                hardware_btn->set_id(HardwareBtn);

                LivehimeLinkButton* software_btn = new LivehimeLinkButton(this, GetLocalizedString(IDS_STREAMING_SETTING_SOFTWARE_BTN_LABEL));
                software_btn->SetAllStateTextColor(clrLinkButtonPressed);
                software_btn->set_id(SoftwareBtn);

                help_view->AddChildView(label);
                help_view->AddChildView(hardware_btn);
                help_view->AddChildView(software_btn);
            }

            AddChildView(help_view);
        }

        ~EncoderErrorBubbleContentView() = default;

    protected:
        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            switch (sender->id())
            {
            case HardwareBtn:
                livehime::ShowHelp(livehime::HelpType::HardwareEncoderError);
                break;
            case SoftwareBtn:
                livehime::ShowHelp(livehime::HelpType::SoftwareEncoderError);
                break;
            default:
                break;
            }
        }
    };

    class CpmBubbleController : public views::WidgetObserver
    {
        struct CpmNotice
        {
            CpmNotice(const base::string16& str, int show_time)
                : text(str)
                , show_time_in_sec(show_time)
            { }

            base::string16 text;
            int show_time_in_sec = 0;
        };

    public:
        CpmBubbleController(views::View* dymn_view)
            : dymn_view_(dymn_view)
            , weakptr_factory_(this)
        {
        }

        virtual ~CpmBubbleController()
        {
            DCHECK(!bubble_view_);
            if (bubble_view_ && bubble_view_->GetWidget())
            {
                bubble_view_->GetWidget()->RemoveObserver(this);
                bubble_view_ = nullptr;
            }
        }

        void AddNotice(const base::string16& text, int show_time_in_sec)
        {
            notices_.push_back({ text , show_time_in_sec});

            if (!bubble_view_ && !task_pending_)
            {
                task_pending_ = true;
                base::MessageLoop::current()->PostTask(FROM_HERE,
                    base::Bind(&CpmBubbleController::ShowBubble, weakptr_factory_.GetWeakPtr()));
            }
        }

        void Clear()
        {
            notices_.clear();

            if (bubble_view_ && bubble_view_->GetWidget())
            {
                bubble_view_->GetWidget()->Close();
            }
        }

        views::View* IsCpmEntranceOutside() const
        {
            views::View* cpm_btn = nullptr;

            return cpm_btn;
        }

    protected:
        // WidgetObserver
        void OnWidgetDestroyed(views::Widget* widget) override
        {
            bubble_view_ = nullptr;
            if (!notices_.empty())
            {
                task_pending_ = true;
                base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                    base::Bind(&CpmBubbleController::ShowBubble, weakptr_factory_.GetWeakPtr()),
                    base::TimeDelta::FromSeconds(2));
            }
        }

    private:
        void ShowBubble()
        {
            task_pending_ = false;

            views::View* entrance_view = IsCpmEntranceOutside();
            if (entrance_view)
            {
                if (!notices_.empty())
                {
                    CpmNotice notice = notices_.front();
                    notices_.pop_front();

                    bubble_view_ = livehime::ShowBubble(entrance_view, views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                        notice.text);
                    bubble_view_->set_text_color(SK_ColorWHITE);
                    bubble_view_->set_background_color(GetColor(Theme));
                    bubble_view_->StartFade(false, std::max(1, notice.show_time_in_sec - 1) * 1000, 1000);
                    DCHECK(bubble_view_->GetWidget());
                    if (bubble_view_->GetWidget())
                    {
                        bubble_view_->GetWidget()->AddObserver(this);
                    }
                }
            }
            else
            {
                Clear();
            }
        }

    private:
        views::View* dymn_view_ = nullptr;
        std::list<CpmNotice> notices_;
        BililiveBubbleView* bubble_view_ = nullptr;
        bool task_pending_ = false;
        base::WeakPtrFactory<CpmBubbleController> weakptr_factory_;
    };

    void SetTag() {
    }

    void ReportLiveDuration(LONGLONG minutes)
    {
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeLiveDuration,
            secret_core->account_info().mid(),
            std::to_string(minutes)).Call();
    }

    void ReportDanmakuHimeState()
    {
    }

    void ReportSetTags()
    {
        auto tag_id = GetBililiveProcess()->secret_core()->anchor_info().tag_id();
        auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
        auto mid = GetBililiveProcess()->secret_core()->account_info().mid();

        std::vector<std::string> vec_taginfo;
        vec_taginfo.push_back(base::StringPrintf("result:%lld", tag_id));
        vec_taginfo.push_back(base::StringPrintf("code:%lld", room_id));
        std::string taginfo;
        taginfo.append(JoinString(vec_taginfo, ";"));

        GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeSetTags, mid, taginfo).Call();
    }

    void ReportLogWhenStartLiveFailed()
    {
        static base::Time ti;
        base::TimeDelta dt = base::Time::Now() - ti;
        if (dt.InSeconds() >= 30)
        {
            livehime::UploadLogFilesAndReport();
            ti = base::Time::Now();
        }
    }

    const int kExpireDayCount = 5;
    bool CalcExpireTime(int64 current_time, int* expire_date) {
        bool is_expire = false;

        auto time = base::Time::FromTimeT(current_time);
        base::Time::Exploded exploded;
        time.LocalExplode(&exploded);

        switch (exploded.month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
        {
            if ((31 - exploded.day_of_month) < kExpireDayCount) {
                is_expire = true;
            }
        }
        break;
        case 4:
        case 6:
        case 9:
        case 11:
        {
            if ((30 - exploded.day_of_month) < kExpireDayCount) {
                is_expire = true;
            }
        }
        break;
        case 2:
        {
            int amount_day = (exploded.year % 400 == 0 || (exploded.year % 4 == 0 && exploded.year % 100 != 0)) ? 29 : 28;
            if ((amount_day - exploded.day_of_month) < kExpireDayCount) {
                is_expire = true;
            }
        }
        break;
        default:
        break;
        }

        *expire_date = exploded.year * 100 + exploded.month;
        return is_expire;
    }
}   // namespace

ToolbarViddupView::ToolbarViddupView()
    : weakptr_factory_(this)
{
}

ToolbarViddupView::~ToolbarViddupView()
{
    EndAnchorEcommerceStatusLoop();
}

void ToolbarViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
            InitData();

            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_LOADING_ROOMINFO,
                base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_FAILED,
                base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_DISABLE_START_LIVE,
                base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_ENABLE_START_LIVE,
                base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED,
                base::NotificationService::AllSources());
            profile_pref_registrar_.Init(GetBililiveProcess()->profile()->GetPrefs());

            OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);
            GetBililiveProcess()->bililive_obs()->live_room_controller()->AddObserver(this);

        }
        else
        {
            notifation_register_.RemoveAll();
            if (OBSProxyService::GetInstance().obs_ui_proxy())
            {
                OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
            }

            obsmonitor_ = nullptr;
            GetBililiveProcess()->bililive_obs()->live_room_controller()->RemoveObserver(this);
        }
    }
}

void ToolbarViddupView::InitViews()
{
    set_background(views::Background::CreateSolidBackground(GetColor(WindowTitle)));
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);  // left
    column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);  // center
    column_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);  // trail(placeholder)
    
    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(3);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    layout->AddPaddingRow(0, GetLengthByDPIScale(8));

    system_volume_ctrl_ = new ToolBarVolumeCtrl(VolumeCtrlType::System);

    mic_volume_ctrl_ = new ToolBarVolumeCtrl(VolumeCtrlType::Mic);

    sound_effect_button_ = new BililiveImageButton(this);
    sound_effect_button_->SetTooltipText(GetLocalizedString(IDS_TOOLBAR_VOLUME_SOUND_EFFECT));
    sound_effect_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_EFFECT));
    sound_effect_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_EFFECT_HV));
    sound_effect_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_EFFECT_HV));
    sound_effect_button_->SetImage(views::Button::STATE_DISABLED, GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_EFFECT_DIS));
    sound_effect_button_->set_id(Button_SoundEffect);
    sound_effect_button_->SetEnabled(false);

    live_btn_ = new ToolBarRecordLiveButton(this, GetLocalizedString(IDS_TOOLBAR_START_LIVE), false);
    live_btn_->set_id(Button_Live);

    views::View* sys_render_warn_view = new View();
    sys_render_warn_view->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal, 0, 0, kPaddingColWidthForCtrlTips));
    sys_render_warn_tip_ = new LivehimeHoverTipButton(GetLocalizedString(IDS_TOOLBAR_SYSTEM_VOICE_WARN),
        L"",
        views::BubbleBorder::Arrow::TOP_RIGHT);
    sys_render_warn_tip_->SetVisible(false);
    sys_render_warn_tip_->SetButtonImage(
        GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_ERROR),
        GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_ERROR));
    sys_render_warn_view->AddChildView(sys_render_warn_tip_);

    auto leadingView = new views::View();
    views::GridLayout* leading_layout = new views::GridLayout(leadingView);
    leadingView->SetLayoutManager(leading_layout);

    column_set = leading_layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0); 
    //column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);  
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0); 
    //column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);

    leading_layout->StartRow(0, 1);
    leading_layout->AddView(sys_render_warn_view);
    leading_layout->AddView(sound_effect_button_);
    leading_layout->AddView(mic_volume_ctrl_);
    leading_layout->AddView(system_volume_ctrl_);

    layout->StartRow(0, 1);
    layout->AddView(leadingView);
    layout->AddView(live_btn_);

    layout->AddPaddingRow(0, GetLengthByDPIScale(8));

    obsmonitor_ = std::make_unique<bililive::OBSStatusMonitor>(this);
}

void ToolbarViddupView::InitData()
{
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::String,
        L"nahimic32.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::String,
        L"AwSoundCenterSvc32.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::String,
        L"ss3svc32.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::String,
        L"nhAsusStrixSvc32.exe")); 
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::Regex,
        L"Nahimic(\\d*|VR)Svc32\\.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::Regex,
        L"Kraken\\d{4}Helper\\.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::Regex,
        L"ElectraV\\d*Helper\\.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::String,
        L"SS2Svc32.exe"));
    process_vector_.push_back(std::pair<int, base::string16>(secret::AppService::DynamicAudioRenderType::String,
        L"NahimicAPISvc32.exe"));

    LivehimeLiveRoomController::GetInstance()->GetBookLiveTimeInfos();
    GetLiveDelayMeasureTime();
}

gfx::Size ToolbarViddupView::GetPreferredSize()
{
    return __super::GetPreferredSize();
    gfx::Size pref_size = __super::GetPreferredSize();
    //pref_size.set_width(std::max(pref_size.width(), GetLengthByDPIScale(900)));
    pref_size.set_height(std::max(pref_size.height(), GetLengthByDPIScale(180)));
    return pref_size;
}

void ToolbarViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    __super::OnPaintBackground(canvas);

    {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(GetColor(WindowTitle));
        paint.setStyle(SkPaint::kFill_Style);
        canvas->DrawRoundRect(GetContentsBounds(), GetLengthByDPIScale(8), paint);
    }

    static base::string16 sys_warn_text = GetLocalizedString(IDS_TOOLBAR_VOLUME_SYSTEM_WARNING);
    static base::string16 mic_warn_text = GetLocalizedString(IDS_TOOLBAR_VOLUME_MIC_WARNING);

    auto warn_draw_func = [&](const base::string16& text, ToolBarVolumeCtrl* vol_ctrl)
    {
        gfx::Rect vol_rect = vol_ctrl->ConvertRectToParent(vol_ctrl->GetVolumeRect());
        canvas->DrawStringInt(text, ftTwelve, GetColor(TextWarning),
                              vol_rect.x() + 2, GetContentsBounds().height() - ftTwelve.GetHeight()/*vol_rect.bottom() - 4*/, vol_rect.width(), ftTwelve.GetHeight(),
                              gfx::Canvas::TEXT_ALIGN_LEFT);
    };

    if(show_sys_warn_)
    {
        warn_draw_func(sys_warn_text, system_volume_ctrl_);
    }

    if(show_mic_warn_)
    {
        warn_draw_func(mic_warn_text, mic_volume_ctrl_);
    }
}

void ToolbarViddupView::OnTimeNotify(bool book_hide)
{
    LivehimeLiveRoomController::GetInstance()->HideBookLiveTime();
}

void ToolbarViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    int id = sender->id();
    switch (id)
    {
    case Button_SoundEffect:
        {
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SHOW_SOUND_EFFECT);
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::ToolbarItemClick, "button_type:3;result:open");
        }
        break;
    case Button_Record:
        {
            bool start_record = !bililive::OutputController::GetInstance()->IsRecording();
            RecordBehaviorEventReport(start_record, start_record ? 0 :
                record_btn_->time_delta().GetTotalMilliseconds());

            if (bililive::OutputController::GetInstance()->IsRecording())
            {
                bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_STOP_RECORDING);
            }
            else if (bililive::OutputController::GetInstance()->IsRecordingReady())
            {
                bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_START_RECORDING);
            }

            string state;
            if (start_record)
            {
                state = "button_state:1;";
            }
            else
            {
                state = "button_state:2;";
            }

            bool is_live = bililive::OutputController::GetInstance()->IsStreaming();
            if (is_live)
            {
                state.append("anchor:1");
            }
            else
            {
                state.append("anchor:2");
            }
            
            livehime::PolarisEventReport(
                secret::LivehimePolarisBehaviorEvent::RecordClick, state);
        }
        break;
    case Button_Live:
        {
            LiveStreamButtonClick();
            GetBililiveProcess()->bililive_obs()->obs_view()->RequestCoverImage();
        }
        break;
    case Button_Preview:
    {
        bool show = preview_switch_btn_->tag() == 0;
        if (show)
        {
            preview_switch_btn_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_ACTIVITY_DANMAKU_ON));
            preview_switch_btn_->set_tag(1);
        }
        else
        {
            preview_switch_btn_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_ACTIVITY_DANMAKU_OFF));
            preview_switch_btn_->set_tag(0);
        }

        bool landscape = LiveModelController::GetInstance()->IsLandscapeModel();
        ShowFauxAudientEffect(show, landscape);

        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::PreviewBtnClick,
            base::StringPrintf("result:%s", show ? "open" : "close"));

        break;
    }
    default:
        break;
    }
}
void ToolbarViddupView::LiveStreamButtonClick() {
    auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
    if (room_id <= -1)
    {
        livehime::ShowOpenLiveDialog(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow());
        return;
    }

    WebLinkInfo report_info;
    report_info.source_event = source_event_;
    report_info.resource_id = resource_id_;
    report_info.game_id = game_id_;
    report_info.virtual_id = virtual_id_;
    report_info.function_type = function_type_;

    bool start_live = LivehimeLiveRoomController::GetInstance()->IsLiveReady();
    LiveBehaviorEventReport(start_live, report_info);

    if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
    {
        LOG(INFO) << "[UI-tbv] click start live button.";
        more_five_min_ = false;
        bool hotkey = false;
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_START_LIVE_STREAMING,
            CommandParams<bool>(&hotkey));
    }
    else
    {
        StopLiveStream();
    }
}

void ToolbarViddupView::SetTheRightTextOfLiveBtn(const base::string16& text)
{
    static base::string16 require_str = GetLocalizedString(IDS_REALNAME_AUTH);
    auto& anchor_info = GetBililiveProcess()->secret_core()->anchor_info();
    if ((anchor_info.identify_status() != 1) /*|| !anchor_info.is_adult()*/)
    {
        live_btn_->SetText(require_str);
    }
    else
    {
        //if (live_btn_->GetText() == require_str)
        {
            live_btn_->SetText(text);
        }
    }
}

void ToolbarViddupView::ShowWarningBubble(views::View* view, const base::string16& text)
{
    BililiveBubbleView* bubble = livehime::ShowBubble(view, views::BubbleBorder::Arrow::BOTTOM_RIGHT,
        text);
    bubble->set_background_color(GetColor(TextWarning));
    bubble->SetAlignment(views::BubbleBorder::BubbleAlignment::ALIGN_EDGE_TO_ANCHOR_EDGE);
    bubble->set_text_color(SK_ColorWHITE);
    bubble->StartFade(false, 2000, 1000);
}

void ToolbarViddupView::OnRealNameAuthTimer()
{
    GetBililiveProcess()->bililive_obs()->user_info_service()->GetRoomInfo();
}

void ToolbarViddupView::OnRequestGetAnchorInnerOperatingIdRes(bool valid_response, int code, const std::string& unique_id) {
    std::unique_ptr<base::DictionaryValue> fields(new base::DictionaryValue());
    fields->SetString(kLiveSeiInsertKey, unique_id);
    std::string data;
    base::JSONWriter::Write(fields.get(), &data);
    data = kLiveSeiChannel + data;
    if (LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
    {
        if (livehime::ThirdPartyOutputController::GetInstance())
        {
            livehime::ThirdPartyOutputController::GetInstance()->AddSEIInfo((unsigned char*)data.c_str(), data.size());
        }
    }
    else
    {
        OBSProxyService::GetInstance().GetOBSCoreProxy()->GetOBSOutputService()->AddSEIInfo((unsigned char*)data.c_str(), data.size());
    }
}

void ToolbarViddupView::OnRequestGetShoppingCartStatusRes(bool valid_response, int code, const int& status) {
    LOG(INFO) << "shopping cart status=" << status;
    if (status == 1) {
        StartAnchorEcommerceStatusLoop();
    }
}

void ToolbarViddupView::StartAnchorEcommerceStatusLoop() {
    LOG(INFO) << "start anchor ecommerce.";
    if (!anchor_ecommerce_status_loop_) {
        if (anchor_ecommerce_thread_.joinable()) {
            anchor_ecommerce_thread_.join();
        }

        anchor_ecommerce_status_loop_ = true;
        anchor_ecommerce_ = true;
        anchor_ecommerce_thread_ = std::thread(&ToolbarViddupView::AnchorEcommerceStatusLoop, this);
    }
}

void ToolbarViddupView::EndAnchorEcommerceStatusLoop() {

    LOG(INFO) << "end anchor ecommerce.";
    anchor_ecommerce_status_loop_ = false;
    anchor_ecommerce_ = false;
    if (anchor_ecommerce_thread_.joinable()) {
        anchor_ecommerce_thread_.join();
    }
}

void ToolbarViddupView::AnchorEcommerceStatusLoop() {
    LOG(INFO) << "anchor ecommerce.";
    while (anchor_ecommerce_status_loop_) {
        if (anchor_ecommerce_) {
            BililiveThread::PostTask(BililiveThread::IO,
                FROM_HERE,
                base::Bind(&ToolbarViddupView::GetAnchorEcommerceStatus,
                    base::Unretained(this)));
        }

        Sleep(1000);
    }
}

void ToolbarViddupView::GetAnchorEcommerceStatus() {

}

void ToolbarViddupView::GetShoppingCartStatus() {
}

void ToolbarViddupView::StopLiveStream() {
    LOG(INFO) << "click stop live button.";

    if (sing_presenter_) {
        sing_presenter_->SwitchArea(false);
        sing_presenter_.reset();
        sing_presenter_ = nullptr;
    }

    if (!LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
    {
        LOG(INFO) << "stop live.";
        bool invoke_by_click = true;
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(),IDC_LIVEHIME_STOP_LIVE_STREAMING,CommandParams<bool>(&invoke_by_click));
    }
    else
    {
        LOG(INFO) << "stop live ask.";
        livehime::MessageBoxEndDialogSignalHandler handler;
        handler.closure = base::Bind(&ToolbarViddupView::TpsStopLiveEndDialog, weakptr_factory_.GetWeakPtr());
        livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
            GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE),
            L"If you stop live streaming, you will also exit the third-party push mode. Please confirm whether to stop live streaming£¿",
            GetLocalizedString(IDS_TOOLBAR_STOP_LIVE) + L"," + GetLocalizedString(IDS_COLIVE_MSGBOX_CANCEL),
            &handler,
            livehime::MessageBoxType_NONEICON,
            GetLocalizedString(IDS_TOOLBAR_STOP_LIVE));
    }
}

void ToolbarViddupView::GetLiveDelayMeasureTime()
{
}

void ToolbarViddupView::OnUpdateSEITimer()
{
    if (LivehimeLiveRoomController::GetInstance()->IsStreaming()) {
        std::unique_ptr<base::DictionaryValue> root_dic(new base::DictionaryValue());
        root_dic->SetString("author", "pc_link");
        root_dic->SetString("author_ver", BililiveContext::Current()->GetExecutableVersionAsASCII());

        int64_t cur_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        root_dic->SetInteger64("curr_ms", cur_time_ms);
        
        std::string json_str;
        base::JSONWriter::Write(root_dic.get(), &json_str);
        std::string data = std::string("BVCLIVETIMESTAMP") + json_str;
        OBSProxyService::GetInstance().GetOBSCoreProxy()->GetOBSOutputService()->AddSEIInfo((unsigned char*)data.c_str(), data.size());
    }
}

void ToolbarViddupView::OnGetLiveDelayMeasureTimer()
{    
    GetLiveDelayMeasureTime();
}

void ToolbarViddupView::TpsStopLiveEndDialog(const base::string16& btn, void* data)
{
    if (btn == GetLocalizedString(IDS_TOOLBAR_STOP_LIVE))
    {
        LOG(INFO) << "stop live ask choose stop live.";
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(),
            IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE);
    }
}

void ToolbarViddupView::Observe(int type, const base::NotificationSource& source,
    const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS:
        {
            bool* first_time = reinterpret_cast<bool*>(details.map_key());
            if (first_time && (*first_time == true))
            {

            }

            if (!LivehimeLiveRoomController::GetInstance()->IsStreaming())
            {
                SetTheRightTextOfLiveBtn(GetLocalizedString(IDS_TOOLBAR_START_LIVE));
            }

            auto& anchor_info = GetBililiveProcess()->secret_core()->anchor_info();
            if ((anchor_info.identify_status() != 1))
            {
                if (!real_name_auth_timer_.IsRunning())
                {
                    real_name_auth_timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(30), this, &ToolbarViddupView::OnRealNameAuthTimer);
                }
            }
            else
            {
                real_name_auth_timer_.Stop();
            }

            livehime::VanishULController::RequestULVanishingStatus();

            if (load_roominfo_tips_)
            {
                load_roominfo_tips_ = false;
                BililiveBubbleView* bubble = livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                    LocalStr(IDS_TOOLBAR_LOADED_ROOMINFO_TIPS));
                bubble->StartFade(false, 2000, 1000);
            }
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_LOADING_ROOMINFO:
        {
            load_roominfo_tips_ = true;
            BililiveBubbleView* bubble = livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                LocalStr(IDS_TOOLBAR_LOADING_ROOMINFO_TIPS));
            bubble->StartFade(false, 2000, 1000);
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_FAILED:
        {
            BililiveBubbleView* bubble = livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT,
                GetLocalizedString(IDS_TOOLBAR_ROOMINFO_LOAD_FAILED));
            bubble->StartFade(false, 2000, 1000);
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_LIVEROOM_NOT_GRANT:
    {
    }
        break;
    case bililive::NOTIFICATION_LIVEHIME_DISABLE_START_LIVE:
        {
            live_btn_->SetEnabled(false);
            base::string16 str;
            if (!!details.map_key())
            {
                str = *reinterpret_cast<base::string16*>(details.map_key());
            }
            else
            {
                str = LocalStr(IDS_TOOLBAR_UPDATE_CHANNEL);
            }
            live_btn_->SetText(str);
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_ENABLE_START_LIVE:
        {
            if (!LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
            {
                live_btn_->SetEnabled(true);
                SetTheRightTextOfLiveBtn(GetLocalizedString(IDS_TOOLBAR_START_LIVE));
            }
            else
            {
                if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
                {
                    OnEnterIntoThirdPartyStreamingMode();
                }
            }
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED:
    {
        if (!!details.map_key()) {
            auto info = reinterpret_cast<UserInfoService::CreateRoomInfo*>(details.map_key());
            if (info) {
                base::string16 toast_msg;
                if (!info->valid_response) {
                    toast_msg = GetLocalizedString(IDS_TOOLBAR_ROOMINFO_LOAD_FAILED);
                }
                else {
                    toast_msg = base::UTF8ToUTF16(info->err_msg);
                }
            }
        }
    }
    break;
    default:
        break;
    }
}

void ToolbarViddupView::OnRecordingStarting()
{
}

void ToolbarViddupView::OnStartRecording()
{
}

void ToolbarViddupView::OnRecordingStopping()
{
}

void ToolbarViddupView::OnStopRecording(const base::FilePath& video_path)
{
}

void ToolbarViddupView::OnRecordingError(const base::FilePath& video_path,
    obs_proxy::RecordingErrorCode error_code, const std::wstring& error_message)
{
}

void ToolbarViddupView::OnAudioSourceActivate(obs_proxy::VolumeController * audio_source)
{
    std::string device_name = audio_source->GetBoundSourceName();
    if (device_name == obs_proxy::kDefaultInputAudio)
    {
        auto volume_controllers = std::make_unique<BililiveVolumeControllersPresenterImpl>();
        volume_controllers->SelectController(base::UTF8ToUTF16(device_name));
        if (volume_controllers->ControllerIsValid())
        {
            sound_effect_button_->SetEnabled(true);

            show_mic_warn_ = false;
            SchedulePaint();
        }
    }
    if (device_name == obs_proxy::kDefaultOutputAudio)
    {
        auto volume_controllers = std::make_unique<BililiveVolumeControllersPresenterImpl>();
        volume_controllers->SelectController(base::UTF8ToUTF16(device_name));
        if (volume_controllers->ControllerIsValid())
        {
            show_sys_warn_ = false;
            SchedulePaint();

            if (volume_controllers->GetRenderError())
            {
                base::string16 process_name;
                if (GetRenderFailedProcess(process_name, process_vector_))
                {
                    sys_render_warn_text_ = GetLocalizedString(IDS_TOOLBAR_VOLUME_SYSTEM_RENDER_WARNING_KILL);
                    sys_render_warn_text_ += process_name;
                    sys_render_warn_text_ += GetLocalizedString(IDS_TOOLBAR_VOLUME_SYSTEM_RENDER_WARNING_RESTART);
                    sys_render_warn_tip_->SetTooltipText(GetLocalizedString(IDS_TOOLBAR_VOLUME_SYSTEM_RENDER_WARNING)
                        , sys_render_warn_text_);
                    sys_render_warn_tip_->SetVisible(true);
                    Layout();
                }
            }
        }
    }
}

bool ToolbarViddupView::GetRenderFailedProcess(base::string16& process,
    const std::vector<std::pair<int, base::string16>>& process_vector)
{
    if (process_vector.empty())
    {
        return false;
    }

    auto regmatch = [](const wchar_t* str, const wchar_t* pattern) {
        return std::regex_match(str, std::wregex(pattern, std::regex_constants::icase));
    };

    bool ret = false;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    HANDLE process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (process_snap == INVALID_HANDLE_VALUE)
    {
        return ret;
    }
    BOOL process_first = Process32First(process_snap, &pe32);
    while (process_first)
    {
        for (auto process_name : process_vector)
        {
            if (process_name.first == secret::AppService::DynamicAudioRenderType::String)
            {
                if (!wcsicmp(pe32.szExeFile, process_name.second.c_str()))
                {
                    process = pe32.szExeFile;
                    ret = true;
                    break;
                }
            }
            else
            {
                if(regmatch(pe32.szExeFile, process_name.second.c_str()))
                {
                    process = pe32.szExeFile;
                    ret = true;
                    break;
                }
            }
        }

        if (true == ret)
        {
            break;
        }

        process_first = Process32Next(process_snap, &pe32);
    }
    CloseHandle(process_snap);
    return ret;
}

// BililiveBroadcastObserver
void ToolbarViddupView::NewAnchorNormalNotifyDanmaku(const AnchorNormalNotifyInfo& info) {

}

void ToolbarViddupView::GetPreLiveConf(bool switch_area)
{

}

void ToolbarViddupView::OnOpenLiveRoom()
{
    live_btn_->SetEnabled(false);
    live_btn_->SetText(LocalStr(IDS_STREAMING_TIP_STARTING));
}

void ToolbarViddupView::OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    if (start_live_info.room_type == (int)StartLiveRoomType::Studio)
    {
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
        BililiveBubbleView* bubble = livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT,
            LocalStr(IDS_STUDIO_LIVE_STREAMING_TIPS_START));
        bubble->StartFade(false, 2000, 1000);
    }

    SetTag();
    //ReportSetTags();

    ReportDanmakuHimeState();

    GetShoppingCartStatus();

    GetPreLiveConf(false);

    base::StringPairs event_ext;
    event_ext.push_back(std::pair<std::string, std::string>("result", "1"));
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::StartLiveResult, event_ext);
}

void ToolbarViddupView::OnOpenLiveRoomError(const std::wstring& error_msg, int error_code, bool need_face_auth, const std::string& qr)
{
    live_btn_->SetEnabled(true);
    live_btn_->StopTimer();
    live_btn_->SetText(LocalStr(IDS_TOOLBAR_START_LIVE));

    if (!error_msg.empty() &&
        (error_code != (int)StartLiveRoomErrorCode::Authentication)&&
        (error_code != (int)StartLiveRoomErrorCode::AuthAccountAnomaly))
    {
        livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT, error_msg);
    }

    base::StringPairs event_ext;
    event_ext.push_back(std::pair<std::string, std::string>("result", "2"));
    event_ext.push_back(std::pair<std::string, std::string>("error_code", std::to_string(error_code)));
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::StartLiveResult, event_ext);
}

void ToolbarViddupView::OnPreLiveRoomStartStreaming(bool is_restreaming)
{
}

void ToolbarViddupView::OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details)
{

    live_btn_->SetEnabled(true);
    live_btn_->SetText(LocalStr(IDS_TOOLBAR_STOP_LIVE));
    live_btn_->StartTimer();

    if (update_SEI_timer_.IsRunning()) {
        update_SEI_timer_.Stop();
    }

    OnUpdateSEITimer();
}

void ToolbarViddupView::OnPreLiveRoomStopStreaming()
{
    if (update_SEI_timer_.IsRunning()) {
        update_SEI_timer_.Stop();
    }
    live_btn_->SetEnabled(false);
    live_btn_->StopTimer();
    live_btn_->SetText(LocalStr(IDS_STREAMING_TIP_STOPPING));
    if (more_five_min_)
    {
        LivehimeLiveRoomController::GetInstance()->HideBookLiveTime();
    }
}

void ToolbarViddupView::OnLiveRoomStreamingStopped()
{
    ReportLiveDuration(live_btn_->time_delta().GetTotalMinutes());
}

void ToolbarViddupView::OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code)
{
    live_btn_->StopTimer();
    if (more_five_min_)
    {
        LivehimeLiveRoomController::GetInstance()->HideBookLiveTime();
    }

    if (streaming_errno == obs_proxy::StreamingErrorCode::StartFailure)
    {
        auto bubble = livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT,
            new EncoderErrorBubbleContentView(error_message));
        bubble->set_close_on_deactivate(false);
        bubble->StartFade(false, 3000, 3000);
    }
    else
    {
        livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT, error_message);
    }
}

void ToolbarViddupView::OnCloseLiveRoom()
{
    OnPreLiveRoomStopStreaming();
}

void ToolbarViddupView::OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    if (sing_presenter_) {
        sing_presenter_->SwitchArea(false);
        sing_presenter_.reset();
        sing_presenter_ = nullptr;
    }
    if (start_live_info.room_type == (int)StartLiveRoomType::Studio)
    {
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
        BililiveBubbleView* bubble = livehime::ShowBubble(live_btn_->button(), views::BubbleBorder::Arrow::BOTTOM_RIGHT,
            LocalStr(IDS_STUDIO_LIVE_STREAMING_TIPS_END));
        bubble->StartFade(false, 2000, 1000);
    }

    if (is_restreaming)
    {
        live_btn_->SetEnabled(false);
        live_btn_->StopTimer();
        live_btn_->SetText(LocalStr(IDS_STREAMING_TIP_RESTREAMING));
    }
    else
    {
        live_btn_->StopTimer();

        if (!LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
        {
            live_btn_->SetEnabled(true);
            live_btn_->SetText(LocalStr(IDS_TOOLBAR_START_LIVE));
        }
        else
        {
            live_btn_->SetEnabled(false);
            live_btn_->SetText(GetLocalizedString(IDS_TOOLBAR_START_LIVE_WAITING));
        }
    }

    EndAnchorEcommerceStatusLoop();
}

void ToolbarViddupView::OnLiveRoomStreamingRetryNextAddr(int index, int total)
{
    live_btn_->SetText(LocalStr(IDS_TOOLBAR_LIVE_RETRY));
}

void ToolbarViddupView::OnNoticeAuthNeedSupplement(const secret::LiveStreamingService::StartLiveNotice& notice)
{
    auto title = base::UTF8ToUTF16(notice.title);
    auto msg = base::UTF8ToUTF16(notice.msg);
    if (notice.button_url.empty())
    {
        livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
            title,
            msg,
            GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));
    }
    else
    {
        auto button_text = base::UTF8ToUTF16(notice.button_text);
        livehime::MessageBoxEndDialogSignalHandler handler;
        handler.closure = base::Bind(&OnVerifyFailDialog);
        scoped_ptr<std::wstring> button_url(new std::wstring(base::UTF8ToUTF16(notice.button_url)));
        handler.data = button_url.release();
        livehime::ShowMessageBox(
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
            title,
            msg,
            GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW) + L"," + button_text,
            &handler, livehime::MessageBoxType_NONEICON, button_text);
    }
}

void ToolbarViddupView::OnEnterIntoThirdPartyStreamingMode()
{
    live_btn_->SetEnabled(false);
    live_btn_->SetText(GetLocalizedString(IDS_TOOLBAR_START_LIVE_WAITING));

    ShowFauxAudientEffect(false, true);
}

void ToolbarViddupView::OnLeaveThirdPartyStreamingMode()
{
    live_btn_->SetEnabled(true);
    SetTheRightTextOfLiveBtn(GetLocalizedString(IDS_TOOLBAR_START_LIVE));
}

void ToolbarViddupView::ShowFauxAudientEffect(bool show, bool landscape)
{
    LiveModelController::GetInstance()->ShowFauxAudientEffect(show, landscape);
}