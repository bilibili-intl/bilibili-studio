#include "danmaku_hime_service_impl.h"

#include <memory>
#include <tuple>

#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/values.h"

#include "url/gurl.h"

#include "bililive/common/bililive_context.h"
#include "bililive/secret/services/json_parse.h"
#include "bililive/secret/services/service_utils.h"


namespace {

using secret::ResponseInfo;
using secret::RequestParams;
using secret::RequestParams2;

const char kArgAccessKey[] = "access_key";
const char kArgRoomId[] = "room_id";
const char kArgBuild[] = "build";

const char kURLSendViddupDanmaku[] = "https://api.bilibili.tv/studio/live/pc/dm/send";    //发送海外弹幕接口

RequestParams NewCommonRequestParams() {
    using namespace secret;

    return RequestParams{
        { kArgAppKey, kAppKey },
        { kArgTimestamp, GetUnixTimestamp() },
        { kArgPlatform, kPlatform },
        { kArgAppVersion, BililiveContext::Current()->GetExecutableVersionAsASCII() },
        { kArgAccessKey, QueryAccessToken() }
    };
}

RequestParams2 NewCommonRequestParams2() {
    using namespace secret;

    return RequestParams2{
        { kArgAppKey, kAppKey },
        { kArgTimestamp, GetUnixTimestamp() },
        { kArgPlatform, kPlatform },
        { kArgAppVersion, BililiveContext::Current()->GetExecutableVersionAsASCII() },
        { kArgAccessKey, QueryAccessToken() }
    };
}

// Get Gift icon
// (valid-response, data)
using DownloadImageResult = std::tuple<bool, std::string>;
using DownloadImageParser = std::function<DownloadImageResult(ResponseInfo, const std::string&)>;

DownloadImageResult ParseDownloadImageResponse(ResponseInfo info, const std::string& data)
{
    if (info.response_code != 200 || data.empty())
    {
        LOG(WARNING) << "Parse get-gift-icon response failure: invalid status or empty data!\n"
            << "response info: " << info;
        return {};
    }
    try
    {
        return DownloadImageResult(true, data);
    }
    catch (std::bad_alloc& ba)
    {
        LOG(ERROR) << "ParseDownloadImageResponse catch std::bad_alloc :" << ba.what();
    }
    catch (std::exception& ex)
    {
        LOG(ERROR) << "ParseDownloadImageResponse catch std::exception : " << ex.what();
    }
    catch (...)
    {
        LOG(ERROR) << "ParseDownloadImageResponse catch ...!";
    }
    return {};
}

// 发送海外弹幕
using SendDanmakuViddupResult = std::tuple<bool, int, std::string>;
using SendDanmakuViddupParser = std::function<SendDanmakuViddupResult(ResponseInfo, const std::string&)>;

SendDanmakuViddupResult ParseSendDanmakuViddupResponse(ResponseInfo info, const std::string& data)
{
    if (info.response_code != 200 || data.empty())
    {
        LOG(WARNING) << "Parse send-danmaku response failure: invalid status or empty data!\n"
            << "response info: " << info;
        return {};
    }

    std::unique_ptr<base::Value> values(base::JSONReader::Read(data));
    const base::DictionaryValue* content = nullptr;
    if (!values || !values->GetAsDictionary(&content))
    {
        LOG(WARNING) << "Parse send-danmaku response failure: not in json format!\n"
            << "received data: " << data;
        return {};
    }

    int code = 0;
    if (!content->GetInteger("code", &code))
    {
        LOG(WARNING) << "Parse send-danmaku response failure: error at `code` field!\n"
            << "received data: " << data;
        return {};
    }

    std::string err_msg;
    if (!content->GetString("message", &err_msg))
    {
        LOG(WARNING) << "Parse send-danmaku response failure: error at `message` field!\n"
            << "received data: " << data;
        return {};
    }

    return SendDanmakuViddupResult(true, code, err_msg);
}

}


namespace secret {

DanmakuHimeServiceImpl::DanmakuHimeServiceImpl(RequestConnectionManager* manager)
    :conn_manager_(manager) {}

RequestProxy DanmakuHimeServiceImpl::DownloadImage(
    const std::string& url, DownloadImageHandler handler)
{
    DCHECK(!url.empty());

    RequestProxy proxy = conn_manager_->NewRequestConnection<
        DownloadImageParser, DownloadImageHandler>(
            GURL(url),
            RequestType::GET,
            RequestParams(),
            ParseDownloadImageResponse,
            handler);

    return proxy;
}

// 发送海外弹幕
RequestProxy DanmakuHimeServiceImpl::SendViddupDanmaku(
    int64_t room_id, const std::string& msg,
    int64_t rnd, int font_size, int mode, SendDanmakuViddupHandler handler)
{
    RequestParams params = NewCommonRequestParams();
    AppendCSRF(params);

    GURL gurl = AppendQueryStringToGURL(GURL(kURLSendViddupDanmaku),
        params.ToRequestContent().second);

    RequestUpload upload_body("name", " ");
    upload_body.extra_params["room_id"] = std::to_string(room_id);
    upload_body.extra_params["content"] = msg;

    RequestProxy proxy = conn_manager_->NewRequestConnection<
        SendDanmakuViddupParser, SendDanmakuViddupHandler>(
            gurl,
            RequestType::POST,
            upload_body,
            ParseSendDanmakuViddupResponse,
            handler);

    return proxy;
}

}