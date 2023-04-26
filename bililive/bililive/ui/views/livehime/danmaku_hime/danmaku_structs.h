#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_STRUCTS_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_STRUCTS_H_

#include <atomic>

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmakuhime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"

// �������ú�̨:
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


// �������Ͷ��������˱���һ��
enum class GiftType {
    NORMAL = 0,
    COMMUNITY = 1,
    ULTIMATE = 2,
    EVENT = 3,
};

// ֱ��С����button����
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

    //����澯����Ϣͼ��
    std::string warning_icon;
};

struct DanmakuData {
    using Flags = std::vector<std::pair<DanmakuFlags, string16>>;

    // ���� SEND_GIFT
    struct Blind {
        // ָ���Ƿ�Ϊä����ʽ
        bool is_blind_gift = false;
        // ä����ʽʱ�����������ﶯ������������
        string16 blind_action;
        // ä����ʽʱ��ԭʼ������
        string16 org_gift_name;
    };

    DanmakuType type;

    // �� UI �����ɵ��û���ݱ�ʶ��
    // Ϊ "��/ү/�ܶ�/�ᶽ/����/����" �е�����������
    Flags flags;

    // �� UI �����ɵ��û�ͷ��
    string16 title;

    // �����ﶯ������Ͷι��
    string16 action;

    //�󺽺���ЧID
    int64_t  effect_id = 0;
    // �󺽺�������λ
    string16 guard_unit;

    std::string uname_color;

    // ע�⣺������ͨ��Ļ������� bg_color ����������ɫ��
    // ��һ����ɫ��Ӧ��ɫ���⣬�ڶ�����ɫ��Ӧ��ɫ����
    std::string bg_color;

    // ��Ŀ�������
    std::string top_color;
    std::string bottom_color;
    int64_t schat_id = 0;
    int64_t schat_create_time = 0;
    string16 schat_remaining_time;
    int schat_status = 0;

    int64_t user_id;
    string16 user_name;
    // ������������ä���淨��Ϊ������������
    string16 gift_name;
    string16 plain_text;
    // �����ж��Ƿ�Ϊ�齱��Ļ
    int special_type;

    int guard_type;
    int64_t gift_id;

    // ������������һ������ťʱ�ͳ�������������
    // ���������в�ʹ�ø��ֶΡ�
    int64_t gift_num;

    /**
     * ������������Ӧ���ܼۣ����ۿۣ���
     * ���㷽��������˼��㣩��gift_batch_cost * ��ǰ������
     */
    int64_t gift_cost;

    /**
    * ������������Ӧ�ĵ��ۣ����ۿۣ���
    *
    */
    int64_t discount_price;

    /**
     * ��һ���������ܼۣ������������������ۿۣ���
     * ���㷽��������˼��㣩��gift_num * ֧������
     */
    int64_t gift_batch_cost = 0;

    //�����������ȼ� 1,2�ͼۣ�3�߼�
    int demarcation = 0;

    //Ʈ����Դ ID
    int64_t float_sc_resource_id = 0;

    // ����Ҽ۸�
    int64_t gift_price;
    int64_t gift_stamp;
    // �������������ڵ�Ļ����ʾ������Ļ
    int64_t gift_batch_num;
    // �����������������ڵ�Ļ����ʾ������Ļ
    int64_t gift_combo_num;
    // ������β����ʾ��������
    int64_t gift_super_num;
    // ���������ַ���(��������ﵯĻʹ��)
    string16 gift_combo_string;
    // ����ָ�� SEND_GIFT ���Ƿ��������ֶΣ����ڽ� SEND_GIFT
    // ת��Ϊ��Ļ����������Ļ
    bool has_combo_send;
    // ������������ͣ��ʱ��
    int combo_stay_time = 0;
    // �����������Ľ���ɫ
    int combo_res_id = 0;
    // �����������İ��仯
    bool is_special_batch = false;
    // һ��������Ψһ��ʶ
    std::string combo_id;
    // ����ʱ�Ƿ����
    bool is_naming = false;

    // ������ä��
    Blind blind_info;
    // ���ﵯĻä��
    Blind combo_blind_info;

    // ���� INTERACT_WORD
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
    int core_user_tag_id = 0;//�Ƿ���ĸ����û�:0:�ޱ�ǩ,1:���ܰ�,2:�󺽺�,3:��˿��,4:�߻�Ծ,5:����

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

    // ���ܰ�����
    int64_t he_rank = 0;

    // ���Ե�Ļ��ͷ�� id��һ�ų�ͼ��
    std::string title_id;

    // ENTRY_EFFECT ҵ������
    int business_type = 0;
    std::vector<int> icon_list;
    int priority;  //�����Ȩֵ,ԽС���ȼ�Խ��
    std::string web_basemap_url;  //��̬ͼ(����)
    double web_effective_time;     //��̬ͼ��ʾʱ��
    std::string web_dynamic_url_webp; //��̬ͼ(��ѡ)
    double effective_time_new;     // ��̬ͼ��ʾʱ��(����0 ѡ��̬ͼ)

    //ͨ��֪ͨ��Ļ
    CommonNoticeDanmakuInfo common_notice_danmaku_data;

    //�Զ�����鵯Ļ
    bool is_emoji_danmaku = false;
    EmojiDanmakuInfo emoji_danmaku_info;

    //���ֵ�Ļ�����emoji
    //��ʽ: 1111[ǰ������]22[�����]3333
    std::vector<EmojiDanmakuInfo> emoji_list;

    //������Ļ
    bool is_play = false;
    std::string voice_url;
    std::string file_format;
    int file_duration = 0;
    string16 text;  //�����ı�

    //���ķ��������������
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;

    //ֱ��С����
    LiveAsistantData asistant_data;

    //��������Ա�����б�ʶ
    bool user_identity_tag_admin_ = false;
    int user_identity_tag_top_ = false;

    //������Ƿ���������ַ�����ʶ
    bool limit_max_display = false;

    bool warning_msg = false;
    bool popup_msg = false;
    bool block_msg = false;

    // ������Ϣ
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
    DRT_VOICE_LINK_NOTICE   //�������뵯Ļ֪ͨ����ʱû�ã��ĳ�ֱ��С���ֽ���֪ͨ
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

    // �� UI �����ɵ��û���ݱ�ʶ��
    // Ϊ "��/ү/�ܶ�/�ᶽ/����/����" �е�����������
    Flags flags;

    string16 user_name;
    string16 plain_text;
    std::string uname_color;
    std::string bg_color;

    int64_t rank = 0;
    std::string title_id;

    //������Ļ
    bool is_click_voice = false;
    std::string voice_url;
    std::string file_format;
    int file_duration = 0;
    string16 text;  //�����ı�

    int guard_type;

    bool is_emoji_danmaku = false;
    std::string emoji_image_data;
    int32_t emoji_width;
    int32_t emoji_height;

    //������������
    bool is_like_danmaku = false;
    std::string like_icon_data;

    std::vector<EmojiDanmakuInfo> emoji_list;

    //��������Ա�����б�ʶ
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

    // ���� Renderer ʱ��������ͼ��
    std::string gift_icon_data;

    //���ķ��������������
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

    // ���� Renderer ʱ��������ͼ��
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
    // �������������ڵ�Ļ����ʾ������Ļ
    int64_t gift_batch_num = 0;
    // �����������������ڵ�Ļ����ʾ������Ļ
    int64_t gift_combo_num;
    // ���������ַ���(��������ﵯĻʹ��)
    string16 gift_combo_string;
    GiftCoinType gift_coin_type;

    // ָ���Ƿ�Ϊä����ʽ
    bool is_blind_gift = false;
    // ä����ʽʱ�����������ﶯ������������
    string16 blind_action;
    // ä����ʽʱ��ԭʼ������
    string16 org_gift_name;

    // �������ʱ���ص�����ͼ��
    std::string gift_icon_data;
    //�Ƿ����
    bool is_naming = false;

    //���ķ��������������
    bool is_join_receiver = false;
    ReceiveUserInfo receive_user;
};

class GuardBuyDmkData : public DmkDataObject {
public:
    GuardBuyDmkData();

    DmkRendererType GetType() const override { return DRT_GUARD_BUY; }

    // �� UI �����ɵ��û�ͷ��
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

    // �� UI �����ɵ��û�ͷ��
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

    // ���� Renderer ʱ����ͼ��
    std::string face_img_data;
    // ���� Renderer ʱ����ͼ��
    std::string frame_img_data;
    // ���� Renderer ʱ����ͼ��
    std::string bg_img_data;
};

class InteractiveDmkData : public DmkDataObject {
public:
    InteractiveDmkData() = default;

    DmkRendererType GetType() const override { return DRT_INTERACTIVE; }

    // �� UI �����ɵ��û�ͷ��
    string16 title;
    int action_type = 0;
    std::vector<int> identities;

    string16 action;
    string16 user_name;
    std::string uname_color;

    int64_t rank = 0;

    // �ƹ��������
    bool is_spread = false;
    string16 spread_text;
    SkColor spread_color_start = 0;
    SkColor spread_color_end = 0;

    // ���Ű����
    bool is_hot = false;

    //�������
    bool is_popularity = false;

    //�������
    bool is_like = false;
    std::string like_icon_data;

    //���ĸ�����Ⱥ��ʾ�������
    int core_user_tag_id = 0;//�Ƿ���ĸ����û�:0:�ޱ�ǩ,1:���ܰ�,2:�󺽺�,3:��˿��,4:�߻�Ծ,5:����
    std::shared_ptr<std::string> tag_white_bg_img_data;//��ɫ������ǩͼƬ����
    std::shared_ptr<std::string> tag_dark_bg_img_data;//��ɫ������ǩͼƬ����
};

class AnnocDmkData : public DmkDataObject {
public:
    AnnocDmkData();

    DmkRendererType GetType() const override { return DRT_ANNOC; }

    bool limit_max_display; //�Ƿ���������ַ���
    string16 user_name; //���ⵯĻ���
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

    // �������ʱ���صĽǱ�
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
    int64_t total_price; // �ܼ�
    int64_t discount_price; //���ۣ����ۿۣ�
    int64_t timestamp;   // ʱ���
    bool is_special_batch = false;
    std::string combo_id;
    bool is_naming = false;

    int stay_time;
    SkColor start_color;
    SkColor end_color;

    // ָ���Ƿ�Ϊä����ʽ
    bool is_blind_gift = false;
    // ä����ʽʱ�����������ﶯ������������
    string16 blind_action;
    // ä����ʽʱ��ԭʼ������
    string16 org_gift_name;

    // ��һ�λ���ʱ��������ͼ��
    std::string gift_icon_data;

    // ��һ�λ�����������gif��ͼ
    std::string gift_gif_data;

    //���ķ��������������
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

    int priority;  //�����Ȩֵ,ԽС���ȼ�Խ��
    std::string web_basemap_url;  //��̬ͼ(����)
    double web_effective_time;     //��̬ͼ��ʾʱ��
    std::string web_dynamic_url_webp; //��̬ͼ(��ѡ)
    double effective_time_new;     // ��̬ͼ��ʾʱ��(����0 ѡ��̬ͼ)
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
        ApplyFor,//����
        CancelApply//ȡ������
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