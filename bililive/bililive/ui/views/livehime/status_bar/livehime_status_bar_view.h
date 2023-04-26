#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STATUS_BAR_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STATUS_BAR_VIEW_H

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/obs_status.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/livehime/streaming_report/streaming_report_service.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/livehime/feedback/feedback_presenter.h"
#include "livehime_streaming_quality_view.h"

namespace
{
    class RecordLiveTimeView;
    struct StreamingQualityInfo
    {
        bool is_streaming = false;//是否开播
        unsigned int start_count = 0;//开始直播计数，开始直播前15秒不统计

        int cal_count = 0;//统计总次数，每秒1次
        int bitrate_zero_count = 0;//码率为0次数
        int frame_loss_secondary_count = 0;//0% < 丢帧率 <= 10%次数
        int frame_loss_poor_count = 0;//丢帧率 > 10%次数
        int quality_secondary_count = 0;//直播质量中等次数
        int quality_poor_count = 0;//直播质量较差次数

		float encode_loss_rate = 0.0f;//0%-0.5%-1%
		float render_loss_rate = 0.0f;//0%-0.5%-1%
        double bitrate = 0.0f;//0kbps-100kbps-...
        double frame_loss_rate = 0.0f;//0%-10%-100%

        void InitCalculationData()
        {
			cal_count = 0;
			bitrate_zero_count = 0;
			frame_loss_secondary_count = 0;
			frame_loss_poor_count = 0;
			quality_secondary_count = 0;
			quality_poor_count = 0;
        }
    };
}

class BililiveLabelButton;
class LivehimeSmallContentLabel;
class LivehimeLinkButton;
class BililiveImageButton;
class LivehimeStatusBarView
    : public views::View
    , public bililive::OBSStatusDelegate
    , public LivehimeLiveRoomObserver
    , public BililiveBroadcastObserver
    , public StreamingReportObserver
    , public views::ButtonListener
    , AppFunctionCtrlObserver
{
public:
    LivehimeStatusBarView();
    virtual ~LivehimeStatusBarView();

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    void ChildPreferredSizeChanged(views::View* child) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    int GetHeightForWidth(int w) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LivehimeLiveRoomObserver
    void OnPreLiveRoomStartStreaming(bool is_restreaming) override;
    void OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code) override;
    void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;

    // ObsStatusDelegate
    void OnStreamingMetricsFeedback(double bandwidth_speed, double frame_loss_rate) override;
    void OnStreamEncodedQPFeedback(int qp) override;

    //BililiveBroadcastObserver
    void RoomAudience(int64_t audience) override;

    // StreamingReportObserver
    void OnStreamingReportSampling(const StreamingReportInfo& info) override;
    void OnStreamingWarning(StreamingWarning warning) override;

    // AppFunctionCtrlObserver
    void OnUpdateAppCtrlInfo() override;
    void OnAllAppKVCtrlInfoCompleted() override;

private:
    void InitViews();
    void UpdateFps();
    string16 SpeedInKByteToFriendlyString(double speedInKBps);
    void UpdateStreamingStatus();
    void UpdateQualityValue();
    void UpdateQPViews(int averaged_qp);
    void SetQualityCheckingEnabled(bool enabled);
    void StartStreamingTimer();
    void StopStreamingTimer();
    void ResetStreamingStatus();

    void JudgeMemoryOccupy(bool is_nv);

    void RequestLiveReplaySetting();

    void GetFeedbackList();

    void AnalysisLiveQuality();
    void UpdateQualityStatus(StatusBarNetworkStatus network_status, StatusBarStreamingQuality streaming_quality);

	void GetQuestionLinkInfo();

private:
    std::unique_ptr<bililive::OBSStatusMonitor> obsmonitor_;

    StreamingQualityInfo quality_info_;
    StatusBarNetworkStatus cur_network_status_ = StatusBarNetworkStatus::Network_Good;
    StatusBarStreamingQuality cur_streaming_quality_= StatusBarStreamingQuality::Quality_Good;
    views::View* data_details_view_ = nullptr;
    views::View* streaming_quality_view_ = nullptr;
    LivehimeSmallContentLabel* bitrate_label_ = nullptr;
    LivehimeSmallContentLabel* fps_label_ = nullptr;
    LivehimeSmallContentLabel* frame_loss_label_ = nullptr;
    LivehimeSmallContentLabel* cpu_label_ = nullptr;
    LivehimeSmallContentLabel* memory_label_ = nullptr;
    LivehimeSmallContentLabel* quality_label_ = nullptr;
    LivehimeSmallContentLabel* status_label_ = nullptr;
    BililiveLabelButton* network_situation_bt_ = nullptr;
    BililiveLabelButton* streaming_quality_bt_ = nullptr;
    LivehimeImageRightButton* extend_button_ = nullptr;

    std::deque<int> qp_deque_;
    std::unique_ptr<base::RepeatingTimer<LivehimeStatusBarView>> streaming_timer_;
    bool is_quality_checking_enabled_ = true;
    bool start_event_ = false;
    int last_send_frames_ = 0;

    int64_t max_audience_ = 0;
    int64_t fans_ = 0;
    int64_t add_fans_ = 0;
    int64_t score_ = 0;
    int64_t add_score_ = 0;
    base::Time live_start_time_;
    int64_t live_duration_ = 0;

    base::WeakPtrFactory<LivehimeStatusBarView> weak_ptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(LivehimeStatusBarView);
};

#endif