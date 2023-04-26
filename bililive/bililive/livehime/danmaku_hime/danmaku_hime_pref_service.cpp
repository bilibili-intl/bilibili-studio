#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"

#include "base/prefs/pref_service.h"
#include "base/win/windows_version.h"

#include "bililive/common/bililive_features.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/public/bililive/bililive_process.h"


namespace prefs {
    const char kDanmakuHimeGiftTrans[] = "danmaku.show_gift_trans";
    const char kDanmakuHimeInteractionTrans[] = "danmaku.show_interaction_trans";
    const char kActivityTaskTrans[] = "danmaku.show_activity_task_trans";
    const char kDanmakuHimeGiftFontSize[] = "danmaku.detail_gift_fontsize";
    const char kDanmakuHimeInteractionFontSize[] = "danmaku.detail_interaction_fontsize";
    const char kActivityTaskFontSize[] = "danmaku.activity_task_fontsize";

    //const char kDanmakuHimeShow[] = "danmaku.show";
    const char kDanmakuHimeGiftTheme[] = "danmaku.gift_theme";
    const char kDanmakuHimeInteractionTheme[] = "danmaku.interaction_theme";
    const char kActivityTaskTheme[] = "danmaku.activity_task_theme";
    const char kDanmakuHimeEnableSuspending[] = "danmaku.enable_suspending";
    const char kDanmakuHimeEnableGiftFilter[] = "danmaku.enable_gift_filter";
    const char kDanmakuHimeFluency[] = "danmakuhime.fluency";
    const char kDanmakuHimeShowGiftIcon[] = "danmakuhime.show_gift_icon";
    const char kDanmakuHimeEnableHWA[] = "danmakuhime.enable_hwa";
    const char kDanmakuHimeShowChargedGifts[] = "danmakuhime.show_charged_gifts";
    const char kDanmakuHimeEnableGiftEffects[] = "danmakuhime.enable_gift_effects";
    const char kDanmakuHimeEnableChargedGiftThresold[] = "danmakuhime.enable_charged_gift_thresold";
    const char kDanmakuHimeChargedGiftThresold[] = "danmakuhime.charged_gift_thresold";
    const char kDanmakuHimeLockGuide[] = "danmakuhime.show_lock_guide";
    const char kDanmakuHimeEnableTTS[] = "danmaku.enable_tts";
    const char kDanmakuHimeTTSSource[] = "danmaku.tts_source";
    const char kDanmakuHimeTTSSpeed[] = "danmaku.tts_speed";
    const char kDanmakuHimeTTSVolume[] = "danmaku.tts_volume";
    const char kDanmakuHimeBlockEnter[] = "danmakuhime.block_enter";
    const char kDanmakuHimeEnterFilter[] = "danmakuhime.enter_filter";
    const char kDanmakuHimeEnterMedalLevel[] = "danmakuhime.enter_medal_level";
    const char kDanmakuHimeShowUserEnterEffect[] = "danmakuhime.show_user_enter_effect";
    const char kDanmakuHimeShowCoreUserArea[] = "danmakuhime.show_core_user_area";
    const char kDanmakuHimeShowFaceEffect[] = "danmakuhime.show_face_effect";
    const char kDanmakuHimeBlockLottery[] = "danmakuhime.block_lottery";

    const char kDanmakuHimeBlockAllEffect[] = "danmakuhime.block_all_effect";
    const char kDanmakuHimeBlockAllCombo[] = "danmakuhime.block_all_combo";
    const char kDanmakuHimeBlockAllGift[] = "danmakuhime.block_all_gift";
    const char kDanmakuHimeBlockAllNavy[] = "danmakuhime.block_all_navy";
    const char kDanmakuHimeFilterCombo[] = "danmakuhime.filter_combo";
    const char kDanmakuHimeFilterGift[] = "danmakuhime.filter_gift";
    const char kDanmakuHimeFilterNavy[] = "danmakuhime.filter_navy";

    const char kDanmakuHimeGiftViewShowMode[] = "danmakuhime.gift_view_show_mode";
    const char kDanmakuHimeGiftViewPosX[] = "danmakuhime.gift_view_pos_x";
    const char kDanmakuHimeGiftViewPosY[] = "danmakuhime.gift_view_pos_y";
    const char kDanmakuHimeGiftViewWidth[] = "danmakuhime.gift_view_width";
    const char kDanmakuHimeGiftViewHeight[] = "danmakuhime.gift_view_height";
    const char kDanmakuHimeGiftViewPin[] = "danmakuhime.gift_view_pin";

    const char kDanmakuHimeInteractionViewShowMode[] = "danmakuhime.interaction_view_show_mode";
    const char kDanmakuHimeInteractionViewPosX[] = "danmakuhime.interaction_view_pos_x";
    const char kDanmakuHimeInteractionViewPosY[] = "danmakuhime.interaction_view_pos_y";
    const char kDanmakuHimeInteractionViewWidth[] = "danmakuhime.interaction_view_width";
    const char kDanmakuHimeInteractionViewHeight[] = "danmakuhime.interaction_view_height";
    const char kDanmakuHimeInteractionViewPin[] = "danmakuhime.interaction_view_pin";
    const char kDanmakuHimeInteractionViewFirstShow[] = "danmakuhime.interaction_view_first_show";

    const char kActivityTaskViewShowMode[] = "danmakuhime.activity_task_view_show_mode";
    const char kActivityTaskViewPosX[] = "danmakuhime.activity_task_view_pos_x";
    const char kActivityTaskViewPosY[] = "danmakuhime.activity_task_view_pos_y";
    const char kActivityTaskViewWidth[] = "danmakuhime.activity_task_view_width";
    const char kActivityTaskViewHeight[] = "danmakuhime.activity_task_view_height";
    const char kActivityTaskViewPin[] = "danmakuhime.activity_task_view_pin";

    const char kDanmakuHimeVoiceBroadcastFirst[] = "danmakuhime.voice_broadcast_first";
    const char kDanmakuHimeVoiceBroadcastSwitch[] = "danmakuhime.voice_broadcast_switch";
    const char kDanmakuHimeVoiceBroadcastDanmaku[] = "danmakuhime.voice_broadcast_danmaku";
    const char kDanmakuHimeVoiceBroadcastEnterAttention[] = "danmakuhime.voice_broadcast_enter_attention";
    const char kDanmakuHimeVoiceBroadcastGifFeed[] = "danmakuhime.voice_broadcast_gif_feed";
    const char kDanmakuHimeVoiceBroadcastGuard[] = "danmakuhime.voice_broadcast_guard";
    const char kDanmakuHimeVoiceBroadcastLeaveWords[] = "danmakuhime.voice_broadcast_leave_words";
    const char kDanmakuHimeVoiceBroadcastVolume[] = "danmakuhime.voice_broadcast_volume";
    const char kDanmakuHimeVoiceBroadcastPitchRate[] = "danmakuhime.voice_broadcast_pitch_rate";
    const char kDanmakuHimeVoiceBroadcastSpeechRate[] = "danmakuhime.voice_broadcast_speech_rate";

    const char kDanmakuHimeFirstPop[] = "danmakuhime.danmaku_first_pop";

    const int kDanmakuHimeDefTrans = 90;
    const int kDanmakuHimeDefFontSize = 12;
    const bool kDanmakuHimeDefShow = true;
    const int kDanmakuHimeDefTheme = 0;

    const bool kDanmakuHimeDefEnableSuspending = false;
    const bool kDanmakuHimeDefEnableGiftFilter = false;
    const int kDanmakuHimeDefFluency = 1;
    const bool kDanmakuHimeDefShowGiftIcon = true;
    const bool kDanmakuHimeDefEnableHWA = true;
    const bool kDanmakuHimeDefShowChargedGifts = true;
    const bool kDanmakuHimeDefEnableGiftEffects = true;
    const bool kDanmakuHimeDefEnableChargedGiftThresold = false;
    const int kDanmakuHimeDefChargedGiftThresold = 1000;
    const bool kDanmakuHimeDefEnableTTS = false;
    const char kDanmakuHimeDefTTSSource[] = "";
    const int kDanmakuHimeDefTTSSpeed = 2;
    const int kDanmakuHimeDefTTSVolume = 60;
    const bool kDanmakuHimeDefLockGuide = false;
    const bool kDanmakuHimeDefBlockEnter = false;
    const int kDanmakuHimeDefEnterFilter = DHEF_SHOW_ALL;
    const int kDanmakuHimeDefMinEnterMedalLevel = 1;
    const bool kDanmakuHimeDefShowUserEnterEffect = true;
    const bool kDanmakuHimeDefShowCoreUserArea = true;
    const bool kDanmakuHimeDefShowFaceEffect = true;
    const bool kDanmakuHimeDefBlockLottery = true;
    const int kDanmakuHimeDefGiftViewHeight = GetLengthByDPIScale(400);
    const int kDanmakuHimeDefInteractionViewHeight = GetLengthByDPIScale(400);
}


void DanmakuHimePrefService::RegisterProfilePrefs(PrefRegistrySimple *registry) {
    // Danmaku Hime
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftTrans, prefs::kDanmakuHimeDefTrans);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionTrans, prefs::kDanmakuHimeDefTrans);
    registry->RegisterIntegerPref(prefs::kActivityTaskTrans, prefs::kDanmakuHimeDefTrans);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftFontSize, prefs::kDanmakuHimeDefFontSize);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionFontSize, prefs::kDanmakuHimeDefFontSize);
    registry->RegisterIntegerPref(prefs::kActivityTaskFontSize, prefs::kDanmakuHimeDefFontSize);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftTheme, prefs::kDanmakuHimeDefTheme);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionTheme, prefs::kDanmakuHimeDefTheme);
    registry->RegisterIntegerPref(prefs::kActivityTaskTheme, prefs::kDanmakuHimeDefTheme);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeEnableSuspending, prefs::kDanmakuHimeDefEnableSuspending);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeEnableGiftFilter, prefs::kDanmakuHimeDefEnableGiftFilter);
    registry->RegisterIntegerPref(
        prefs::kDanmakuHimeFluency, prefs::kDanmakuHimeDefFluency);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeShowGiftIcon, prefs::kDanmakuHimeDefShowGiftIcon);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeShowChargedGifts, prefs::kDanmakuHimeDefShowChargedGifts);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeEnableGiftEffects, prefs::kDanmakuHimeDefEnableGiftEffects);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeEnableChargedGiftThresold, prefs::kDanmakuHimeDefEnableChargedGiftThresold);
    registry->RegisterIntegerPref(
        prefs::kDanmakuHimeChargedGiftThresold, prefs::kDanmakuHimeDefChargedGiftThresold);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeLockGuide, prefs::kDanmakuHimeDefLockGuide);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeBlockEnter, prefs::kDanmakuHimeDefBlockEnter);
    registry->RegisterIntegerPref(
        prefs::kDanmakuHimeEnterFilter, prefs::kDanmakuHimeDefEnterFilter);
    registry->RegisterIntegerPref(
        prefs::kDanmakuHimeEnterMedalLevel, prefs::kDanmakuHimeDefMinEnterMedalLevel);
	registry->RegisterBooleanPref(
		prefs::kDanmakuHimeShowUserEnterEffect, prefs::kDanmakuHimeDefShowUserEnterEffect);
	registry->RegisterBooleanPref(
		prefs::kDanmakuHimeShowCoreUserArea, prefs::kDanmakuHimeDefShowCoreUserArea);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeShowFaceEffect, prefs::kDanmakuHimeDefShowFaceEffect);
    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeBlockLottery, prefs::kDanmakuHimeDefBlockLottery);

    registry->RegisterBooleanPref(prefs::kDanmakuHimeBlockAllEffect, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeBlockAllCombo, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeBlockAllGift, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeBlockAllNavy, true);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeFilterCombo, 0);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeFilterGift, 0);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeFilterNavy, 0);

    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftViewShowMode, 0);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftViewPosX, -1);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftViewPosY, -1);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftViewWidth, -1);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeGiftViewHeight, -1);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeGiftViewPin, true);

    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionViewShowMode, 0);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionViewPosX, -1);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionViewPosY, -1);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionViewWidth, -1);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeInteractionViewHeight, -1);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeInteractionViewPin, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeInteractionViewFirstShow, true);

    registry->RegisterIntegerPref(prefs::kActivityTaskViewShowMode, 0);
    registry->RegisterIntegerPref(prefs::kActivityTaskViewPosX, -1);
    registry->RegisterIntegerPref(prefs::kActivityTaskViewPosY, -1);
    registry->RegisterIntegerPref(prefs::kActivityTaskViewWidth, -1);
    registry->RegisterIntegerPref(prefs::kActivityTaskViewHeight, -1);
    registry->RegisterBooleanPref(prefs::kActivityTaskViewPin, true);

    registry->RegisterBooleanPref(prefs::kDanmakuHimeFirstPop, true);

    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastFirst, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastSwitch, false);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastDanmaku,true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastEnterAttention, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastGifFeed,true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastGuard, true);
    registry->RegisterBooleanPref(prefs::kDanmakuHimeVoiceBroadcastLeaveWords, true);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeVoiceBroadcastVolume, 50);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeVoiceBroadcastPitchRate, 500);
    registry->RegisterIntegerPref(prefs::kDanmakuHimeVoiceBroadcastSpeechRate, 50);

    bool hwa_default_value = prefs::kDanmakuHimeDefEnableHWA;
    if (IsHardwareAccelerationNeeded()) {
        hwa_default_value = true;
    }

    registry->RegisterBooleanPref(
        prefs::kDanmakuHimeEnableHWA, hwa_default_value);

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        // Danmaku TTS
        registry->RegisterBooleanPref(
            prefs::kDanmakuHimeEnableTTS, prefs::kDanmakuHimeDefEnableTTS);
        registry->RegisterStringPref(
            prefs::kDanmakuHimeTTSSource, prefs::kDanmakuHimeDefTTSSource);
        registry->RegisterIntegerPref(
            prefs::kDanmakuHimeTTSSpeed, prefs::kDanmakuHimeDefTTSSpeed);
        registry->RegisterIntegerPref(
            prefs::kDanmakuHimeTTSVolume, prefs::kDanmakuHimeDefTTSVolume);
    }
}

bool DanmakuHimePrefService::IsHardwareAccelerationNeeded() {
    // Windows 10 10240/10586 系统上，Direct2D + WIC 存在多线程问题，会导致崩溃。
    // 因此在这些系统上开启硬件加速
    if (base::win::GetVersion() == base::win::VERSION_WIN10) {
        int cur_os_build = base::win::OSInfo::GetInstance()->version_number().build;
        if (cur_os_build == 10240 || cur_os_build == 10586) {
            return true;
        }
    }

    return false;
}

DanmakuHimePreferenceInfo DanmakuHimePrefService::LoadPref()
{
    DanmakuHimePreferenceInfo ret;

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    ret.show_gift_icon = pref->GetBoolean(prefs::kDanmakuHimeShowGiftIcon);
    ret.font_size_gift = pref->GetInteger(prefs::kDanmakuHimeGiftFontSize);
    ret.font_size_interaction = pref->GetInteger(prefs::kDanmakuHimeInteractionFontSize);
    ret.font_size_activity_task = pref->GetInteger(prefs::kActivityTaskFontSize);

    ret.fluency = pref->GetInteger(prefs::kDanmakuHimeFluency);
    ret.trans_gift = pref->GetInteger(prefs::kDanmakuHimeGiftTrans);
    ret.trans_interaction = pref->GetInteger(prefs::kDanmakuHimeInteractionTrans);
    ret.trasn_activity_task = pref->GetInteger(prefs::kActivityTaskTrans);
    ret.enable_suspending = pref->GetBoolean(prefs::kDanmakuHimeEnableSuspending);
    ret.enable_gift_filter = pref->GetBoolean(prefs::kDanmakuHimeEnableGiftFilter);
    ret.enable_HWA = pref->GetBoolean(prefs::kDanmakuHimeEnableHWA);
    ret.show_charged_gifts = pref->GetBoolean(prefs::kDanmakuHimeShowChargedGifts);
    ret.enable_gift_effects = pref->GetBoolean(prefs::kDanmakuHimeEnableGiftEffects);
    ret.enable_charged_gift_thresold = pref->GetBoolean(prefs::kDanmakuHimeEnableChargedGiftThresold);
    ret.charged_gift_thresold = pref->GetInteger(prefs::kDanmakuHimeChargedGiftThresold);
    ret.theme_gift = pref->GetInteger(prefs::kDanmakuHimeGiftTheme);
    ret.theme_interaction = pref->GetInteger(prefs::kDanmakuHimeInteractionTheme);
    ret.theme_activity_task = pref->GetInteger(prefs::kActivityTaskTheme);

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku))
    {
        ret.support_TTS = true;
        ret.enable_TTS = pref->GetBoolean(prefs::kDanmakuHimeEnableTTS);
        ret.TTS_source = pref->GetString(prefs::kDanmakuHimeTTSSource);
        ret.TTS_speend = pref->GetInteger(prefs::kDanmakuHimeTTSSpeed);
        ret.TTS_volume = pref->GetInteger(prefs::kDanmakuHimeTTSVolume);
    }
    else
    {
        ret.support_TTS = false;
    }

    ret.block_enter = pref->GetBoolean(prefs::kDanmakuHimeBlockEnter);
    ret.enter_filter = pref->GetInteger(prefs::kDanmakuHimeEnterFilter);
    ret.min_enter_medal_level = pref->GetInteger(prefs::kDanmakuHimeEnterMedalLevel);
    ret.block_lottery = pref->GetBoolean(prefs::kDanmakuHimeBlockLottery);
    ret.show_user_enter_effect = pref->GetBoolean(prefs::kDanmakuHimeShowUserEnterEffect);
    ret.show_core_user_area = pref->GetBoolean(prefs::kDanmakuHimeShowCoreUserArea);
    ret.show_face_effect = pref->GetBoolean(prefs::kDanmakuHimeShowFaceEffect);
    ret.first_pop = pref->GetBoolean(prefs::kDanmakuHimeFirstPop);
    return ret;
}
