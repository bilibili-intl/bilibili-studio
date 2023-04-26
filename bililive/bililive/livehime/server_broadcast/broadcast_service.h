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

// Ӧ�� ENTRY_EFFECT ����Ӧ�ֶζ���һ��
// 1�󺽺�,2�,3���ܰ�,4��ҫװ��
enum EntryBusinessType {
    ENTRY_BUSINESS_FLEET = 1,  // �󺽺�����
    ENTRY_BUSINESS_HER = 3,    // ���ܰ����
};

enum DanmakuType
{
    DT_NOSUPPORT = -1,
    DT_DANMU_MSG,    //��ͨ��Ϣ��Ļ(����������Ļ,voice_url��Ϊ��)
    DT_SYS_MSG,
    DT_NOTICE_MSG,
    DT_SEND_GIFT,
    DT_WARNING,
    DT_CUT_OFF,
    DT_NORMALNOTIFY,

    //ֱ��С����
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

// Ӧ�뵯ĻЭ���е���Ӧ�ֶζ�����ͬ
enum class DanmakuSpecialType {
    NORMAL = 0,       // ��ͨ��Ļ
    RHYTHM_STORM = 1, // ����籩��Ļ
    TV = 2            // С���ӵ�Ļ
};

// Ӧ�� USER_TOAST_MSG �е���Ӧ�ֶζ�����ͬ
enum class UTMOpType {
    Open = 1,      // ��ͨ
    Renew = 2,     // ����
    AutoRenew = 3, // �Զ�����
};

// Ӧ�� INTERACT_WORD �е���Ӧ�ֶζ�����ͬ
enum class InteractIdentity {
    Normal = 1,      // ��ͨ��
    Manager = 2,     // ����
    Fan = 3,         // ��ͨ��˿
    MonthlyLord = 4, // �·���ү
    YearlyLord = 5,  // �����ү
    Captain = 6,     // ����
    Admiral = 7,     // �ᶽ
    Governor = 8,    // �ܶ�
};

// �󺽺��ȼ�
enum class GuardLevel {
    None = 0,
    Governor = 1,   // ����
    Admiral = 2,    // �ᶽ
    Captain = 3     // �ܶ�
};

// Ӧ�� INTERACT_WORD �е���Ӧ�ֶζ�����ͬ
enum class InteractType {
    Enter = 1,            // ����
    Attention = 2,        // ��ע
    Share = 3,            // ����
    SpecialAttention = 4, // �ر��ע
    FanEachOther = 5,     // ����
    UserLike = 6,         // ����
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
    int64_t rnd = 0;    //���͵�Ļ��ʱ���
    std::string uname;
    std::string face_url;
    std::string danmaku;
    int manager = 0;
    int vip = 0;
    int svip = 0;
    int barrage = 0;
    int id = 0;
    int special_type = 0;

    //�Ƿ����
    bool is_naming = false;

    // �󺽺�����/���ܰ�
    int priority;  //�����Ȩֵ,ԽС���ȼ�Խ��
    std::string web_basemap_url;  //��̬ͼ(����)
    double web_effective_time;     //��̬ͼ��ʾʱ��
    std::string web_dynamic_url_webp; //��̬ͼ(��ѡ)
    double effective_time_new;     // ��̬ͼ��ʾʱ��(����0 ѡ��̬ͼ)

    // ע������� bg_color ����������ɫ��
    // ��һ����ɫ��Ӧ��ɫ���⣬�ڶ�����ɫ��Ӧ��ɫ����
    std::string bg_color;

    int category = 0;
    int pop_type = 0;
    std::string pop_title;
    std::string pop_msg;
    std::string pop_button_text;
    std::string pop_button_url;

    // ��ͨ/���Ѵ󺽺�
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
    //����
    int64_t face_effect_id = 0;
    int64_t face_effect_type = 0;
    int demarcation = 0;
    int64_t float_sc_resource_id = 0;

    // �����˵Ĵ󺽺��ȼ�, ���������Ż���ֵ
    // 1:�ܶ� 2:�ᶽ 3:����
    GuardLevel guard_level = GuardLevel::None;

    // ä�������������
    bool is_blind_gift = false;
    std::string original_gift_name;
    std::string blind_action;

    // ä�������Ļ��
    bool is_combo_blind_gift = false;
    std::string combo_original_gift_name;
    std::string combo_blind_action;

    int64_t start_time = 0;
    int64_t end_time = 0;
    bool has_combo_send = false;

    // һ�������Ľ�������������ۿۣ���
    // ��ǰ��������ʷ��Ļ��
    int64_t batch_total_coin = 0;

    // ����������������Ӧ�Ľ�������������ۿۣ���
    // �� batch_total_coin �Ĳ�֮ͬ������ total_coin ����������
    int64_t total_coin = 0;

    //�������ﵥ�ۣ����ۿ�
    int64_t discount_price = 0;
    int64_t price = 0;   //����ʵ�ʼ�ֵ

    GiftCoinType gift_coin_type = GCT_SILVER;

    // ����/ǧ�˽��ӳɾ�ʹ��
    std::string frame_url;
    std::string danmaku_bg_url;
    std::string dialog_bg_url;
    int ga_event_type = 0;
    int achievement_level = 0;
    int show_time = 0;            // ������չʾʱ��
    bool is_first_new = false;    // �Ƿ����״δ��
    std::string first_line_text;  // ��һ���ı����и�������<%...%>
    std::string second_line_text; // �ڶ����ı����������ʽ
    std::string highlight_color;  // ��һ���ı��������ֵ���ɫ

    // INTERACT_WORD
    std::vector<int> interact_identities;
    int interact_type = 0;
    int64_t interact_ts = 0;
    int64_t interact_score = 0;
    bool is_spread = false;
    std::string spread_info;
    std::string spread_desc;
    int tail_icon = 0;
    int core_user_tag_id = 0;//�Ƿ���ĸ����û�:0:�ޱ�ǩ,1:���ܰ�,2:�󺽺�,3:��˿��,4:�߻�Ծ,5:����

    // ���ܰ�����
    int64_t he_rank = 0;

    // ���Ե�Ļͷ�� id
    std::string title_id;

    //������Ļ
    std::string voice_url;
    std::string file_format;
    int file_duration = 0;
    string16 text;  //�����ı�

    // ENTRY_EFFECT ʹ��
    int business_type = 0;
    std::vector<int> icon_list;

    //���鵯Ļʹ��
    bool is_emoji_danmaku = false;
    EmojiDanmakuInfo emoji_danmaku_info;

    //���ֵ�Ļ�����emoji
    //��ʽ: 1111[ǰ������]22[�����]3333
    std::vector<EmojiDanmakuInfo> emoji_list;

    //���ķ��������������
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
    int32_t type;                           // 1:����ǩ�ı���2��ͼƬ��3����ť
    std::string text;                       // <%%> ������ɫ������<$$>����Ӵ�, <%<$$>%> ������ɫ�����ҼӴ�
    SkColor font_color;                     // ������ɫ
    SkColor hightlight_font_color;          //����������ɫ
    SkColor font_color_dark;                //����������ɫ
    SkColor highlight_font_color_dark;      //��������ҹ��ģʽ��ɫ
};

struct CommonNoticeImageSegment 
{
    int32_t type;
    std::string img_url;                     //ͼƬ��ַ
    std::string img_url_dark;               //ҹ��ģʽͼƬ��ַ
    std::string imgae_data;
    std::string imgae_data_dark;
    int32_t img_width;
    int32_t img_height;
};

struct CommonNoticeButtonSegment 
{
    int32_t type;
    std::vector<SkColor> background_color;              // ��ť������ɫ
    std::vector<SkColor> background_color_dark;         // ��ť����ҹ��ģʽ��ɫ
    SkColor font_color_dark;
    SkColor font_color;
    bool font_bold;                                     // �Ƿ�Ӵ�
    std::string text;
    std::string uri;                                    // ��ť��תuri ����bilibili://live/xxxx��https://xxx.xxx.com.xxx.xxx
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

    int32_t terminals;                                          // Ͷ���ն�,1:�۰�ֱ���� 2: blink 3: �۰沥�� 4: web  5: pc_link
    struct
    {
        std::vector<SkColor> background_color;                  // ��Ļ������ɫ
        std::vector<SkColor> background_color_dark;             // ��Ļ����ҹ��ģʽ��ɫ
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