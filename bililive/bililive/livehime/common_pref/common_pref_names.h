#pragma once

namespace prefs {
    // �ǹ�δͶ��������ʾ��
    extern const char kLivehimeStarlightGuideShow[];
    extern const bool kDefaultStarlightGuideShow;

    // �ǹ��ϴ�Ͷ��Ч������
    extern const char kLivehimeLastCpmEffectBubbleShow[];
    extern const char kLivehimeLastCpmEffectRedpointShow[];

    // ��¼�������ǹ��ʱ�䣨��/�� xx/xx��
    extern const char kLivehimeStarsCoinExpireTimeBubble[];
    extern const char kLivehimeStarsCoinExpireTimeRedPoint[];
    extern const char kLivehimeStarsCoinExpireType[];

    // ��ݼ����
    extern const char kHotkeyMicSwitch[];
    extern const char kHotkeySysVolSwitch[];
    extern const char kHotkeyScene1[];
    extern const char kHotkeyScene2[];
    extern const char kHotkeyScene3[];
    extern const char kHotkeyLiveSwitch[];
    extern const char kHotkeyRecordSwitch[];
    extern const char kHotkeyClearDanmakuTTSQueue[];

    // ��������
    extern const char kBililiveObsNoviceGuideShow[];
    extern const bool kDefaultNoviceGuideShow;

    // ����ͷ��������
    extern const char kBililiveObsCameraTipsShow[];
    extern const bool kDefaultCameraTipsShow;

    //�ط�ת����tips�Ƿ�չʾ
    extern const char kBililiveObsLiveReplayTipsShow[];
    extern const bool kDefaultLiveReplayTipsShow;

    //x264���������û�
    extern const char kBililiveObsX264CrashWarningShow[];
    extern const bool kDefaultX264CrashWarningShow;

    //sonic OSD���������û�
    extern const char kBililiveObsOSDCrashWarningShow[];
    extern const bool kDefaultOSDCrashWarningShow;

    // �����״��ع������û�
    extern const char kBililiveTalkSubjectShow[];
    extern const bool kDefaultLiveTalkSubjectShow;

     //�������ᵯ������
    extern const char kBililiveVtuberJoinPopShow[];
    extern const bool kDefaultVtuberJoinPopShow;

    extern const char kBililiveVtuberMatchPopShow[];
    extern const bool kDefaultVtuberMatchPopShow;

    //һ��������¼
    extern const char kBililiveTogetherPop[];

    extern const char kBililiveNebulaRed[];
    extern const char kBililiveNebulaRedValue[];
    extern const char kBililiveMultiVideoConnClick[];

    //ֱ��������
    extern const char kBililiveGiftPanelRed[];

	//�����״ε���
    extern const char kBililiveRoomLikeShow[];

    // ��������ʾ�б�
    extern const char kBililiveMissionActivityList[];

    //������
    extern const char kBililivePluginClick[];

    // ��ʷ�������
    extern const char kHistoryTitleName[];
    extern const char kChangedTitleBefore[];  // �Ƿ񽫱���Ķ���

    // ֱ��������λ���Լ�Դ�����Ϣ
    extern const char kLastMainWinX[];                // ���һ�δ��ڵ�����
    extern const char kLastMainWinY[];
    extern const char kLastMainWinWidth[];            // ���һ�δ��ڵĳߴ�
    extern const char kLastMainWinHeight[];
    extern const char kLastMainWinMaximized[];        // ���һ�δ����Ƿ������״̬
    extern const char kLastTabAreaMaterialsHeight[];  // ���һ��Դ�б�ĸ߶�
    extern const char kTabAreaToolsTpsNewFlag[];      // ��Ҫ��new��ʶ��Դ�������

    // ֱ�����������
    extern const char kLivehimeClientUpdateIgnoreVersion[];
    extern const char kLivehimeClientUpdateIgnoreValue[];
    extern const int kDefaultClientUpdateIgnoreVersion;
    extern const bool kDefaultClientUpdateIgnoreValue;

    extern const char kLivehimeNormalUpdateVersionTip[];
    extern const char kLivehimeNormalUpdateTipValue[];
    extern const char kLivehimeNormalUpdateType[];
    extern const int  kDefaultNormalUpdateVersionTip;
    extern const bool kDefaultNormalUpdateTipValue;
    extern const int  kDefaultUpdateType;

    // ��ĻͶƱ��¼
    extern const char kDanmakuVoteRecords[];
    extern const char kDanmakuVoteDefRecords[];

    // �ۺ����
    extern const char kAppPermanentList[];// �ϴ����õĳ�פ�б����飬��������map������Ӧ�õ�һЩ������Ϣ
    extern const char kAppLastValidList[];// �ϴ���ȡ����˽ӿ��õ��Ŀ�������б���¼����string(AppType)
    extern const char kAppModuleSignList[];

    // �������������
    extern const char kLivehimeLiveModelType[];     // �ϴεĺ���������
    extern const char kLivehimeLastFocusLandscapeScene[];   // �ϴε�ѡ�еĺ�������
    extern const char kLivehimeLastFocusPortraitScene[];    // �ϴε�ѡ�г�������
    extern const char kLivehimeLiveModelButtonGuideBubbleShow[];    // ��������ť�ع�����
    extern const char kLivehimeLiveModelPreviewButtonGuideBubbleShow[];    // ֱ����Ԥ����ť�ع�����

    // ������ȫ������tabҳ�ع�
    extern const char kLivehimeGlobalSettingShow[];
    extern const char kLivehimeDanmuSettingShow[];
    extern const char kLivehimeDownloadSetShow[];
    extern const char kLiveSetGuide[];

    extern const char kLivehimeCachePath[];

    // ��PK���
    extern const char kLivehimeNewPkGuideShow[];   // PK����Ƶ����������ڵ� New ��־������
    extern const bool kDefaultNewPkGuideShow;

    // �°�����������tabҳ�ع�
    extern const char kLivehimeBeautySettingShow[];

    // �Զ��������á����ٽ��
    extern const char kSpeedTestLastDate[];         // �ϴβ��ٵ�ʱ�������λ��
    extern const char kSpeedTestLastSpeedInKbps[];  // �ϴβ��ٵĽ�����ʣ���λǧλ������
    extern const char kSpeedTestLastBaseline[];     // �ϴβ��ٵĻ�׼����
    extern const char kAutoVideoSwitch[];   // �Զ����ʿ���
    extern const char kAutoVideoResolutionInited[]; // �Զ�ģʽ��û�м������������Ŀ
    extern const char kChangedCustomToAutoOnceBefore[]; // �Ƿ�������Զ����е����Զ�
    // �û��������Ƶ������copy from "obs\obs_proxy\public\common\pref_names.h��kVideoXXXXX"
    // �������UI�����������ݽ���չʾ���û����ѡ�����Զ��壬��ô���������ͬʱҲ����obs-proxy�����ã�
    // �����ײ������Ǵ�obs-proxy�������ò������б����
    extern const char kCustomVideoSettingInited[];
    extern const char kCustomVideoBitRate[];
    extern const char kCustomVideoBitRateControl[];
    extern const char kCustomVideoFPSCommon[];
    extern const char kCustomVideoOutputCX[];
    extern const char kCustomVideoOutputCY[];
    extern const char kCustomOutputStreamVideoEncoder[];
    extern const char kCustomOutputStreamVideoQuality[];
    extern const char kCustomVideoBitRateControlUseKv[];
    extern const char kCustomStreamVideoEncoderUseKv[];

    // ��¼�ͽ������ٽ������������
    extern const char kLivehimeGoldPkDate[];

    // �������õ��ʷ�����ⴥ�����С�����ʾ
    extern const char kLivehimeCoverRedPointShow[];

    extern const char kLiveChatRoomBubbleShow[];

    extern const char kLiveCefCache[];

    extern const char kBililiveVtuberPart[];
    extern const char kBililiveThridVtuberPart[];

}   // namespace prefs

