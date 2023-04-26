#ifndef BILILIVE_SECRET_PUBLIC_DANMAKU_HIME_SERVICE_H_
#define BILILIVE_SECRET_PUBLIC_DANMAKU_HIME_SERVICE_H_

#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "base/strings/string_piece.h"

#include "bililive/secret/public/request_connection_proxy.h"


namespace secret {

class DanmakuHimeService {
public:
    virtual ~DanmakuHimeService() = default;

    // Download image
    using DownloadImageHandler = std::function<void(
        bool valid_response, const std::string& data)>;

    virtual RequestProxy DownloadImage(
        const std::string& url, DownloadImageHandler handler) = 0;
    struct GiftConfigCountMap
    {
        int         num = 0;
        int64_t     effect_id = 0;
    };
    // Get gift config
    struct GiftConfigInfo
    {
        int64_t                         id = 0;
        int                             type = 0;
        int                             stay_time;
        std::string                     icon_url;
        std::string                     coin_type;
        std::string                     gif_url;
        int                             combo_res_id = 0;

        int64_t                         effect_id = 0;
        std::vector<GiftConfigCountMap> count_map;
    };
    using SendDanmakuViddupHandler = std::function<void(bool valid_response, int code, const std::string& msg)>;
    virtual RequestProxy SendViddupDanmaku(
        int64_t room_id, const std::string& msg,
        int64_t rnd, int font_size, int mode, SendDanmakuViddupHandler handler) = 0;
};

}  // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_DANMAKU_HIME_SERVICE_H_