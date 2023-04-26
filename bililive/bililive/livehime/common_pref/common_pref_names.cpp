#include "bililive/bililive/livehime/common_pref/common_pref_names.h"

namespace prefs {
    // 星光未投放引导提示窗
    const char kLivehimeStarlightGuideShow[] = "livehime.starlight_guide_show";
    const bool kDefaultStarlightGuideShow = true;
    const char kLivehimeLastCpmEffectBubbleShow[] = "livehime.last_cpm_effect_show";
    const char kLivehimeLastCpmEffectRedpointShow[] = "livehime.last_cpm_effect_redpoint_show";

    // 记录过到期星光的时间（年/月 xx/xx）
    const char kLivehimeStarsCoinExpireTimeBubble[] = "livehime.starscoin.expiretime.bubble";
    const char kLivehimeStarsCoinExpireTimeRedPoint[] = "livehime.starscoin.expiretime.redpoint";
    const char kLivehimeStarsCoinExpireType[] = "livehime.starscoin.expiretime.type";

    // 快捷键相关
    const char kHotkeyMicSwitch[] = "hotkey.mic_switch";
    const char kHotkeySysVolSwitch[] = "hotkey.sysvol_switch";
    const char kHotkeyScene1[] = "hotkey.scene_1";
    const char kHotkeyScene2[] = "hotkey.scene_2";
    const char kHotkeyScene3[] = "hotkey.scene_3";
    const char kHotkeyLiveSwitch[] = "hotkey.live_switch";
    const char kHotkeyRecordSwitch[] = "hotkey.record_switch";
    const char kHotkeyClearDanmakuTTSQueue[] = "hotkey.clear_danmaku_tts_queue";

    //开播引导
    const char kBililiveObsNoviceGuideShow[] = "livehime.novice_guide_show";
    const bool kDefaultNoviceGuideShow = true;

    //摄像头美颜引导
    const char kBililiveObsCameraTipsShow[] = "livehime.camera_tips_show";
    const bool kDefaultCameraTipsShow = true;

    //回放转剪辑tips是否展示
    const char kBililiveObsLiveReplayTipsShow[] = "livehime.live_archive_tips_show";
    const bool kDefaultLiveReplayTipsShow = true;

    //x264崩溃提醒用户
    const char kBililiveObsX264CrashWarningShow[] = "livehime.x264_crash_warning_show";
    const bool kDefaultX264CrashWarningShow = true;

    //sonic OSD崩溃提醒用户
    const char kBililiveObsOSDCrashWarningShow[] = "livehime.osd_crash_warning_show";
    const bool kDefaultOSDCrashWarningShow = true;

    //话题首次曝光提醒用户
    const char kBililiveTalkSubjectShow[] = "livehime.live_talk_subject_show";
    const bool kDefaultLiveTalkSubjectShow = true;

    //虚拟PK弹窗触发
    const char kBililiveVtuberJoinPopShow[] = "livehime.vtuber_join_pop_show";
    const bool kDefaultVtuberJoinPopShow = true;

    // 虚拟匹配提醒
    const char kBililiveVtuberMatchPopShow[] = "livehime.vtuber_match_show";
    const bool kDefaultVtuberMatchPopShow = true;

    //一起玩记录
    const char kBililiveTogetherPop[] = "livehime.together.pop";

    //互动游戏红点
    const char kBililiveNebulaRed[] = "livehime.nebula.red";
    const char kBililiveNebulaRedValue[] = "";

	//多人连线new flag
	const char kBililiveMultiVideoConnClick[] = "livehime.multi.video.conn.click";

    //直播间礼物红点
    const char kBililiveGiftPanelRed[] = "livehime.gift_panel.red";

	// 房间首次点赞
    const char kBililiveRoomLikeShow[] = "livehime.room_like_show";

    //插件点击
    const char kBililivePluginClick[] = "livehime.browser_plugin_click";

    //游戏PK捕获方式
    const char kBililiveGamePKType[] = "livehime.live_talk_subject_show";
    const bool kDefaultLiveGamePKType = true;

    // 任务活动已提示列表
    const char kBililiveMissionActivityList[] = "livehime.mission_activity.history";

    // 历史房间标题
    const char kHistoryTitleName[] = "livehime.roominfo.title_history";
    const char kChangedTitleBefore[] = "livehime.roominfo.title_change_ever";

    // 直播姬窗口位置以及源相关信息
    const char kLastMainWinX[] = "livehime.lastbounds.x";
    const char kLastMainWinY[] = "livehime.lastbounds.y";
    const char kLastMainWinWidth[] = "livehime.lastbounds.width";
    const char kLastMainWinHeight[] = "livehime.lastbounds.height";
    const char kLastMainWinMaximized[] = "livehime.last_states.maximized";
    const char kLastTabAreaMaterialsHeight[] = "livehime.tabarea.materials.height";
    const char kTabAreaToolsTpsNewFlag[] = "livehime.tabarea.tools.tps.new";

    // 直播姬更新相关
    const char kLivehimeClientUpdateIgnoreVersion[] = "livehime.client_update.version";
    const char kLivehimeClientUpdateIgnoreValue[] = "livehime.client_update.ignore";
    const int kDefaultClientUpdateIgnoreVersion = 0;
    const bool kDefaultClientUpdateIgnoreValue = false;

    // 直播姬普通更新弹窗提示
    const char kLivehimeNormalUpdateVersionTip[] = "livehime.normal_update.version";
    const char kLivehimeNormalUpdateTipValue[] = "livehime.normal_update.tip";
    const char kLivehimeNormalUpdateType[] = "livehime.normal_update.type";
    const int  kDefaultNormalUpdateVersionTip = 0;
    const bool kDefaultNormalUpdateTipValue = false;
    const int  kDefaultUpdateType = 0;

    // 弹幕投票记录
    const char kDanmakuVoteRecords[] = "danmaku_vote.records";
    const char kDanmakuVoteDefRecords[] = "";

    // 聚合入口
    const char kAppPermanentList[] = "appmng.permanent";
    const char kAppLastValidList[] = "appmng.lastvalid";
    const char kAppModuleSignList[] = "appmng.sign";

    // 横竖屏开播相关
    const char kLivehimeLiveModelType[] = "livehime.livemodel.type";
    const char kLivehimeLastFocusLandscapeScene[] = "livehime.livemodel.h_scene_name";
    const char kLivehimeLastFocusPortraitScene[] = "livehime.livemodel.v_scene_name";
    const char kLivehimeLiveModelButtonGuideBubbleShow[] = "livehime.livemodel.change_button";
    const char kLivehimeLiveModelPreviewButtonGuideBubbleShow[] = "livehime.livemodel.preview_button";

    // 设置项上下滑页面曝光
    const char kLivehimeGlobalSettingShow[] = "livehime.global_setting_show";
    const char kLivehimeDownloadSetShow[] = "livehime.download_set_show";
    const char kLiveSetGuide[] = "livehime.live_set_tool_show";

    //互动玩法&unity缓存路径
    const char kLivehimeCachePath[] = "livehime.cache_path";

   //语音弹幕红点
    const char kLivehimeDanmuSettingShow[] = "livehime.voice_danmu_show";
    // 新PK相关
    const char kLivehimeNewPkGuideShow[] = "livehime.new_pk.guide";
    const bool kDefaultNewPkGuideShow = true;

    // 自动码率设置、测速结果
    const char kSpeedTestLastDate[] = "livehime.speed_test.date";
    const char kSpeedTestLastSpeedInKbps[] = "livehime.speed_test.kbps";
    const char kSpeedTestLastBaseline[] = "livehime.speed_test.baseline";
    const char kAutoVideoSwitch[] = "livehime.auto_cfg.video_codec_switch";
    const char kAutoVideoResolutionInited[] = "livehime.auto_cfg.video_resolution_inited";
    const char kChangedCustomToAutoOnceBefore[] = "livehime.auto_cfg.custom_to_auto_once_before";
    const char kCustomVideoSettingInited[] = "livehime.auto_cfg.video_setting_inited";
    const char kCustomVideoBitRate[] = "livehime.custom_cfg.video_bitrate";
    const char kCustomVideoBitRateControl[] = "livehime.custom_cfg.video_bitrate_control";
    const char kCustomVideoFPSCommon[] = "livehime.custom_cfg.video_fps";
    const char kCustomVideoOutputCX[] = "livehime.custom_cfg.video_width";
    const char kCustomVideoOutputCY[] = "livehime.custom_cfg.video_height";
    const char kCustomOutputStreamVideoEncoder[] = "livehime.custom_cfg.video_encoder";
    const char kCustomOutputStreamVideoQuality[] = "livehime.custom_cfg.video_quality_mode";
    const char kCustomVideoBitRateControlUseKv[] = "livehime.custom_cfg.bitrate_control_init";
    const char kCustomStreamVideoEncoderUseKv[] = "livehime.custom_cfg.video_encode_init";

    // 新版美颜设置相tab页曝光
    const char kLivehimeBeautySettingShow[] = "livehime.beauty_setting_show";

    // 记录赏金赛不再接受邀请的日期
    const char kLivehimeGoldPkDate[] = "livehime.gold_pk.refuse_date";

    // 封面设置低质封面标题触达教育小红点显示
    const char kLivehimeCoverRedPointShow[] = "livehime.cover_redpoint_show";

    // 语聊房提醒气泡
    const char kLiveChatRoomBubbleShow[] = "livehime.chat.apply_msg_show";

    //CEF cache
    const char kLiveCefCache[] = "cef_cache";

    //虚拟改变分区
    const char kBililiveVtuberPart[] = "livehime.vtuber.thrid";
    const char kBililiveThridVtuberPart[] = "livehime.vtuber.two";

}   // namespace prefs