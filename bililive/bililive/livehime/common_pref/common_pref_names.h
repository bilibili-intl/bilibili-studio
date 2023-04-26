#pragma once

namespace prefs {
    // 星光未投放引导提示窗
    extern const char kLivehimeStarlightGuideShow[];
    extern const bool kDefaultStarlightGuideShow;

    // 星光上次投放效果气泡
    extern const char kLivehimeLastCpmEffectBubbleShow[];
    extern const char kLivehimeLastCpmEffectRedpointShow[];

    // 记录过到期星光的时间（年/月 xx/xx）
    extern const char kLivehimeStarsCoinExpireTimeBubble[];
    extern const char kLivehimeStarsCoinExpireTimeRedPoint[];
    extern const char kLivehimeStarsCoinExpireType[];

    // 快捷键相关
    extern const char kHotkeyMicSwitch[];
    extern const char kHotkeySysVolSwitch[];
    extern const char kHotkeyScene1[];
    extern const char kHotkeyScene2[];
    extern const char kHotkeyScene3[];
    extern const char kHotkeyLiveSwitch[];
    extern const char kHotkeyRecordSwitch[];
    extern const char kHotkeyClearDanmakuTTSQueue[];

    // 开播引导
    extern const char kBililiveObsNoviceGuideShow[];
    extern const bool kDefaultNoviceGuideShow;

    // 摄像头美颜引导
    extern const char kBililiveObsCameraTipsShow[];
    extern const bool kDefaultCameraTipsShow;

    //回放转剪辑tips是否展示
    extern const char kBililiveObsLiveReplayTipsShow[];
    extern const bool kDefaultLiveReplayTipsShow;

    //x264崩溃提醒用户
    extern const char kBililiveObsX264CrashWarningShow[];
    extern const bool kDefaultX264CrashWarningShow;

    //sonic OSD崩溃提醒用户
    extern const char kBililiveObsOSDCrashWarningShow[];
    extern const bool kDefaultOSDCrashWarningShow;

    // 话题首次曝光提醒用户
    extern const char kBililiveTalkSubjectShow[];
    extern const bool kDefaultLiveTalkSubjectShow;

     //虚拟连结弹窗触发
    extern const char kBililiveVtuberJoinPopShow[];
    extern const bool kDefaultVtuberJoinPopShow;

    extern const char kBililiveVtuberMatchPopShow[];
    extern const bool kDefaultVtuberMatchPopShow;

    //一起玩点击记录
    extern const char kBililiveTogetherPop[];

    extern const char kBililiveNebulaRed[];
    extern const char kBililiveNebulaRedValue[];
    extern const char kBililiveMultiVideoConnClick[];

    //直播间礼物
    extern const char kBililiveGiftPanelRed[];

	//房间首次点赞
    extern const char kBililiveRoomLikeShow[];

    // 任务活动已提示列表
    extern const char kBililiveMissionActivityList[];

    //插件点击
    extern const char kBililivePluginClick[];

    // 历史房间标题
    extern const char kHistoryTitleName[];
    extern const char kChangedTitleBefore[];  // 是否将标题改动过

    // 直播姬窗口位置以及源相关信息
    extern const char kLastMainWinX[];                // 最后一次窗口的坐标
    extern const char kLastMainWinY[];
    extern const char kLastMainWinWidth[];            // 最后一次窗口的尺寸
    extern const char kLastMainWinHeight[];
    extern const char kLastMainWinMaximized[];        // 最后一次窗口是否是最大化状态
    extern const char kLastTabAreaMaterialsHeight[];  // 最后一次源列表的高度
    extern const char kTabAreaToolsTpsNewFlag[];      // 需要打new标识的源入口类型

    // 直播姬更新相关
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

    // 弹幕投票记录
    extern const char kDanmakuVoteRecords[];
    extern const char kDanmakuVoteDefRecords[];

    // 聚合入口
    extern const char kAppPermanentList[];// 上次设置的常驻列表，数组，数组项是map，包含应用的一些基础信息
    extern const char kAppLastValidList[];// 上次拉取服务端接口拿到的开放入口列表，记录的是string(AppType)
    extern const char kAppModuleSignList[];

    // 横竖屏开播相关
    extern const char kLivehimeLiveModelType[];     // 上次的横竖屏类型
    extern const char kLivehimeLastFocusLandscapeScene[];   // 上次的选中的横屏场景
    extern const char kLivehimeLastFocusPortraitScene[];    // 上次的选中场竖屏景
    extern const char kLivehimeLiveModelButtonGuideBubbleShow[];    // 横竖屏按钮曝光气泡
    extern const char kLivehimeLiveModelPreviewButtonGuideBubbleShow[];    // 直播间预览按钮曝光气泡

    // 设置项全局设置tab页曝光
    extern const char kLivehimeGlobalSettingShow[];
    extern const char kLivehimeDanmuSettingShow[];
    extern const char kLivehimeDownloadSetShow[];
    extern const char kLiveSetGuide[];

    extern const char kLivehimeCachePath[];

    // 新PK相关
    extern const char kLivehimeNewPkGuideShow[];   // PK和视频连线整合入口的 New 标志和气泡
    extern const bool kDefaultNewPkGuideShow;

    // 新版美颜设置相tab页曝光
    extern const char kLivehimeBeautySettingShow[];

    // 自动码率设置、测速结果
    extern const char kSpeedTestLastDate[];         // 上次测速的时间戳，单位秒
    extern const char kSpeedTestLastSpeedInKbps[];  // 上次测速的结果速率，单位千位比特率
    extern const char kSpeedTestLastBaseline[];     // 上次测速的基准码率
    extern const char kAutoVideoSwitch[];   // 自动码率开关
    extern const char kAutoVideoResolutionInited[]; // 自动模式有没有检测过场景项的数目
    extern const char kChangedCustomToAutoOnceBefore[]; // 是否曾今从自定义切到过自动
    // 用户自填的视频参数，copy from "obs\obs_proxy\public\common\pref_names.h：kVideoXXXXX"
    // 设置面板UI从这里拿数据进行展示，用户如果选择了自定义，那么更新这里的同时也更新obs-proxy的配置，
    // 推流底层最终是从obs-proxy的配置拿参数进行编码的
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

    // 记录赏金赛不再接受邀请的日期
    extern const char kLivehimeGoldPkDate[];

    // 封面设置低质封面标题触达教育小红点显示
    extern const char kLivehimeCoverRedPointShow[];

    extern const char kLiveChatRoomBubbleShow[];

    extern const char kLiveCefCache[];

    extern const char kBililiveVtuberPart[];
    extern const char kBililiveThridVtuberPart[];

}   // namespace prefs

