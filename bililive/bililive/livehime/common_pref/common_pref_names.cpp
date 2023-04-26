#include "bililive/bililive/livehime/common_pref/common_pref_names.h"

namespace prefs {
    // �ǹ�δͶ��������ʾ��
    const char kLivehimeStarlightGuideShow[] = "livehime.starlight_guide_show";
    const bool kDefaultStarlightGuideShow = true;
    const char kLivehimeLastCpmEffectBubbleShow[] = "livehime.last_cpm_effect_show";
    const char kLivehimeLastCpmEffectRedpointShow[] = "livehime.last_cpm_effect_redpoint_show";

    // ��¼�������ǹ��ʱ�䣨��/�� xx/xx��
    const char kLivehimeStarsCoinExpireTimeBubble[] = "livehime.starscoin.expiretime.bubble";
    const char kLivehimeStarsCoinExpireTimeRedPoint[] = "livehime.starscoin.expiretime.redpoint";
    const char kLivehimeStarsCoinExpireType[] = "livehime.starscoin.expiretime.type";

    // ��ݼ����
    const char kHotkeyMicSwitch[] = "hotkey.mic_switch";
    const char kHotkeySysVolSwitch[] = "hotkey.sysvol_switch";
    const char kHotkeyScene1[] = "hotkey.scene_1";
    const char kHotkeyScene2[] = "hotkey.scene_2";
    const char kHotkeyScene3[] = "hotkey.scene_3";
    const char kHotkeyLiveSwitch[] = "hotkey.live_switch";
    const char kHotkeyRecordSwitch[] = "hotkey.record_switch";
    const char kHotkeyClearDanmakuTTSQueue[] = "hotkey.clear_danmaku_tts_queue";

    //��������
    const char kBililiveObsNoviceGuideShow[] = "livehime.novice_guide_show";
    const bool kDefaultNoviceGuideShow = true;

    //����ͷ��������
    const char kBililiveObsCameraTipsShow[] = "livehime.camera_tips_show";
    const bool kDefaultCameraTipsShow = true;

    //�ط�ת����tips�Ƿ�չʾ
    const char kBililiveObsLiveReplayTipsShow[] = "livehime.live_archive_tips_show";
    const bool kDefaultLiveReplayTipsShow = true;

    //x264���������û�
    const char kBililiveObsX264CrashWarningShow[] = "livehime.x264_crash_warning_show";
    const bool kDefaultX264CrashWarningShow = true;

    //sonic OSD���������û�
    const char kBililiveObsOSDCrashWarningShow[] = "livehime.osd_crash_warning_show";
    const bool kDefaultOSDCrashWarningShow = true;

    //�����״��ع������û�
    const char kBililiveTalkSubjectShow[] = "livehime.live_talk_subject_show";
    const bool kDefaultLiveTalkSubjectShow = true;

    //����PK��������
    const char kBililiveVtuberJoinPopShow[] = "livehime.vtuber_join_pop_show";
    const bool kDefaultVtuberJoinPopShow = true;

    // ����ƥ������
    const char kBililiveVtuberMatchPopShow[] = "livehime.vtuber_match_show";
    const bool kDefaultVtuberMatchPopShow = true;

    //һ�����¼
    const char kBililiveTogetherPop[] = "livehime.together.pop";

    //������Ϸ���
    const char kBililiveNebulaRed[] = "livehime.nebula.red";
    const char kBililiveNebulaRedValue[] = "";

	//��������new flag
	const char kBililiveMultiVideoConnClick[] = "livehime.multi.video.conn.click";

    //ֱ����������
    const char kBililiveGiftPanelRed[] = "livehime.gift_panel.red";

	// �����״ε���
    const char kBililiveRoomLikeShow[] = "livehime.room_like_show";

    //������
    const char kBililivePluginClick[] = "livehime.browser_plugin_click";

    //��ϷPK����ʽ
    const char kBililiveGamePKType[] = "livehime.live_talk_subject_show";
    const bool kDefaultLiveGamePKType = true;

    // ��������ʾ�б�
    const char kBililiveMissionActivityList[] = "livehime.mission_activity.history";

    // ��ʷ�������
    const char kHistoryTitleName[] = "livehime.roominfo.title_history";
    const char kChangedTitleBefore[] = "livehime.roominfo.title_change_ever";

    // ֱ��������λ���Լ�Դ�����Ϣ
    const char kLastMainWinX[] = "livehime.lastbounds.x";
    const char kLastMainWinY[] = "livehime.lastbounds.y";
    const char kLastMainWinWidth[] = "livehime.lastbounds.width";
    const char kLastMainWinHeight[] = "livehime.lastbounds.height";
    const char kLastMainWinMaximized[] = "livehime.last_states.maximized";
    const char kLastTabAreaMaterialsHeight[] = "livehime.tabarea.materials.height";
    const char kTabAreaToolsTpsNewFlag[] = "livehime.tabarea.tools.tps.new";

    // ֱ�����������
    const char kLivehimeClientUpdateIgnoreVersion[] = "livehime.client_update.version";
    const char kLivehimeClientUpdateIgnoreValue[] = "livehime.client_update.ignore";
    const int kDefaultClientUpdateIgnoreVersion = 0;
    const bool kDefaultClientUpdateIgnoreValue = false;

    // ֱ������ͨ���µ�����ʾ
    const char kLivehimeNormalUpdateVersionTip[] = "livehime.normal_update.version";
    const char kLivehimeNormalUpdateTipValue[] = "livehime.normal_update.tip";
    const char kLivehimeNormalUpdateType[] = "livehime.normal_update.type";
    const int  kDefaultNormalUpdateVersionTip = 0;
    const bool kDefaultNormalUpdateTipValue = false;
    const int  kDefaultUpdateType = 0;

    // ��ĻͶƱ��¼
    const char kDanmakuVoteRecords[] = "danmaku_vote.records";
    const char kDanmakuVoteDefRecords[] = "";

    // �ۺ����
    const char kAppPermanentList[] = "appmng.permanent";
    const char kAppLastValidList[] = "appmng.lastvalid";
    const char kAppModuleSignList[] = "appmng.sign";

    // �������������
    const char kLivehimeLiveModelType[] = "livehime.livemodel.type";
    const char kLivehimeLastFocusLandscapeScene[] = "livehime.livemodel.h_scene_name";
    const char kLivehimeLastFocusPortraitScene[] = "livehime.livemodel.v_scene_name";
    const char kLivehimeLiveModelButtonGuideBubbleShow[] = "livehime.livemodel.change_button";
    const char kLivehimeLiveModelPreviewButtonGuideBubbleShow[] = "livehime.livemodel.preview_button";

    // ���������»�ҳ���ع�
    const char kLivehimeGlobalSettingShow[] = "livehime.global_setting_show";
    const char kLivehimeDownloadSetShow[] = "livehime.download_set_show";
    const char kLiveSetGuide[] = "livehime.live_set_tool_show";

    //�����淨&unity����·��
    const char kLivehimeCachePath[] = "livehime.cache_path";

   //������Ļ���
    const char kLivehimeDanmuSettingShow[] = "livehime.voice_danmu_show";
    // ��PK���
    const char kLivehimeNewPkGuideShow[] = "livehime.new_pk.guide";
    const bool kDefaultNewPkGuideShow = true;

    // �Զ��������á����ٽ��
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

    // �°�����������tabҳ�ع�
    const char kLivehimeBeautySettingShow[] = "livehime.beauty_setting_show";

    // ��¼�ͽ������ٽ������������
    const char kLivehimeGoldPkDate[] = "livehime.gold_pk.refuse_date";

    // �������õ��ʷ�����ⴥ�����С�����ʾ
    const char kLivehimeCoverRedPointShow[] = "livehime.cover_redpoint_show";

    // ���ķ���������
    const char kLiveChatRoomBubbleShow[] = "livehime.chat.apply_msg_show";

    //CEF cache
    const char kLiveCefCache[] = "cef_cache";

    //����ı����
    const char kBililiveVtuberPart[] = "livehime.vtuber.thrid";
    const char kBililiveThridVtuberPart[] = "livehime.vtuber.two";

}   // namespace prefs