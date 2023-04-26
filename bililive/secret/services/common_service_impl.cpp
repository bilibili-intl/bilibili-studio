#include "live_streaming_service_impl.h"
/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/services/live_streaming_service_impl.h"

#include <memory>
#include <tuple>
#include <vector>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/values.h"
#include "base/ext/callable_callback.h"
#include "base/json/json_reader.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "net/http/http_response_headers.h"

#include "url/gurl.h"

#include "bilibase/basic_types.h"
#include "bilibase/math_util.h"
#include "bilibase/scope_guard.h"

#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/services/live_streaming_common_data.h"
#include "bililive/secret/services/json_parse.h"
#include "bililive/secret/services/no_secure_requests.h"
#include "base/json/json_writer.h"


namespace {

using namespace std::placeholders;

using secret::ResponseInfo;
using secret::RequestParams;

using IntlRoomInfo = secret::LiveStreamingService::IntlRoomInfo;
using ProtocolInfos = secret::LiveStreamingService::ProtocolInfos;
using ProtocolInfo = secret::LiveStreamingService::ProtocolInfo;
using StartLiveNotice = secret::LiveStreamingService::StartLiveNotice;
using StartLiveInfo = secret::LiveStreamingService::StartLiveInfo;

using PopularRankInfo = secret::AnchorInfo::PopularRankInfo;

const char kArgUID[] = "uid";
const char kArgRoomid[] = "roomid";
const char kArgRoomId[] = "room_id";

const char kURLGetIntlRoomInfo[] = "https://api.bilibili.tv/studio/live/pc/info";

// (valid-response, code, error-message, RoomInfo)
using GetIntlRoomInfoResult = std::tuple<bool, int, std::string, IntlRoomInfo>;
using GetIntlRoomInfoParser = std::function<GetIntlRoomInfoResult(ResponseInfo, const std::string&)>;
GetIntlRoomInfoResult ParseGetIntlRoomInfoResponse(ResponseInfo info, const std::string& data) {
    int code = 0;

    std::unique_ptr<base::DictionaryValue> content(CommonParse(info, data, "get-room-info", &code));
    if (!content) {
        return {};
    }

    GetIntlRoomInfoResult result(true, code, std::string(), IntlRoomInfo());

    if (code != 0) {
        content->GetString("msg", &std::get<2>(result));
    }
    else {
        auto& room_info = std::get<3>(result);
        bool rv = content->GetInteger64("data.room_id", &room_info.room_id) &&
            content->GetString("data.website_link", &room_info.website_link) &&
            content->GetString("data.server_address", &room_info.server_address) &&
            content->GetString("data.backup_address", &room_info.backup_address) &&
            content->GetString("data.stream_key", &room_info.stream_key);

        if (rv) {
         
           
        }
        else {
            LOG(WARNING) << "Parse get-room-info response failure: missing fields!\n"
                << "received data: " << data;
            std::get<0>(result) = false;
        }
    }

    return result;
}

// (valid-response, code, error-message, RoomInfo)
using CreateLiveRoomResult = std::tuple<bool, int, std::string, int64_t>;
using CreateLiveRoomParser = std::function<CreateLiveRoomResult(ResponseInfo, const std::string&)>;
CreateLiveRoomResult ParseCreateLiveRoomResponse(ResponseInfo info, const std::string& data) {
    int code = 0;

    std::unique_ptr<base::DictionaryValue> content(CommonParse(info, data, "create-liveroom", &code));
    if (!content) {
        return {};
    }

    int64_t room_id = 0;
    std::string  str_room_id;
    std::string msg;
    bool valid = true;

    if (code != 0) {
        content->GetString("message", &msg);
    }
    else{
        if (!content->GetString("data.roomID", &str_room_id)){
            valid = false;
            LOG(WARNING) << "Parse create-liveroom response require field failure!\n"
                << "received data: " << data;
        }
        else {
            base::StringToInt64(str_room_id, &room_id);
        }
    }
    CreateLiveRoomResult result(valid, code, msg, room_id);
    return result;
}


using OpenLiveRoomResult = std::tuple<bool, int, std::string, StartLiveInfo>;
using OpenLiveRoomParser = std::function<OpenLiveRoomResult(ResponseInfo, const std::string&)>;
OpenLiveRoomResult ParseOpenLiveRoomResponse(ResponseInfo info, const std::string& data) {
    int code = 0;

    std::unique_ptr<base::DictionaryValue> content(CommonParse(info, data, "open-live-room", &code));
    if (!content) {
        return {};
    }

    std::string msg;
    StartLiveInfo start_live_info;

    content->GetString("msg", &msg);
    content->GetInteger("data.room_type", &start_live_info.room_type);

    // 人脸识别相关参数
    content->GetBoolean("data.need_face_auth", &start_live_info.need_face_auth);
    content->GetString("data.qr", &start_live_info.qr);

    if (code == 0) {

        start_live_info.rtmp_info.protocol = "rtmp";
        content->GetString("data.live_key", &start_live_info.rtmp_info.live_key);

        const base::ListValue* protocols_data = nullptr;
        if (content->GetList("data.protocols", &protocols_data) && protocols_data) {
           for (size_t i = 0; i < protocols_data->GetSize(); i++) {
               const base::Value* protocol_info = nullptr;
               protocols_data->Get(i, &protocol_info);

               if (protocol_info) {
                   const base::DictionaryValue* protocol_details = nullptr;
                   protocol_info->GetAsDictionary(&protocol_details);
                   ProtocolInfo info;
                   info.live_key = start_live_info.rtmp_info.live_key;
                   protocol_details->GetString("protocol", &info.protocol);
                   protocol_details->GetString("addr", &info.addr);
                   protocol_details->GetString("code", &info.key);
                   protocol_details->GetString("new_link", &info.new_link);
                   start_live_info.protocol_list.push_back(info);
               }
           }
        }

        bool rv = content->GetString("data.rtmp.new_link", &start_live_info.rtmp_info.new_link) &&
            content->GetString("data.rtmp.code", &start_live_info.rtmp_info.key) &&
            content->GetString("data.rtmp.addr", &start_live_info.rtmp_info.addr);

        if (!rv) {
            LOG(WARNING) << "Parse open-live-room failure: missing fields!\n"
                << "received data: " << data;
            return {};
        }

        // 如果接口给的协议列表为空，在这里就把rtmp地址追加到协议列表里
        if (start_live_info.protocol_list.empty())
        {
            start_live_info.protocol_list.push_back(start_live_info.rtmp_info);
        }

        // livekey在底层直接记录下来
        GetSecretCore()->anchor_info().set_live_key(start_live_info.rtmp_info.live_key);

        // 仅供V2埋点使用的标识一场直播中的某些细节变动的flag
        std::string sub_session_key;
        content->GetString("data.sub_session_key", &sub_session_key);
        GetSecretCore()->anchor_info().set_sub_session_key(sub_session_key);
    }

    content->GetInteger("data.notice.type", &start_live_info.notice.type);
    content->GetInteger("data.notice.status", &start_live_info.notice.status);
    content->GetString("data.notice.title", &start_live_info.notice.title);
    content->GetString("data.notice.msg", &start_live_info.notice.msg);
    content->GetString("data.notice.button_text", &start_live_info.notice.button_text);
    content->GetString("data.notice.button_url", &start_live_info.notice.button_url);

    return OpenLiveRoomResult(true, code, msg, start_live_info);
}

// 海外开播 - 新增开播接口
OpenLiveRoomResult ParseOpenViddupLiveRoomResponse(ResponseInfo info, const std::string& data) {
    int code = 0;

    std::unique_ptr<base::DictionaryValue> content(CommonParse(info, data, "open-live-room", &code));
    if (!content) {
        return {};
    }

    std::string msg;
    StartLiveInfo start_live_info;

    GetIntlRoomInfoResult result(true, code, std::string(), IntlRoomInfo());

    if (code != 0) {
        content->GetString("msg", &std::get<2>(result));
    }
    else {
        auto& room_info = std::get<3>(result);
        bool rv = content->GetInteger64("data.room_id", &room_info.room_id) &&
            content->GetString("data.website_link", &room_info.website_link) &&
            content->GetString("data.server_address", &room_info.server_address) &&
            content->GetString("data.backup_address", &room_info.backup_address) &&
            content->GetString("data.stream_key", &room_info.stream_key);

        if (rv) {
            ProtocolInfo rtmp_info;
            rtmp_info.protocol = "rtmp";
            rtmp_info.live_key = room_info.stream_key;
            rtmp_info.new_link = room_info.backup_address;
            rtmp_info.addr = room_info.server_address;
            rtmp_info.key = room_info.stream_key;
            start_live_info.protocol_list.push_back(rtmp_info);
            
            ProtocolInfo rtmp_info_backup;
            rtmp_info_backup.protocol = "rtmp";
            rtmp_info_backup.live_key = room_info.stream_key;
            rtmp_info_backup.new_link = room_info.backup_address;
            rtmp_info_backup.addr = room_info.backup_address;
            rtmp_info_backup.key = room_info.stream_key;
            start_live_info.protocol_list.push_back(rtmp_info_backup);
        }
        else {
            LOG(WARNING) << "Parse open-live-room response failure: missing fields!\n"
                << "received data: " << data;
            std::get<0>(result) = false;
        }
    }

    return OpenLiveRoomResult(true, code, msg, start_live_info);
}

}   // namespace

namespace secret {

LiveStreamingServiceImpl::LiveStreamingServiceImpl(RequestConnectionManager* manager)
    : conn_manager_(manager)
{}

LiveStreamingServiceImpl::~LiveStreamingServiceImpl()
{}

RequestProxy LiveStreamingServiceImpl::GetIntlRoomInfo(int64_t mid, GetIntlRoomInfoHandler handler)
{
    RequestParams params = CommonRequestParams();
    /*params["uId"] = std::to_string(mid);
    params[kArgSign] = GenerateSignForRequestParams(params);*/

    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString()}
    };

    RequestProxy proxy = conn_manager_->NewRequestConnection<GetIntlRoomInfoParser, GetIntlRoomInfoHandler>(
        GURL(AllowInsecure(kURLGetIntlRoomInfo)),
        RequestType::GET,
        params,
        ParseGetIntlRoomInfoResponse,
        handler);

    return proxy;
}

// 海外开播 - 新增开播接口
RequestProxy LiveStreamingServiceImpl::StartViddupLive(OpenLiveRoomHandler handler)
{
    RequestParams params = CommonRequestParams();
    /*params["uId"] = std::to_string(mid);
    params[kArgSign] = GenerateSignForRequestParams(params);*/

    RequestHeaders extra_headers{
        { net::HttpRequestHeaders::kCookie, conn_manager_->GetCookieString()}
    };

    RequestProxy proxy = conn_manager_->NewRequestConnection<OpenLiveRoomParser, OpenLiveRoomHandler>(
        GURL(AllowInsecure(kURLGetIntlRoomInfo)),
        RequestType::GET,
        params,
        ParseOpenViddupLiveRoomResponse,
        handler);

    return proxy;
}

}   // namespace secret
