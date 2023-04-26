#ifndef BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_SERVICE_H_
#define BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_SERVICE_H_

#include "base/prefs/pref_registry_simple.h"


namespace prefs {

    enum DanmakuHimeEnterFilter {
        // 显示全部用户进场
        DHEF_SHOW_ALL,
        // 仅显示佩戴指定等级及以上粉丝勋章用户进场
        DHEF_ONLY_MEDAL_LEVEL,
        // 仅显示佩戴本房间粉丝勋章用户的进场
        DHEF_ONLY_SELF_MEDAL,
    };

    extern const char kDanmakuHimeGiftTrans[];
    extern const char kDanmakuHimeInteractionTrans[];
    extern const char kActivityTaskTrans[];
    extern const char kDanmakuHimeGiftFontSize[];
    extern const char kDanmakuHimeInteractionFontSize[];
    extern const char kActivityTaskFontSize[];
    //extern const char kDanmakuHimeShow[];
    extern const char kDanmakuHimeGiftTheme[];
    extern const char kDanmakuHimeInteractionTheme[];
    extern const char kActivityTaskTheme[];
    extern const char kDanmakuHimeEnableSuspending[];
    extern const char kDanmakuHimeEnableGiftFilter[];
    extern const char kDanmakuHimeFluency[];
    extern const char kDanmakuHimeShowGiftIcon[];
    extern const char kDanmakuHimeEnableHWA[];
    extern const char kDanmakuHimeShowChargedGifts[];
    extern const char kDanmakuHimeEnableGiftEffects[];
    extern const char kDanmakuHimeEnableChargedGiftThresold[];
    extern const char kDanmakuHimeChargedGiftThresold[];
    extern const char kDanmakuHimeEnableTTS[];
    extern const char kDanmakuHimeTTSSource[];
    extern const char kDanmakuHimeTTSSpeed[];
    extern const char kDanmakuHimeTTSVolume[];
    extern const char kDanmakuHimeLockGuide[];
    extern const char kDanmakuHimeBlockEnter[];
    extern const char kDanmakuHimeEnterFilter[];
    extern const char kDanmakuHimeEnterMedalLevel[];
    extern const char kDanmakuHimeShowUserEnterEffect[];
    extern const char kDanmakuHimeShowCoreUserArea[];
    extern const char kDanmakuHimeShowFaceEffect[];
    extern const char kDanmakuHimeBlockLottery[];
    extern const char kDanmakuHimeGiftViewShowMode[];
    extern const char kDanmakuHimeGiftViewPosX[];
    extern const char kDanmakuHimeGiftViewPosY[];
    extern const char kDanmakuHimeGiftViewWidth[];
    extern const char kDanmakuHimeGiftViewHeight[];
    extern const char kDanmakuHimeGiftViewPin[];
    extern const char kDanmakuHimeInteractionViewShowMode[];
    extern const char kDanmakuHimeInteractionViewPosX[];
    extern const char kDanmakuHimeInteractionViewPosY[];
    extern const char kDanmakuHimeInteractionViewWidth[];
    extern const char kDanmakuHimeInteractionViewHeight[];
    extern const char kDanmakuHimeInteractionViewPin[];
    extern const char kDanmakuHimeInteractionViewFirstShow[];

    extern const char kDanmakuHimeBlockAllEffect[];
    extern const char kDanmakuHimeBlockAllCombo[];
    extern const char kDanmakuHimeBlockAllGift[];
    extern const char kDanmakuHimeBlockAllNavy[];
    extern const char kDanmakuHimeFilterCombo[];
    extern const char kDanmakuHimeFilterGift[];
    extern const char kDanmakuHimeFilterNavy[];

    extern const char kActivityTaskViewShowMode[];
    extern const char kActivityTaskViewPosX[];
    extern const char kActivityTaskViewPosY[] ;
    extern const char kActivityTaskViewWidth[] ;
    extern const char kActivityTaskViewHeight[];
    extern const char kActivityTaskViewPin[];
    extern const char kDanmakuHimeFirstPop[];

    extern const char kDanmakuHimeVoiceBroadcastFirst[];
    extern const char kDanmakuHimeVoiceBroadcastSwitch[];
    extern const char kDanmakuHimeVoiceBroadcastDanmaku[];
    extern const char kDanmakuHimeVoiceBroadcastEnterAttention[];
    extern const char kDanmakuHimeVoiceBroadcastGifFeed[];
    extern const char kDanmakuHimeVoiceBroadcastGuard[];
    extern const char kDanmakuHimeVoiceBroadcastLeaveWords[];
    extern const char kDanmakuHimeVoiceBroadcastVolume[];
    extern const char kDanmakuHimeVoiceBroadcastPitchRate[];
    extern const char kDanmakuHimeVoiceBroadcastSpeechRate[];

    extern const int kDanmakuHimeDefTrans;
    extern const int kDanmakuHimeDefFontSize;
    extern const bool kDanmakuHimeDefShow;
    extern const int kDanmakuHimeDefTheme;
    extern const bool kDanmakuHimeDefEnableSuspending;
    extern const bool kDanmakuHimeDefEnableGiftFilter;
    extern const int kDanmakuHimeDefFluency;
    extern const bool kDanmakuHimeDefShowGiftIcon;
    extern const bool kDanmakuHimeDefEnableHWA;
    extern const bool kDanmakuHimeDefShowChargedGifts;
    extern const bool kDanmakuHimeDefEnableGiftEffects;
    extern const bool kDanmakuHimeDefEnableChargedGiftThresold;
    extern const int kDanmakuHimeDefChargedGiftThresold;
    extern const bool kDanmakuHimeDefEnableTTS;
    extern const char kDanmakuHimeDefTTSSource[];
    extern const int kDanmakuHimeDefTTSSpeed;
    extern const int kDanmakuHimeDefTTSVolume;
    extern const bool kDanmakuHimeDefLockGuide;
    extern const bool kDanmakuHimeDefBlockEnter;
    extern const int kDanmakuHimeDefEnterFilter;
    extern const int kDanmakuHimeDefMinEnterMedalLevel;
    extern const bool kDanmakuHimeDefShowUserEnterEffect;
    extern const bool kDanmakuHimeDefShowCoreUserArea;
    extern const bool kDanmakuHimeDefShowFaceEffect;
    extern const bool kDanmakuHimeDefBlockLottery;
    extern const int kDanmakuHimeDefGiftViewHeight;
    extern const int kDanmakuHimeDefInteractionViewHeight;
}

struct DanmakuHimePreferenceInfo
{
    bool show_gift_view = false;
    bool show_gift_icon = false;
    int font_size_gift = 0;
    int font_size_interaction = 0;
    int font_size_activity_task = 0;

    int fluency = 0;
    int trans_gift = 0;
    int trans_interaction = 0;
    int trasn_activity_task = 0;
    bool enable_suspending = false;
    bool enable_gift_filter = false;
    bool enable_HWA = false;
    bool show_charged_gifts = false;
    bool enable_gift_effects = false;
    bool enable_charged_gift_thresold = false;
    int charged_gift_thresold = false;
    int theme_gift = 0;
    int theme_interaction = 0;
    int theme_activity_task = 0;
    bool support_TTS = false;
    bool enable_TTS = false;
    std::string TTS_source;
    int TTS_speend = 0;
    int TTS_volume = 0;
    bool block_enter = false;
    int enter_filter = 0;
    int min_enter_medal_level = 0;
    bool block_lottery = false;
    int show_mode = 0;
    int gift_view_activate_view = 0;
    int split_gift_view_height = 0;
    int split_interaction_view_height = 0;
    bool first_pop = true;//是否第一次弹出，是则显示引导
    bool show_user_enter_effect = true;
    bool show_face_effect = true;
    bool show_core_user_area = true;//是否显示核心付费用户(大R)轮播区域
};

class DanmakuHimePrefService {
public:
    static void RegisterProfilePrefs(PrefRegistrySimple *registry);

    static bool IsHardwareAccelerationNeeded();

    static DanmakuHimePreferenceInfo LoadPref();
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKU_HIME_SERVICE_H_