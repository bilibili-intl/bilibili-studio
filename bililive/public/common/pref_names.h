#ifndef BILILIVE_PUBLIC_COMMON_PREF_NAMES_H_
#define BILILIVE_PUBLIC_COMMON_PREF_NAMES_H_

namespace prefs {

extern const char kProfileCreatedByVersion[];
extern const char kCurrentThemeID[];
extern const char kCurrentThemePackFilename[];
extern const char kWindowCaptureOptimizeForPerformance[];
extern const char kExcludeMainWindowFromCapture[];
extern const char kDragFullWindows[];             // 拖拽主窗口时是否显示内容
extern const char kAvatarCacheTag[];
extern const char kLastLoginDate[];
extern const char kApplicationExitMode[];
extern const char kApplicationExitRememberChoice[];

// 虚拟直播相关配置数据
extern const char kVtuberBubbleShowed[];
extern const char kVtuberPkBgFileName[];
// 主界面新手引导
extern const char kMainViewNoviceGuide[];
//投屏相关新功能提示
extern const char kAddBtnBubbleShowed[];
// 配饰配置相关数据
//extern const char kOrnamentLocalList[];
// 互动游戏中心
extern const char kPlayCenterLoaclList[];

extern const char kUnityVersionList[];
// 脸萌
extern const char kFaceEffectLoaclList[];

extern const char kSecretFigmaList[];
//预设素材
extern const char kPresetMaterialGuide[];
// 唱歌识别
extern const char kSingIdentifyNoticeShow[];
extern const char kSingIdentifyNoticeTime[];
//连麦窗口
extern const char kColiveMainWindowFirstShow[];
extern const char kColoveMainWindowHorizCrop[];
extern const char kColoveMainWindowVertCrop[];

//主播任务
extern const char kAnchorTaskGuide[];
//连麦提醒引导
extern const char kVoiceLinkGuide[];
//推流质量气泡
extern const char kStreamingQualityGuide[];
//弹幕功能区红点提醒
extern const char kDanmakuFunctionViewRedPointGuide[];
//切换环境相关
extern const char kFastForwardSwitch[];
extern const char kFastForwardHeadersProxyId[];
extern const char kFastForwardHeadersProxyEnv[];
extern const char kFastForwardWebviewHostPrefix[];
extern const char kFastForwardWebviewQueryArgsProxyId[];
//核心付费用户(大R)弹幕区展示
extern const char kCoreUserSettingBtRedPointGuide[];
extern const char kCoreUserSettingViewNewFlagGuide[];
extern const char kCoreUserSettingBtDisturbGuide[];
//主播任务入口气泡每周提醒一次，这里记录最后一次显示气泡时间
extern const char kAnchorTaskGuideLastShowTime[];
//rtc掉线率埋点相关
extern const char kRTCDropRateIsException[];
extern const char kRTCDropRateChannelId[];
extern const char kRTCDropRateRoomId[];
extern const char kRTCDropRateLinkType[];
extern const char kRTCDropRateMid[];
extern const char kRTCDropRateTimeStamp[];

// 直播姬本地多语言
extern const char kLivehimeAppLocale[];

}   // namespace prefs

#endif  // BILILIVE_PUBLIC_COMMON_PREF_NAMES_H_