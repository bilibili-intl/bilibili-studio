#include "bililive/secret/services/event_tracking_service_impl.h"

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
        return (0 == live_status) ? base::WideToUTF8(L"ÏÐÖÃ") : base::WideToUTF8(L"Ö±²¥");
    }

    // ÏÔÊ¾Æ÷Ã¶¾Ù
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
        return nullptr;
    }

}   // namespace

namespace secret
{
    RequestProxy EventTrackingServiceImpl::ReportLivehimeBehaviorEventV2(
        LivehimeBehaviorEventV2 event_id,
        const base::StringPairs& event_params)
    {
        return ReportLivehimeBehaviorEventV2(kLivehimeBehaviorEventV2Table.at(event_id), event_params);
    }

    RequestProxy EventTrackingServiceImpl::ReportLivehimeBehaviorEventV2(
        const std::string& event_id,
        const base::StringPairs& event_params)
    {       
        return RequestProxy(nullptr);
    }

}   // namespace secret
