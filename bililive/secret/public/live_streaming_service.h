/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_PUBLIC_LIVE_STREAMING_SERVICE_H_
#define BILILIVE_SECRET_PUBLIC_LIVE_STREAMING_SERVICE_H_

#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>
#include "base/values.h"
#include "bililive/secret/public/request_connection_proxy.h"
#include "bililive/bililive/livehime/feedback/feedback_presenter.h"
#include "bililive/bililive/livehime/obs/source_creator.h"

namespace secret {

class LiveStreamingService {
public:

    enum class VirtualSceneType
    {
        kVideoPk,
        kColiveVideo
    };

    enum ErrorCode : int {
        OK = 0,
    };



    const static int kInvalidRoomId = 0;  //
    const static int kInvalidChannelId = 0;  // 若分区id<=0，则为无效分区

    virtual ~LiveStreamingService() {}

    struct IntlRoomInfo {
        int64_t room_id = 0;
        std::string website_link;
        std::string server_address;
        std::string backup_address;
        std::string stream_key;
    };

    using GetIntlRoomInfoHandler = std::function<void(bool valid_response, int code, const std::string& err_msg,
        const IntlRoomInfo& room_info)>;

    virtual RequestProxy GetIntlRoomInfo(int64_t mid, GetIntlRoomInfoHandler handler) = 0;

    struct ProtocolInfo
    {
        std::string protocol;
        std::string key;    // code
        std::string addr;
        std::string new_link;
        std::string live_key;
    };
    using ProtocolInfos = std::vector<ProtocolInfo>;

    struct StartLiveNotice
    {
        int type = 0;
        int status = 0;
        std::string title;
        std::string msg;
        std::string button_text;
        std::string button_url;
    };

    struct StartLiveInfo {
        int room_type = 0;
        bool need_face_auth;          // 是否开启人脸识别认证
        std::string qr;               // 人脸识别二维码地址
        StartLiveNotice notice;
        ProtocolInfo rtmp_info;
        ProtocolInfos protocol_list;
        StartLiveInfo()
            : room_type(0)
            , need_face_auth(false)
            , qr(""){}
    };

    // Open live room
    using OpenLiveRoomHandler = std::function<void(
        bool valid,
        int code,
        const std::string& err_msg,
        const StartLiveInfo& start_live_info)>;


    // 海外开播 - 接口
    virtual RequestProxy StartViddupLive(OpenLiveRoomHandler handler) = 0;

    // Close live room
    using CloseLiveRoomHandler = std::function<void(bool success,
                                                    int result_code,
                                                    const std::string& error_msg)>;


    // 醒目留言
    struct MarkednessMessageInfo
    {
        int64_t id = 0; // 唯一标识
        int64_t uid = 0;
        int64_t price = 0;  // 人民币
        int rate = 0;       // 乘以人民币，得到金瓜子数
        std::string background_color;  // 选项卡上部分背景色
        std::string background_image;
        std::string background_icon;
        // 选项卡下部分背景色，也是胶囊填充色。
        // 胶囊背景色为该颜色的 50% 透明度
        std::string background_bottom_color;
        std::string background_price_color;
        std::string face;
        int guard_level = 0;
        std::string uname;
        std::string name_color;
        std::string message_content;
        int64_t remain_time = 0;
        int64_t start_time = 0;
        int64_t end_time = 0;
        int64_t current_time = 0;
        int64_t time_duration = 0;

        double color_point = 0;
        std::string background_color_start;
        std::string background_color_end;
        std::string message_font_color;

        // 勋章
        bool has_medal = false;
        std::string medal_name;
        int medal_color_start = 0;
        int medal_color_end = 0;
        int medal_color_border = 0;
        int medal_level = 0;
        int medal_guard_level = 0;
        int64_t anchor_id = 0;

        // 礼物
        int64_t gift_id = 0;
        int64_t gift_num = 0;
        std::string gift_name;

        int64_t ts = 0;
        std::string token;

        // 测试用
        bool is_test = false;
    };

};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_LIVE_STREAMING_SERVICE_H_

