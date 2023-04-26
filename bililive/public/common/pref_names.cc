#include "bililive/public/common/pref_names.h"

namespace prefs {

const char kProfileCreatedByVersion[] = "profile.created_by_version";
const char kCurrentThemeID[] = "extensions.theme.id";
const char kCurrentThemePackFilename[] = "extensions.theme.pack";
const char kWindowCaptureOptimizeForPerformance[] = "obs.window_capture.optimize_for_performance";
const char kExcludeMainWindowFromCapture[] = "application.mainwin.exclude_from_capture";
const char kDragFullWindows[] = "application.mainwin.drag_full_windows";
const char kAvatarCacheTag[] = "application.avatar_cache_tag";
const char kLastLoginDate[] = "application.last_login_date";
const char kApplicationExitMode[] = "application.program_exit_mode";
const char kApplicationExitRememberChoice[] = "application.program_exit_style_remember";

const char kVtuberBubbleShowed[] = "vtuber.bubble_showed";
const char kVtuberPkBgFileName[] = "vtuber_pk.bg_image_file_name";

const char kAddBtnBubbleShowed[] = "addbtn.bubble_showed";

const char kMainViewNoviceGuide[] = "main_view_novice_guide.show";
const char kPlayCenterLoaclList[] = "play_center_.list";
//Unity版本信息
const char kUnityVersionList[] = "unity.list";
//脸萌贴纸
const char kFaceEffectLoaclList[] = "face_effect.list";

//已下载的加密figma
const char kSecretFigmaList[] = "secret_figma.list";
//预设素材
const char kPresetMaterialGuide[] = "preset_material_guide_show";
const char kSingIdentifyNoticeShow[] = "sing_identify_notice.show";
const char kSingIdentifyNoticeTime[] = "sing_identify_notice.time";
const char kColiveMainWindowFirstShow[] = "colive_main_window.show";
const char kColoveMainWindowHorizCrop[] = "colive_main_window.horiz.crop";
const char kColoveMainWindowVertCrop[] = "colive_main_window.vert.crop";
const char kAnchorTaskGuide[] = "anchor_task_guide_show";
const char kVoiceLinkGuide[] = "voice_link_guide_show";
const char kStreamingQualityGuide[] = "streaming_quality_guide_show";
const char kDanmakuFunctionViewRedPointGuide[] = "danmaku_function_view_red_point_guide_show";
//切换环境相关
const char kFastForwardSwitch[] = "fast_forward.switch";
const char kFastForwardHeadersProxyId[] = "fast_forward.headers.proxy_id";
const char kFastForwardHeadersProxyEnv[] = "fast_forward.headers.proxy_env";
const char kFastForwardWebviewHostPrefix[] = "fast_forward.webview.host_prefix";
const char kFastForwardWebviewQueryArgsProxyId[] = "fast_forward.webview.query_args.proxy_id";
//核心付费用户(大R)弹幕区展示相关
const char kCoreUserSettingBtRedPointGuide[] = "core_user_setting_bt_red_point_guide_show";
const char kCoreUserSettingViewNewFlagGuide[] = "core_user_setting_view_new_flag_guide_show";
const char kCoreUserSettingBtDisturbGuide[] = "core_user_setting_bt_disturb_guide_show";
//主播任务入口气泡每周提醒一次，这里记录最后一次显示气泡时间
const char kAnchorTaskGuideLastShowTime[] = "anchor_task_guide_last_show_time";
//rtc掉线率埋点相关
const char kRTCDropRateIsException[] = "rtc_drop_rate.is_exception";
const char kRTCDropRateChannelId[] = "rtc_drop_rate.channel_id";
const char kRTCDropRateRoomId[] = "rtc_drop_rate.room_id";
const char kRTCDropRateLinkType[] = "rtc_drop_rate.link_type";
const char kRTCDropRateMid[] = "rtc_drop_rate.mid";
const char kRTCDropRateTimeStamp[] = "rtc_drop_rate.time_stamp";

// 直播姬本地多语言
const char kLivehimeAppLocale[] = "livehime.locale";

}   // namespace prefs