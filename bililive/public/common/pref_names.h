#ifndef BILILIVE_PUBLIC_COMMON_PREF_NAMES_H_
#define BILILIVE_PUBLIC_COMMON_PREF_NAMES_H_

namespace prefs {

extern const char kProfileCreatedByVersion[];
extern const char kCurrentThemeID[];
extern const char kCurrentThemePackFilename[];
extern const char kWindowCaptureOptimizeForPerformance[];
extern const char kExcludeMainWindowFromCapture[];
extern const char kDragFullWindows[];             // ��ק������ʱ�Ƿ���ʾ����
extern const char kAvatarCacheTag[];
extern const char kLastLoginDate[];
extern const char kApplicationExitMode[];
extern const char kApplicationExitRememberChoice[];

// ����ֱ�������������
extern const char kVtuberBubbleShowed[];
extern const char kVtuberPkBgFileName[];
// ��������������
extern const char kMainViewNoviceGuide[];
//Ͷ������¹�����ʾ
extern const char kAddBtnBubbleShowed[];
// ���������������
//extern const char kOrnamentLocalList[];
// ������Ϸ����
extern const char kPlayCenterLoaclList[];

extern const char kUnityVersionList[];
// ����
extern const char kFaceEffectLoaclList[];

extern const char kSecretFigmaList[];
//Ԥ���ز�
extern const char kPresetMaterialGuide[];
// ����ʶ��
extern const char kSingIdentifyNoticeShow[];
extern const char kSingIdentifyNoticeTime[];
//���󴰿�
extern const char kColiveMainWindowFirstShow[];
extern const char kColoveMainWindowHorizCrop[];
extern const char kColoveMainWindowVertCrop[];

//��������
extern const char kAnchorTaskGuide[];
//������������
extern const char kVoiceLinkGuide[];
//������������
extern const char kStreamingQualityGuide[];
//��Ļ�������������
extern const char kDanmakuFunctionViewRedPointGuide[];
//�л��������
extern const char kFastForwardSwitch[];
extern const char kFastForwardHeadersProxyId[];
extern const char kFastForwardHeadersProxyEnv[];
extern const char kFastForwardWebviewHostPrefix[];
extern const char kFastForwardWebviewQueryArgsProxyId[];
//���ĸ����û�(��R)��Ļ��չʾ
extern const char kCoreUserSettingBtRedPointGuide[];
extern const char kCoreUserSettingViewNewFlagGuide[];
extern const char kCoreUserSettingBtDisturbGuide[];
//���������������ÿ������һ�Σ������¼���һ����ʾ����ʱ��
extern const char kAnchorTaskGuideLastShowTime[];
//rtc������������
extern const char kRTCDropRateIsException[];
extern const char kRTCDropRateChannelId[];
extern const char kRTCDropRateRoomId[];
extern const char kRTCDropRateLinkType[];
extern const char kRTCDropRateMid[];
extern const char kRTCDropRateTimeStamp[];

// ֱ�������ض�����
extern const char kLivehimeAppLocale[];

}   // namespace prefs

#endif  // BILILIVE_PUBLIC_COMMON_PREF_NAMES_H_