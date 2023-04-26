#include "broadcast_service.h"

#include <regex>

#include "base/json/json_writer.h"
#include "base/json/json_reader.h"

#include "base/notification/notification_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/base64.h"

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/bililive_database.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"

#include "bilibase/basic_types.h"

#include "grit/generated_resources.h"
#include "bililive/public/protobuf/broadcast.pb.h"
using namespace bilibili::live::gift::v1;

#define PARSE_CMD(cmd_str, type, parser)  \
    else if (cmd == cmd_str) {            \
        if (parser(local_danmaku)) {      \
            statistics_.OnDanmaku(        \
                type, livehime::DanmakuStatistics::Code::NONE);  \
        } else {                          \
            statistics_.OnDanmaku(type);  \
        }                                 \
    }

namespace
{
    const char kGiftCoinTypeSilver[] = "silver";
    const char kGiftCoinTypeGold[] = "gold";

    int64_t gGiftCost = -1;
    BililiveBroadcastService *g_danmaku_service_ = nullptr;
}

int64_t BililiveBroadcastService::audience_ = 0;
int64_t BililiveBroadcastService::attention_ = 0;


BililiveBroadcastService::BililiveBroadcastService()
    : invoke_stop_listening_(false)
    //, presenter_(std::make_unique<DanmakuBroadcastPresenter>())
{
    DCHECK(g_danmaku_service_ == nullptr);
    g_danmaku_service_ = this;

    registrar_.Add(this, bililive::NOTIFICATION_BILILIVE_DANMAKU_MSG, base::NotificationService::AllSources());
    registrar_.Add(this, bililive::NOTIFICATION_BILILIVE_AUDIENCES_COUNT, base::NotificationService::AllSources());
    registrar_.Add(this, bililive::NOTIFICATION_BILILIVE_DANMAKU_STATUS, base::NotificationService::AllSources());
    registrar_.Add(this, bililive::NOTIFICATION_BILILIVE_ATTENTION_COUNT, base::NotificationService::AllSources());
    registrar_.Add(this, bililive::NOTIFICATION_BILILIVE_WIDGET_MESSAGE, base::NotificationService::AllSources());
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_MAYBE_USER_CONFUSED, base::NotificationService::AllSources());
}

BililiveBroadcastService::~BililiveBroadcastService()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
    DCHECK(invoke_stop_listening_) << "StopListening must be called before destruct";

    registrar_.RemoveAll();

    g_danmaku_service_ = nullptr;
}


void BililiveBroadcastService::StartListening(int64_t uid, int64_t room_id) {
    //presenter_->StartListening(uid, room_id);
}

void BililiveBroadcastService::StopListening() {
    invoke_stop_listening_ = true;
    //presenter_->StopListening();
    statistics_.Flush();
}

void BililiveBroadcastService::Observe(
    int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_BILILIVE_DANMAKU_MSG:
    {
        statistics_.BeginScope();

        auto dm_ptr = reinterpret_cast<scoped_ptr<base::DictionaryValue>*>(details.map_key());
        auto local_danmaku = dm_ptr->Pass();

        std::string cmd;
        local_danmaku->GetString("cmd", &cmd);

        // 普通弹幕
        if (cmd.find("DANMU_MSG") != std::string::npos)
        {
            if (cmd == "DANMU_MSG") {
                ParseDanmuMsg(local_danmaku);
            } else {
                std::regex reg("^DANMU_MSG(:\\d+){5}:(\\d+)");
                std::smatch match;
                std::string result;
                if (std::regex_search(cmd, match, reg))
                {
                    result = match[2].str();
                }

                if (!result.empty()) {
                    auto show = (std::stoi(result) & (1 << 0)) ? false : true;
                    if (show) {
                        ParseDanmuMsg(local_danmaku);
                    }
                } else {
                    statistics_.OnDanmaku(
                        DT_DANMU_MSG,
                        livehime::DanmakuStatistics::Code::REGEX_MATCHING_ERROR);
                }
            }
        }
        PARSE_CMD("NOTICE_MSG",              DT_NOTICE_MSG,        ParseNoticeMsg)     // 新公告
        PARSE_CMD("SEND_GIFT",               DT_SEND_GIFT,         ParseSendGift)      // 礼物（道具）提示
        PARSE_CMD("WARNING",                 DT_WARNING,           ParseWarning)       // 超管警告原因
        PARSE_CMD("CUT_OFF",                 DT_CUT_OFF,           ParseCutoff)        // 切断原因       
        PARSE_CMD("ANCHOR_NORMAL_NOTIFY",    DT_NORMALNOTIFY,      ParseAnchorNormalNotify)
        statistics_.EndScope();
    }
    break;
    case bililive::NOTIFICATION_BILILIVE_AUDIENCES_COUNT:
    {
        auto audience = reinterpret_cast<int64_t*>(details.map_key());
        if (audience)
        {
            audience_ = *audience;
        }
        FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, RoomAudience(audience_));
    }
    break;
    case bililive::NOTIFICATION_BILILIVE_DANMAKU_STATUS:
    {
        auto status = reinterpret_cast<int*>(details.map_key());
        if (status)
        {
            FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, DanmakuStatus(*status));
        }
    }
    break;
    case bililive::NOTIFICATION_BILILIVE_ATTENTION_COUNT:
    {
        auto attention = reinterpret_cast<int64_t*>(details.map_key());
        if (attention)
        {
            attention_ = *attention;
        }
        FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, RoomAttention(attention_));
    }
    break;
    case bililive::NOTIFICATION_BILILIVE_WIDGET_MESSAGE:
    {
        auto msg = reinterpret_cast<MSG*>(details.map_key());
        if (msg)
        {
            FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, OnWidgetMessage(*msg));
        }
    }
        break;
    case bililive::NOTIFICATION_LIVEHIME_MAYBE_USER_CONFUSED:
    {
        statistics_.Flush();
        break;
    }
    default:
        break;
    }
}

//static
int64_t BililiveBroadcastService::gift()
{
    if (gGiftCost < 0)
        return 0;
    return gGiftCost;
}

void BililiveBroadcastService::RecordDanmakuInfoToDB(const DanmakuInfo &danmaku)
{
    BililiveDatabase * const db = GetBililiveProcess()->database();
    if (db)
    {
        db->InsertDanmaku(danmaku);
    }
}

void BililiveBroadcastService::FillCommonNoticeDanmakuInfo(scoped_ptr<base::DictionaryValue>& local_danmaku, CommonNoticeDanmakuInfo& common_notice_danmaku)
{
    base::ListValue* js_list = nullptr;
    base::DictionaryValue* js_dict = nullptr;
    base::DictionaryValue* js_data_dict = nullptr;
    int index = 0;

    if (!local_danmaku->GetDictionary("data", &js_data_dict))
    {
        return;
    }

    if (js_data_dict->GetList("terminals", &js_list))
    {
        for (auto &value : *js_list)
        {
            int t{};
            value->GetAsInteger(&t);

            // 投放终端,1:粉版直播间 2: blink 3: 粉版播端 4: web  5: pc_link
            if (t == 5)
            {
                common_notice_danmaku.terminals = 5;
                break;
            }
        }
    }

    auto& danmaku_style = common_notice_danmaku.danmaku_style;
    if (js_data_dict->GetDictionary("danmaku_style", &js_dict))
    {
        if (js_dict->GetList("background_color", &js_list))
        {
            for (auto &value : *js_list)
            {
                SkColor color;
                std::string color_text;
                if (value->GetAsString(&color_text) &&
                    bililive::ParseColorString(color_text, &color))
                {
                    danmaku_style.background_color.push_back(color);
                }
            }

            if (js_dict->GetList("background_color_dark", &js_list))
            {
                for (auto& value : *js_list)
                {
                    SkColor color;
                    std::string color_text;
                    if (value->GetAsString(&color_text) &&
                        bililive::ParseColorString(color_text, &color))
                    {
                        danmaku_style.background_color_dark.push_back(color);
                    }
                }
            }
        }
    }

    if (js_data_dict->GetList("content_segments", &js_list))
    {
        for (auto& value : *js_list)
        {
            CommonNoticeDanmakuElem elem;
            base::DictionaryValue* js_content_dict = nullptr;
            if (value->GetAsDictionary(&js_content_dict))
            {
                int type{};
                js_content_dict->GetInteger("type", &type);
                elem.elem_type = type;
                switch (type)
                {
                    case 1:
                        elem.content_segments = std::reinterpret_pointer_cast<void>(ParseLabelTextContentSegment(js_content_dict));
                    break;

                    case 2:
                        elem.content_segments = std::reinterpret_pointer_cast<void>(ParseImageContentSegment(js_content_dict));
                    break;

                    case 3:
                        elem.content_segments = std::reinterpret_pointer_cast<void>(ParseButtonContentSegment(js_content_dict));
                    break;
                }
            }

            common_notice_danmaku.vec_segment.push_back(elem);
        }
    }
}

std::shared_ptr<CommonNoticeTextSegment> BililiveBroadcastService::ParseLabelTextContentSegment(base::DictionaryValue* js_dict)
{
    CommonNoticeTextSegment segment;
    std::string color_text;
    SkColor color;

    js_dict->GetInteger("type", &segment.type);

    js_dict->GetString("text", &segment.text);

    if (js_dict->GetString("font_color", &color_text))
    {
        if (bililive::ParseColorString(color_text, &color))
        {
            segment.font_color = color;
        }
    }

    if (js_dict->GetString("highlight_font_color", &color_text))
    {
        if (bililive::ParseColorString(color_text, &color))
        {
            segment.hightlight_font_color = color;
        }
    }

    if (js_dict->GetString("font_color_dark", &color_text))
    {
        if (bililive::ParseColorString(color_text, &color))
        {
            segment.font_color_dark = color;
        }
    }

    if (js_dict->GetString("highlight_font_color_dark", &color_text))
    {
        if (bililive::ParseColorString(color_text, &color))
        {
            segment.highlight_font_color_dark = color;
        }
    }

    return std::make_shared<CommonNoticeTextSegment>(segment);
}

std::shared_ptr<CommonNoticeImageSegment> BililiveBroadcastService::ParseImageContentSegment(base::DictionaryValue* js_dict)
{
    CommonNoticeImageSegment segment;

    js_dict->GetInteger("type", &segment.type);

    js_dict->GetString("img_url", &segment.img_url);

    js_dict->GetString("img_url_dark", &segment.img_url_dark);

    js_dict->GetInteger("img_width", &segment.img_width);

    js_dict->GetInteger("img_height", &segment.img_height);

    return std::make_shared<CommonNoticeImageSegment>(segment);
}

std::shared_ptr<CommonNoticeButtonSegment> BililiveBroadcastService::ParseButtonContentSegment(base::DictionaryValue* js_dict)
{
    CommonNoticeButtonSegment segment;
    base::ListValue* js_list = nullptr;
    SkColor color{};
    std::string color_text;

    js_dict->GetInteger("type", &segment.type);

    if (js_dict->GetList("background_color", &js_list))
    {
        for (auto& tmp_value : *js_list)
        {
            if (tmp_value->GetAsString(&color_text) &&
                bililive::ParseColorString(color_text, &color))
            {
                segment.background_color.push_back(color);
            }
        }
    }

    if (js_dict->GetList("background_color_dark", &js_list))
    {
        for (auto& tmp_value : *js_list)
        {
            if (tmp_value->GetAsString(&color_text) &&
                bililive::ParseColorString(color_text, &color))
            {
                segment.background_color_dark.push_back(color);
            }
        }
    }

    if (js_dict->GetString("font_color_dark", &color_text))
    {
        if (bililive::ParseColorString(color_text, &color))
        {
            segment.font_color_dark = color;
        }
    }

    if (js_dict->GetString("font_color", &color_text))
    {
        if (bililive::ParseColorString(color_text, &color))
        {
            segment.font_color = color;
        }
    }

    js_dict->GetBoolean("font_bold", &segment.font_bold);

    js_dict->GetString("text", &segment.text);

    js_dict->GetString("uri", &segment.uri);

    return std::make_shared<CommonNoticeButtonSegment>(segment);
}

//void BililiveBroadcastService::FillLabelTextContentSegment(base::DictionaryValue* js_content_dict, CommonNoticeDanmakuInfo& common_notice_danmaku)
//{
//    auto& text_segment = common_notice_danmaku.content_segments.text_segment;
//    SkColor color;
//    std::string color_text;
//
//    common_notice_danmaku.has_text = true;
//
//    js_content_dict->GetInteger("type", &text_segment.type);
//
//    js_content_dict->GetString("text", &text_segment.text);
//
//    if (js_content_dict->GetString("font_color", &color_text))
//    {
//        if (bililive::ParseColorString(color_text, &color))
//        {
//            text_segment.font_color = color;
//        }
//    }
//
//    if (js_content_dict->GetString("highlight_font_color", &color_text))
//    {
//        if (bililive::ParseColorString(color_text, &color))
//        {
//            text_segment.hightlight_font_color = color;
//        }
//    }
//
//    if (js_content_dict->GetString("font_color_dark", &color_text))
//    {
//        if (bililive::ParseColorString(color_text, &color))
//        {
//            text_segment.font_color_dark = color;
//        }
//    }
//
//    if (js_content_dict->GetString("highlight_font_color_dark", &color_text))
//    {
//        if (bililive::ParseColorString(color_text, &color))
//        {
//            text_segment.highlight_font_color_dark = color;
//        }
//    }
//}
//
//void BililiveBroadcastService::FillImageContentSegment(base::DictionaryValue* js_content_dict, CommonNoticeDanmakuInfo& common_notice_danmaku)
//{
//    auto& image_segment = common_notice_danmaku.content_segments.image_segment;
//    base::Value* js_value = nullptr;
//
//    common_notice_danmaku.has_image = true;
//
//    js_content_dict->GetInteger("type", &image_segment.type);
//
//    js_content_dict->GetString("img_url", &image_segment.img_url);
//
//    js_content_dict->GetInteger("img_width", &image_segment.img_width);
//
//    js_content_dict->GetInteger("img_height", &image_segment.img_height);
//}
//
//void BililiveBroadcastService::FillButtonContentSegment(base::DictionaryValue* js_content_dict, CommonNoticeDanmakuInfo& common_notice_danmaku)
//{
//    auto& button_segment = common_notice_danmaku.content_segments.button_segment;
//    base::ListValue* js_list = nullptr;
//    int32_t index{};
//    SkColor color;
//    std::string color_text;
//
//    common_notice_danmaku.has_button = true;
//
//    js_content_dict->GetInteger("type", &button_segment.type);
//
//    if (js_content_dict->GetList("background_color", &js_list))
//    {
//        index = 0;
//        button_segment.background_color_count = std::min(static_cast<uint32_t>(js_list->GetSize()), 3u);
//        for (auto& tmp_value : *js_list)
//        {
//            if (index >= 3)
//            {
//                break;
//            }
//
//            if (tmp_value->GetAsString(&color_text) &&
//                bililive::ParseColorString(color_text, &color))
//            {
//                button_segment.background_color[index] = color;
//            }
//
//            index++;
//        }
//    }
//
//    if (js_content_dict->GetList("background_color_dark", &js_list))
//    {
//        index = 0;
//        button_segment.background_color_dark_count = std::min(static_cast<uint32_t>(js_list->GetSize()), 3u);
//        for (auto& tmp_value : *js_list)
//        {
//            if (index >= 3)
//            {
//                break;
//            }
//
//            if (tmp_value->GetAsString(&color_text) &&
//                bililive::ParseColorString(color_text, &color))
//            {
//                button_segment.background_color_dark[index] = color;
//            }
//
//            index++;
//        }
//    }
//
//    if (js_content_dict->GetString("font_color_dark", &color_text))
//    {
//        if (bililive::ParseColorString(color_text, &color))
//        {
//            button_segment.font_color_dark = color;
//        }
//    }
//
//    if (js_content_dict->GetString("font_color", &color_text))
//    {
//        if (bililive::ParseColorString(color_text, &color))
//        {
//            button_segment.font_color = color;
//        }
//    }
//
//    js_content_dict->GetBoolean("font_bold", &button_segment.font_bold);
//
//    js_content_dict->GetString("text", &button_segment.text);
//
//    js_content_dict->GetString("uri", &button_segment.uri);
//}

bool BililiveBroadcastService::ParseDanmuMsg(scoped_ptr<base::DictionaryValue> &local_danmaku)
{
    int64_t uid = 0;
    int64_t stime = 0;
    int64_t rnd = 0;
    int64_t ts = 0;
    int64_t he_rank = 0;
    std::string uname;
    std::string text;
    std::string ct;
    int manager = 0;
    int vip = 0;
    int svip = 0;
    int barrage = 0;
    int dmk_special_type = 0;
    std::string bg_color;
    std::string uname_color;
    std::string title_id;
    int32_t dmk_type = 0;

    std::string voice_url;
    std::string voice_text;
    std::string file_format;
    int file_duration = 0;

    bool is_emoji_danmaku = false;
    EmojiDanmakuInfo emoji_danmaku_info;
    std::vector<EmojiDanmakuInfo> emoji_list;


    const base::ListValue* info_list = nullptr;
    if (local_danmaku->GetList("info", &info_list))
    {
        if (info_list->GetSize() >= 4)
        {
            const base::ListValue* list0 = nullptr;
            if (info_list->GetList(0, &list0))
            {
                if (list0->GetSize() >= 12)
                {
                    // 时间戳，单位为毫秒
                    list0->GetInteger64(4, &stime);
                    list0->GetInteger64(5, &rnd);
                    // 弹幕类型
                    list0->GetInteger(9, &dmk_special_type);
                    // 注意这里的 bg_color 里有三个颜色！颜色以逗号分隔。
                    // 第一个颜色对应白色主题，第二个颜色对应黑色主题
                    list0->GetString(11, &bg_color);

                    list0->GetInteger(12, &dmk_type);
                }
            }
            info_list->GetString(1, &text);

            const base::ListValue* list2 = nullptr;
            if (info_list->GetList(2, &list2))
            {
                if (list2->GetSize() >= 8)
                {
                    list2->GetInteger64(0, &uid);
                    list2->GetString(1, &uname);
                    list2->GetInteger(2, &manager);
                    list2->GetInteger(3, &vip);
                    list2->GetInteger(4, &svip);
                    list2->GetString(7, &uname_color);
                }
            }
            info_list->GetInteger(7, &barrage);          

            const ListValue* list4 = nullptr;
            if (info_list->GetList(4, &list4) && !list4->empty()) {
                bool rv = list4->GetInteger64(4, &he_rank);
                if (!rv) {
                    LOG(WARNING) << "Parse danmaku msg failed: invalid rank data!";
                }
            }

            const ListValue* list5 = nullptr;
            if (info_list->GetList(5, &list5)) {
                list5->GetString(1, &title_id);
            }

            const base::DictionaryValue* dict_value = nullptr;
            if (info_list->GetDictionary(9, &dict_value)) {
                if (dict_value) {
                    bool rv = dict_value->GetInteger64("ts", &ts);
                    rv &= dict_value->GetString("ct", &ct);

                    if (!rv) {
                        LOG(WARNING) << "Parse danmaku msg failed: invalid verify data!";
                    }
                }
            }
        }
    }

    // 回调添加弹幕
    DanmakuInfo danmaku;
    danmaku.stime = stime / 1000;
    danmaku.rnd = rnd;
    danmaku.uid = uid;
    danmaku.uname = uname;
    danmaku.danmaku = text;
    danmaku.dt = DT_DANMU_MSG;
    danmaku.manager = manager;
    danmaku.vip = vip;
    danmaku.svip = svip;
    danmaku.barrage = barrage;
    danmaku.ts = ts;
    danmaku.ct = ct;
    danmaku.special_type = dmk_special_type;
    danmaku.bg_color = bg_color;
    danmaku.highlight_color = uname_color;
    danmaku.he_rank = he_rank;
    danmaku.title_id = title_id;
    danmaku.is_emoji_danmaku = is_emoji_danmaku;
    if (is_emoji_danmaku)
    {
        danmaku.emoji_danmaku_info = emoji_danmaku_info;
    }

    danmaku.voice_url = voice_url;
    danmaku.file_duration = file_duration;
    danmaku.file_format = file_format;
    danmaku.text = UTF8ToUTF16(voice_text);
    danmaku.emoji_list = emoji_list;

    FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewDanmaku(danmaku));

    return true;
}

bool BililiveBroadcastService::ParseNoticeMsg(scoped_ptr<base::DictionaryValue> &local_danmaku) {
    NoticeDanmakuInfo info;
    if (local_danmaku->GetInteger("msg_type", &info.msg_type) &&
        local_danmaku->GetInteger64("real_roomid", &info.room_id) &&
        local_danmaku->GetString("msg_common", &info.msg_common) &&
        local_danmaku->GetString("msg_self", &info.msg_self))
    {
        FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewNoticeDanmaku(info));
        return true;
    }
    return false;
}

bool BililiveBroadcastService::ParseSendGift(scoped_ptr<base::DictionaryValue> &local_danmaku)
{
    bool succeeded = false;
    const base::DictionaryValue* data = nullptr;
    if (local_danmaku->GetDictionary("data", &data))
    {
        bool gift_switch = true;
        data->GetBoolean("switch", &gift_switch);
        if (!gift_switch) {
            LOG(INFO) << "ParseSendGift switch value is false ";
            return false;
        }
        DanmakuInfo danmaku;
        if (data->GetString("action", &danmaku.action) &&
            data->GetInteger64("uid", &danmaku.uid) &&
            data->GetString("uname", &danmaku.uname) &&
            data->GetInteger64("giftId", &danmaku.gift_id) &&
            data->GetString("giftName", &danmaku.giftName) &&
            // 批量数（点一次送礼按钮时赠送的数量）
            data->GetInteger64("num", &danmaku.num) &&
            data->GetInteger64("timestamp", &danmaku.stime) &&
            // 连击数，用于连击条
            data->GetInteger64("super_batch_gift_num", &danmaku.super_gift_num))
        {
            danmaku.danmaku = base::StringPrintf(
                "%s %s %s x %lld",
                danmaku.uname.c_str(), danmaku.action.c_str(), danmaku.giftName.c_str(), danmaku.num);
            danmaku.dt = DT_SEND_GIFT;

            data->GetBoolean("is_naming", &danmaku.is_naming);
            data->GetString("face", &danmaku.face_url);
            data->GetInteger("guard_level", &danmaku.barrage);
            data->GetInteger("combo_stay_time", &danmaku.combo_stay_time);
            data->GetInteger("combo_resources_id", &danmaku.combo_res_id);
            data->GetString("batch_combo_id", &danmaku.batch_combo_id);
            data->GetInteger64("face_effect_id", &danmaku.face_effect_id);
            data->GetInteger64("face_effect_type", &danmaku.face_effect_type);

            std::string coin_type;
            data->GetString("coin_type", &coin_type);

            // 仅一次批量的总价（带折扣），不算连击数
            data->GetInteger64("total_coin", &danmaku.batch_total_coin);
            // 在 total_coin 的基础上考虑连击数
            data->GetInteger64("combo_total_coin", &danmaku.total_coin);
            data->GetInteger("demarcation", &danmaku.demarcation);
            data->GetInteger64("float_sc_resource_id", &danmaku.float_sc_resource_id);

            data->GetInteger64("discount_price", &danmaku.discount_price);

            int special_batch = 0;
            data->GetInteger("is_special_batch", &special_batch);
            danmaku.is_special_batch = special_batch == 1;
            data->GetString("name_color", &danmaku.highlight_color);

            int guard_level = 0;
            data->GetInteger("guard_level", &guard_level);
            danmaku.guard_level = static_cast<GuardLevel>(guard_level);

            bool is_join_receiver = false;
            data->GetBoolean("is_join_receiver",&is_join_receiver);
            if (is_join_receiver) {
                danmaku.is_join_receiver = is_join_receiver;
                data->GetInteger64("receive_user_info.uid",&danmaku.receive_user.uid);
                data->GetString("receive_user_info.uname",&danmaku.receive_user.uname);
                danmaku.action += base::UTF16ToUTF8(L"给");
            }

            // 用于标识是金瓜子礼物还是银瓜子礼物。
            // 银瓜子：silver、金瓜子：gold
            if (coin_type == kGiftCoinTypeGold)
            {
                danmaku.gift_coin_type = GCT_GOLD;
            }
            else
            {
                danmaku.gift_coin_type = GCT_SILVER;
            }

            // 新的连击机制
            if (data->HasKey("batch_combo_send")) {
                if (data->GetInteger64("batch_combo_send.batch_combo_num", &danmaku.combo_num) &&
                    data->GetInteger64("batch_combo_send.gift_num", &danmaku.batch_num))
                {
                    danmaku.has_combo_send = true;

                    // 盲盒
                    const DictionaryValue* blind_dict;
                    if (data->GetDictionary("blind_gift", &blind_dict)) {
                        danmaku.is_combo_blind_gift = true;
                        blind_dict->GetString("original_gift_name", &danmaku.combo_original_gift_name);
                        blind_dict->GetString("gift_action", &danmaku.combo_blind_action);
                    }
                }
            }

            // 盲盒
            const DictionaryValue* blind_dict;
            if (data->GetDictionary("blind_gift", &blind_dict)) {
                danmaku.is_blind_gift = true;
                blind_dict->GetString("original_gift_name", &danmaku.original_gift_name);
                blind_dict->GetString("gift_action", &danmaku.blind_action);
            }

            FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewDanmaku(danmaku));
            succeeded = true;

            // 辣条不记录
            if (danmaku.gift_id != 1)
            {
                // 调用IO线程将弹幕数据存表
                BililiveThread::PostTask(BililiveThread::DB, FROM_HERE,
                    base::Bind(&BililiveBroadcastService::RecordDanmakuInfoToDB, danmaku));
            }
        }

        data->GetInteger64("rcost", &gGiftCost);
    }

    return succeeded;
}

bool BililiveBroadcastService::ParseWarning(scoped_ptr<base::DictionaryValue> &local_danmaku)
{
    DanmakuInfo danmaku;
    if (local_danmaku->GetString("msg", &danmaku.msg) &&
        local_danmaku->GetInteger64("roomid", &danmaku.room_id))
    {
        danmaku.dt = DT_WARNING;
        FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewDanmaku(danmaku));
        return true;
    }
    return false;
}

bool BililiveBroadcastService::ParseCutoff(scoped_ptr<base::DictionaryValue> &local_danmaku)
{
    DanmakuInfo danmaku;
    if (local_danmaku->GetString("msg", &danmaku.msg) &&
        local_danmaku->GetInteger64("roomid", &danmaku.room_id))
    {
        local_danmaku->GetInteger("category", &danmaku.category);
        if (danmaku.category == 1) {
            local_danmaku->GetInteger("pop.type", &danmaku.pop_type);
            local_danmaku->GetString("pop.title", &danmaku.pop_title);
            local_danmaku->GetString("pop.msg", &danmaku.pop_msg);
            local_danmaku->GetString("pop.button_text", &danmaku.pop_button_text);
            local_danmaku->GetString("pop.button_url", &danmaku.pop_button_url);
        }

        danmaku.dt = DT_CUT_OFF;
        FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewDanmaku(danmaku));
        return true;
    }
    return false;
}

bool BililiveBroadcastService::ParseAnchorNormalNotify(scoped_ptr<base::DictionaryValue>& local_danmaku) {
    const base::DictionaryValue* data = nullptr;
    if (local_danmaku->GetDictionary("data", &data)) {
        AnchorNormalNotifyInfo info;
        if (data->GetInteger64("type", &info.type) &&
            data->GetInteger64("show_type", &info.show_type) &&
            data->GetString("info.icon", &info.icon) &&
            data->GetString("info.title", &info.title) &&
            data->GetString("info.content", &info.content))
        {
            info.dt = DT_NORMALNOTIFY;
            FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewAnchorNormalNotifyDanmaku(info));
            return true;
        }
    }
    return false;
}

bool BililiveBroadcastService::ParseLotCheckStatus(scoped_ptr<base::DictionaryValue>& local_danmaku) {
    const base::DictionaryValue* data = nullptr;
    if (local_danmaku->GetDictionary("data", &data)) {
        int64_t status = 0;
        if (data->GetInteger64("status", &status))
        {
            base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_LOTTERY_ANCHOR_LOT_CHECKSTATUS,
                base::NotificationService::AllSources(),
                base::Details<int64_t>(&status));
        }
    }

    std::string change_info;
    base::JSONWriter::Write(local_danmaku.get(), &change_info);

    if (!change_info.empty()) {
        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_LOTTERY_STATUS_CHANGED_S,
            base::NotificationService::AllSources(),
            base::Details<std::string>(&change_info));
    }
    return true;
}

bool BililiveBroadcastService::ParseLotEnd(scoped_ptr<base::DictionaryValue>& local_danmaku) {
    const base::DictionaryValue* data = nullptr;
    if (local_danmaku->GetDictionary("data", &data)) {
        int64_t lottery_id = 0;
        if (data->GetInteger64("id", &lottery_id))
        {
            base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_LOTTERY_ANCHOR_LOT_END,
                base::NotificationService::AllSources(),
                base::Details<int64_t>(&lottery_id));
            return true;
        }
    }
    return false;
}

bool BililiveBroadcastService::ParseLotAward(scoped_ptr<base::DictionaryValue>& local_danmaku) {
    std::string change_info;
    base::JSONWriter::Write(local_danmaku.get(), &change_info);

    if (!change_info.empty()) {
        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_LOTTERY_ANCHOR_LOT_AWARD,
            base::NotificationService::AllSources(),
            base::Details<std::string>(&change_info));
    }
    return true;
}

bool BililiveBroadcastService::ParseAnchorPromotionUpdate(scoped_ptr<base::DictionaryValue>& local_danmaku) {
    const base::DictionaryValue* data = nullptr;
    if (local_danmaku->GetDictionary("data", &data)) {
        AnchorPromotionDanmakuInfo info;
        data->GetInteger("order_type", &info.order_type);
        data->GetInteger64("uid", &info.uid);
        data->GetInteger64("order_id", &info.order_id);
        data->GetInteger64("show", &info.show);
        data->GetInteger64("click", &info.click);
        data->GetInteger64("total_online", &info.popularity);
        data->GetInteger64("coin_num", &info.coin_num);
        data->GetInteger64("coin_cost", &info.coin_cost);
        data->GetString("live_key", &info.live_key);
        data->GetInteger("status", &info.status);

        //FOR_EACH_OBSERVER(BililiveBroadcastObserver, observer_list_, NewAnchorPromotionDanmaku(info));
        return true;
    }
    return false;
}