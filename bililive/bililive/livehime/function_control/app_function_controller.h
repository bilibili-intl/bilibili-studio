#pragma once

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/timer/timer.h"
#include "base/strings/string16.h"


// 大乱斗相关
struct MeleeCtrl
{
    std::string battle_result_url;
};

// 推流质量检测相关
struct StreamingQMCSCtrl
{
    bool switch_on = true;  // 控制推流质量面板入口展示、推流质量数据上报
    bool notify_default = false; // 控制推流异常提示与否
    bool enable_auto_change_link = false;   // 推流异常自动切线路与否
    int remote_detect_duration = 300;   // 视频云推流质量信息获取时间间隔，单位秒
    int remote_report_duration = 60;   // 向视频云上报本地质量信息时间间隔，单位秒

    double local_net_lose_rate_threshold = 0.4f; // 网络丢帧异常阈值
    double local_edf_lose_rate_threshold = 0.4f; // 编码丢帧异常阈值
    double local_rdf_lose_rate_threshold = 0.4f; // 渲染丢帧异常阈值
};

// 直播封面设置相关
struct LiveRoomCoverCtrl
{
    std::string room_cover_setting_url;
    std::string room_cover_desc_url;
};

// APM设置相关
struct ApmCtrl
{
    bool valid = false;

    int apm_cumulant = 1;
    int apm_sample_interval = 5;
};

//APP TIME埋点上报控制
struct AppTimeCtrl {
    bool valid = true;
    int64 report_time = 60;
};

//语聊房插针开关和时间
struct ChatRoomSeiCtrl {
    bool valid = true;
    int sei_time = 1;
};

#pragma region (多媒体参数设置相关)

//分区默认码率设置相关

struct LiveAreaVideoBitrateSettings {
    int parent_id = 0;
    int max_video_bitrate = 0;
    std::string area_name;
};

struct BitrateFpsResolutionConfig {
    int bitrate = 0;
    int fps_select = 0;
    std::string video_resolution_select;
};

// 多媒体参数设置相关
struct VideoSettings
{
    std::vector<std::pair<int, std::string>> video_bitrate_list;    // 可选视频码率列表
    int video_bitrate_default;  // 默认视频码率

    std::vector<std::pair<int, std::string>> video_fps_list;    // 可选视频帧率列表
    int video_fps_default;  // 默认视频帧率

    std::vector<std::pair<std::string, std::string>> video_bitrate_ctrl_list;    // 可选视频码率控制列表
    std::string video_bitrate_ctrl_default;  // 默认视频码率控制

    std::vector<std::pair<std::string, std::string>> video_resolution_list;    // 可选分辨率列表
    std::string video_resolution_default;    // 默认分辨率

    std::string codec_device_default; //默认编码器

    std::vector<std::pair<int, std::string>> video_quality_list;    // 可选编码质量列表
    int video_quality_default;    // 默认编码质量
};

struct AudioSettings
{
    std::vector<std::pair<int, std::string>> audio_bitrate_list;    // 可选音频码率列表
    int audio_bitrate_default;  // 默认音频码率
};

struct AppFunctionSettings {
    bool new_live_room_switch = false;
    bool realname_auth_enabled = true;
};

struct AutoStreamSettings
{
    // 自动设置开关，不影响UI的展示，影响底层是否切实执行测速这个流程
    // 这个开关的主要作用是防止线上全面开放自动码率之后（最大允许8M码率），
    // 人均码率飙升从而受到视频云等第三方压力时，能进行线上热处理
    bool auto_video_setting_switch = false;
    double stream_speed_test_coefficient = 0.8f;   // 测速结果与码率设置系数
    int max_video_bitrate = 2000;   // 测速结果能应用的最大视频码率
    int min_video_bitrate = 800;   // 测速结果能应用的最小视频码率
};

struct MediaSettings
{
    bool valid = false;

    VideoSettings video_settings;
    AudioSettings audio_settings;
    AutoStreamSettings auto_stream_settings;

    std::vector<BitrateFpsResolutionConfig> bitrate_fps_resolution_settings;
    std::map<std::string, std::vector<std::string>> codec_blacklist;
};

struct CoreUserTagInfo
{
	int tag_id = 0;
	std::string tag_white_bg_url;
	std::string tag_dark_bg_url;
	std::string tag_white_bg_local_path;
	std::string tag_dark_bg_local_path;
};

#pragma endregion


class AppFunctionCtrlObserver
{
public:
    virtual void OnUpdateAppCtrlInfo() {}
    virtual void OnUpdateMediaSettingsInfo() {}

    virtual void OnAllAppKVCtrlInfoCompleted() {}

protected:
    virtual ~AppFunctionCtrlObserver() = default;
};

class AppFunctionController
{
public:
    static AppFunctionController* GetInstance();

    void AddObserver(AppFunctionCtrlObserver* observer) { observer_list_.AddObserver(observer); }
    void RemoveObserver(AppFunctionCtrlObserver* observer) { observer_list_.RemoveObserver(observer); }

    void Initialize();

    const MeleeCtrl& melee_ctrl() const
    {
        return melee_ctrl_;
    }

    const StreamingQMCSCtrl& streaming_qmcs_ctrl() const
    {
        return streaming_qmcs_ctrl_;
    }

    const LiveRoomCoverCtrl& liveroom_cover_ctrl() const
    {
        return liveroom_cover_ctrl_;
    }

    const ApmCtrl& apm_ctrl() const
    {
        return apm_ctrl_;
    }
    const AppFunctionSettings& app_function_settings() const
    {
        return app_function_settings_;
    }

    const std::string& get_jump_url() const
    {
        return activity_center_jump_more_url_;
    }

    const bool talk_topic_open() const
    {
        return (talk_topic_switch_ == "1" ? true : false);
    }

    const bool get_gamepk_switch() const
    {
        return (game_pk_switch_ == "1" ? true : false);
    }

    const bool get_vtuber_pk_switch() const
    {
        return (vtuber_pk_switch_ == "1" ? true : false);
    }

    const bool get_star_switch() const
    {
        return (star_switch_ == "1" ? true : false);
    }

    const bool get_plugin_switch() const
    {
        return (plugin_switch_ == "1" ? true : false);
    }

    const int get_plugin_check_timer() const
    {
        return std::atoi(check_timer_.c_str());
    }

    const bool get_plugin_open_switch() const
    {
        return (plugin_open_ == "1" ? true:false);
    }

    const MediaSettings& media_settings() const;

    int get_media_video_bitRate(int parent_id);   //通过分区id，获取默认下的视频码率

    const std::string& get_live_quality_report_paras()
    {
        return live_quality_report_paras_;
    }

    const bool get_danmaku_function_area_switch() const
    {
        return danmaku_function_area_switch_;
    }

    const double& get_memory_occupy() const
    {
        return memory_occupy_;
    }

    const bool get_vtuber_match() const
    {
        return (vtuber_match_ == "1" ? true : false);
    }

    const std::string& get_nebula_red() {
        return nebula_red_point_;
    }

    const bool get_dynamic_title_switch() {
        return (dynamic_title_ == "1" ? true : false);
    }

    const base::string16& get_high_rank() {
        return high_rank_text_;
    }

    const bool get_face_switch() const
    {
        return (face_switch_ == 1 ? true : false);
    }

    const int get_face_timeout() const
    {
        return face_timeout;
    }

    const bool get_log_report_switch() const
    {
        return log_report_switch_;
    }

	const bool get_webview_offline_switch() const{
        return webview_offline_switch_;
    }

    const bool get_webview_resource_switch() const {
        return webview_resource_switch_;
    }

    const bool get_vtuber_change_part_switch() const {
        return vtuber_change_part_switch_;
    }

    const double get_jsbridge_report_sampling_rat() const
    {
        return jsbridge_report_sampling_rat_;
    }


    const AppTimeCtrl get_apptime_kv_control() const {
        return apptime_kv_control_;
    }

    const ChatRoomSeiCtrl get_chatroom_sei_kv_control() const {
        return chatroom_sei_control_;
    }
    
    const bool get_chat_switch() const {
        return chat_room_switch_;
    }

    const bool get_chat_mini_switch() const
    {
        return chat_mini_switch_;
    }

    const int get_chat_free_timeout() const
    {
        return chat_multi_timeout_;
    }

    const bool get_chat_back_setting_switch() const {
        return chat_room_back_setting_switch_ == "1";
    }

	const std::string& get_hot_rank_url() const
	{
		return hot_rank_url_;
	}

	const bool fans_rank_open() const
	{
		return (fans_rank_switch_ == "1" ? true : false);
	}

    const bool area_rank_open() const
    {
        return (area_rank_switch_ == "1" ? true : false);
    }

    const bool get_gift_panel_switch() const
    {
        return gift_panel_switch_;
    }

    const std::string& get_gift_panel_url() {
        return gift_panel_url_;
    }

    const std::map<int,CoreUserTagInfo>& get_core_user_tag_info()
    {
        return core_user_tag_info_;
    }
    const int get_rtc_realtime_interval() {
        return rtc_realtime_interval_;
    }

private:
    AppFunctionController();
    ~AppFunctionController();

    void OnGetFuncControlInfo(bool valid, int code, const std::string& err_msg,
        const std::string& data);
    void OnGetMediaSettingsInfo(bool valid, int code, const std::string& err_msg,
        const std::string& data);
    void OnAllAppKVCtrlInfoCompleted();

    void RestoreFuncCtrlInfo();
    void RequestNewLiveRoom(int new_live);

private:
    friend class BililiveOBS;
    friend struct base::DefaultDeleter<AppFunctionController>;

    int default_max_video_bitrate_ = 0;
    int pending_request_ = -1;
    MeleeCtrl melee_ctrl_;
    StreamingQMCSCtrl streaming_qmcs_ctrl_;
    LiveRoomCoverCtrl liveroom_cover_ctrl_;
    ApmCtrl apm_ctrl_;
    MediaSettings media_settings_;

    std::vector<LiveAreaVideoBitrateSettings> live_area_video_bitrate_settings_;
    std::vector<BitrateFpsResolutionConfig> bitrate_fps_resolution_settings_;

    double memory_occupy_ = 0.0f;

    std::string activity_center_jump_more_url_;

    std::string talk_topic_switch_ = "0";
    std::string game_pk_switch_ = "0";
    std::string fans_rank_switch_ = "0";
    std::string area_rank_switch_ = "0";//分区榜&功能榜开关

    std::string vtuber_pk_switch_ = "0";

    std::string star_switch_ = "1";

    std::string plugin_switch_ = "1";
    std::string check_timer_ = "0";
    std::string plugin_open_ = "0";

    std::string vtuber_match_ = "1";

    std::string nebula_red_point_ = "-1";
    //脸萌开关
    int face_switch_ = 1;
    int face_timeout = 0;

    //粉丝团URL
    std::string  hot_rank_url_;


    std::string live_quality_report_paras_;
    // 动态头衔
    std::string dynamic_title_ = "1";
    bool danmaku_function_area_switch_ = true;

    bool webview_offline_switch_ = false;
    bool webview_resource_switch_ = false;
    //高能榜文案
    base::string16 high_rank_text_ = L"";

    // 语聊房白名单kv
    bool chat_mini_switch_ = false;
    int  chat_multi_timeout_ = 7;
    bool chat_room_switch_ = false;
    std::string chat_room_back_setting_switch_ = "0";

    // 礼物面板开关
    bool gift_panel_switch_ = true;
    std::string gift_panel_url_;

    bool log_report_switch_ = false;

    bool vtuber_change_part_switch_ = true;

    //jsbridge采样率0-1
    double jsbridge_report_sampling_rat_ = 1.0f;

    //apptime 使用时长间隔上报
    AppTimeCtrl apptime_kv_control_;
	//语聊房插针开关
    ChatRoomSeiCtrl chatroom_sei_control_;
    //核心付费用户弹幕标签图片下载地址相关
    std::map<int,CoreUserTagInfo>core_user_tag_info_;
	
	int rtc_realtime_interval_ = 60;
    
    AppFunctionSettings app_function_settings_;

    ObserverList<AppFunctionCtrlObserver> observer_list_;

    base::WeakPtrFactory<AppFunctionController> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(AppFunctionController);
};