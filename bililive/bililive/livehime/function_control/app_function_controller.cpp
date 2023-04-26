#include "bililive/bililive/livehime/function_control/app_function_controller.h"

#include <regex>

#include "base/ext/callable_callback.h"
#include "base/json/json_reader.h"
#include "base/notification/notification_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/hybrid_ui/hybrid_ui.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/preset_material/preset_material_ui_presenter.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_mp4_player_viewer.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/user_info.h"
#include "bililive/bililive/livehime/obs/output_controller.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

namespace
{
    AppFunctionController* g_single_instance = nullptr;

    const int kApmReportTimeInterval = 5;  // 多少秒生成一条埋点数据
    const int kApmReportCumulant = 1;    // 积压多少条埋点数据然后一次性拼接发出

    // 网络、编码、渲染丢帧率异常阈值
    const float kAbnormalNetLoseRateThreshold = 0.4f;
    const float kAbnormalEdfLoseRateThreshold = 0.4f;
    const float kAbnormalRdfLoseRateThreshold = 0.4f;

}


AppFunctionController* AppFunctionController::GetInstance()
{
    return g_single_instance;
}

AppFunctionController::AppFunctionController():
    weak_ptr_factory_(this)
{
    g_single_instance = this;
}

AppFunctionController::~AppFunctionController()
{
    g_single_instance = nullptr;
}

void AppFunctionController::Initialize()
{
    pending_request_ = 2;
}


int AppFunctionController::get_media_video_bitRate(int parent_id) {
    if (live_area_video_bitrate_settings_.size()) {
        bool exist = false;
        for (auto item : live_area_video_bitrate_settings_) {
            if (item.parent_id == parent_id) {
                if (item.max_video_bitrate < 8000 && item.max_video_bitrate > 800) {
                    media_settings_.auto_stream_settings.max_video_bitrate = item.max_video_bitrate;
                }
                exist = true;
                break;
            }
        }
        //不在所配置的分区，就用默认码率
        if (default_max_video_bitrate_ > 0 && !exist) {
            media_settings_.auto_stream_settings.max_video_bitrate = default_max_video_bitrate_;
        }
    }
    return media_settings_.auto_stream_settings.max_video_bitrate;
}


const MediaSettings& AppFunctionController::media_settings() const
{
    if (media_settings_.valid)
    {
        return media_settings_;
    }

    static MediaSettings media_prefs_info;
    if (media_prefs_info.valid)
    {
        return media_prefs_info;
    }

    media_prefs_info.valid = true;

    // 自动码率的默认控制
    media_prefs_info.auto_stream_settings.auto_video_setting_switch = false;
    media_prefs_info.auto_stream_settings.stream_speed_test_coefficient = 0.8;
    media_prefs_info.auto_stream_settings.max_video_bitrate = 8000;
    media_prefs_info.auto_stream_settings.min_video_bitrate = 800;

    // 按以前硬写在View中的方式进行初始化

    // 码率
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 800, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_800)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 1000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_1000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 1500, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_1500)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 2000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_2000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 3000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_3000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 4000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_4000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 5000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_5000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 8000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_8000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 10000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_10000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 12000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_12000)) });
    media_prefs_info.video_settings.video_bitrate_list.push_back({ 14000, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_CORERATE_14000)) });

    media_prefs_info.video_settings.video_bitrate_default = 1500;

    // 码率控制下拉框
    media_prefs_info.video_settings.video_bitrate_ctrl_list.push_back({ "CBR", base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_BITRATE_CONTROL_CBR)) });
    media_prefs_info.video_settings.video_bitrate_ctrl_list.push_back({ "ABR", base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_BITRATE_CONTROL_ABR)) });

    media_prefs_info.video_settings.video_bitrate_ctrl_default = "CBR";

    // 帧率
    media_prefs_info.video_settings.video_fps_list.push_back({ 24, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_FRAMERATE_24)) });
    media_prefs_info.video_settings.video_fps_list.push_back({ 30, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_FRAMERATE_30)) });
    media_prefs_info.video_settings.video_fps_list.push_back({ 60, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_FRAMERATE_60)) });

    media_prefs_info.video_settings.video_fps_default = 30;

    // 输出分辨率
    media_prefs_info.video_settings.video_resolution_list.push_back({ "712x400", base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_STREAMING_RES_712)) });
    media_prefs_info.video_settings.video_resolution_list.push_back({ "1280x720", base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_STREAMING_RES_1280)) });
    media_prefs_info.video_settings.video_resolution_list.push_back({ "1920x1080", base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_STREAMING_RES_1920)) });

    media_prefs_info.video_settings.video_resolution_default = "1280x720";

    // 输出质量
    media_prefs_info.video_settings.video_quality_list.push_back({ 2, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_PERFORMANCE_BALANCE_2)) });
    media_prefs_info.video_settings.video_quality_list.push_back({ 1, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_PERFORMANCE_BALANCE_1)) });
    media_prefs_info.video_settings.video_quality_list.push_back({ 3, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_VIDEO_CMB_PERFORMANCE_BALANCE_3)) });

    media_prefs_info.video_settings.video_quality_default = 2;

    // 音频码率
    media_prefs_info.audio_settings.audio_bitrate_list.push_back({ 96, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_AUDIO_CMB_CORERATE_96)) });
    media_prefs_info.audio_settings.audio_bitrate_list.push_back({ 128, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_AUDIO_CMB_CORERATE_128)) });
    media_prefs_info.audio_settings.audio_bitrate_list.push_back({ 192, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_AUDIO_CMB_CORERATE_192)) });
    media_prefs_info.audio_settings.audio_bitrate_list.push_back({ 256, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_AUDIO_CMB_CORERATE_256)) });
    media_prefs_info.audio_settings.audio_bitrate_list.push_back({ 320, base::UTF16ToUTF8(GetLocalizedString(IDS_CONFIG_AUDIO_CMB_CORERATE_320)) });

    media_prefs_info.audio_settings.audio_bitrate_default = 256;

    // 编码器显卡黑名单
    media_prefs_info.codec_blacklist[prefs::kVideoStreamQSV] = {
        "Intel(R) Iris(R) Plus Graphics",
        "Intel(R) HD Graphics",
        "Intel(R) HD Graphics Family",
        "Intel(R) HD Graphics 4000",
        "Intel(R) HD Graphics 4400",
        "Intel(R) HD Graphics 4600"
    };

    return media_prefs_info;
}

void AppFunctionController::OnAllAppKVCtrlInfoCompleted()
{
    if (0 == pending_request_)
    {
        LOG(INFO) << "[KV] all kv ctrl info response completed.";
        FOR_EACH_OBSERVER(AppFunctionCtrlObserver, observer_list_, OnAllAppKVCtrlInfoCompleted());
    }
}

void AppFunctionController::RequestNewLiveRoom(int new_live) {

}

void AppFunctionController::RestoreFuncCtrlInfo()
{
    streaming_qmcs_ctrl_.switch_on = true;

    melee_ctrl_.battle_result_url = "";

    liveroom_cover_ctrl_.room_cover_setting_url.clear();
    liveroom_cover_ctrl_.room_cover_desc_url.clear();

    apm_ctrl_.valid = false;
}