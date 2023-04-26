#include "broadcast_viddup_service.h"

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
    BililiveBroadcastViddupService *g_danmaku_service_ = nullptr;
}

int64_t BililiveBroadcastViddupService::audience_ = 0;
int64_t BililiveBroadcastViddupService::attention_ = 0;


BililiveBroadcastViddupService::BililiveBroadcastViddupService()
    : invoke_stop_listening_(false)
    , presenter_(std::make_unique<DanmakuBroadcastViddupPresenter>())
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

BililiveBroadcastViddupService::~BililiveBroadcastViddupService()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
    DCHECK(invoke_stop_listening_) << "StopListening must be called before destruct";

    registrar_.RemoveAll();

    g_danmaku_service_ = nullptr;
}

void BililiveBroadcastViddupService::StartListening(int64_t uid, int64_t room_id) {
    presenter_->StartListening(uid, room_id);
}

void BililiveBroadcastViddupService::StopListening() {
    invoke_stop_listening_ = true;
    presenter_->StopListening();
    statistics_.Flush();
}

void BililiveBroadcastViddupService::Observe(
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

        if (cmd.find("DANMU_MSG") != std::string::npos)
        {
            if (cmd == "DANMU_MSG") {
                ParseDanmuMsg(local_danmaku);
            } else {
            }
        }

        statistics_.EndScope();
    }
    break;
    default:
        break;
    }
}

//static
int64_t BililiveBroadcastViddupService::gift()
{
    if (gGiftCost < 0)
        return 0;
    return gGiftCost;
}

bool BililiveBroadcastViddupService::ParseDanmuMsg(scoped_ptr<base::DictionaryValue> &local_danmaku)
{
    std::string danmaku_json;
    local_danmaku->GetAsString(&danmaku_json);

    int type = 0;
    int id = 0;
    std::string content;
    std::string userName;
    bool isAdmin = false;
    std::string gifFileUrl;  //url
    std::string giftName;
    std::string comboString;
    int treasureID = 0;

    std::vector<DanmakuViddupInfo> danmaku;
    const base::ListValue* info_list = nullptr;
    if (local_danmaku->GetList("info", &info_list)) {
        for (int i = 0; i < info_list->GetSize(); i++) {
            const base::DictionaryValue* dict_value = nullptr;
            if (info_list->GetDictionary(i, &dict_value)) {
                DanmakuViddupInfo info;
                dict_value->GetInteger("type", (int*)&info.type);
                dict_value->GetInteger("id", (int*)&info.id);
                dict_value->GetString("content", &info.content);
                dict_value->GetString("user_name", &info.userName);
                dict_value->GetBoolean("is_admin", &info.isAdmin);
                dict_value->GetString("gif_file_url", &info.gifFileUrl);
                dict_value->GetString("gift_name", &info.giftName);
                dict_value->GetString("combo_string", &info.comboString);
                dict_value->GetInteger("treasure_iD", &info.treasureID);
                dict_value->GetString("notice", &info.notice);

                dict_value->GetString("popup_title", &info.popupTitle);
                dict_value->GetString("popup_content", &info.popupContent);
                dict_value->GetInteger("duration", &info.duration);
                danmaku.push_back(info);
            }
        }
    }

    FOR_EACH_OBSERVER(BililiveBroadcastViddupObserver, observer_list_, NewDanmaku(danmaku));
    return true;
}