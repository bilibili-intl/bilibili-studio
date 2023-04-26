#include "livehime_status_bar_view.h"

#include <shellapi.h>

#include "base/ext/callable_callback.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/animation/bounds_animator.h"
#include "ui/views/animation/bounds_animator_observer.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/utils/setting_util.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/streaming_diagrams/streaming_diagrams_main_view.h"
#include "bililive/bililive/ui/views/livehime/streaming_diagrams/streaming_warning_notify_view.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/log_ext/log_constants.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

namespace
{
    const int kQpDequeCapacity = 10;
    const int kPresentQpInterval = 1;
    const int kLabelPaddingColWidth = 70;
    const int kBitrateLabelPaddingColWidth = 90;
    const int kStatusLabelPaddingColWidth = 50;
    const int kFpsLabelPaddingColWidth = 60;
    const int kRefreshNewsTimeDeltaInMin = 30; // 30min
    const int kAnimationTimeDelta = 5;
    const double kEps = 1e-6;

    enum CtrlID
    {
        Button_Statistical = 1,
        Button_Extend
    };

    static void SaveSceneConfig()
    {
        //每隔300秒保存一次直播姬场景源配置,从1开始，防止一起来就保存一下
        static int index = 1;
        if (index % 300 == 0)
        {
            obs_proxy::OBSCoreProxy* core_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
            if (core_proxy)
            {
                core_proxy->GetCurrentSceneCollection()->Save();
            }
        }
        index++;
    }

    class AnimateNoticeView
        : public views::View
        , public views::ButtonListener
        , public views::BoundsAnimatorObserver
    {
    public:
        AnimateNoticeView()
            : animator_(this)
            , weakptr_factory_(this)
        {
        }

    protected:
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    notice_list_view_ = new views::View();
                    notice_list_view_->set_notify_enter_exit_on_child(true);
                    AddChildView(notice_list_view_);

                    set_notify_enter_exit_on_child(true);

                    animator_.AddObserver(this);

                    RefreshNotices();
                }
                else
                {
                    animator_.RemoveObserver(this);
                }
            }
        }

        gfx::Size GetPreferredSize() override
        {
            return gfx::Size(notice_list_view_->GetPreferredSize().width(), ftPrimary.GetHeight());
        }

        // ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {

        }

        // BoundsAnimatorObserver
        void OnBoundsAnimatorProgressed(views::BoundsAnimator* animator) override
        {}

        void OnBoundsAnimatorDone(views::BoundsAnimator* animator) override
        {
            if (notice_list_view_->y() <=
                -notice_list_view_->height() + notice_list_view_->height() / notice_list_view_->child_count())
            {
                notice_list_view_->SetBounds(0, 0, width(), notice_list_view_->height());
            }
        }

    private:
        void RefreshNotices()
        {

        }

        void ResetNoticeListView()
        {
            notice_list_view_->RemoveAllChildViews(true);
            notice_list_view_->SetLayoutManager(nullptr);

            views::GridLayout* layout = new views::GridLayout(notice_list_view_);
            views::ColumnSet* columnset = layout->AddColumnSet(0);
            columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            notice_list_view_->SetLayoutManager(layout);
        }

        void AddNoticeItemView(views::View* view)
        {
            views::GridLayout* layout = (views::GridLayout*)notice_list_view_->GetLayoutManager();
            if (layout)
            {
                layout->StartRow(0, 0);
                layout->AddView(view);
            }
        }

        void AnimationCountdownTimerMethod()
        {
            if (!notice_list_view_->visible() || (notice_list_view_->child_count() < 2))
            {
                return;
            }

            if (IsMouseHovered()) {
                return;
            }

            current_frame_holder_time_count_++;

            if (current_frame_holder_time_count_ < kAnimationTimeDelta) {
                return;
            } else {
                current_frame_holder_time_count_ = 0;
            }

            int dst_y = notice_list_view_->y() - notice_list_view_->height() / notice_list_view_->child_count();
            DCHECK(std::abs(dst_y) <= notice_list_view_->height());
            animator_.AnimateViewTo(notice_list_view_, gfx::Rect(0, dst_y, width(), notice_list_view_->height()));
        }

    private:
        int current_frame_holder_time_count_ = 0;
        views::View* notice_list_view_ = nullptr;
        views::BoundsAnimator animator_;
        base::RepeatingTimer<AnimateNoticeView> animation_countdown_timer_;
        base::WeakPtrFactory<AnimateNoticeView> weakptr_factory_;
    };

    class RecordLiveTimeView : public views::View
    {
    public:
        explicit RecordLiveTimeView(bool record)
            : is_record_(record)
        {
            static base::string16 pad_str(L"00:00:00");
            label_ = new LivehimeSmallContentLabel(pad_str);
            label_->SetImage(GetImageSkiaNamed(is_record_ ?
                                               IDR_LIVEHIME_V3_STATUSBAR_RECORD :
                                               IDR_LIVEHIME_V3_STATUSBAR_LIVE));
            label_->SetPreferredSize(gfx::Size(ftTwelve.GetStringWidth(pad_str), 1));
            AddChildView(label_);
        }
        virtual ~RecordLiveTimeView() = default;

        void StartTimer()
        {
            if (!timer_.IsRunning())
            {
                time_ = base::Time::Now();
                timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(1), this, &RecordLiveTimeView::OnTimer);
                label_->SetText(L"00:00:00");

                PreferredSizeChanged();
                SetVisible(true);
            }
        };

        void StopTimer()
        {
            if (timer_.IsRunning())
            {
                timer_.Stop();
                label_->SetText(L"");

                SetVisible(false);
                PreferredSizeChanged();
            }
        };

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            gfx::Size label_size = label_->GetPreferredSize();
            gfx::Size pref_size(is_record_ ? kRecordButtonWidth : kLiveButtonWidth, 1);
            pref_size.SetToMax(label_size);
            return pref_size;
        }

        void Layout() override
        {
            if (label_)
            {
                gfx::Size label_size = label_->GetPreferredSize();
                label_->SetBounds((width() - label_size.width()) / 2,
                                  (height()- label_size.height()) / 2,
                                  label_size.width(), label_size.height());
            }
        }

        void OnBoundsChanged(const gfx::Rect& previous_bounds) override
        {
            InvalidateLayout();
        }

    private:
        void OnTimer()
        {
            bililive::TimeSpan delta(base::Time::Now() - time_);
            std::wstring str = base::StringPrintf(
                L"%02d:%02d:%02d",
                static_cast<LONG>(delta.GetTotalHours()), delta.GetMinutes(), delta.GetSeconds());
            bool need_relayout = (str.length() != label_->text().length());
            label_->SetText(str);
            if (need_relayout)
            {
                PreferredSizeChanged();
            }
        };

    private:
        base::RepeatingTimer<RecordLiveTimeView> timer_;
        base::Time time_;
        bool is_record_ = false;
        LivehimeSmallContentLabel* label_ = nullptr;

        DISALLOW_COPY_AND_ASSIGN(RecordLiveTimeView);
    };

}


LivehimeStatusBarView::LivehimeStatusBarView()
    :weak_ptr_factory_(this)
{
}

LivehimeStatusBarView::~LivehimeStatusBarView()
{
}

void LivehimeStatusBarView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            GetBililiveProcess()->bililive_obs()->broadcast_service()->AddObserver(this);
            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
            StreamingReportService::GetInstance()->AddObserver(this);
            AppFunctionController::GetInstance()->AddObserver(this);

            obsmonitor_ = std::make_unique<bililive::OBSStatusMonitor>(this);

            streaming_timer_.reset(new base::RepeatingTimer<LivehimeStatusBarView>());
            PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
            //SetQualityCheckingEnabled(pref->GetBoolean(prefs::kDanmakuHimeEnableQualityChecking));
            RequestLiveReplaySetting();
        }
        else
        {
            GetBililiveProcess()->bililive_obs()->broadcast_service()->RemoveObserver(this);
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
            StreamingReportService::GetInstance()->RemoveObserver(this);
            AppFunctionController::GetInstance()->RemoveObserver(this);
            obsmonitor_.reset();
        }
    }
}

void LivehimeStatusBarView::ChildPreferredSizeChanged(views::View* child)
{
    Layout();
}

int LivehimeStatusBarView::GetHeightForWidth(int w)
{
    return GetLengthByDPIScale(31);
}

void LivehimeStatusBarView::OnPaintBackground(gfx::Canvas* canvas)
{
    return __super::OnPaintBackground(canvas);
    int radus = GetLengthByDPIScale(0);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SkColorSetARGB(0x33, 0x53, 0x67, 0x77));
    paint.setStyle(SkPaint::kFill_Style);
    canvas->DrawRoundRect(GetContentsBounds(), radus, paint);
}

void LivehimeStatusBarView::InitViews()
{
    //set_background(views::Background::CreateSolidBackground(SkColorSetRGB(0xE6, 0xE6, 0xE6)));
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    OnThemeChanged();
    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(400));
	column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF,
		0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(20));
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF,
        0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(6));

    layout->AddPaddingRow(1.0f, GetLengthByDPIScale(2));
    layout->StartRow(0, 0);

    //数据指标详情view
    data_details_view_ = new views::View();
	views::GridLayout* details_view_layout = new views::GridLayout(data_details_view_);
    data_details_view_->SetLayoutManager(details_view_layout);
	column_set = details_view_layout->AddColumnSet(0);
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF,0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(6));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(6));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(6));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

    details_view_layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));
    details_view_layout->StartRow(0, 0);

    base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_BITRATE);
    label_str += L"0kbps";
    bitrate_label_ = new LivehimeSmallContentLabel(label_str);
    bitrate_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(90), GetLengthByDPIScale(12)));
    details_view_layout->AddView(bitrate_label_);

    label_str = rb.GetLocalizedString(IDS_TOOLBAR_FRAME_LOSS);
    label_str += L"0.0%";
    frame_loss_label_ = new LivehimeSmallContentLabel(label_str);
    frame_loss_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(74), GetLengthByDPIScale(12)));
    details_view_layout->AddView(frame_loss_label_);

    label_str = rb.GetLocalizedString(IDS_TOOLBAR_CPU);
    label_str += L"0%";
    cpu_label_ = new LivehimeSmallContentLabel(label_str);
    cpu_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(74), GetLengthByDPIScale(12)));
    details_view_layout->AddView(cpu_label_);

    label_str = rb.GetLocalizedString(IDS_TOOLBAR_MEMORY);
    label_str += L"0%";
    memory_label_ = new LivehimeSmallContentLabel(label_str);
    memory_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(74), GetLengthByDPIScale(12)));
    details_view_layout->AddView(memory_label_);
    details_view_layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));

    //推流质量view
    streaming_quality_view_ = new views::View();
	views::GridLayout* quality_view_layout = new views::GridLayout(streaming_quality_view_);
    streaming_quality_view_->SetLayoutManager(quality_view_layout);
	column_set = quality_view_layout->AddColumnSet(0);
    column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(6));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
		GetLengthByDPIScale(87), GetLengthByDPIScale(16));
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(6));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::FIXED,
		GetLengthByDPIScale(110), GetLengthByDPIScale(16));

    network_situation_bt_ = new BililiveLabelButton(this,L"网络较好");
    network_situation_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_NETWORK_GOOD));
    network_situation_bt_->SetFont(ftTwelve);
    network_situation_bt_->SetTextColor(views::Button::ButtonState::STATE_NORMAL,SkColorSetA(clrTextALL, 0.6f * 255));
    network_situation_bt_->SetTextColor(views::Button::ButtonState::STATE_HOVERED, clrLivehime);
    network_situation_bt_->SetTextColor(views::Button::ButtonState::STATE_PRESSED, clrLivehime);
    network_situation_bt_->set_round_corner(false);
    network_situation_bt_->SetCursor(::LoadCursor(nullptr, IDC_HAND));
	streaming_quality_bt_ = new BililiveLabelButton(this, L"直播质量较好");
    streaming_quality_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_QUALITY_GOOD));
    streaming_quality_bt_->SetFont(ftTwelve);
    streaming_quality_bt_->SetTextColor(views::Button::ButtonState::STATE_NORMAL, SkColorSetA(clrTextALL, 0.6f * 255));
    streaming_quality_bt_->SetTextColor(views::Button::ButtonState::STATE_HOVERED, clrLivehime);
    streaming_quality_bt_->SetTextColor(views::Button::ButtonState::STATE_PRESSED, clrLivehime);
    streaming_quality_bt_->set_round_corner(false);
    streaming_quality_bt_->SetCursor(::LoadCursor(nullptr, IDC_HAND));

    quality_view_layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));
    quality_view_layout->StartRow(0, 0);
    quality_view_layout->AddView(network_situation_bt_);
    quality_view_layout->AddView(streaming_quality_bt_);
    quality_view_layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));

    auto status_view = new BililiveSingleChildShowContainerView;
    status_view->AddChildView(data_details_view_);
    status_view->AddChildView(streaming_quality_view_);
    streaming_quality_view_->SetVisible(false);

    extend_button_ = new LivehimeImageRightButton(this, L"更多详情");
    extend_button_->SetFont(ftTwelve);
    extend_button_->SetColor(views::Button::STATE_NORMAL, SkColorSetA(clrTextALL, 0.6f * 255));
    extend_button_->SetCursor(::LoadCursor(nullptr, IDC_HAND));
	gfx::ImageSkia* image_normal = GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_ARROW_RIGHT);
	gfx::ImageSkia* image_hover = GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_ARROW_RIGHT_HOVER);
    extend_button_->SetImage(views::Button::STATE_NORMAL, image_normal);
    extend_button_->SetImage(views::Button::STATE_HOVERED, image_hover);
    extend_button_->set_id(Button_Extend);

    layout->AddView(status_view);
    layout->AddView(extend_button_);
    layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));
}

void LivehimeStatusBarView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender->id() == Button_Extend)
    {
        StreamingDiagramsMainView::ShowWindow();
    }
    else if (sender == network_situation_bt_ || sender == streaming_quality_bt_)
    {
        StreamingQualityView::ShowWindow(sender);
        StreamingQualityView::SetStreamingQuality(cur_streaming_quality_);
    }
}

void LivehimeStatusBarView::OnPreLiveRoomStartStreaming(bool is_restreaming)
{
    if (!is_restreaming)
    {
        //GetRoomInfo(true);//GetRoomInfo响应函数里已经不做任何处理了，所以这里不需要再请求了
        max_audience_ = 0;

        //网络状况和直播推流质量
        if (!LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
        {
			quality_info_.is_streaming = true;
			quality_info_.start_count = 0;
			quality_info_.InitCalculationData();
			data_details_view_->SetVisible(false);
			streaming_quality_view_->SetVisible(true);
			//气泡引导提醒
			auto prefs = GetBililiveProcess()->global_profile()->GetPrefs();
			bool showed = prefs->GetBoolean(prefs::kStreamingQualityGuide);
			if (!showed)
			{
				prefs->SetBoolean(prefs::kStreamingQualityGuide, true);

				BililiveBubbleView* bubble = livehime::ShowBubble(streaming_quality_bt_, views::BubbleBorder::Arrow::BOTTOM_CENTER,
					L" 这里可以对直播质量进行自我检测啦～ ");
				bubble->set_background_color(GetColor(Theme));
				bubble->set_text_color(SK_ColorWHITE);
				bubble->StartFade(false, 5000, 500);
				bubble->set_close_on_deactivate(false);
			}
        }
    }
    if (status_label_)
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_STARTING);
        status_label_->SetText(label_str);
    }
    StartStreamingTimer();
}

void LivehimeStatusBarView::OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    if (status_label_)
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_STOPPED);
        status_label_->SetText(label_str);
    }

    if (!is_restreaming)
    {
        //放到了调研请求里面
        //GetLiveRealInfo();
        //GetRoomInfo(false);//GetRoomInfo响应函数里已经不做任何处理了，所以这里不需要再请求了
        GetQuestionLinkInfo();
       
    }


    StopStreamingTimer();
    ResetStreamingStatus();

    quality_info_.is_streaming = false;
    quality_info_.start_count = 0;
    quality_info_.InitCalculationData();
	data_details_view_->SetVisible(true);
	streaming_quality_view_->SetVisible(false); 
    UpdateQualityStatus(StatusBarNetworkStatus::Network_Good, StatusBarStreamingQuality::Quality_Good);
}

void LivehimeStatusBarView::OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code)
{
    if (status_label_)
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_STOPPED);
        status_label_->SetText(label_str);
    }
}

void LivehimeStatusBarView::UpdateFps()
{
    if (fps_label_)
    {
        int total_frames = bililive::OutputController::GetInstance()->GetOutputTotalFrames();
        int dropped_frames = bililive::OutputController::GetInstance()->GetOutputDroppedFrames();
        int fps = total_frames - last_send_frames_ - dropped_frames;
        if (fps < 0)
        {
            fps = 0;
        }
        ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
        base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_FPS);
        label_str += base::StringPrintf(L"%d", fps);
        fps_label_->SetText(label_str);
        last_send_frames_ = total_frames - dropped_frames;
    }
}

void LivehimeStatusBarView::OnStreamingMetricsFeedback(double bandwidth_speed, double frame_loss_rate)
{
    quality_info_.bitrate = bandwidth_speed * 8.0f;
    quality_info_.frame_loss_rate = frame_loss_rate;
    if (bitrate_label_ && frame_loss_label_)
    {
        // bandwidth_speed 单位为KB/s
        base::string16 bitrate_str = GetLocalizedString(IDS_TOOLBAR_BITRATE);
        bitrate_str += SpeedInKByteToFriendlyString(bandwidth_speed);
        bitrate_label_->SetText(bitrate_str);

        base::string16 loss_str = GetLocalizedString(IDS_TOOLBAR_FRAME_LOSS);
        loss_str += base::StringPrintf(L"%.01f%%", frame_loss_rate);
        frame_loss_label_->SetText(loss_str);
        StreamingQualityView::SetBitrateAndLossLabel(bitrate_str, loss_str);

        static bool too_low_reported = false;
        if (bandwidth_speed < 50)
        {
            if (!too_low_reported)
            {
                too_low_reported = true;

                LOG(INFO) << app_log::kLogStreamSpeedExcept << "speed:" << bandwidth_speed;
            }
        }
        else
        {
            too_low_reported = false;
        }
    }
}

void LivehimeStatusBarView::OnStreamEncodedQPFeedback(int qp)
{
    if (quality_label_ && is_quality_checking_enabled_)
    {
        qp_deque_.push_back(qp);
        if (qp_deque_.size() > kQpDequeCapacity)
        {
            qp_deque_.pop_front();
        }
    }
}

string16 LivehimeStatusBarView::SpeedInKByteToFriendlyString(double speedInKBps)
{
    static const int64 Mb = 1000;
    static const int64 Gb = 1000 * Mb;
    static const int64 Tb = 1000 * Gb;
    static const int64 Pb = 1000 * Tb;
    static const int64 Eb = 1000 * Pb;

    double speedInKbps = speedInKBps * 8;

    double value = 0;
    const wchar_t* unit = L"";

    if (speedInKbps >= Eb)
    {
        unit = L"ebps";
        value = speedInKbps / Eb;
    }
    else if (speedInKbps >= Pb)
    {
        unit = L"pbps";
        value = speedInKbps / Pb;
    }
    else if (speedInKbps >= Tb)
    {
        unit = L"tbps";
        value = speedInKbps / Tb;
    }
    else if (speedInKbps >= Gb)
    {
        unit = L"gbps";
        value = speedInKbps / Gb;
    }
    else if (speedInKbps >= Mb)
    {
        unit = L"mbps";
        value = speedInKbps / Mb;
    }
    else
    {
        unit = L"kbps";
        value = speedInKbps;
    }

    auto str = base::StringPrintf(L"%.8lf", value);

    str = str.substr(0, 4);
    if (!str.empty() && *str.rbegin() == L'.')
        str.erase(str.begin() + str.size() - 1);

    str += unit;

    return str;
}

void LivehimeStatusBarView::UpdateQualityValue()
{
    int acc = 0;
    int size = qp_deque_.size();
    if (size > 0)
    {
        for (auto it = qp_deque_.begin();
            it != qp_deque_.end(); ++it)
        {
            acc += (*it);
        }

        UpdateQPViews(acc / size);
    }
}

void LivehimeStatusBarView::UpdateQPViews(int averaged_qp)
{
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();

    if (averaged_qp <= 25)
    {
        base::string16 label_str = rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_DEFAULT);
        label_str += rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_BEST);
        quality_label_->SetText(label_str);
    }
    else if (averaged_qp > 25 && averaged_qp <= 30)
    {
        base::string16 label_str = rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_DEFAULT);
        label_str += rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_CLEAR);
        quality_label_->SetText(label_str);
    }
    else if (averaged_qp > 30 && averaged_qp <= 35)
    {
        base::string16 label_str = rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_DEFAULT);
        label_str += rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_SLIGHTLY_BLURRED);
        quality_label_->SetText(label_str);
    }
    else if (averaged_qp > 35 && averaged_qp <= 40)
    {
        base::string16 label_str = rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_DEFAULT);
        label_str += rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_BLURRED);
        quality_label_->SetText(label_str);
    }
    else
    {
        base::string16 label_str = rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_DEFAULT);
        label_str += rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_SERIOUSLY_BLURRED);
        quality_label_->SetText(label_str);
    }
}

void LivehimeStatusBarView::SetQualityCheckingEnabled(bool enabled)
{
    if (enabled != is_quality_checking_enabled_)
    {
        is_quality_checking_enabled_ = enabled;

        if (enabled)
        {
            if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady())
            {
                StartStreamingTimer();
            }
        }
        else
        {
            StopStreamingTimer();

            if (quality_label_)
            {
                ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();

                quality_label_->SetText(
                    rb.GetLocalizedString(IDS_DANMAKUHIME_STATUS_UPLOAD_QUALITY_DEFAULT));
            }
        }
    }
}

void LivehimeStatusBarView::StartStreamingTimer()
{
    //streaming_timer_定时器主要是更新quality_label_和fps_label_，所以这里判断一下，目前这两个label已经删除了
    if ((quality_label_ || fps_label_ ) && streaming_timer_ && !streaming_timer_->IsRunning())
    {
        streaming_timer_->Start(
            FROM_HERE, base::TimeDelta::FromSeconds(kPresentQpInterval), this,
            &LivehimeStatusBarView::UpdateStreamingStatus);
    }
}

void LivehimeStatusBarView::StopStreamingTimer()
{
    if (streaming_timer_ && streaming_timer_->IsRunning()) {
        streaming_timer_->Stop();
    }

    qp_deque_.clear();
}

void LivehimeStatusBarView::UpdateStreamingStatus()
{
    if (is_quality_checking_enabled_)
    {
        UpdateQualityValue();
    }
    UpdateFps();
}

void LivehimeStatusBarView::ResetStreamingStatus()
{
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    base::string16 bitrate_str = rb.GetLocalizedString(IDS_TOOLBAR_BITRATE);
    bitrate_str += L"0kbps";

    if (bitrate_label_)
    {
        bitrate_label_->SetText(bitrate_str);
    }

    base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_FPS);
    label_str += L"0";

    if (fps_label_)
    {
        fps_label_->SetText(label_str);
    }

    base::string16 loss_str = rb.GetLocalizedString(IDS_TOOLBAR_FRAME_LOSS);
    loss_str += L"0.0%";

    if (frame_loss_label_)
    {
        frame_loss_label_->SetText(loss_str);
    }
    StreamingQualityView::SetBitrateAndLossLabel(bitrate_str, loss_str);

    label_str = rb.GetLocalizedString(IDS_TOOLBAR_QUALITY);
    label_str += rb.GetLocalizedString(IDS_TOOLBAR_QUALITY_NONE);

    if (quality_label_)
    {
        quality_label_->SetText(label_str);
    }

    last_send_frames_ = 0;
}

void LivehimeStatusBarView::RoomAudience(int64_t audience)
{
    max_audience_ = std::max(audience, max_audience_);
}

void LivehimeStatusBarView::OnStreamingReportSampling(const StreamingReportInfo& info)
{
	quality_info_.encode_loss_rate = info.data[StreamingSampleType::EDF].data;
	quality_info_.render_loss_rate = info.data[StreamingSampleType::RDF].data;

	base::string16 cpu_str = GetLocalizedString(IDS_TOOLBAR_CPU);
	cpu_str += base::StringPrintf(L"%d%%", (int)info.data[StreamingSampleType::CPU].data);

	base::string16 memory_str = GetLocalizedString(IDS_TOOLBAR_MEMORY);
	memory_str += base::StringPrintf(L"%d%%", (int)info.data[StreamingSampleType::MEM].data);

    if (cpu_label_)
    {
        cpu_label_->SetText(cpu_str);
    }
    if (memory_label_)
    {
        memory_label_->SetText(memory_str);
    }
    StreamingQualityView::SetCpuAndMemoryLabel(cpu_str,memory_str);
    //保存场景源配置，借用一下这个获取系统信息的定时器，免得去开一个新的
    SaveSceneConfig();
    //分析网络状况和直播质量，这里每秒调用一次
    AnalysisLiveQuality();
}

void LivehimeStatusBarView::OnStreamingWarning(StreamingWarning warning)
{
    switch (warning)
    {
    case WARNING_USE_HARD_ENCODE:
        StreamingWarningNotifyView::ShowWindow( GetLocalizedString(IDS_STREAMING_WARNING_USE_HARD_ENCODE));
        break;
    case WARNING_NET_LOSS:
        {
            PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
            bool auto_test = pref->GetBoolean(prefs::kAutoVideoSwitch);
            if (auto_test)
            {
                StreamingWarningNotifyView::ShowWindow(GetLocalizedString(IDS_STREAMING_WARNING_NET_LOSS));
            }
            else
            {
                StreamingWarningNotifyView::ShowWindow(GetLocalizedString(IDS_STREAMING_WARNING_NET_LOSS_AUTO));
            }
        }
        break;
    case WARNING_CHANGE_GAME_SETTING:
        StreamingWarningNotifyView::ShowWindow(GetLocalizedString(IDS_STREAMING_WARNING_CHANGE_GAME_SETTING));
        break;
    case WARNING_CHANGE_LIVEHIME_SETTING:
        StreamingWarningNotifyView::ShowWindow(GetLocalizedString(IDS_STREAMING_WARNING_CHANGE_LIVEHIME_SETTING));
        break;
    case WARNING_MEMORY_OCCUPY:
    case WARNING_MEMORY_OCCUPY_MAX:
        JudgeMemoryOccupy(false);
        break;
    case WARNING_NV_MEMORY_OCCUPY:
    case WARNING_NV_MEMORY_OCCUPY_MAX:
        JudgeMemoryOccupy(true);
        break;
    default:
        break;
    }
}

void LivehimeStatusBarView::JudgeMemoryOccupy(bool is_nv)
{
    auto obs_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
    obs_proxy::Scene* scene = obs_proxy->GetCurrentSceneCollection()->current_scene();
    bool has_image = false;
    for (auto item : scene->GetItems())
    {
        if (item->type() == obs_proxy::SceneItemType::Image)
        {
            has_image = true;
        }
    }

    base::string16 warn_content;
    if (is_nv) {
        if (!has_image) {
            warn_content = GetLocalizedString(IDS_STREAM_NV_WARN);
        }
        else {
            warn_content = GetLocalizedString(IDS_STREAM_NV_IMAGE_WARN);
        }
    }
    else {
        if (has_image) {
            warn_content = GetLocalizedString(IDS_STREAM_IMAGE_WARN);
        }
        else {
            warn_content = GetLocalizedString(IDS_STREAM_MATER_WARN);
        }
    }
    StreamingWarningNotifyView::ShowWindow(warn_content);
}

void LivehimeStatusBarView::OnUpdateAppCtrlInfo()
{
    if (AppFunctionController::GetInstance())
    {
        extend_button_->SetVisible(AppFunctionController::GetInstance()->streaming_qmcs_ctrl().switch_on);
        InvalidateLayout();
        Layout();
    }
}

void LivehimeStatusBarView::OnAllAppKVCtrlInfoCompleted()
{
    // 自动音视频参数设置结算，放在这里的目的是因为这里是明确的晚于“从旧版升新版的配置文件更新时机 UpdateCustomVideoSettings”的地方，
    // 放在这里不管是针对新用户的每次程序启动的自动参数设置还是针对旧用户的场景项判断都能进行统一收束处理
    livehime::ApplyAutoMediaConfig();

    // 程序每次启动都检查一遍配置文件里面记录的编码器当前还可不可用，不可用就主动换一个性质一样的编码器
    livehime::CheckProfileVideoCodec();

}

void LivehimeStatusBarView::RequestLiveReplaySetting()
{

}

void LivehimeStatusBarView::GetFeedbackList()
{

}

void LivehimeStatusBarView::AnalysisLiveQuality()
{
    if (!quality_info_.is_streaming)//未开播
    {
        return;
    }
    quality_info_.start_count++;
    if (quality_info_.start_count < 15)//开始直播前15不统计
    {
        return;
    }
	//网络情况良好 -- 码率 > 0 && 丢帧率 == 0 %
	//网络情况中等 -- 码率 > 0 && 0 % < 丢帧率 <= 10 %
	//网络情况较差 -- 码率 > 0 && 丢帧率 > 10 %
	//网络情况异常 -- 码率 = 0
	//直播质量较好 -- 网络情况良好 && 渲染丢帧率 <= 0 % &&编码器丢帧率 <= 0 %
	//直播质量中等 -- 网络情况中等 || 0 % < 渲染丢帧率 <= 30 % || 0 % < 编码器丢帧率 <= 30 %
	//直播质量较差 -- 网络情况较差或网络情况异常 || 渲染丢帧率 > 30 % || 编码器丢帧率 > 30 %
	//以上指标都是按每秒统计的，最终的网络情况和直播质量需要按时间段来统计:
    //网络情况:10秒为一个统计周期，统计 码率 == 0 的次数，丢帧率 > 10 % 的次数, 0 % < 丢帧率 <= 10 % 的次数(丢帧率 > 10 % 时，0 % < 丢帧率 <= 10 % 的次数也要 + 1)
    //统计次数
    float compare_zero = 0.001f;
    quality_info_.cal_count++;
    StatusBarNetworkStatus this_network_status = StatusBarNetworkStatus::Network_Good;

    if (quality_info_.bitrate < compare_zero)//码率 = 0
    {
        quality_info_.bitrate_zero_count++;
        this_network_status = StatusBarNetworkStatus::Network_Anomaly;
    }
    
    if (quality_info_.frame_loss_rate >  10.0f)//丢帧率> 10 %
    {
        quality_info_.frame_loss_poor_count++;
        quality_info_.frame_loss_secondary_count++;
        if (this_network_status != StatusBarNetworkStatus::Network_Anomaly)
        {
            this_network_status = StatusBarNetworkStatus::Network_Poor;
        }
    }
    else if (compare_zero < quality_info_.frame_loss_rate &&
        quality_info_.frame_loss_rate <= 10.0f )//0 % < 丢帧率 <= 10 %
    {
        quality_info_.frame_loss_secondary_count++;
		if (this_network_status != StatusBarNetworkStatus::Network_Anomaly)
		{
			this_network_status = StatusBarNetworkStatus::Network_Secondary;
		}
    }
    //直播质量
    if (this_network_status == StatusBarNetworkStatus::Network_Poor ||
        this_network_status == StatusBarNetworkStatus::Network_Anomaly ||
        quality_info_.render_loss_rate > 0.3f ||
        quality_info_.encode_loss_rate > 0.3f )//直播质量较差
    {
        quality_info_.quality_poor_count++;
        quality_info_.quality_secondary_count++;
    }
    else if (this_network_status == StatusBarNetworkStatus::Network_Secondary ||
        (compare_zero < quality_info_.render_loss_rate && quality_info_.render_loss_rate <= 0.3f) ||
        (compare_zero < quality_info_.encode_loss_rate && quality_info_.encode_loss_rate <= 0.3f) )//直播质量中等
    {
        quality_info_.quality_secondary_count++;
    }

    //开始判定
    if (quality_info_.cal_count >= 10)
    {
        StatusBarNetworkStatus cur_network_status = StatusBarNetworkStatus::Network_Good;
        StatusBarStreamingQuality cur_streaming_quality = StatusBarStreamingQuality::Quality_Good;
        //判定网络
		if (quality_info_.bitrate_zero_count >= 5)
		{
            cur_network_status = StatusBarNetworkStatus::Network_Anomaly;
		}
		else//else这里可以认为码率是正常的，下面继续判断丢帧率
		{
			if (quality_info_.frame_loss_poor_count >= 5)
			{
                cur_network_status = StatusBarNetworkStatus::Network_Poor;
			}
			else if (quality_info_.frame_loss_secondary_count >= 5)
			{
                cur_network_status = StatusBarNetworkStatus::Network_Secondary;
			}
		}
        //判定直播质量
		if (quality_info_.quality_poor_count >= 5)
		{
            cur_streaming_quality = StatusBarStreamingQuality::Quality_Poor;
		}
		else if (quality_info_.quality_secondary_count >= 5)
		{
            cur_streaming_quality = StatusBarStreamingQuality::Quality_Secondary;
		}
        //更新状态
        UpdateQualityStatus(cur_network_status, cur_streaming_quality);
        //重新开始
        quality_info_.InitCalculationData();
    }
}

void LivehimeStatusBarView::UpdateQualityStatus(StatusBarNetworkStatus network_status, StatusBarStreamingQuality streaming_quality)
{
    if (network_status != cur_network_status_)
    {
        cur_network_status_ = network_status;
        if (cur_network_status_ == StatusBarNetworkStatus::Network_Good)
        {
            network_situation_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_NETWORK_GOOD));
            network_situation_bt_->SetText(L"网络较好");
        } 
        else if (cur_network_status_ == StatusBarNetworkStatus::Network_Secondary)
        {
            network_situation_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_NETWORK_SECONDARY));
            network_situation_bt_->SetText(L"网络中等");
        }
		else if (cur_network_status_ == StatusBarNetworkStatus::Network_Poor)
		{
            network_situation_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_NETWORK_POOR));
            network_situation_bt_->SetText(L"网络较差");
        }
		else if (cur_network_status_ == StatusBarNetworkStatus::Network_Anomaly)
		{
            network_situation_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_NETWORK_ANOMALY));
            network_situation_bt_->SetText(L"网络异常");
        }
    }

    if (streaming_quality != cur_streaming_quality_)
    {
        cur_streaming_quality_ = streaming_quality;
        if (cur_streaming_quality_ == StatusBarStreamingQuality::Quality_Good)
        {
            streaming_quality_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_QUALITY_GOOD));
            streaming_quality_bt_->SetText(L"直播质量较好");
        } 
        else if (cur_streaming_quality_ == StatusBarStreamingQuality::Quality_Secondary)
        {
            streaming_quality_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_QUALITY_SECONDARY));
            streaming_quality_bt_->SetText(L"直播质量中等");
        }
		else if (cur_streaming_quality_ == StatusBarStreamingQuality::Quality_Poor)
		{
            streaming_quality_bt_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_STATUSBAR_QUALITY_POOR));
            streaming_quality_bt_->SetText(L"直播质量较差");
        }
        StreamingQualityView::SetStreamingQuality(cur_streaming_quality_);
    }
}

void LivehimeStatusBarView::GetQuestionLinkInfo()
{

}