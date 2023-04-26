#include "bililive/bililive/livehime/common_pref/common_pref_service.h"

#include "base/prefs/pref_service.h"

#include "bilibase/basic_types.h"

#include "ui/views/widget/widget.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_constants.h"

//
//namespace prefs {
//    const char kHotkeyMicSwitch[] = "hotkey.mic_switch";
//    const char kHotkeySysVolSwitch[] = "hotkey.sysvol_switch";
//    const char kHotkeyScene1[] = "hotkey.scene_1";
//    const char kHotkeyScene2[] = "hotkey.scene_2";
//    const char kHotkeyScene3[] = "hotkey.scene_3";
//    const char kHotkeyLiveSwitch[] = "hotkey.live_switch";
//    const char kHotkeyRecordSwitch[] = "hotkey.record_switch";
//    const char kHotkeyClearDanmakuTTSQueue[] = "hotkey.clear_danmaku_tts_queue";
//}

void CommonPrefService::RegisterProfilePrefs(PrefRegistrySimple *registry)
{
    // --------星光投放相关
    registry->RegisterBooleanPref(prefs::kLivehimeStarlightGuideShow, prefs::kDefaultStarlightGuideShow);
    registry->RegisterIntegerPref(prefs::kLivehimeLastCpmEffectBubbleShow, 0);
    registry->RegisterIntegerPref(prefs::kLivehimeLastCpmEffectRedpointShow, 0);
    registry->RegisterInt64Pref(prefs::kLivehimeStarsCoinExpireTimeBubble, 0);
    registry->RegisterIntegerPref(prefs::kLivehimeStarsCoinExpireTimeRedPoint, 0);
    // --------星光投放相关

    // --------快捷键部分
    registry->RegisterIntegerPref(prefs::kHotkeyMicSwitch, 0);
    registry->RegisterIntegerPref(prefs::kHotkeySysVolSwitch, 0);
    registry->RegisterIntegerPref(prefs::kHotkeyScene1, 0);
    registry->RegisterIntegerPref(prefs::kHotkeyScene2, 0);
    registry->RegisterIntegerPref(prefs::kHotkeyScene3, 0);
    registry->RegisterIntegerPref(prefs::kHotkeyLiveSwitch, 0);
    registry->RegisterIntegerPref(prefs::kHotkeyRecordSwitch, 0);

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        registry->RegisterIntegerPref(prefs::kHotkeyClearDanmakuTTSQueue, 0);
    }
    // --------快捷键部分

    registry->RegisterBooleanPref(prefs::kBililiveObsNoviceGuideShow, prefs::kDefaultNoviceGuideShow);
    registry->RegisterBooleanPref(prefs::kBililiveObsCameraTipsShow, prefs::kDefaultCameraTipsShow);
    registry->RegisterBooleanPref(prefs::kBililiveObsLiveReplayTipsShow, prefs::kDefaultLiveReplayTipsShow);
    registry->RegisterBooleanPref(prefs::kBililiveObsX264CrashWarningShow, prefs::kDefaultX264CrashWarningShow);
    registry->RegisterBooleanPref(prefs::kBililiveObsOSDCrashWarningShow, prefs::kDefaultOSDCrashWarningShow);
    registry->RegisterBooleanPref(prefs::kBililiveTalkSubjectShow, prefs::kDefaultLiveTalkSubjectShow);
    registry->RegisterBooleanPref(prefs::kBililiveVtuberJoinPopShow, prefs::kDefaultVtuberJoinPopShow);
    registry->RegisterBooleanPref(prefs::kBililiveVtuberMatchPopShow, prefs::kDefaultVtuberMatchPopShow);
    registry->RegisterBooleanPref(prefs::kBililiveTogetherPop, true);
    registry->RegisterBooleanPref(prefs::kBililivePluginClick, true);
    registry->RegisterStringPref(prefs::kBililiveNebulaRed, prefs::kBililiveNebulaRedValue);
    registry->RegisterBooleanPref(prefs::kBililiveMultiVideoConnClick, false);
	registry->RegisterBooleanPref(prefs::kBililiveRoomLikeShow, true);

    registry->RegisterListPref(prefs::kBililiveMissionActivityList);
    registry->RegisterListPref(prefs::kHistoryTitleName);
    registry->RegisterBooleanPref(prefs::kChangedTitleBefore, false);

    // --------直播姬窗口位置以及源相关信息
    registry->RegisterIntegerPref(prefs::kLastMainWinX, 0);
    registry->RegisterIntegerPref(prefs::kLastMainWinY, 0);
    registry->RegisterIntegerPref(prefs::kLastMainWinWidth, 0);
    registry->RegisterIntegerPref(prefs::kLastMainWinHeight, 0);
    registry->RegisterBooleanPref(prefs::kLastMainWinMaximized, false);
    registry->RegisterIntegerPref(prefs::kLastTabAreaMaterialsHeight, 0);
    // 默认带new标识的入口（随着版本的逐步迭代，一些“新”项会变成旧项，此时就可以将其从默认new列表移除了）
    registry->RegisterBooleanPref(prefs::kTabAreaToolsTpsNewFlag, true);
    // --------直播姬窗口位置以及源相关信息

    // 弹幕投票记录
    registry->RegisterStringPref(prefs::kDanmakuVoteRecords, prefs::kDanmakuVoteDefRecords);

    // --------聚合入口
    // 默认常驻的入口
    scoped_ptr<base::ListValue> permanent_list(new base::ListValue);
    scoped_ptr<base::DictionaryValue> app_dict(new base::DictionaryValue);
    app_dict->SetString("type", input_to_string(livehime::AppType::GiftBox));
    permanent_list->Append(app_dict.get());
    registry->RegisterListPref(prefs::kAppPermanentList, permanent_list.get());
    registry->RegisterListPref(prefs::kAppLastValidList);
    registry->RegisterListPref(prefs::kAppModuleSignList);

    app_dict.release();
    permanent_list.release();
    // --------聚合入口

    registry->RegisterDictionaryPref(prefs::kLiveCefCache);

    // 横竖屏开播[
    registry->RegisterIntegerPref(prefs::kLivehimeLiveModelType, (int)LiveModelController::ModelType::Landscape);
    registry->RegisterStringPref(prefs::kLivehimeLastFocusLandscapeScene, "");
    registry->RegisterStringPref(prefs::kLivehimeLastFocusPortraitScene, "");
    registry->RegisterBooleanPref(prefs::kLivehimeLiveModelButtonGuideBubbleShow, false);
    registry->RegisterBooleanPref(prefs::kLivehimeLiveModelPreviewButtonGuideBubbleShow, false);
    // ]

    // 设置项全局设置tab页曝光
    registry->RegisterBooleanPref(prefs::kLivehimeGlobalSettingShow, true);
    registry->RegisterBooleanPref(prefs::kLivehimeDanmuSettingShow, true);
    registry->RegisterBooleanPref(prefs::kLivehimeDownloadSetShow, true);
    registry->RegisterBooleanPref(prefs::kLiveSetGuide, true);

    // 新PK相关
    registry->RegisterBooleanPref(prefs::kLivehimeNewPkGuideShow, prefs::kDefaultNewPkGuideShow);

    // 美颜设置tab页曝光
    registry->RegisterBooleanPref(prefs::kLivehimeBeautySettingShow, true);

    // 自动码率设置、测速结果 [
    registry->RegisterInt64Pref(prefs::kSpeedTestLastDate, 0);
    registry->RegisterIntegerPref(prefs::kSpeedTestLastSpeedInKbps, 2500);
    registry->RegisterIntegerPref(prefs::kSpeedTestLastBaseline, 8000);
    registry->RegisterBooleanPref(prefs::kAutoVideoSwitch, true);
    registry->RegisterBooleanPref(prefs::kAutoVideoResolutionInited, false);
    registry->RegisterBooleanPref(prefs::kChangedCustomToAutoOnceBefore, false);
    // 自定义视频参数设置
    registry->RegisterBooleanPref(prefs::kCustomVideoSettingInited, false);
    registry->RegisterIntegerPref(prefs::kCustomVideoBitRate, prefs::kDefaultVideoBitRate);
    registry->RegisterStringPref(prefs::kCustomVideoBitRateControl, prefs::kDefaultVideoBitRateControl);
    registry->RegisterStringPref(prefs::kCustomVideoFPSCommon, prefs::kDefaultFPSCommon);
    registry->RegisterIntegerPref(prefs::kCustomVideoOutputCX, prefs::kDefaultVideoOutputWidth);
    registry->RegisterIntegerPref(prefs::kCustomVideoOutputCY, prefs::kDefaultVideoOutputHeight);
    registry->RegisterStringPref(prefs::kCustomOutputStreamVideoEncoder, prefs::kDefaultOutputStreamVideoEncoder);
    registry->RegisterIntegerPref(prefs::kCustomOutputStreamVideoQuality, prefs::kVideoStreamQualityDefault);
    registry->RegisterBooleanPref(prefs::kCustomVideoBitRateControlUseKv, false);
    registry->RegisterBooleanPref(prefs::kCustomStreamVideoEncoderUseKv, false);

    // ]

    // 赏金赛不再接受邀请的时间
    registry->RegisterIntegerPref(prefs::kLivehimeGoldPkDate, 0);

    // 封面设置低质封面标题触达教育小红点显示
    registry->RegisterBooleanPref(prefs::kLivehimeCoverRedPointShow, false);

    registry->RegisterBooleanPref(prefs::kLiveChatRoomBubbleShow, true);

    //unity下载路径
    registry->RegisterStringPref(prefs::kLivehimeCachePath, "");
    registry->RegisterBooleanPref(prefs::kBililiveGiftPanelRed, true);

    registry->RegisterBooleanPref(prefs::kBililiveVtuberPart, true);
    registry->RegisterBooleanPref(prefs::kBililiveThridVtuberPart, true);
}

bool CommonPrefService::RegisterBililiveHotkey()
{
    bool bRet = true;
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SENCE1);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SENCE2);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SENCE3);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SWITCH_LIVE);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SWITCH_RECORD);
    bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_FAST_FORWARD);
    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        bRet &= RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE);
    }
    return bRet;
}

bool CommonPrefService::RegisterBililiveHotkey(int id)
{
    bool reg = true;
    int cfg = 0;
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    switch (id)
    {
    case IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME:
        cfg = pref->GetInteger(prefs::kHotkeyMicSwitch);
        break;
    case IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME:
        cfg = pref->GetInteger(prefs::kHotkeySysVolSwitch);
        break;
    case IDC_LIVEHIME_HOTKEY_SENCE1:
        cfg = pref->GetInteger(prefs::kHotkeyScene1);
        break;
    case IDC_LIVEHIME_HOTKEY_SENCE2:
        cfg = pref->GetInteger(prefs::kHotkeyScene2);
        break;
    case IDC_LIVEHIME_HOTKEY_SENCE3:
        cfg = pref->GetInteger(prefs::kHotkeyScene3);
        break;
    case IDC_LIVEHIME_HOTKEY_SWITCH_LIVE:
        cfg = pref->GetInteger(prefs::kHotkeyLiveSwitch);
        break;
    case IDC_LIVEHIME_HOTKEY_SWITCH_RECORD:
        cfg = pref->GetInteger(prefs::kHotkeyRecordSwitch);
        break;
    case IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE:
        cfg = pref->GetInteger(prefs::kHotkeyClearDanmakuTTSQueue);
        break;
    case IDC_LIVEHIME_HOTKEY_FAST_FORWARD:
    {
        UINT nModify = 0;
        UINT nVk = VK_F10;
        nModify |= MOD_CONTROL;
        nModify |= MOD_SHIFT;
        cfg = MAKELONG(nVk, nModify);
    } break;
    default:
        reg = false;
        break;
    }
    if (reg)
    {
        return RegisterBililiveHotkey(id, HIWORD(cfg), LOWORD(cfg));
    }
    return true;
}

void CommonPrefService::UnregisterBililiveHotKey(int id)
{
    HWND hWnd = NULL;
    BililiveOBSView *view =
        (BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime)) ?
        OBSProxyService::GetInstance().GetBililiveOBSView() : nullptr;

    if (view && view->GetWidget())
    {
        hWnd = view->GetWidget()->GetNativeWindow();
    }
    if (hWnd)
    {
        UnregisterHotKey(hWnd, id);
    }
}

bool CommonPrefService::RegisterBililiveHotkey(int id, UINT nModify, UINT nVk)
{
    bool bRet = true;
    HWND hWnd = nullptr;
    BililiveOBSView *view =
        (BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime)) ?
        OBSProxyService::GetInstance().GetBililiveOBSView() : nullptr;

    if (view && view->GetWidget())
    {
        hWnd = view->GetWidget()->GetNativeWindow();
    }
    if (hWnd)
    {
        ::UnregisterHotKey(hWnd, id);
        if (nVk)
        {
            bRet = ::RegisterHotKey(hWnd, id, nModify, nVk) == TRUE;
        }
    }
    return bRet;
}

// static
bool CommonPrefService::IsBililiveHotKey(UINT nModify, UINT nVk) {
    int keys = MAKELONG(nVk, nModify);
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    if (pref->HasPrefPath(prefs::kHotkeyMicSwitch) &&
        pref->GetInteger(prefs::kHotkeyMicSwitch) == keys)
    {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeySysVolSwitch) &&
        pref->GetInteger(prefs::kHotkeySysVolSwitch) == keys)
    {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeyScene1) &&
        pref->GetInteger(prefs::kHotkeyScene1) == keys)
    {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeyScene2) &&
        pref->GetInteger(prefs::kHotkeyScene2) == keys)
    {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeyScene3) &&
        pref->GetInteger(prefs::kHotkeyScene3) == keys)
    {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeyLiveSwitch) &&
        pref->GetInteger(prefs::kHotkeyLiveSwitch) == keys) {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeyRecordSwitch) &&
        pref->GetInteger(prefs::kHotkeyRecordSwitch) == keys)
    {
        return true;
    }

    if (pref->HasPrefPath(prefs::kHotkeyClearDanmakuTTSQueue) &&
        pref->GetInteger(prefs::kHotkeyClearDanmakuTTSQueue) == keys)
    {
        return true;
    }

    return false;
}