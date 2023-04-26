#include "bililive/secret/services/event_tracking_service_intl_impl.h"

#include <tuple>

#include "base/command_line.h"
#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/guid.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/values.h"

#include "net/http/http_request_headers.h"

#include "url/gurl.h"

#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/services/service_utils.h"
#include "bililive/public/common/bililive_constants.h"

#define DETERMINE_FIELD_VALUE(value)    \
    value <= 0 ? secret::kRequireValueDummyInt : value;

#define DETERMINE_FIELD_VALUE_STR(value)    \
    value.empty() ? secret::kRequireValueDummy : value;


namespace
{
    using secret::LivehimeBehaviorEventV2;

    // @https://info.bilibili.co/pages/viewpage.action?pageId=122234791
    const char kLivehimeLogIDV2[] = "";   // b_ods.rods_s_live_action_livehime_l_hr
    const char kLivehimePlatformV2[] = ""; // 平台（iphone=1 ipad=2 android=3 pc=4 mac=5）

    const std::map<LivehimeBehaviorEventV2, std::string> kLivehimeBehaviorEventV2Table
    {
        { LivehimeBehaviorEventV2::Install, "live.live-zbj.0.0.install" },
        { LivehimeBehaviorEventV2::ObsPluginInstall, "live.live-zbj.inunit.0.install" },
        { LivehimeBehaviorEventV2::LoginResult, "live.live-zbj.land.0.show" },
        { LivehimeBehaviorEventV2::LiveGuideShow, "live.live-zbj.source.0.show" },

        { LivehimeBehaviorEventV2::GuideTutorialClick, "live.live-zbj.source.set-guide.click" },
        { LivehimeBehaviorEventV2::GuideAddSource, "live.live-zbj.source.choose-source.show" },
        { LivehimeBehaviorEventV2::GuideCompleteClick, "live.live-zbj.source.set-source.click" },

        { LivehimeBehaviorEventV2::MainWndShow, "live.live-zbj.0.0.show" },
        { LivehimeBehaviorEventV2::UserInfoWndShow, "live.live-zbj.intro.0.show" },
        { LivehimeBehaviorEventV2::UserInfoItemClick, "live.live-zbj.intro.anchorbutton.click" },
        { LivehimeBehaviorEventV2::TitlebarItemClick, "live.live-zbj.prim-entr.0.click" },
        { LivehimeBehaviorEventV2::MoreMenuItemClick, "live.live-zbj.prim-entr.more-set.click" },
        { LivehimeBehaviorEventV2::LiveShareItemClick, "live.live-zbj.prim-entr.share-set.click" },
        { LivehimeBehaviorEventV2::SceneChange, "live.live-zbj.scene.exchange.click" },
        { LivehimeBehaviorEventV2::RoomTitleSaveClick, "live.live-zbj.edit.0.click" },
        { LivehimeBehaviorEventV2::MaterialItemClick, "live.live-zbj.material.0.click" },
        { LivehimeBehaviorEventV2::SceneItemMenuClick, "live.live-zbj.right-set.0.click" },
        { LivehimeBehaviorEventV2::AppMngClick, "live.live-zbj.more-appli.0.click" },
        //{ LivehimeBehaviorEventV2::ApplistShow, "live.live-zbj.appli-list.0.show" },
        { LivehimeBehaviorEventV2::AppClick, "live.live-zbj.appli-list.0.click" },
        { LivehimeBehaviorEventV2::MissionButtonShow, "live.live-zbj.banner.0.show" },
        //{ LivehimeBehaviorEventV2::MissionButtonClick, "live.live-zbj.banner.0.click" },
        { LivehimeBehaviorEventV2::MissionProject, "live.live-zbj.banner.pro.click" },
        { LivehimeBehaviorEventV2::ToolbarItemClick, "live.live-zbj.setten.0.click" },
        { LivehimeBehaviorEventV2::StatusbarNoticeClick, "live.live-zbj.notie.0.click" },
        { LivehimeBehaviorEventV2::LiveRecord, "live.live-zbj.record.0.click" },
        { LivehimeBehaviorEventV2::StartLive, "live.live-zbj.startlive.0.click" },
        { LivehimeBehaviorEventV2::AnchorPromotionPanelShow, "live.live-zbj.extension.panel.show" },
        { LivehimeBehaviorEventV2::AnchorPromotionHoverClick, "live.live-zbj.extension.hover.click" },
        { LivehimeBehaviorEventV2::ActivityCenterTabClick, "live.live-zbj.activity.tab.click" },
        { LivehimeBehaviorEventV2::ActivityCenterCloseClick, "live.live-zbj.activity.close.click" },
        { LivehimeBehaviorEventV2::CpmGuideWindowShow, "live.live-zbj.extension.prelive-pop.show" },
        //{ LivehimeBehaviorEventV2::CpmPreLiveBubbleShow, "live.live-zbj.extension.prelive-bubble.show" },
        //{ LivehimeBehaviorEventV2::CpmLivingBubbleShow, "live.live-zbj.extension.live-bubble.show" },
        { LivehimeBehaviorEventV2::LiveModelButtonClick, "live.live-zbj.screen-status.button.click" },
        { LivehimeBehaviorEventV2::GuardWarningShow, "live.live-zbj.intro.guard-warning.show" },
        { LivehimeBehaviorEventV2::GuardWarningClick, "live.live-zbj.intro.guard-warning.click" },
        { LivehimeBehaviorEventV2::ActivityViewShow, "live.live-zbj.popups.0.show" },
        { LivehimeBehaviorEventV2::ActivityViewClick, "live.live-zbj.popups.0.click" },
        { LivehimeBehaviorEventV2::SkinClick, "live.live-zbj.camera.skin.click" },
        { LivehimeBehaviorEventV2::FilterClick, "live.live-zbj.camera.filter.click" },
        { LivehimeBehaviorEventV2::StickersClick, "live.live-zbj.camera.stickers.click" },
    };

    const std::set<std::string> kIgnoreExtendedCommonFieldsEventV2Set
    {
        kLivehimeBehaviorEventV2Table.at(LivehimeBehaviorEventV2::Install),
        kLivehimeBehaviorEventV2Table.at(LivehimeBehaviorEventV2::ObsPluginInstall),
        kLivehimeBehaviorEventV2Table.at(LivehimeBehaviorEventV2::LoginResult),
    };

    std::string LocalTime()
    {
        base::Time::Exploded time_exploded;
        base::Time::Now().LocalExplode(&time_exploded);

        return base::StringPrintf("%d-%02d-%02d %02d:%02d:%02d",
            time_exploded.year,
            time_exploded.month,
            time_exploded.day_of_month,
            time_exploded.hour,
            time_exploded.minute,
            time_exploded.second);
    }

    std::string LiveStatus(int live_status)
    {
        return (0 == live_status) ? base::WideToUTF8(L"闲置") : base::WideToUTF8(L"直播");
    }

    // 显示器枚举
    BOOL CALLBACK MonitorEnumCallbackProc(HMONITOR, HDC, LPRECT rect, LPARAM param)
    {
        std::vector<std::string>& monitors = *reinterpret_cast<std::vector<std::string>*>(param);

        monitors.emplace_back(base::StringPrintf("%d*%d", rect->right - rect->left, rect->bottom - rect->top));

        return true;
    }

    std::string FormatLivehimeEventParams(
        const std::string& event_id,
        const std::vector<std::pair<std::string, std::string>>& event_params,
        const std::string& buvid,
        const std::string& ip,
        secret::BehaviorEventNetwork network,
        const std::string& browser_resolution)
    {
        using namespace secret;

        std::string content;
        // 埋点固定格式：logid+时间戳（毫秒精度）
        auto logid = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
            bililive::kSwitchCustomEventLogId);
        content.append(logid.empty() ? kLivehimeLogIDV2 : logid).append(GetUnixTimestamp<std::chrono::milliseconds>());

        int64_t uid = DETERMINE_FIELD_VALUE(GetSecretCore()->account_info().mid());
        std::string mid = base::Int64ToString(uid);
        int64_t room_id = DETERMINE_FIELD_VALUE(GetSecretCore()->user_info().room_id());
        int area_id = DETERMINE_FIELD_VALUE(GetSecretCore()->anchor_info().current_area());
        int parent_area_id = DETERMINE_FIELD_VALUE(GetSecretCore()->anchor_info().current_parent_area());
        std::string live_key = DETERMINE_FIELD_VALUE_STR(GetSecretCore()->anchor_info().live_key());
        std::string sub_session_key = DETERMINE_FIELD_VALUE_STR(GetSecretCore()->anchor_info().sub_session_key());

        // 其他字段按顺序以竖线分隔紧挨追加到固定头
        std::vector<std::string> vec;

        // 端上自己主动上报http头相关的信息，因为数仓当前已经不再支持自动从http请求头获取这些字段的集成任务了
        vec.push_back(base::StringPrintf("Bililive/%s (LiveHime; Windows)",
            BililiveContext::Current()->GetExecutableVersionAsASCII().c_str()));  // ua
        vec.push_back(secret::kRequireValueDummy);  // http_refer
        vec.push_back(mid);  // mid
        vec.push_back(buvid);  // buvid
        vec.push_back(ip);  // ip

        vec.push_back(event_id);  // event_id
        vec.push_back(BililiveContext::Current()->GetExecutableVersionAsASCII());  // version
        vec.push_back(std::to_string(BililiveContext::Current()->GetExecutableBuildNumber()));  // build号
        vec.push_back(std::to_string((int)network));  // 网络状态
        vec.push_back(GetUnixTimestamp<std::chrono::milliseconds>());  // 事件发生的时间戳
        vec.push_back(LocalTime());  // 事件发生的时间标准格式（YYYY-MM-DD hh:mm:ss）
        vec.push_back(browser_resolution);  // 设备分辨率
        vec.push_back(kLivehimePlatformV2);  // 平台来源

        // 私有字段除了事件相关的特殊参数外还有公共的一系列参数
        // {room_id;live_status;parent_area_id;area_id;up_id;live_key;sub_session_key}
        std::unique_ptr<base::DictionaryValue> fields(new base::DictionaryValue());

        // 少数几个事件不需要扩展字段
        if (kIgnoreExtendedCommonFieldsEventV2Set.count(event_id) == 0)
        {
            // 埋点事件公共字段
            fields->SetString("room_id", base::Int64ToString(room_id));
            fields->SetString("live_status", LiveStatus(GetSecretCore()->anchor_info().live_status()));
            fields->SetString("parent_area_id", base::IntToString(parent_area_id));
            fields->SetString("area_id", base::IntToString(area_id));
            fields->SetString("up_id", mid);
            fields->SetString("live_key", live_key);
            fields->SetString("sub_session_key", sub_session_key);
        }

        // 埋点事件的额外字段
        for (auto& iter : event_params)
        {
            fields->SetString(iter.first, iter.second);
        }

        std::string extended_fields;
        base::JSONWriter::Write(fields.get(), &extended_fields);

        vec.push_back(extended_fields);  // 私有字段

        content.append(JoinString(vec, "|"));

        return content;
    }

}   // namespace

namespace secret
{
    RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEventV2(
        LivehimeBehaviorEventV2 event_id,
        const base::StringPairs& event_params)
    {
        secret::LivehimeIntlBehaviorEvent intl_event;
        if (CheckInCommonBehaviorEventTable("LivehimeBehaviorEventV2",
            base::IntToString(static_cast<int>(event_id)), intl_event)) {
            return ReportLivehimeIntlBehaviorEvent(intl_event, event_params);
        }
        else
        {
            return RequestProxy(nullptr);
        }
    }

    RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEventV2(
        const std::string& event_id,
        const base::StringPairs& event_params)
    {
        // 当前网络状态
        if (network_ == BehaviorEventNetwork::Unknown)
        {
            auto ct = bililive::GetCurrentConnectionType();
            switch (ct)
            {
            case net::NetworkChangeNotifier::ConnectionType::CONNECTION_ETHERNET:
                network_ = BehaviorEventNetwork::Ethernet;
                break;
            case net::NetworkChangeNotifier::ConnectionType::CONNECTION_WIFI:
                network_ = BehaviorEventNetwork::Wifi;
                break;
            default:
                network_ = BehaviorEventNetwork::Ethernet;
                break;
            }

            GetSecretCore()->network_info().net_type_ = (int)network_;
        }

        // 所有显示器分辨率
        if (browser_resolution_.empty() || (browser_resolution_ == kRequireValueDummy))
        {
            std::vector<std::string> monitors;
            EnumDisplayMonitors(NULL, NULL, MonitorEnumCallbackProc, (LPARAM)&monitors);
            browser_resolution_ = JoinString(monitors, ";");

            if (browser_resolution_.empty())
            {
                browser_resolution_ = kRequireValueDummy;
            }
        }

        RequestRaw params(FormatLivehimeEventParams(
            event_id,
            event_params,
            buvid_,
            ethernet_address_info_.ip,
            network_,
            browser_resolution_)
        );

        return ReportEventTracking(params);
    }

}   // namespace secret
