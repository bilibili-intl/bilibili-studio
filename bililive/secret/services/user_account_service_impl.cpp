/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/services/user_account_service_impl.h"

#include <string>
#include <tuple>

#include "base/logging.h"
#include "base/values.h"
#include "base/json/json_reader.h"
#include "base/json/json_string_value_serializer.h"
#include "base/strings/utf_string_conversions.h"

#include "net/http/http_response_headers.h"

#include "url/gurl.h"

#include "bililive/common/bililive_features.h"
#include "bililive/secret/services/service_utils.h"
#include "bililive/secret/services/live_streaming_common_data.h"


namespace {

using secret::ResponseInfo;
using secret::RequestParams;

using AvatarData = secret::UserAccountService::AvatarData;
using UserInfoData = secret::UserAccountService::UserInfoData;
using ViddupUserInfoData = secret::UserAccountService::ViddupUserInfoData;
using UserAvatarInfo = secret::UserAccountService::UserAvatarInfo;

const char kURLGetUserInfoViddup[] = "https://api.bilibili.tv/x/intl/member/web/account?platform=web&s_locale=en_US";
const char kURLLogoutViddup[] = "https://passport.bilibili.tv/x/intl/passport-login/web/login/exit";

const char kResponseCode[] = "code";

RequestParams NewCommonRequestParams()
{
    using namespace secret;

    return RequestParams {
        { kArgAppKey, kAppKey },
        { kArgTimestamp, GetUnixTimestamp() },
        { kArgPlatform, kPlatform }
    };
}
bool ExtractCookiesAndDomains(const DictionaryValue* content, std::string* cookies_str, std::string* domains_str)
{
    DCHECK(content && cookies_str && domains_str);

    *cookies_str = *domains_str = "";

    if (cookies_str)
    {
        const base::ListValue* cookies = nullptr;
        if (content->GetList("data.cookie_info.cookies", &cookies))
        {
            JSONStringValueSerializer cookies_serializer(cookies_str);
            cookies_serializer.Serialize(*cookies);
        }
    }

    if (domains_str)
    {
        const base::ListValue* domains = nullptr;
        if (content->GetList("data.cookie_info.domains", &domains))
        {
            JSONStringValueSerializer cookies_serializer(domains_str);
            cookies_serializer.Serialize(*domains);
        }
    }
    return !cookies_str->empty() && !domains_str->empty();
}

//海外开播用户信息接口解析相关
// (valid-response, code, user-info-data)
using GetViddupUserInfoResult = std::tuple<bool, int, ViddupUserInfoData>;
using GetViddupUserInfoParser = std::function<GetViddupUserInfoResult(ResponseInfo, const std::string&)>;
GetViddupUserInfoResult ParseGetViddupUserInfoByCookieResponse(ResponseInfo info, const std::string& data)
{
    if (info.response_code != 200 || data.empty()) {
        LOG(WARNING) << "Parse nav-user-info response failure: invalid status, or data is empty!\n"
            << "response info: " << info;
        return {};
    }

    std::unique_ptr<base::Value> values(base::JSONReader::Read(data));
    const DictionaryValue* content = nullptr;
    if (!values || !values->GetAsDictionary(&content)) {
        LOG(WARNING) << "Parse nav-user-info response failure: not in json format!\n"
            << "received data: " << data;
        return {};
    }

    int code = 0;
    if (!content->GetInteger(kResponseCode, &code)) {
        LOG(WARNING) << "Parse nav-user-info response failure: no valid `code` field!\n"
            << "received data: " << data;
        return {};
    }

    GetViddupUserInfoResult result(true, code, ViddupUserInfoData());

    if (code == 0) {
        auto& info_data = std::get<2>(result);
        bool rv = content->GetString("data.uname", &info_data.uname) &&
            content->GetString("data.face", &info_data.face) &&
            content->GetInteger64("data.mid", &info_data.mid);

        content->GetString("data.sex", &info_data.sex);
        content->GetString("data.sign", &info_data.sign);
        content->GetString("data.birthday", &info_data.birthday);

        if (!rv) {
            LOG(WARNING) << "Parse nav-user-info response failure: missing fields!\n"
                << "received data: " << data;
            std::get<0>(result) = false;
        }
    }
    else
    {
        std::string msg;
        content->GetString("data.message", &msg);

        LOG(WARNING) << "nav-user-info failure, code: " << code << ", msg:" << msg;
    }

    return result;
}

// (success, avatar_info)
using GetUserAvatarResult = std::tuple<bool, UserAvatarInfo>;
using GetUserAvatarParser = std::function<GetUserAvatarResult(ResponseInfo, const std::string&)>;

GetUserAvatarResult ParseGetUserAvatarResponse(ResponseInfo info, const std::string& data)
{
    UserAvatarInfo avatar_info;
    if (info.response_code == 304) {
        DLOG(INFO) << "Avatar cache still counts!";
        return GetUserAvatarResult(true, avatar_info);
    }

    if (info.response_code != 200 || data.empty()) {
        LOG(WARNING) << "Parse get-user-avatar response failure: invalid state, or data is empty!\n"
                     << "response info: " << info;
        return GetUserAvatarResult(false, avatar_info);
    }

    if (info.response_headers->GetContentLength() != data.size()) {
        LOG(WARNING) << "get-user-avatar response data is incomplete!\n"
            << "response info: " << info;
        avatar_info.incomplete = true;
    }

    std::string etag;
    info.response_headers->EnumerateHeader(nullptr, "Etag", &avatar_info.etag);
    avatar_info.avatar = AvatarData(data.begin(), data.end());
    avatar_info.content_length = info.response_headers->GetContentLength();

    return GetUserAvatarResult(true, avatar_info);
}

using RequestViddupLogoutResult = std::tuple<bool>;
using RequestViddupLogoutParser = std::function<RequestViddupLogoutResult(ResponseInfo, const std::string&)>;
RequestViddupLogoutResult ParseRequestViddupLogoutResponse(ResponseInfo info, const std::string& data) {
    if (info.response_code != 200 || data.empty()) {
        LOG(WARNING) << "Parse get-user-face-status response failure: invalid state, or data is empty!\n"
            << "response info: " << info;
        return { false };
    }

    LOG(INFO) << __FUNCTION__ << " data:" << data;

    std::unique_ptr<base::Value> values(base::JSONReader::Read(data));
    const DictionaryValue* content = nullptr;
    if (!values || !values->GetAsDictionary(&content)) {
        LOG(WARNING) << "Parse get-user-face-status response failure: not in json format!\n"
            << "received data: " << data;
        return { false };
    }

    int code = 0;
    if (!content->GetInteger(kResponseCode, &code)) {
        LOG(WARNING) << "Parse get-user-face-status response failure: no valid `code` field!\n"
            << "received data: " << data;
        return { false };
    }

    RequestViddupLogoutResult result { true };
    return result;
}


}   // namespace

namespace secret {

UserAccountServiceImpl::UserAccountServiceImpl(RequestConnectionManager* manager)
    : conn_manager_(manager)
{}

UserAccountServiceImpl::~UserAccountServiceImpl()
{}

void UserAccountServiceImpl::SetAuthCookie(const cef_proxy::cookies& cookie)
{
    conn_manager_->SetAuthCookie(cookie);
}

// 海外开播 - 登出逻辑
RequestProxy UserAccountServiceImpl::RequsetLogoutViddup(RequsetLogoutViddupHandler handler, const std::string& token)
{
    RequestParams params = CommonRequestParams();
    AppendCSRF(params);
    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString()}
    };
    
    std::string url = kURLLogoutViddup;
  
    //LOG(INFO) << "[logout] RequsetLogoutViddup cookie：" << conn_manager_->GetCookieString() << "csrf=" << QueryAccessToken();
    auto proxy = conn_manager_->NewRequestConnection<RequestViddupLogoutParser, RequsetLogoutViddupHandler>(
        GURL(url.append("?csrf=").append(token)),
        RequestType::POST,
        params,
        ParseRequestViddupLogoutResponse,
        handler);

    return proxy;
}

RequestProxy UserAccountServiceImpl::GetViddupUserInfo(const std::string& token, GetViddupUserInfoHandler handler)
{
    //海外开播暂时不需要NativeLogin
    //if (BililiveFeatures::current()->Enabled(BililiveFeatures::NativeLogin))
    //{
    //    return GetUserInfoByToken(token, handler);
    //}
    return GetViddupUserInfoByCookie(handler);
}

// 海外直播版本账号登录，获取用户信息，数据结构沿用国内版本
RequestProxy UserAccountServiceImpl::GetViddupUserInfoByCookie(GetViddupUserInfoHandler handler)
{
    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString()}
    };
    //LOG(INFO) << "[GetViddupUserInfoByCookie] conn_manager_->GetCookieString, " << conn_manager_->GetCookieString();

    auto proxy = conn_manager_->NewRequestConnection<GetViddupUserInfoParser, GetViddupUserInfoHandler>(
        GURL(kURLGetUserInfoViddup),
        RequestType::GET,
        RequestParams(),
        ParseGetViddupUserInfoByCookieResponse,
        handler);

    return proxy;
}

RequestProxy UserAccountServiceImpl::GetUserAvatar(const std::string& avatar_url,
                                                   const std::string& etag,
                                                   GetUserAvatarHandler handler)
{
    RequestHeaders extra_headers;
    if (!etag.empty()) {
        extra_headers.insert({ net::HttpRequestHeaders::kIfNoneMatch, etag });
    }

    auto proxy = conn_manager_->NewRequestConnection<GetUserAvatarParser, GetUserAvatarHandler>(
        GURL(avatar_url),
        RequestType::GET,
        extra_headers,
        RequestParams(),
        ParseGetUserAvatarResponse,
        handler);

    return proxy;
}

}   // namespace secret
