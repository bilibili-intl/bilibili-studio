#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TOOLBAR_TOP_TOOLBAR_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TOOLBAR_TOP_TOOLBAR_VIEW_H

#include <mutex>
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/controls/bilibili_search_list_view.h"

class BililiveLabel;
class BililiveImageButton;
class BililiveLabelButton;
class LivehimeCapsuleButton;
class BililiveBubbleView;
class BililiveSingleChildShowContainerView;
class LivehimeImageView;
class LivehimeLinkButton;
class LivehimeCoverModeButton;

#define NUMBER_CONVERT(number) \
    if (number >= 100000000) { \
        show_count = base::StringPrintf(L"%0.1f hundred million", number / 100000000.0f); \
    } \
    else if (number >= 10000) { \
        show_count = base::StringPrintf(L"%0.1f wan", number / 10000.0f); \
        if (show_count == L"10000.0 wan") { \
        show_count = L"1.0 hundred million"; \
        } \
    } \
    else { \
        show_count = base::Int64ToString16(number); \
    }

namespace
{
    class TopToolbarRoomTitleEdit;
    class BookLiveTimeView;
    class PartitionButton;
    class BililiveMultipleView;
    class CoverButtonView;
    class ToolRandomButtonView;
}

class LivehimeTopToolbarViddupView
    : public views::View
    , public views::ButtonListener
    , BilibiliSearchListListener
    , public base::NotificationObserver
    , LivehimeLiveRoomObserver
    , views::WidgetObserver
    , AppFunctionCtrlObserver
    , public BililiveBroadcastObserver
{
public:
    LivehimeTopToolbarViddupView();
    virtual ~LivehimeTopToolbarViddupView();

    struct CompletedMarkInfo {
        int order_type = 0;
        int show_times_in_sec = 0;
    };

    void RequestCoverStatus(bool inited);
    void RoomTitleEditFocus();

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    //void Layout() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // WidgetObserver
    void OnWidgetClosing(views::Widget* widget) override;

    // BilibiliSearchListListener
    void OnSearchListResultSelected(BilibiliSearchListView* search_edit,
                                    const base::string16 &text, int64 data) override;
    void OnSearchEditContentsChanged(BilibiliSearchListView* search_edit, const string16& new_contents) override;
    void OnSearchEditFocus(BilibiliSearchListView* search_edit) override;
    void PreSearchEditBlur(BilibiliSearchListView* search_edit, const gfx::Point& pt_in_screen) override;
    void OnSearchEditBlur(BilibiliSearchListView* search_edit, BiliveSearchListBlurChangeReason reason) override;

    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    // LivehimeLiveRoomObserver
    void OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;
    void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) override;
    void OnBookLiveInfoChanged(int type, const base::string16& book_info) override;
    void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;

    // AppFunctionCtrlObserver
    void OnAllAppKVCtrlInfoCompleted() override;

    // BililiveBroadcastObserver
    void RoomAudience(int64_t audience) override;

    void UpdateWatchedChange(int64_t number);
    void UpdateLikeNumberChange(int64_t number);

    void OnUpdateTitle(bool success, int err_code, const std::string& error_msg);

    void SetAreaName();

    void ShowTalkBubble();

    void ShowUpdateView();

    void OnGetCoverImage(bool is_vert, bool valid, int code, std::string& image_data, int audit_status);

private:
    void InitViews();
    void ShowTitleEdit(bool show);
    void UpdateTitle(const base::string16& title);
    void SetLiveRoomTitle();
    void SaveTitleHistory();
    void RefreshTitleEdit();
    std::vector<string16> GetTitleHistroy();
    void OnCpmOrderCompleted(int order_type);
    void AddCompletedMarks(int order_type);
    void ShowCompletedMark();
    void HideCompletedMark();
    void OnUpdateTitleErrorMsgBoxHandle(const base::string16&, void* data);
    void RefreshRoomInfo();

    void OnDelayTalkBubbleTask();
    void OnRequestGetCoverRedPointShowRes(bool valid_response, int code, const bool& need_red_point);
    void GetCoverRedPointShow(bool need_report);

    void OnPartionChanged();
    void OnPopDialog();

    void SetLikeInfoVisible(bool visible);

    void SetFansRankVisible(bool visible);

    void SetHotRankInfoVisible();

    void OnCountDownTimer();

private:

    BililiveLabelButton* room_title_button_ = nullptr;
    ToolRandomButtonView* change_title_button_ = nullptr;
    views::View* title_button_area_view_ = nullptr;
    TopToolbarRoomTitleEdit* room_title_edit_ = nullptr;
    BililiveLabel* title_num_label_ = nullptr;

    //PartitionButton* partition_button_ = nullptr;
    //PartitionButton* cover_button_ = nullptr;
    //PartitionButton* talk_subject_button_ = nullptr;
    PartitionButton* notice_button_ = nullptr;
    BililiveMultipleView* topic_area_view_ = nullptr;
    BookLiveTimeView* book_view_ = nullptr;
    BililiveSingleChildShowContainerView* title_area_view_ = nullptr;
    LivehimeImageView* main_button_ = nullptr;
    BililiveLabel* audience_num_label_ = nullptr;
    BililiveLabel* like_num_label_text_;
    BililiveLabel* like_num_label_count_;
    LivehimeCoverModeButton* partition_button_ = nullptr;
    LivehimeCoverModeButton* talk_subject_button_ = nullptr;
    LivehimeLinkButton* share_button_ = nullptr;

    views::View* button_area_view_ = nullptr;

    int current_order_type_ = 0;
    bool need_show_bubble_ = false;
    bool completed_mark_ = false;
    bool completed_showing_ = false;
    std::list<CompletedMarkInfo> completed_notice_;
    base::Closure completed_task_;
    bool is_show_talk_bubble = false;
    bool vtuber_partion_ = false;
    //bool vtuber_ornament_open_ = false;
    bool vtuber_storm_open_ = false;
    bool like_count_show_ = true;
    int64_t cur_topic_id_ = 0;
    int64_t cur_act_id_ = 0;

    bool fist_live_ = false;
    base::Time pre_Live_time_;
    base::NotificationRegistrar notifation_register_;
    bool cover_show_need_report_ = false;

    BililiveBubbleView* preview_button_guide_bubble_ = nullptr;
    std::mutex watched_mtx_;

    base::string16 default_room_title_;

    int countdown_ = 0;

    base::RepeatingTimer<LivehimeTopToolbarViddupView> hot_rank_timer_;

    bool  hot_rank_broadcast_switch_ = true;
   
    base::WeakPtrFactory<LivehimeTopToolbarViddupView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeTopToolbarViddupView);
};


#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TOOLBAR_TOP_TOOLBAR_VIEW_H