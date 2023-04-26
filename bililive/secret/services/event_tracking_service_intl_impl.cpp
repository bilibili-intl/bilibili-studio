#include "bililive/secret/services/event_tracking_service_intl_impl.h"

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
#include "base/strings/string_number_conversions.h"
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
using secret::LivehimeIntlBehaviorEvent;

const wchar_t kLiveHimeBUVIDFileName[] = L"livehime_buvid";

const char kURLLancerDataCenter[] = "https://data.bilibili.tv/log/web";

const char kLivehimeTaskID[] = "018098";
const char kLivehimePlatform[] = "pc";
const char kLivehimeBiz[] = "zbj";

const std::map<LivehimeIntlBehaviorEvent, std::string> kLivehimeIntlBehaviorEventTable
{
    // 参数国内埋点Table此处重新定义埋点字段
    { LivehimeIntlBehaviorEvent::StartLive, "live.live-zbj.startlive.0.click" },
    { LivehimeIntlBehaviorEvent::LivehimeInstall, "live.live-zbj.livehiem.install" },

    //{ LivehimeIntlBehaviorEvent::StartLiveResult, "live.live-zbj.startlive.result.show"},
    //{ LivehimeIntlBehaviorEvent::StartLiveTag,"live.live-zbj.startlive-tag.result.show"},
    //{ LivehimeIntlBehaviorEvent::ActiveClick, "live.live-zbj.sys-active.0.click" },
    //{ LivehimeIntlBehaviorEvent::StartUpShow, "live.live-zbj.startup.0.show" },
};

// 通用模块埋点表Table
// 海外需要兼容和国内通用业务模块的埋点
// 目前先通过通用模块埋点表Table来维护共建
// 如果是通用Table中的埋点，则需要转换映射为海外对应的埋点（埋点顺序不同）
// 如果不是通用Table中的埋点，则说明是国内的埋点，不需要处理
// 通用埋点包括以下几项
// 技术埋点：LivehimeBehaviorEvent
// V2业务埋点：LivehimeBehaviorEventV2
// 通过服务端进行的埋点：LivehimeViaServerBehaviorEvent
// 通过服务端进行的埋点-新接口：LivehimeViaServerBehaviorEventNew
// 北极星业务埋点：LivehimePolarisBehaviorEvent
// 通用Table中key采用"Event英文字符" + '_' + "Event值"的方式实现唯一性，value映射到海外的Event
// eg：LivehimeBehaviorEvent::LivehimeInstall埋点 --> LivehimeBehaviorEvent_0
//     LivehimeBehaviorEventV2::Install埋点 --> LivehimeBehaviorEventV2_1
const std::map<std::string, secret::LivehimeIntlBehaviorEvent> kLivehimeCommonBehaviorEventTable
{
     { "LivehimeBehaviorEvent_0", secret::LivehimeIntlBehaviorEvent::LivehimeInstall },
     { "LivehimeBehaviorEventV2_25", secret::LivehimeIntlBehaviorEvent::StartLive }
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
    //content.append(kLivehimeTaskID).append(GetUnixTimestamp<std::chrono::milliseconds>());

    //std::vector<std::string> vec;
    //vec.push_back(kLivehimeIntlBehaviorEventTable.at(event_id));
    //auto common_vec = FormatCommonParams(mid, event_msg, buvid);
    //vec.insert(vec.end(), common_vec.begin(), common_vec.end());

    //content.append(JoinString(vec, "|"));

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

std::string FormatLivehimeEventParams(
    const std::string& event_id,
    const base::StringPairs& event_msg)
{
    using namespace secret;
    auto buvid = GetSecretCore()->network_info().buvid();
    auto mid = GetSecretCore()->account_info().mid();

    std::string content;
    content.append(kLivehimeTaskID).append(GetUnixTimestamp<std::chrono::milliseconds>());

    std::vector<std::string> vec;
    vec.push_back(event_id);

    std::string msg;
    for each (auto var in event_msg)
    {
        &msg.append(var.first + ":" + var.second + ";");
    }
    auto common_vec = FormatCommonParams(mid, msg, buvid);
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

secret::EthernetAddressInfo EventTrackingServiceIntlImpl::ethernet_address_info_;

EventTrackingServiceIntlImpl::EventTrackingServiceIntlImpl(RequestConnectionManager* manager)
    : conn_manager_(manager),
      fts_("0"),
      weak_ptr_factory_(this)
{
    SetupBuvidForLivehime();
    SetupEthernetAddressInfo();

    base::PlatformFileInfo file_info;
    if (file_util::GetFileInfo(BililiveContext::Current()->GetMainDLLPath(), &file_info)) {
        fts_ = std::to_string(FileTimeToUnixEpoch(file_info.creation_time.ToFileTime()));
    } else {
        LOG(WARNING) << "Failed to read creation time of main dll!";
    }
}

EventTrackingServiceIntlImpl::~EventTrackingServiceIntlImpl()
{}

void EventTrackingServiceIntlImpl::SetupBuvidForLivehime()
{
    DCHECK(BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime));

    // 首次运行需要生成buvid；平时正常运行要检查是否有buvid文件，没有也要生成一份buvid
    bool need_gen_buvid = true;

    if (GetSecretCore()->bililive_process()->first_run())
    {
        // do nothing
    }
    else
    {
        auto buvid_path = GetBuvidSaveDir().Append(kLiveHimeBUVIDFileName);
        if (buvid_path.ReferencesParent())
        {
            buvid_path = base::MakeAbsoluteFilePath(buvid_path);
        }

        if (base::PathExists(buvid_path))
        {
            if (file_util::ReadFileToString(buvid_path, &buvid_))
            {
                need_gen_buvid = false;
            }
            else
            {
                LOG(WARNING) << "Failed to read buvid from file " << buvid_path.AsUTF8Unsafe();
            }
        }
    }

    if (need_gen_buvid)
    {
        GetBuvid();
    }
    else
    {
        GetSecretCore()->network_info().buvid_ = buvid_;

        PostBuvidGot();
    }
}

void EventTrackingServiceIntlImpl::SetupEthernetAddressInfo()
{
    static GURL url("http://www.cip.cc/");
    secret::RequestHeaders headers;

    // 伪造一下UA，毕竟获取IP的这个网站不是我站的（腾讯云），被对方检测到使用量增加可能会找过来刚；
    // 就算不过来刚，其也能通过我们自己的UA统计出我们的DAU、MAU等关键数据
    headers[net::HttpRequestHeaders::kUserAgent] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36";
    RequestProxy proxy =
        conn_manager_->NewRequestConnection<GetEthernetAddressParser, GetEthernetAddressHandler>(
            url,
            RequestType::GET,
            headers,
            RequestRaw(""),
            ParseSetupEthernetAddressInfoResponse,
            HandleGetEthernetAddressResponse);
    proxy.Call();
}

void EventTrackingServiceIntlImpl::HandleGetEthernetAddressResponse(bool valid, const EthernetAddressInfo& info)
{
    if (valid)
    {
        ethernet_address_info_ = info;

        LOG(INFO) << app_log::kLogNetStatus <<
            base::StringPrintf("ip: %s; region:%s; isp:%s; network:%s",
                info.ip.c_str(), base::UTF16ToUTF8(info.addr).c_str(), base::UTF16ToUTF8(info.communications_operator).c_str(),
                bililive::GetCurrentConnectionTypeName().c_str());

        GetSecretCore()->network_info().ip_ = info.ip;
        GetSecretCore()->network_info().region_ = info.addr;
        GetSecretCore()->network_info().co_ = info.communications_operator;
    }
    else
    {
        LOG(WARNING) << "Get ethernet address info failed.";
    }
}

RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEvent(
    LivehimeBehaviorEvent event_id, int64_t mid, const std::string& event_msg)
{
    base::StringPairs params;
    base::SplitStringIntoKeyValuePairs(event_msg, ':', ';', &params);
    secret::LivehimeIntlBehaviorEvent intl_event;
    if (CheckInCommonBehaviorEventTable("LivehimeBehaviorEvent",
        base::IntToString(static_cast<int>(event_id)), intl_event)) {
        return ReportLivehimeIntlBehaviorEvent(intl_event, params);
    }
    else
    {
        return RequestProxy(nullptr);
    }
}

RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeBehaviorEvent(
    const std::string& event_id,
    const std::string& event_msg)
{
    auto mid = GetSecretCore()->account_info().mid();
    RequestRaw params(FormatLivehimeEventParams(event_id, mid, event_msg, buvid_));

    return ReportEventTracking(params);
}

RequestProxy EventTrackingServiceIntlImpl::ReportEventTracking(RequestRaw params)
{

    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString() }
    };

    RequestProxy proxy =
        conn_manager_->NewRequestConnection<EventTrackingParser, DefaultHandler>(
            GURL(kURLLancerDataCenter),
            RequestType::GET,
            extra_headers,
            params,
            ParseEventTrackingResponse,
            HandleReportResponse);

    return proxy;
}

void EventTrackingServiceIntlImpl::GetBuvid()
{
   /* auto proxy = conn_manager_->NewRequestConnection<GetBuvidParser, GetBuvidHandler>(
        GURL(kURLGetBuvid),
        RequestType::GET,
        RequestParams(),
        ParseGetBuvidResponse,
        base::MakeCallable(base::Bind(&EventTrackingServiceIntlImpl::HandleBuvidGot,
                                      weak_ptr_factory_.GetWeakPtr())));

    proxy.Call();*/
}

void EventTrackingServiceIntlImpl::HandleBuvidGot(const std::string& buvid)
{
    // The function is currently used only in livehime mode.
    DCHECK(BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime));

    if (buvid.empty()) {
        LOG(INFO) << "Making a user-generated buvid";
        buvid_ = base::StringPrintf("%s%duser", base::GenerateGUID().c_str(), GetCurrentProcessId());
    } else {
        buvid_ = buvid;
    }

    GetSecretCore()->network_info().buvid_ = buvid_;

    auto buvid_path = GetBuvidSaveDir().Append(kLiveHimeBUVIDFileName);
    file_util::WriteFile(buvid_path, buvid_.data(), buvid_.length());

    PostBuvidGot();
}

void EventTrackingServiceIntlImpl::PostBuvidGot()
{
    // 安装埋点，首次运行的埋点放在buvid获取成功之后的这个时机
    if (GetSecretCore()->bililive_process()->first_run()) {
        ReportLivehimeBehaviorEvent(secret::LivehimeBehaviorEvent::LivehimeInstall, 0, std::string()).Call();

        // 直播姬安装业务埋点
        ReportLivehimeBehaviorEventV2(LivehimeBehaviorEventV2::Install, { {"tag_type", "1"} }).Call();

        // OBS插件安装埋点
        if (base::PathExists(livehime::GetObsInjectFlagPath()))
        {
            ReportLivehimeBehaviorEventV2(LivehimeBehaviorEventV2::ObsPluginInstall, {}).Call();
        }

        //只有第一次安装上报
        ReportLivehimeBehaviorEventPolaris(LivehimePolarisBehaviorEvent::InstallNew, {}).Call();
        base::StringPairs params;
        base::SplitStringIntoKeyValuePairs("result:1", ':', ';', &params);
        ReportLivehimeBehaviorEventPolaris(LivehimePolarisBehaviorEvent::InstalledResultNew, params).Call();
    }

    base::ThreadRestrictions::ScopedAllowIO allow;
    base::FilePath temp;
    if (file_util::GetTempDir(&temp))
    {
        auto install_click = temp.Append(L"livehime_install_click");
        auto run_click = temp.Append(L"livehime_run_click");

        if (base::PathExists(install_click))
        {
            ReportLivehimeBehaviorEventPolaris(LivehimePolarisBehaviorEvent::Install, {}).Call();
            base::DeleteFile(install_click, false);
        }

        if (base::PathExists(run_click))
        {
            std::string str;
            file_util::ReadFileToString(run_click, &str);
            if (!str.empty() && str.compare("0") != 0)
            {
                base::StringPairs params;
                base::SplitStringIntoKeyValuePairs("result:1", ':', ';', &params);
                ReportLivehimeBehaviorEventPolaris(LivehimePolarisBehaviorEvent::InstalledResult, params).Call();
            }

            base::DeleteFile(run_click, false);
        }
    }
}

RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeIntlBehaviorEvent(
    LivehimeIntlBehaviorEvent event_id, const base::StringPairs& event_msg)
{
    RequestRaw params(FormatLivehimeEventParams(kLivehimeIntlBehaviorEventTable.at(event_id), event_msg));
   
    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString() }
    };

    RequestProxy proxy =
        conn_manager_->NewRequestConnection<EventTrackingParser, DefaultHandler>(
            GURL(kURLLancerDataCenter),
            RequestType::GET,
            extra_headers,
            params,
            ParseEventTrackingResponse,
            HandleReportResponse);

    return RequestProxy(proxy);
}

RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeIntlBehaviorEvent(
    const std::string& event_id,
    const base::StringPairs& event_msg)
{
    RequestRaw params(FormatLivehimeEventParams(event_id, event_msg));
  
    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString() }
    };

    RequestProxy proxy =
        conn_manager_->NewRequestConnection<EventTrackingParser, DefaultHandler>(
            GURL(kURLLancerDataCenter),
            RequestType::GET,
            extra_headers,
            params,
            ParseEventTrackingResponse,
            HandleReportResponse);

    return RequestProxy(proxy);
}

bool EventTrackingServiceIntlImpl::CheckInCommonBehaviorEventTable(std::string event_name, std::string event_id, secret::LivehimeIntlBehaviorEvent& intl_event)
{
    std::string event_key = event_name + '_' + event_id;
    auto found_iter = std::find_if(kLivehimeCommonBehaviorEventTable.begin(),
        kLivehimeCommonBehaviorEventTable.end(),
        [&](const std::map<std::string, secret::LivehimeIntlBehaviorEvent>::value_type& iter)->bool {
            return iter.first == event_key;
        });
    if (found_iter == kLivehimeCommonBehaviorEventTable.end())
    {
        return false;
    }

    intl_event = found_iter->second;
    return true;
}
}   // namespace secret
