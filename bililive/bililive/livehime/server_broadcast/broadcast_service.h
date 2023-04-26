#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_DANMAKU_SERVICE_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_DANMAKU_SERVICE_H_

#include "build/build_config.h"

#include "SkColor.h"
#include "base/compiler_specific.h"
#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/memory/ref_counted.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"
#include "base/values.h"
#include "base/observer_list.h"

//#include "bililive/bililive/livehime/server_broadcast/danmaku_broadcast_presenter.h"
#include "bililive/bililive/livehime/server_broadcast/danmaku_statistics.h"
#include "bililive/secret/public/live_streaming_service.h"


enum BarrageType
{
    BARRAGE_GOVERNOR = 1,
    BARRAGE_ADMIRAL = 2,
    BARRAGE_CAPTAIN = 3,
};

enum FleetAchievementLevel {
    FLEET_ACH_NORMAL = 0,
    FLEET_ACH_HUNDRED = 100,
    FLEET_ACH_THOUSAND = 1000,
    FLEET_ACH_TEN_THOUSAND = 10000,
};

// 应与 ENTRY_EFFECT 的相应字段定义一致
// 1大航海,2活动,3高能榜,4荣耀装扮
enum EntryBusinessType {
    ENTRY_BUSINESS_FLEET = 1,  // 大航海进场
    ENTRY_BUSINESS_HER = 3,    // 高能榜进场
};

enum DanmakuType
{
    DT_NOSUPPORT = -1,
    DT_DANMU_MSG,    //普通消息弹幕(包括语音弹幕,voice_url不为空)
    DT_SYS_MSG,
    DT_NOTICE_MSG,
    DT_SEND_GIFT,
    DT_WARNING,
    DT_CUT_OFF,
    DT_NORMALNOTIFY,

    //直播小助手
    DT_ANCHOR_BROADCAST,
};

enum NoticeMsgType {
    NMT_SYS_MSG = 1,
    NMT_TV_SKY_LOTTERY = 2,
    NMT_GOV_LOTTERY = 3,
    NMT_GOV_ENTER = 4,
    NMT_TV_SKY_DRAWN = 5,
    NMT_HIGN_ENERGY = 6,
    NMT_HOUR_NO1 = 7,
    NMT_DOOR_LOTTERY = 8,
    NMT_MELEE_LEVEL_MSG = 9,
};

enum GiftCoinType
{
    GCT_SILVER,
    GCT_GOLD,
};

// 应与弹幕协议中的相应字段定义相同
enum class DanmakuSpecialType {
    NORMAL = 0,       // 普通弹幕
    RHYTHM_STORM = 1, // 节奏风暴弹幕
    TV = 2            // 小电视弹幕
};

// 应与 USER_TOAST_MSG 中的相应字段定义相同
enum class UTMOpType {
    Open = 1,      // 开通
    Renew = 2,     // 续费
    AutoRenew = 3, // 自动续费
};

// 应与 INTERACT_WORD 中的相应字段定义相同
enum class InteractIdentity {
    Normal = 1,      // 普通人
    Manager = 2,     // 房管
    Fan = 3,         // 普通粉丝
    MonthlyLord = 4, // 月费老爷
    YearlyLord = 5,  // 年费老爷
    Captain = 6,     // 舰长
    Admiral = 7,     // 提督
    Governor = 8,    // 总督
};

// 大航海等级
enum class GuardLevel {
    None = 0,
    Governor = 1,   // 舰长
    Admiral = 2,    // 提督
    Captain = 3     // 总督
};

// 应与 INTERACT_WORD 中的相应字段定义相同
enum class InteractType {
    Enter = 1,            // 进场
    Attention = 2,        // 关注
    Share = 3,            // 分享
    SpecialAttention = 4, // 特别关注
    FanEachOther = 5,     // 互粉
    UserLike = 6,         // 点赞
};



struct EmojiDanmakuInfo
{
    int32_t is_dynamic = 0;
    std::string url ="";
    std::string id = "";
    std::string data = "";
    int32_t in_player_area = 0;
    int32_t height = 0;
    int32_t width = 0;
    string16 name = L"";
    EmojiDanmakuInfo() = default;
    EmojiDanmakuInfo(std::string url, int32_t width, int32_t height, string16 name)
    :url(url),width(width), height(height), name(name){
    }
};

struct ReceiveUserInfo {
    string16 uname;
    int64_t uid = 0;
};

struct DanmakuInfo
{
    DanmakuType dt = DT_NOSUPPORT;

    // DANMAKU
    int64_t uid = 0;
    int color = 0xFFFFFFFF;
    int64_t stime = 0;
    int64_t rnd = 0;    //发送弹幕的时间戳
    std::string uname;
    std::string face_url;
    std::string danmaku;
    int manager = 0;
    int vip = 0;
    int svip = 0;
    int barrage = 0;
    int id = 0;
    int special_type = 0;

    //是否冠名
    bool is_naming = false;

    // 大航海进场/高能榜
    int priority;  //排序的权值,越小优先级越高
    std::string web_basemap_url;  //静态图(兜底)
    double web_effective_time;     //静态图显示时常
    std::string web_dynamic_url_webp; //动态图(首选)
    double effective_time_new;     // 动态图显示时常(大于0 选择动态图)

    // 注意这里的 bg_color 里有三个颜色！
    // 第一个颜色对应白色主题，第二个颜色对应黑色主题
    std::string bg_color;

    int category = 0;
    int pop_type = 0;
    std::string pop_title;
    std::string pop_msg;
    std::string pop_button_text;
    std::string pop_button_url;

    // 开通/续费大航海
    int utm_op_type = 0;
    std::string count_unit;
    std::string role_name;

    int64_t room_id = 0;
    std::string msg;

    // verify
    int64_t ts = 0;
    std::string ct;

    // GIFT
    int64_t gift_id = 0;
    int64_t effect_id = 0;
    std::string action;
    std::string giftName;
    int64_t num = 0;
    int64_t super_gift_num = 0;
    std::string r_uname;
    int64_t batch_num = 0;
    int64_t combo_num = 0;
    int combo_stay_time = 0;
    int combo_res_id = 0;
    bool is_special_batch = false;
    std::string batch_combo_id;
    //脸萌
    int64_t face_effect_id = 0;
    int64_t face_effect_type = 0;
    int demarcation = 0;
    int64_t float_sc_resource_id = 0;

    // 送礼人的大航海等级, 金瓜子礼物才会有值
    // 1:总督 2:提督 3:舰长
    GuardLevel guard_level = GuardLevel::None;

    // 盲盒礼物（连击条）
    bool is_blind_gift = false;
    std::string original_gift_name;
    std::string blind_action;

    // 盲盒礼物（弹幕）
    bool is_combo_blind_gift = false;
    std::string combo_original_gift_name;
    std::string combo_blind_action;

    int64_t start_time = 0;
    int64_t end_time = 0;
    bool has_combo_send = false;

    // 一次批量的金瓜子总数（带折扣）。
    // 当前仅用于历史弹幕。
    int64_t batch_total_coin = 0;

    // 与赠送礼物总数对应的金瓜子总数（带折扣）。
    // 与 batch_total_coin 的不同之处在于 total_coin 考虑连击数
    int64_t total_coin = 0;

    //赠送礼物单价，带折扣
    int64_t discount_price = 0;
    int64_t price = 0;   //礼物实际价值

    GiftCoinType gift_coin_type = GCT_SILVER;

    // 百人/千人舰队成就使用
    std::string frame_url;
    std::string danmaku_bg_url;
    std::string dialog_bg_url;
    int ga_event_type = 0;
    int achievement_level = 0;
    int show_time = 0;            // 主播端展示时间
    bool is_first_new = false;    // 是否是首次达成
    std::string first_line_text;  // 第一行文本，有高亮文字<%...%>
    std::string second_line_text; // 第二行文本，无特殊格式
    std::string highlight_color;  // 第一行文本高亮文字的颜色

    // INTERACT_WORD
    std::vector<int> interact_identities;
    int interact_type = 0;
    int64_t interact_ts = 0;
    int64_t interact_score = 0;
    bool is_spread = false;
    std::string spread_info;
    std::string spread_desc;
    int tail_icon = 0;
    int core_user_tag_id = 0;//是否核心付费用户:0:无标签,1:超能榜,2:大航海,3:粉丝团,4:高活跃,5:高能

    // 高能榜排名
    int64_t he_rank = 0;

    // 发言弹幕头衔 id
    std::string title_id;

    //语音弹幕
    std::string voice_url;
    std::string file_format;
    int file_duration = 0;
    string16 text;  //语音文本

    // ENTRY_EFFECT 使用
    int business_type = 0;
    std::vector<int> icon_list;

    //表情弹幕使用
    bool is_emoji_danmaku = false;
    EmojiDanmakuInfo emoji_danmaku_info;

    //文字弹幕中添加emoji
    //形式: 1111[前方高能]22[你真棒]3333
    std::vector<EmojiDanmakuInfo> emoji_list;

    //语聊房送礼添加送礼人
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;
};

struct NoticeDanmakuInfo {
    NoticeDanmakuInfo()
        : msg_type(0),
          room_id(0) {
    }

    int msg_type;
    int64_t room_id;
    std::string msg_common;
    std::string msg_self;
};

struct CommonNoticeTextSegment 
{
    int32_t type;                           // 1:带标签文本，2：图片，3：按钮
    std::string text;                       // <%%> 代表颜色高亮，<$$>代表加粗, <%<$$>%> 代表颜色高亮且加粗
    SkColor font_color;                     // 字体颜色
    SkColor hightlight_font_color;          //高亮字体颜色
    SkColor font_color_dark;                //高亮字体颜色
    SkColor highlight_font_color_dark;      //高亮字体夜间模式颜色
};

struct CommonNoticeImageSegment 
{
    int32_t type;
    std::string img_url;                     //图片地址
    std::string img_url_dark;               //夜间模式图片地址
    std::string imgae_data;
    std::string imgae_data_dark;
    int32_t img_width;
    int32_t img_height;
};

struct CommonNoticeButtonSegment 
{
    int32_t type;
    std::vector<SkColor> background_color;              // 按钮背景颜色
    std::vector<SkColor> background_color_dark;         // 按钮背景夜间模式颜色
    SkColor font_color_dark;
    SkColor font_color;
    bool font_bold;                                     // 是否加粗
    std::string text;
    std::string uri;                                    // 按钮跳转uri 例如bilibili://live/xxxx或https://xxx.xxx.com.xxx.xxx
};

struct CommonNoticeDanmakuElem 
{
    uint32_t elem_type;
    std::shared_ptr<void> content_segments;
};

struct CommonNoticeDanmakuInfo
{
    CommonNoticeDanmakuInfo() :
        terminals{},
        danmaku_style{}
    {

    }

    enum class TerminalType { kPinkLive = 1, kBlink = 2, kPinkPlayer = 3, kWeb = 4, kPC_Link = 5 };

    int32_t terminals;                                          // 投放终端,1:粉版直播间 2: blink 3: 粉版播端 4: web  5: pc_link
    struct
    {
        std::vector<SkColor> background_color;                  // 弹幕背景颜色
        std::vector<SkColor> background_color_dark;             // 弹幕背景夜间模式颜色
    }danmaku_style;

    std::vector<CommonNoticeDanmakuElem> vec_segment;
};

struct AnchorNormalNotifyInfo {
    AnchorNormalNotifyInfo()
        : dt(DT_NORMALNOTIFY)
        , type(0)
        , show_type(0) {}

    DanmakuType dt;

    int64_t type;
    int64_t show_type;
    std::string icon;
    std::string title;
    std::string content;
};

struct SelectedDanmakuDetails {
    int64_t user_id = 0;
    std::string danmaku;
    std::string reason;
    int64_t ts = 0;
    std::string ct;
};

struct AnchorPromotionDanmakuInfo {
    int order_type = 0;
    int64_t uid = 0;
    int64_t order_id = 0;
    int64_t show = 0;
    int64_t click = 0;
    int64_t popularity = 0;
    int64_t coin_num = 0;
    int64_t coin_cost = 0;
    std::string live_key;
    int status = 0;
};

class BililiveBroadcastObserver
{
public:
    virtual ~BililiveBroadcastObserver() = default;

    virtual void NewDanmaku(const DanmakuInfo &danmaku) {}
    virtual void RoomAudience(const int64_t audience) {}
    virtual void DanmakuStatus(const int status) {}
    virtual void RoomAttention(const int64_t attention) {}
    virtual void OnWidgetMessage(const MSG &msg) {}
    virtual void NewNoticeDanmaku(const NoticeDanmakuInfo& danmaku) {}
    virtual void NewCommonNoticeDanmaku(const CommonNoticeDanmakuInfo& danmaku) {};
    virtual void NewAnchorNormalNotifyDanmaku(const AnchorNormalNotifyInfo& danmaku) {}
    virtual void NewSuperChatMessageDanmaku(const secret::LiveStreamingService::MarkednessMessageInfo& danmaku) {}
};

class BililiveBroadcastService : public base::NotificationObserver
{
public:
    BililiveBroadcastService();
    virtual ~BililiveBroadcastService();

    void StartListening(int64_t uid, int64_t room_id);
    void StopListening();

    static int64_t audience() { return audience_; }
    static int64_t attention() { return attention_; }
    static int64_t gift();

    void AddObserver(BililiveBroadcastObserver *observer) { observer_list_.AddObserver(observer); }
    void RemoveObserver(BililiveBroadcastObserver *observer) { observer_list_.RemoveObserver(observer); }

protected:
    // NotificationObserver
    void Observe(int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) OVERRIDE;

private:
    bool ParseDanmuMsg(scoped_ptr<base::DictionaryValue> &local_danmaku);
    bool ParseNoticeMsg(scoped_ptr<base::DictionaryValue> &local_danmaku);
    bool ParseSendGift(scoped_ptr<base::DictionaryValue> &local_danmaku);
    bool ParseWarning(scoped_ptr<base::DictionaryValue> &local_danmaku);
    bool ParseCutoff(scoped_ptr<base::DictionaryValue> &local_danmaku);

    bool ParseAnchorNormalNotify(scoped_ptr<base::DictionaryValue>& local_danmaku);

    bool ParseLotCheckStatus(scoped_ptr<base::DictionaryValue>& local_danmaku);
    bool ParseLotEnd(scoped_ptr<base::DictionaryValue>& local_danmaku);
    bool ParseLotAward(scoped_ptr<base::DictionaryValue>& local_danmaku);

    bool ParseAnchorPromotionUpdate(scoped_ptr<base::DictionaryValue>& local_danmaku);

    void FillCommonNoticeDanmakuInfo(scoped_ptr<base::DictionaryValue>& local_danmaku, CommonNoticeDanmakuInfo &common_notice_danmaku);

    std::shared_ptr<CommonNoticeTextSegment> ParseLabelTextContentSegment(base::DictionaryValue* js_dict);
    std::shared_ptr<CommonNoticeImageSegment> ParseImageContentSegment(base::DictionaryValue* js_dict);
    std::shared_ptr<CommonNoticeButtonSegment> ParseButtonContentSegment(base::DictionaryValue* js_dict);

    static void RecordDanmakuInfoToDB(const DanmakuInfo &danmaku);

private:
    base::NotificationRegistrar registrar_;
    static int64_t audience_;
    static int64_t attention_;

    bool invoke_stop_listening_;

    livehime::DanmakuStatistics statistics_;
    //std::unique_ptr<DanmakuBroadcastPresenter> presenter_;

    ObserverList<BililiveBroadcastObserver> observer_list_;

    DISALLOW_COPY_AND_ASSIGN(BililiveBroadcastService);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_DANMAKU_SERVICE_H_