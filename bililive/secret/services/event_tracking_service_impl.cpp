#include "bililive/secret/services/event_tracking_service_impl.h"

#include <regex>
#include <tuple>

#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/guid.h"
#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"

#include "net/http/http_request_headers.h"

#include "url/gurl.h"

#include "bililive/bililive/livehime/obs/obs_inject_util.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/services/service_utils.h"
#include "bililive/public/log_ext/log_constants.h"


namespace {

using secret::ResponseInfo;
using secret::LivehimeBehaviorEvent;
using secret::EthernetAddressInfo;

const char kLivehimeTaskID[] = "";
const char kLivehimePlatform[] = "";
const char kLivehimeBiz[] = "";

const std::map<LivehimeBehaviorEvent, std::string> kLivehimeEventTable
{
    { LivehimeBehaviorEvent::LivehimeInstall, "zbj_install" },
    { LivehimeBehaviorEvent::LivehimeLogin, "zbj_login" },
    { LivehimeBehaviorEvent::LivehimeSource, "zbj_source" },
    { LivehimeBehaviorEvent::LivehimeSet, "zbj_prefs" },
    { LivehimeBehaviorEvent::LivehimeCameraFilter, "zbj_camera_filter" },
    { LivehimeBehaviorEvent::LivehimeDPI, "zbj_monitor_info" },
    { LivehimeBehaviorEvent::LivehimeRoomInfo, "zbj_set_room_info" },
    { LivehimeBehaviorEvent::LivehimeDanmaku, "zbj_danmuku_more_history" },
    { LivehimeBehaviorEvent::LivehimeDanmakuHimeState, "zbj_danmakuhime_state" },
    { LivehimeBehaviorEvent::LivehimeCameraProperties, "zbj_camera" },
    { LivehimeBehaviorEvent::LivehimeLiveDuration, "zbj_live_duration" },
    { LivehimeBehaviorEvent::LivehimeStreamingOutcome, "zbj_streaming_outcome" },
    { LivehimeBehaviorEvent::LivehimeCrash, "zbj_crash" },
    { LivehimeBehaviorEvent::LivehimeGiftboxClick, "zbj_gift_box_click" },
    { LivehimeBehaviorEvent::LivehimeIncompleteAvatar, "zbj_incomplete_avatar" },

    { LivehimeBehaviorEvent::LivehimeMorePrejection, "zbj_more_projection" },
    { LivehimeBehaviorEvent::LivehimeMoreHelp, "zbj_more_help" },
    { LivehimeBehaviorEvent::LivehimeMoreVersionsCheck, "zbj_more_versions_check" },
    { LivehimeBehaviorEvent::LivehimeMoreLogfile, "zbj_more_logfile" },
    { LivehimeBehaviorEvent::LivehimeSceneChange, "zbj_scene_change" },
    { LivehimeBehaviorEvent::LivehimeSceneTop1, "zbj_scene_top1" },
    { LivehimeBehaviorEvent::LivehimeSceneLow1, "zbj_scene_low1" },
    { LivehimeBehaviorEvent::LivehimeSceneTop, "zbj_scene_top" },
    { LivehimeBehaviorEvent::LivehimeSceneLow, "zbj_scene_low" },
    { LivehimeBehaviorEvent::LivehimeSceneCompile, "zbj_scene_compile" },
    { LivehimeBehaviorEvent::LivehimeSceneHide, "zbj_scene_hide" },
    { LivehimeBehaviorEvent::LivehimeSceneLock, "zbj_scene_lock" },
    { LivehimeBehaviorEvent::LivehimeSceneDelete, "zbj_scene_delete" },
    { LivehimeBehaviorEvent::LivehimeRightKeyOriginalSize, "zbj_rightkey_original_size" },
    { LivehimeBehaviorEvent::LivehimeRightKeyFullScreen, "zbj_rightkey_full_screen" },
    { LivehimeBehaviorEvent::LivehimeRightKeyTop, "zbj_rightkey_top" },
    { LivehimeBehaviorEvent::LivehimeRightKeyLow, "zbj_rightkey_low" },
    { LivehimeBehaviorEvent::LivehimeRightKeyZoom, "zbj_rightkey_zoom" },
    { LivehimeBehaviorEvent::LivehimeRightKeyStretch, "zbj_rightkey_stretch" },
    { LivehimeBehaviorEvent::LivehimeRightKeyCut, "zbj_rightkey_cut" },
    { LivehimeBehaviorEvent::LivehimeRightKeyDelete, "zbj_rightkey_delete" },
    { LivehimeBehaviorEvent::LivehimeRightKeyInteraction, "zbj_rightkey_interaction" },
    { LivehimeBehaviorEvent::LivehimeRightKeyProperty, "zbj_rightkey_property" },
    { LivehimeBehaviorEvent::LivehimeBarrageRoom, "zbj_barrage_room" },
    { LivehimeBehaviorEvent::LivehimeBarrageReport, "zbj_barrage_report" },
    { LivehimeBehaviorEvent::LivehimeBarrageBlackList, "zbj_barrage_blacklist" },
    { LivehimeBehaviorEvent::LivehimeBarrageManagerSet, "zbj_barrage_manager_set" },
    { LivehimeBehaviorEvent::LivehimeBarrageBanned, "zbj_barrage_banned" },
    { LivehimeBehaviorEvent::LivehimeBarrageNameList, "zbj_barrage_namelist" },
    { LivehimeBehaviorEvent::LivehimeBarrageSearch, "zbj_barrage_search" },
    { LivehimeBehaviorEvent::LivehimeBarrageFlush, "zbj_barrage_flush" },
    { LivehimeBehaviorEvent::LivehimeHoverMyRoom, "zbj_hover_myroom" },
    { LivehimeBehaviorEvent::LivehimeHoverMyRoom1, "zbj_hover_myroom1" },
    { LivehimeBehaviorEvent::LivehimeHoverLiveData, "zbj_hover_live_data" },
    { LivehimeBehaviorEvent::LivehimeGuideType, "zbj_guide_type" },
    { LivehimeBehaviorEvent::LivehimeGuideMain, "zbj_guide_main" },
    { LivehimeBehaviorEvent::LivehimeGuidePcgame, "zbj_guide_pcgame" },
    { LivehimeBehaviorEvent::LivehimeGuideMobilegame, "zbj_guide_mobilegame_down_load" },
    { LivehimeBehaviorEvent::LivehimeGuideRecreation, "zbj_guide_recreation" },
    { LivehimeBehaviorEvent::LivehimeGuideDrawing, "zbj_guide_drawing" },
    { LivehimeBehaviorEvent::LivehimeGuideCourse, "zbj_guide_course" },
    { LivehimeBehaviorEvent::LivehimeGuideRadio, "zbj_guide_radio" },

    { LivehimeBehaviorEvent::LivehimeGameAdaptation, "zbj_game_adaptation" },
    { LivehimeBehaviorEvent::LivehimeGuideGameAdaptation, "zbj_guide_game_adaptation" },
    { LivehimeBehaviorEvent::LivehimeMobileGame, "zbj_mobilegame" },
    { LivehimeBehaviorEvent::LivehimeGuideMobileGame, "zbj_guide_mobilegame" },
    { LivehimeBehaviorEvent::LivehimeSceneRename, "zbj_scene_rename" },
    { LivehimeBehaviorEvent::LivehimeSceneRename1, "zbj_scene_rename1" },
    { LivehimeBehaviorEvent::LivehimeComputerSystem, "zbj_computer_system" },

    { LivehimeBehaviorEvent::LivehimeLiveData, "zbj_live_data" },

    { LivehimeBehaviorEvent::LivehimeMeleeChaosfightActivityShow, "chaosfight_activity_show" },
    { LivehimeBehaviorEvent::LivehimeMeleeChaosfightActivityClick, "chaosfight_activity_click" },
    { LivehimeBehaviorEvent::LivehimeMeleeChaosfightInfocardShow, "chaosfight_infocard_show" },

    { LivehimeBehaviorEvent::LivehimeMecha, "zbj_mecha" },

    { LivehimeBehaviorEvent::LivehimeVlinkClick, "stream_vcommunication_click" },
    { LivehimeBehaviorEvent::LivehimeVlinkSwitch, "stream_vcommunication_switch" },
    { LivehimeBehaviorEvent::LivehimeVlinkListShow, "stream_vcommunication_application_show" },
    { LivehimeBehaviorEvent::LivehimeVlinkReject, "stream_vcommunication_application_reject" },
    { LivehimeBehaviorEvent::LivehimeVlinkToBlacklist, "stream_vcommunication_application_shield" },
    { LivehimeBehaviorEvent::LivehimeVlinkConditionClick, "stream_vcommunication_condition_click" },
    { LivehimeBehaviorEvent::LivehimeVlinkConditionConfirmed, "stream_vcommunication_condition_confirm" },
    { LivehimeBehaviorEvent::LivehimeVlinkConnect, "stream_vcommunication_application_connect" },
    { LivehimeBehaviorEvent::LivehimeVlinkHangup, "stream_vcommunication_application_hangup" },
    { LivehimeBehaviorEvent::LivehimeVlinkStart, "voice_connection_start" },
    { LivehimeBehaviorEvent::LivehimeVlinkSelfSucceeded, "voice_connection_self_success" },
    { LivehimeBehaviorEvent::LivehimeVlinkRemoteSucceeded, "voice_connection_other_success" },
    { LivehimeBehaviorEvent::LivehimeVlinkEnd, "voice_connection_end" },
    { LivehimeBehaviorEvent::LivehimeVlinkError, "voice_connection_error" },
    { LivehimeBehaviorEvent::LivehimeQuicStreaming, "quic_streaming"},
    { LivehimeBehaviorEvent::LivehimeSetTags, "zbj_set_tags" },
    { LivehimeBehaviorEvent::LivehimeUpgrade, "zbj_upgrade" },
    //{ LivehimeBehaviorEvent::LivehimeCameraBeautySetting, "beautycamera_option" },
    { LivehimeBehaviorEvent::LivehimeFilterNameBeauty, "filtername_beauty" },
    { LivehimeBehaviorEvent::LivehimeLotteryClick, "anchordraw_icon_anchor_click" },
    { LivehimeBehaviorEvent::LivehimeExceptNames, "except_func_names" },
    { LivehimeBehaviorEvent::LivehimeUpgradeDownload, "zbj_upgrade_download" },
    { LivehimeBehaviorEvent::LivehimeMissionActivityClick, "zbj_activity" },
    { LivehimeBehaviorEvent::LivehimeMissionActivityProjection, "zbj_websource_new" },
    { LivehimeBehaviorEvent::LivehimeRecordError, "zbj_record_error" },
    { LivehimeBehaviorEvent::LivehimeCefError, "zbj_cef_error" },
    { LivehimeBehaviorEvent::LivehimeBannerOperate, "zbj_banner_operate" },
    { LivehimeBehaviorEvent::LivehimeStatusBarOperate, "zbj_statusbar_operate" },
    { LivehimeBehaviorEvent::LivehimeVideoConnError, "video_connection_error" },
    { LivehimeBehaviorEvent::LivehimeVideoConnStart, "video_connection_start" },
    { LivehimeBehaviorEvent::LivehimeVideoConnSelfSuccess, "video_connection_self_success" },
    { LivehimeBehaviorEvent::LivehimeVideoConnOtherSuccess, "video_connection_other_success" },
    { LivehimeBehaviorEvent::LivehimeVideoConnEnd, "video_connection_end" },
    { LivehimeBehaviorEvent::LivehimeVideoEditClickClip, "video_edit_click_clip" },
    { LivehimeBehaviorEvent::LivehimeVideoEditClickBtn, "video_edit_click_btn" },
    { LivehimeBehaviorEvent::LivehimeIntegrityChecking, "zbj_integrity_checking" },

    { LivehimeBehaviorEvent::LivehimeStreamingQMCSShow, "zbj_streaming_qmcs_show" },

    { LivehimeBehaviorEvent::LivehimeStreamQuality, "zbj_stream_quality" },
    { LivehimeBehaviorEvent::LivehimeLocalStreamQuality, "zbj_local_stream_quality" },
    { LivehimeBehaviorEvent::LivehimeColiveSdkReport, "zbj_colive_sdk_report" },
	{ LivehimeBehaviorEvent::LivehimeLogFilesUpload, "zbj_log_files_report" },

    { LivehimeBehaviorEvent::LivehimeColiveStart, "zbj_colive_start" },
    { LivehimeBehaviorEvent::LivehimeColiveReply, "zbj_colive_reply" },
    { LivehimeBehaviorEvent::LivehimeColiveFlow, "zbj_colive_flow" },
    { LivehimeBehaviorEvent::LivehimeColiveMatched, "zbj_colive_matched" },

    { LivehimeBehaviorEvent::LivehimeUploadVirtualConnectionZip, "zbj_vtuber_upload" },

    { LivehimeBehaviorEvent::LivehimeWebviewOfflineZipDownload, "zbj_webviewzip_download" },
    { LivehimeBehaviorEvent::LivehimeWebviewOfflineUnZip, "zbj_webviewzip_unzip" },
    { LivehimeBehaviorEvent::LivehimeWebviewOfflineUse, "zbj_webviewzip_use" },
    { LivehimeBehaviorEvent::LivehimeWebviewOfflineLoadTime, "zbj_webview_loading_time" },
    { LivehimeBehaviorEvent::LivehimeWebviewLoadSucc, "zbj_webview_loading_succ" },
    { LivehimeBehaviorEvent::LivehimeWebviewOfflineFailCount, "zbj_webview_offline_resource_fails" },
    { LivehimeBehaviorEvent::LivehimeIosWiredStatus, "blink-screen.projection.wireline-result.track" },

    { LivehimeBehaviorEvent::LivehimeMultiVideoConnInfoReport, "zbj_multi_video_conn_info_report" },
    { LivehimeBehaviorEvent::LivehimeInteractPlayOpen, "zbj_interact_play_open_track" },
    { LivehimeBehaviorEvent::LivehimeScreenProjectionShow, "zbj_screen_projection_success_show" },
   
};

using DefaultHandler = std::function<void(bool)>;

// We now do nothing even if the report fails.
void HandleReportResponse(bool)
{}

std::string LocalTime()
{
    base::Time::Exploded time_exploded;
    base::Time::Now().LocalExplode(&time_exploded);

    return base::StringPrintf("%d%02d%02d%02d%02d%02d",
                              time_exploded.year,
                              time_exploded.month,
                              time_exploded.day_of_month,
                              time_exploded.hour,
                              time_exploded.minute,
                              time_exploded.second);
}

std::string MakeKeyValue(const std::string& key, const std::string& value)
{
    return key + "=" + net::EscapeQueryParamValue(value, false);
}

std::vector<std::string> FormatCommonParams(int64_t mid,
    const std::string& event_msg, const std::string& buvid) {
    using namespace secret;

    return {BililiveContext::Current()->GetExecutableVersionAsASCII(),
            std::to_string(mid),
            buvid,
            kLivehimePlatform,
            kLivehimeBiz,
            GetUnixTimestamp<std::chrono::milliseconds>(),
            event_msg};
}

std::string FormatLivehimeEventParams(
    LivehimeBehaviorEvent event_id,
    int64_t mid,
    const std::string& event_msg,
    const std::string& buvid)
{
    using namespace secret;

    std::string content;
    content.append(kLivehimeTaskID).append(GetUnixTimestamp<std::chrono::milliseconds>());

    std::vector<std::string> vec;
    vec.push_back(kLivehimeEventTable.at(event_id));
    auto common_vec = FormatCommonParams(mid, event_msg, buvid);
    vec.insert(vec.end(), common_vec.begin(), common_vec.end());

    content.append(JoinString(vec, "|"));

    return content;
}

std::string FormatLivehimeEventParams(
    const std::string& event_id,
    int64_t mid,
    const std::string& event_msg,
    const std::string& buvid)
{
    using namespace secret;

    std::string content;
    content.append(kLivehimeTaskID).append(GetUnixTimestamp<std::chrono::milliseconds>());

    std::vector<std::string> vec;
    vec.push_back(event_id);
    auto common_vec = FormatCommonParams(mid, event_msg, buvid);
    vec.insert(vec.end(), common_vec.begin(), common_vec.end());

    content.append(JoinString(vec, "|"));

    return content;
}


// ip地址获取
using GetEthernetAddressResult = std::tuple<bool, EthernetAddressInfo>;
using GetEthernetAddressParser = std::function<GetEthernetAddressResult(ResponseInfo, const std::string&)>;
GetEthernetAddressResult ParseSetupEthernetAddressInfoResponse(ResponseInfo info, const std::string& data)
{
    if (info.response_code == 200)
    {
        // HTML转码
        base::string16 raw = net::UnescapeForHTML(base::UTF8ToUTF16(data));

        // 处理一下内容文本，把链接地址分离出来
        try
        {
            EthernetAddressInfo addr;

            static std::wregex ip_reg(LR"(IP\t: (\d+.\d+.\d+.\d+))");
            static std::wregex addr_reg(LR"(地址\t: (.+))");
            static std::wregex oper_reg(LR"(运营商\t: (.+))");

            auto fn = [&](const std::wregex& reg, std::wstring* pstr)
            {
                std::wsmatch mat;
                if (std::regex_search(raw, mat, reg))
                {
                    if (mat.size() > 1)
                    {
                        *pstr = mat[1];
                    }
                }
            };

            std::wstring ip;
            fn(ip_reg, &ip);
            fn(addr_reg, &addr.addr);
            fn(oper_reg, &addr.communications_operator);

            addr.ip = base::UTF16ToUTF8(ip);

            return GetEthernetAddressResult(true, addr);
        }
        catch (std::regex_error &ex)
        {
            NOTREACHED() << ex.what();
        }
    }
    else
    {
        LOG(WARNING) << "Parse ethernet address info response failure: invalid status!\n"
            << "code: " << info.response_code;
    }

    return GetEthernetAddressResult(false, {});
}

using GetEthernetAddressHandler = std::function<void(bool, const EthernetAddressInfo&)>;

// 埋点常规处理
using EventTrackingResult = std::tuple<bool>;
using EventTrackingParser = std::function<EventTrackingResult(ResponseInfo, const std::string&)>;

EventTrackingResult ParseEventTrackingResponse(ResponseInfo info, const std::string& data)
{
    if (info.response_code != 200) {
        LOG(WARNING) << "Parse event_tracking response failure: invalid status!\n"
                     << "response info: " << info;
        return EventTrackingResult(false);
    }

    return EventTrackingResult(true);
}

using GetBuvidHandler = std::function<void(const std::string&)>;
using GetBuvidResult = std::tuple<std::string>;
using GetBuvidParser = std::function<GetBuvidResult(ResponseInfo, const std::string&)>;

GetBuvidResult ParseGetBuvidResponse(ResponseInfo info, const std::string& data)
{
    if (info.response_code != 200 || data.empty()) {
        LOG(WARNING) << "Parse get-buvid response failure: invalid status or empty data\n"
                     << "response info: " << info;
        return {};
    }

    return GetBuvidResult(data);
}

base::FilePath GetBuvidSaveDir()
{
    return GetSecretCore()->global_profile_path();
}

}   // namespace

namespace secret {

secret::EthernetAddressInfo EventTrackingServiceImpl::ethernet_address_info_;

EventTrackingServiceImpl::EventTrackingServiceImpl(RequestConnectionManager* manager)
    : conn_manager_(manager),
      fts_("0"),
      weak_ptr_factory_(this)
{
    base::PlatformFileInfo file_info;
    if (file_util::GetFileInfo(BililiveContext::Current()->GetMainDLLPath(), &file_info)) {
        fts_ = std::to_string(FileTimeToUnixEpoch(file_info.creation_time.ToFileTime()));
    } else {
        LOG(WARNING) << "Failed to read creation time of main dll!";
    }
}

EventTrackingServiceImpl::~EventTrackingServiceImpl()
{}

RequestProxy EventTrackingServiceImpl::ReportLivehimeBehaviorEvent(
    LivehimeBehaviorEvent event_id, int64_t mid, const std::string& event_msg)
{
    return RequestProxy(nullptr);
}

RequestProxy EventTrackingServiceImpl::ReportLivehimeBehaviorEvent(
    const std::string& event_id,
    const std::string& event_msg)
{
    return RequestProxy(nullptr);
}
}   // namespace secret
