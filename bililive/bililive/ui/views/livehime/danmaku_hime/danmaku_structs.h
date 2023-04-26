#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_STRUCTS_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_STRUCTS_H_

#include <atomic>

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmakuhime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"

// 道具配置后台:
// @ http://info.bilibili.co/pages/viewpage.action?pageId=8477304


enum class DanmakuFlags {
    SELF,
    SVIP,
    VIP,
    MANAGER
};

enum class DanmakuViewType {
    NONE,
    DANMAKU,
    ANTI_BRUSHING,
    GIFT_SUPER_CHAT,
    COMBO_EFFECT,
};

enum class SuperChatStatus {
    LIVING,
    REPORTED,
    EXPIRED,
    REMOVED,
};

enum class DmkSysMsgType {
    NORMAL,
    MELEE,
    HOT_RANK,
};


// 礼物类型定义与服务端保持一致
enum class GiftType {
    NORMAL = 0,
    COMMUNITY = 1,
    ULTIMATE = 2,
    EVENT = 3,
};

// 直播小助手button类型
enum class AsistantButtonType {
    INLINE_H5 = 0,
    JUMP_WEB = 1,
    NATIVE = 2
};

enum class AssistantNativeType {
    OPEN_VOICE_LINK = 1000,
    OPEN_SHARE = 2000,
    OPEN_LIVE_NOTICE = 3000,
    OPEN_FACE_SOURCE = 4000,
    OPEN_AUDICE = 9000,
};

struct LiveAsistantData {

    base::string16 asit_title;
    base::string16 asit_msg;
    bool asit_has_button = false;
    base::string16 asit_button_name;
    AsistantButtonType button_type;
    bool has_guide_icon_ = false;
    int button_h5_type = 0;
    std::string button_target;
    std::string button_extra;
    std::string url;

    std::string report_type;
    std::string together_uid;
    std::string milestone_type;
    int  milestone_index;
    int  milestone_value;

    //海外版警告消息图标
    std::string warning_icon;
};

struct DanmakuData {
    using Flags = std::vector<std::pair<DanmakuFlags, string16>>;

    // 用于 SEND_GIFT
    struct Blind {
        // 指出是否为盲盒样式
        bool is_blind_gift = false;
        // 盲盒样式时，爆出的礼物动作，“爆出”
        string16 blind_action;
        // 盲盒样式时，原始礼物名
        string16 org_gift_name;
    };

    DanmakuType type;

    // 在 UI 层生成的用户身份标识，
    // 为 "主/爷/总督/提督/舰长/房管" 中的零个或多个组合
    Flags flags;

    // 在 UI 层生成的用户头衔
    string16 title;

    // 主礼物动作，“投喂”
    string16 action;

    //大航海特效ID
    int64_t  effect_id = 0;
    // 大航海个数单位
    string16 guard_unit;

    std::string uname_color;

    // 注意：对于普通弹幕，这里的 bg_color 里有三个颜色！
    // 第一个颜色对应白色主题，第二个颜色对应黑色主题
    std::string bg_color;

    // 醒目留言相关
    std::string top_color;
    std::string bottom_color;
    int64_t schat_id = 0;
    int64_t schat_create_time = 0;
    string16 schat_remaining_time;
    int schat_status = 0;

    int64_t user_id;
    string16 user_name;
    // 礼物名，对于盲盒玩法，为爆出的礼物名
    string16 gift_name;
    string16 plain_text;
    // 用于判断是否为抽奖弹幕
    int special_type;

    int guard_type;
    int64_t gift_id;

    // 批量数，即点一次送礼按钮时送出的礼物数量。
    // 连击过程中不使用该字段。
    int64_t gift_num;

    /**
     * 与送礼总数对应的总价（带折扣）。
     * 计算方法（服务端计算）：gift_batch_cost * 当前连击数
     */
    int64_t gift_cost;

    /**
    * 与送礼总数对应的单价（带折扣）。
    *
    */
    int64_t discount_price;

    /**
     * 仅一次批量的总价，不考虑连击数（带折扣）。
     * 计算方法（服务端计算）：gift_num * 支付单价
     */
    int64_t gift_batch_cost = 0;

    //动画队列优先级 1,2低价，3高价
    int demarcation = 0;

    //飘屏资源 ID
    int64_t float_sc_resource_id = 0;

    // 人民币价格
    int64_t gift_price;
    int64_t gift_stamp;
    // 批量数，用于在弹幕区显示连击弹幕
    int64_t gift_batch_num;
    // 批量连击数，用于在弹幕区显示连击弹幕
    int64_t gift_combo_num;
    // 连击条尾部显示的连击数
    int64_t gift_super_num;
    // 批量连击字符串(海外版礼物弹幕使用)
    string16 gift_combo_string;
    // 用来指明 SEND_GIFT 中是否有连击字段，用于将 SEND_GIFT
    // 转化为弹幕区的连击弹幕
    bool has_combo_send;
    // 用于连击条的停留时间
    int combo_stay_time = 0;
    // 用于连击条的渐变色
    int combo_res_id = 0;
    // 用于连击条文案变化
    bool is_special_batch = false;
    // 一次连击的唯一标识
    std::string combo_id;
    // 连击时是否冠名
    bool is_naming = false;

    // 连击条盲盒
    Blind blind_info;
    // 礼物弹幕盲盒
    Blind combo_blind_info;

    // 用于 INTERACT_WORD
    std::vector<int> interact_identities;
    int interact_type = 0;
    int64_t interact_ts = 0;
    int64_t interact_score = 0;
    bool is_spread = false;
    string16 spread_text;
    SkColor spread_color_start = 0;
    SkColor spread_color_end = 0;
    bool is_hot = false;
    bool is_popularity = false;
    int core_user_tag_id = 0;//是否核心付费用户:0:无标签,1:超能榜,2:大航海,3:粉丝团,4:高活跃,5:高能

    GiftCoinType gift_coin_type;

    DmkSysMsgType sys_msg_type = DmkSysMsgType::NORMAL;

    std::string face_url;
    std::string frame_url;
    std::string background_url;

    // DTC_GENERIC_DANMAKU
    std::string img_data;
    std::string img_url;
    bool use_default_color;
    SkColor text_color;

    // 高能榜排名
    int64_t he_rank = 0;

    // 发言弹幕的头衔 id（一张长图）
    std::string title_id;

    // ENTRY_EFFECT 业务类型
    int business_type = 0;
    std::vector<int> icon_list;
    int priority;  //排序的权值,越小优先级越高
    std::string web_basemap_url;  //静态图(兜底)
    double web_effective_time;     //静态图显示时常
    std::string web_dynamic_url_webp; //动态图(首选)
    double effective_time_new;     // 动态图显示时常(大于0 选择动态图)

    //通用通知弹幕
    CommonNoticeDanmakuInfo common_notice_danmaku_data;

    //自定义表情弹幕
    bool is_emoji_danmaku = false;
    EmojiDanmakuInfo emoji_danmaku_info;

    //文字弹幕中添加emoji
    //形式: 1111[前方高能]22[你真棒]3333
    std::vector<EmojiDanmakuInfo> emoji_list;

    //语音弹幕
    bool is_play = false;
    std::string voice_url;
    std::string file_format;
    int file_duration = 0;
    string16 text;  //语音文本

    //语聊房送礼添加送礼人
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;

    //直播小助手
    LiveAsistantData asistant_data;

    //海外版管理员、排行标识
    bool user_identity_tag_admin_ = false;
    int user_identity_tag_top_ = false;

    //海外版是否限制最大字符数标识
    bool limit_max_display = false;

    bool warning_msg = false;
    bool popup_msg = false;
    bool block_msg = false;

    // 禁播信息
    std::string notice;
    int64 end_time;

    DanmakuData();
};

namespace dmkhime {

enum DmkRendererType {
    DRT_COMMON,
    DRT_GIFT,
    DRT_DETAILED_GIFT,
    DRT_COMBO,
    DRT_GUARD_BUY,
    DRT_DETAILED_GUARD_BUY,
    DRT_GUARD_ACHIEVEMENT,
    DRT_INTERACTIVE,
    DRT_ANNOC,
    DRT_USER_GAIN_MEDAL,
    DRT_GENERIC,
    DRT_SUPER_CHAT,
    DRT_DETAILED_SUPER_CHAT,
    DRT_HE_TOP3,
    DRT_TIPS,
    DRT_NEW_PK_MVP,

    DRT_EFFECT_COMBO,
    DRT_EFFECT_GUARD_BUY,
    DRT_EFFECT_GUARD_ENTER,

    DRT_COMMON_NOTICE_DANMAKU,

    DRT_ANCHOR_BROADCAST,
    DRT_RED_PACKET,
    DRT_VOICE_LINK_NOTICE   //连麦邀请弹幕通知，暂时没用，改成直播小助手进行通知
};

enum class DmkHitType {
    None,
    Danmaku,
    SuperChat,
    CommonNoticeDanmakuBtn,
    LiveAsistantDanmakuBtn,
    VoiceLinkNoticeDanmakuBtn
};

enum class DmkClickItemType {
    None,
    VoiceImg
};


class DmkDataObject {
public:
    DmkDataObject();

    virtual ~DmkDataObject() = default;

    virtual DmkRendererType GetType() const = 0;

    uint64_t GetId() const;

private:
    static std::atomic_uint64_t id_counter;
    uint64_t identifier_;
};

class CommonDmkData : public DmkDataObject {
public:
    using Flags = std::vector<std::pair<DanmakuFlags, string16>>;

    CommonDmkData();

    DmkRendererType GetType() const override { return DRT_COMMON; }

    // 在 UI 层生成的用户身份标识，
    // 为 "主/爷/总督/提督/舰长/房管" 中的零个或多个组合
    Flags flags;

    string16 user_name;
    string16 plain_text;
    std::string uname_color;
    std::string bg_color;

    int64_t rank = 0;
    std::string title_id;

    //语音弹幕
    bool is_click_voice = false;
    std::string voice_url;
    std::string file_format;
    int file_duration = 0;
    string16 text;  //语音文本

    int guard_type;

    bool is_emoji_danmaku = false;
    std::string emoji_image_data;
    int32_t emoji_width;
    int32_t emoji_height;

    //点赞所需数据
    bool is_like_danmaku = false;
    std::string like_icon_data;

    std::vector<EmojiDanmakuInfo> emoji_list;

    //海外版管理员、排行标识
    std::string user_identity_tag_admin_;
    std::string user_identity_tag_top_;
};

class GiftDmkData : public DmkDataObject {
public:
    GiftDmkData();

    DmkRendererType GetType() const override { return DRT_GIFT; }

    string16 action;

    string16 user_name;
    string16 gift_name;

    int64_t gift_id;
    int64_t gift_num;
    GiftCoinType gift_coin_type;

    // 生成 Renderer 时下载礼物图标
    std::string gift_icon_data;

    //语聊房送礼添加送礼人
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;
};

class RedPacketDmkData :public DmkDataObject {
public:
    RedPacketDmkData() {}
    DmkRendererType GetType() const override { return DRT_RED_PACKET; }

    int64_t gift_id = 0;
    int64_t gift_num = 0;

    string16 action;
    string16 user_name;
    string16 gift_name;
    std::string uname_color;
};

class DetailedGiftDmkData : public DmkDataObject {
public:
    DetailedGiftDmkData();

    DmkRendererType GetType() const override { return DRT_DETAILED_GIFT; }

    string16 action;
    string16 user_name;
    string16 gift_name;

    int guard_type;
    int64_t gift_id;
    int64_t gift_cost;
    int64_t gift_num;
    int64_t gift_stamp;
    GiftCoinType gift_coin_type;
    std::string uname_color;

    bool is_super_chat = false;

    // 生成 Renderer 时下载礼物图标
    std::string gift_icon_data;
};

class GiftComboDmkData : public DmkDataObject {
public:
    GiftComboDmkData();

    DmkRendererType GetType() const override { return DRT_COMBO; }

    string16 action;
    string16 user_name;
    string16 gift_name;
    std::string uname_color;

    bool is_last_one = false;
    int total_count = 0;

    int64_t gift_id;
    // 批量数，用于在弹幕区显示连击弹幕
    int64_t gift_batch_num = 0;
    // 批量连击数，用于在弹幕区显示连击弹幕
    int64_t gift_combo_num;
    // 批量连击字符串(海外版礼物弹幕使用)
    string16 gift_combo_string;
    GiftCoinType gift_coin_type;

    // 指出是否为盲盒样式
    bool is_blind_gift = false;
    // 盲盒样式时，爆出的礼物动作，“爆出”
    string16 blind_action;
    // 盲盒样式时，原始礼物名
    string16 org_gift_name;

    // 初次添加时下载的礼物图标
    std::string gift_icon_data;
    //是否冠名
    bool is_naming = false;

    //语聊房送礼添加送礼人
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;
};

class GuardBuyDmkData : public DmkDataObject {
public:
    GuardBuyDmkData();

    DmkRendererType GetType() const override { return DRT_GUARD_BUY; }

    // 在 UI 层生成的用户头衔
    string16 title;

    string16 action;
    string16 user_name;

    int guard_type;
    int64_t gift_num;

    string16 guard_unit;
    std::string uname_color;
};

class DetailedGuardBuyDmkData : public DmkDataObject {
public:
    DetailedGuardBuyDmkData();

    DmkRendererType GetType() const override { return DRT_DETAILED_GUARD_BUY; }

    // 在 UI 层生成的用户头衔
    string16 title;

    string16 action;
    string16 user_name;

    int guard_type;
    int64_t gift_num;
    int64_t gift_cost;
    int64_t gift_stamp;

    string16 guard_unit;
    std::string uname_color;
};

class GuardAchievementDmkData : public DmkDataObject {
public:
    GuardAchievementDmkData();

    DmkRendererType GetType() const override { return DRT_GUARD_ACHIEVEMENT; }

    std::string face_url;
    std::string frame_url;
    std::string background_url;

    // 生成 Renderer 时下载图标
    std::string face_img_data;
    // 生成 Renderer 时下载图标
    std::string frame_img_data;
    // 生成 Renderer 时下载图标
    std::string bg_img_data;
};

class InteractiveDmkData : public DmkDataObject {
public:
    InteractiveDmkData() = default;

    DmkRendererType GetType() const override { return DRT_INTERACTIVE; }

    // 在 UI 层生成的用户头衔
    string16 title;
    int action_type = 0;
    std::vector<int> identities;

    string16 action;
    string16 user_name;
    std::string uname_color;

    int64_t rank = 0;

    // 推广相关数据
    bool is_spread = false;
    string16 spread_text;
    SkColor spread_color_start = 0;
    SkColor spread_color_end = 0;

    // 热门榜相关
    bool is_hot = false;

    //人气相关
    bool is_popularity = false;

    //点赞相关
    bool is_like = false;
    std::string like_icon_data;

    //核心付费人群显示二期相关
    int core_user_tag_id = 0;//是否核心付费用户:0:无标签,1:超能榜,2:大航海,3:粉丝团,4:高活跃,5:高能
    std::shared_ptr<std::string> tag_white_bg_img_data;//白色背景标签图片数据
    std::shared_ptr<std::string> tag_dark_bg_img_data;//深色背景标签图片数据
};

class AnnocDmkData : public DmkDataObject {
public:
    AnnocDmkData();

    DmkRendererType GetType() const override { return DRT_ANNOC; }

    bool limit_max_display; //是否限制最大字符数
    string16 user_name; //海外弹幕添加
    string16 plain_text;
    DmkSysMsgType type = DmkSysMsgType::NORMAL;

    bool popup_msg = false;
    bool block_msg = false;
};

class UserGainMedalDmkData : public DmkDataObject {
public:
    UserGainMedalDmkData() = default;

    DmkRendererType GetType() const override { return DRT_USER_GAIN_MEDAL; }

    string16 user_name;
};

class GenericDmkData : public DmkDataObject {
public:
    GenericDmkData() = default;

    DmkRendererType GetType() const override { return DRT_GENERIC; }

    string16 plain_text;
    std::string img_url_;
    std::string img_data_;

    bool use_default_color;
    SkColor text_color;
    bool use_background_color;
};

class SuperChatDmkData : public DmkDataObject {
public:
    SuperChatDmkData() = default;

    DmkRendererType GetType() const override { return DRT_SUPER_CHAT; }

    string16 uname;
    std::string uname_color;
    int64_t price = 0;
    string16 content_text;
    std::string top_color;
    std::string bottom_color;

    // 初次添加时下载的角标
    std::string corner_icon_data;
};

class DetailedSuperChatDmkData : public DmkDataObject {
public:
    DetailedSuperChatDmkData() = default;

    DmkRendererType GetType() const override { return DRT_DETAILED_SUPER_CHAT; }

    int64_t id = 0;
    int64_t uid = 0;
    string16 uname;
    std::string uname_color;
    int guard_type = 0;
    int64_t price = 0;
    string16 content_text;
    string16 remaining_time;
    int64_t create_time = 0;
    int status = 0;
};

class HighEnergyRankDmkData : public DmkDataObject {
public:
    HighEnergyRankDmkData() = default;

    DmkRendererType GetType() const override { return DRT_HE_TOP3; }

    int64_t rank = 0;
    string16 content_text;
};

class TipsDmkData : public DmkDataObject {
public:
    TipsDmkData() = default;

    DmkRendererType GetType() const override { return DRT_TIPS; }

    int parent_height = 0;
    string16 tips_text;
};

class NewPkMVPDmkData : public DmkDataObject {
public:
    NewPkMVPDmkData() = default;

    DmkRendererType GetType() const override { return DRT_NEW_PK_MVP; }

    string16 uname;
    std::string hl_color;
    std::string bg_color;
};

class ComboStripDmkData : public DmkDataObject {
public:
    ComboStripDmkData();

    DmkRendererType GetType() const override { return DRT_EFFECT_COMBO; }

    int guard_type;
    string16 guard_title;
    string16 user_name;
    std::string face_url;
    int64_t user_id;
    string16 action;
    string16 gift_name;
    int64_t gift_id;
    int64_t gift_number = 0;
    int64_t combo_number;
    int64_t total_price; // 总价
    int64_t discount_price; //单价（带折扣）
    int64_t timestamp;   // 时间戳
    bool is_special_batch = false;
    std::string combo_id;
    bool is_naming = false;

    int stay_time;
    SkColor start_color;
    SkColor end_color;

    // 指出是否为盲盒样式
    bool is_blind_gift = false;
    // 盲盒样式时，爆出的礼物动作，“爆出”
    string16 blind_action;
    // 盲盒样式时，原始礼物名
    string16 org_gift_name;

    // 第一次绘制时下载礼物图标
    std::string gift_icon_data;

    // 第一次绘制下载礼物gif动图
    std::string gift_gif_data;

    //语聊房送礼添加送礼人
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;
};

class GuardBuyBannerDmkData : public DmkDataObject {
public:
    GuardBuyBannerDmkData();

    DmkRendererType GetType() const override { return DRT_EFFECT_GUARD_BUY; }

    string16 user_name;
    string16 action;
    int guard_type;
    string16 guard_title;
    string16 guard_unit;
    int64_t guard_time;

    int stay_time;
};

class GuardEnterBannerDmkData : public DmkDataObject {
public:
    GuardEnterBannerDmkData();

    DmkRendererType GetType() const override { return DRT_EFFECT_GUARD_ENTER; }

    int guard_type;
    std::string uname_color;
    string16 plain_text;
    std::string face_url;
    int64_t stay_time;

    int business_type = 0;
    std::vector<int> icon_list;

    int priority;  //排序的权值,越小优先级越高
    std::string web_basemap_url;  //静态图(兜底)
    double web_effective_time;     //静态图显示时常
    std::string web_dynamic_url_webp; //动态图(首选)
    double effective_time_new;     // 动态图显示时常(大于0 选择动态图)
};

class CommonNoticeDanmakuRenderData : public DmkDataObject{
public:
    CommonNoticeDanmakuRenderData();

    DmkRendererType GetType() const override { return DRT_COMMON_NOTICE_DANMAKU; }

    CommonNoticeDanmakuInfo common_notice_danmaku_data;

};

class LiveAsistantDanmakuRenderData : public DmkDataObject {
public:
    LiveAsistantDanmakuRenderData();

    DmkRendererType GetType() const override { return DRT_ANCHOR_BROADCAST; }

    LiveAsistantData live_asistant_data;
};

class VoiceLinkDmkRenderData : public DmkDataObject {
public:
    VoiceLinkDmkRenderData() {};

	DmkRendererType GetType() const override { return DRT_VOICE_LINK_NOTICE; }

    enum class ApplyType
    {
        ApplyFor,//申请
        CancelApply//取消申请
    };
    ApplyType apply_type;
	int64_t user_id;
	string16 user_name;
};

}


struct DanmakuRenderingParams {
    int opacity;
    int base_font_size;
    dmkhime::Theme theme;
    bool show_icon;
    bool hardware_acceleration_;

    enum Flags {
        Flag_Opacity =      1,
        Flag_BaseFontSize = 1 << 1,
        Flag_Theme =        1 << 2,
        Flag_ShowIcon =     1 << 3,
        Flag_HW =           1 << 4
    };

    uint32_t flags = 0;

    DanmakuRenderingParams();
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_STRUCTS_H_