
#ifndef BILILIVE_SECRET_PUBLIC_EVENT_TRACKING_SERVICE_H_
#define BILILIVE_SECRET_PUBLIC_EVENT_TRACKING_SERVICE_H_

#include <map>
#include <string>

#include "base/strings/string_split.h"

#include "bililive/secret/public/request_connection_proxy.h"

namespace infoc
{
    class AppEvent;
}

namespace secret {

// 技术埋点
enum class LivehimeBehaviorEvent : int {
    LivehimeInstall = 0,
    LivehimeLogin,
    LivehimeSource,
    LivehimeSet,
    LivehimeCameraFilter,
    LivehimeDPI,
    LivehimeRoomInfo,
    LivehimeDanmaku,
    LivehimeDanmakuHimeState,
    LivehimeCameraProperties,
    LivehimeLiveDuration,
    LivehimeStreamingOutcome,
    LivehimeCrash,
    LivehimeGiftboxClick,
    LivehimeIncompleteAvatar,
    LivehimeMorePrejection,
    LivehimeMoreHelp,
    LivehimeMoreVersionsCheck,
    LivehimeMoreLogfile,
    LivehimeSceneChange,
    LivehimeSceneTop1,
    LivehimeSceneLow1,
    LivehimeSceneTop,
    LivehimeSceneLow,
    LivehimeSceneCompile,
    LivehimeSceneHide,
    LivehimeSceneLock,
    LivehimeSceneDelete,
    LivehimeRightKeyOriginalSize,
    LivehimeRightKeyFullScreen,
    LivehimeRightKeyTop,
    LivehimeRightKeyLow,
    LivehimeRightKeyZoom,
    LivehimeRightKeyStretch,
    LivehimeRightKeyCut,
    LivehimeRightKeyDelete,
    LivehimeRightKeyInteraction,
    LivehimeRightKeyProperty,
    LivehimeBarrageRoom,
    LivehimeBarrageReport,
    LivehimeBarrageBlackList,
    LivehimeBarrageManagerSet,
    LivehimeBarrageBanned,
    LivehimeBarrageNameList,
    LivehimeBarrageSearch,
    LivehimeBarrageFlush,
    LivehimeHoverMyRoom,
    LivehimeHoverMyRoom1,
    LivehimeHoverLiveData,
    LivehimeGuideType,
    LivehimeGuideMain,
    LivehimeGuidePcgame,
    LivehimeGuideMobilegame,
    LivehimeGuideRecreation,
    LivehimeGuideDrawing,
    LivehimeGuideCourse,
    LivehimeGuideRadio,
    LivehimeGameAdaptation,
    LivehimeGuideGameAdaptation,
    LivehimeMobileGame,
    LivehimeGuideMobileGame,
    LivehimeSceneRename,
    LivehimeSceneRename1,
    LivehimeComputerSystem,
    LivehimeLiveData,
    LivehimeMeleeChaosfightActivityShow,
    LivehimeMeleeChaosfightActivityClick,
    LivehimeMeleeChaosfightInfocardShow,
    LivehimeMecha,
    LivehimeVlinkClick,
    LivehimeVlinkSwitch,
    LivehimeVlinkListShow,
    LivehimeVlinkReject,
    LivehimeVlinkToBlacklist,
    LivehimeVlinkConditionClick,
    LivehimeVlinkConditionConfirmed,
    LivehimeVlinkConnect,
    LivehimeVlinkHangup,
    LivehimeVlinkStart,
    LivehimeVlinkSelfSucceeded,
    LivehimeVlinkRemoteSucceeded,
    LivehimeVlinkEnd,
    LivehimeVlinkError,
    LivehimeQuicStreaming,
    LivehimeSetTags,   //改成埋到北极星业务埋点了
    LivehimeUpgrade,
    //LivehimeCameraBeautySetting,
    LivehimeFilterNameBeauty,
    LivehimeLotteryClick,
    LivehimeExceptNames,
    LivehimeUpgradeDownload,
    LivehimeMissionActivityClick,
    LivehimeMissionActivityProjection,
    LivehimeRecordError,
    LivehimeCefError,
    LivehimeBannerOperate,
    LivehimeStatusBarOperate,
    LivehimeVideoConnError,
    LivehimeVideoConnStart,
    LivehimeVideoConnSelfSuccess,
    LivehimeVideoConnOtherSuccess,
    LivehimeVideoConnEnd,
    LivehimeVideoEditClickClip, //全局设置里面直播回放设置的埋点
    LivehimeVideoEditClickBtn,  //弹窗询问的情况下，点保存还是删除的埋点
    LivehimeIntegrityChecking,
    LivehimeStreamingQMCSShow,  // 推流质量监控面板曝光埋点
    LivehimeStreamQuality,  // 质量埋点，包含服务端的质量
    LivehimeLocalStreamQuality,  // 本地质量埋点，不包含服务端的质量
    LivehimeColiveSdkReport, //连麦底层SDK上报
    LivehimeLogFilesUpload, // 日志上传

    LivehimeColiveStart,         // 连麦发起
    LivehimeColiveReply,         // 连麦应答（邀请模式的连麦才有）
    LivehimeColiveFlow,          // 连麦流程
    LivehimeColiveMatched,       //服务器匹配

    LivehimeUploadVirtualConnectionZip,     //虚拟连接上传

    LivehimeWebviewOfflineZipDownload,       //离线资源包下载
    LivehimeWebviewOfflineUnZip,       //离线资源包解压
    LivehimeWebviewOfflineUse,         //离线资源包使用率
    LivehimeWebviewOfflineLoadTime,    //首次加载耗时
    LivehimeWebviewLoadSucc,            //H5页面加载是否成功
    LivehimeWebviewOfflineFailCount,     //离线资源失败
    LivehimeIosWiredStatus,              //Ios有线投屏的状态
    LivehimeScreenProjectionShow,        //投屏状态成功

    LivehimeMultiVideoConnInfoReport,           //多人连线房间统计信息技术埋点上报
    LivehimeInteractPlayOpen,                   //互动玩法打开

};

// V2业务埋点
enum class LivehimeBehaviorEventV2
{
    // 业务埋点取消或是由于交互变更了导致旧的埋点没有意义了或失去了埋点时机了在这里注释就好，
    // 就当做个代码层面的记录留存，不要删
    Unknown,
    Install,
    ObsPluginInstall,
    LoginResult,
    LiveGuideShow,
    GuideTutorialClick,
    GuideAddSource,
    GuideCompleteClick,
    MainWndShow,
    UserInfoWndShow,
    UserInfoItemClick,
    TitlebarItemClick,
    MoreMenuItemClick,
    LiveShareItemClick,
    SceneChange,
    RoomTitleSaveClick,
    MaterialItemClick,
    SceneItemMenuClick,
    AppMngClick,
    //ApplistShow,
    AppClick,
    MissionButtonShow,
    //MissionButtonClick,
    MissionProject,
    ToolbarItemClick,
    StatusbarNoticeClick,
    LiveRecord,
    StartLive,
    AnchorPromotionPanelShow,
    AnchorPromotionHoverClick,
    ActivityCenterTabClick,
    ActivityCenterCloseClick,
    CpmGuideWindowShow,
    //CpmPreLiveBubbleShow,
    //CpmLivingBubbleShow,
    LiveModelButtonClick,
    GuardWarningShow,
    GuardWarningClick,
    ActivityViewShow,
    ActivityViewClick,
    SkinClick,
    FilterClick,
    StickersClick,
};

enum class BehaviorEventNetwork
{
    Unknown,
    Wifi,
    Cellular,
    Offline,
    Other,
    Ethernet,
};

// 此处埋点不合理，使用此处枚举 SceneItemType  合理，历史原因无法修改
enum class BehaviorEventMaterialType
{
    Unknown = 0,
    Monitor = 1,
    Screen = 2,
    Window = 3,
    Game = 4,
    Text = 5,
    Image = 6,
    Camera = 7,
    Media = 8,
    Projection = 9,
    Browser = 10,
    ThirdPartyStreaming = 12,
    DanmakuHime = 13,
    Microphone = 15,
    Loudspeaker = 16,
    //Ornament = 17,
    Plugin = 18,
    ThreeVtuber = 19,
    Vtuber = 20,
};

struct EthernetAddressInfo
{
    std::string ip;
    std::wstring addr;
    std::wstring communications_operator;
};

// V2埋点字段不允许为空，必须有值，值获取不到的要根据埋点规则要求填充特定数据
const char kOptionalValueDummy[] = "-99999";
const char kRequireValueDummy[] = "-99998";
const int kOptionalValueDummyInt = -99999;   // 产品不要求上报
const int kRequireValueDummyInt = -99998;    // 要求上报但无法获取

// 通过服务端进行的埋点
enum class LivehimeViaServerBehaviorEvent
{
    Other,
    StartLive,
    StopLive,
    ChangeArea,
    PK,
    MediaLink,
    BackgroundMusic_Unsupported,
    CameraBeauty,
    Lottery_Unsupported,
    CPM_Unsupported,
    Shared,
    ChangeDefinition,
    ChangeRoomInfo,
    LiveAcrossTheDay,
    ScanQRCode,
};

// 通过服务端进行的埋点-新接口
enum class LivehimeViaServerBehaviorEventNew
{
   SingIdentifyStart = 200001,  //开始唱歌
   SingIdentifyStop  = 200002,  //停止唱歌
   SingIdentifyLeave = 200003,  //离开检测分区

   //通过广播的形式通知，被动检测触发回调
   SingIdentifyCallbackStart = 200011, //回调开始唱歌
   SingIdentifyCallbackStop  = 200012, //回调停止唱歌
   SingIdentifyCallbackLeave = 200013, //回调离开检测分区

};

// 北极星业务埋点
enum class LivehimePolarisBehaviorEvent
{
    StartLive,          // 点击开播按钮时上报
    StartLiveResult,    // 直播姬开播结果回调
    StartLiveTag,       //开播标签信息上报
    ActiveClick,        //PC直播姬拉活来源
    StartUpShow,        // 冷启动时上报
    MaterialItemClick,  // 点击素材时上报
    ApplistShow,        // 直播互动工具露出时上报
    AppClick,           // 点击互动工具时上报
    BannerItemShow,     // 直播姬主页活动入口曝光
    BannerItemClick,    // 直播姬主页活动入口点击

    ActivityCenterClick,// 	点击活动中心时上报
    GiftFoldClick,      // 礼物区收起/展开icon点击
    DanmakuFoldClick,   // 弹幕区收起/展开icon点击
    DmkGiftTabClick,    // 高能榜/礼物流水tab切换点击
    PreviewBtnClick,    // 预览开关点击
    DmkMsgSend,         // 弹幕发送点击
    MuteBlacklistCommit,// 禁言确认点击，点击禁言时上报
    DmkMsgReport,       // 弹幕举报确定点击，点击举报弹幕时上报
    DmkBlockWordAddCommit,  // 添加屏蔽词点击

    LiveStopMoreDataClick,  //关播页查看详细数据
    LiveStopStarShow,       //星光卡片曝光
    LiveStopStarTipClick,   //星光提示点击
    LiveStopClipShow,       //去剪辑卡片曝光
    LiveStopPublicClick,    //发布动态点击
    LiveStopWatchReplayClick,      //点击查看回放
    LiveStopAutoReplayClick,      //关播页设置 自动回放
    LiveStopReplayDlgShow,      //关播页设置 回放面板曝光
    LiveStopReplayDlgClick,     //关播页设置 回放面板按钮点击
    LiveStopHighlightsDlgShow,    //关播页设置 高光切片模块曝光
    LiveStopHighlightsBtnClick,      //关播页设置 高光切片点击
    LiveStopHighlightsBtnAllClick,     //关播页设置 查看全部高光切片按钮点击

    SourceSettingShow,  // 素材设置弹窗露出时上报
    SourceTextSensitiveShow, //文本素材触发敏感词上报
    SubtractSwitchClick,// 点击开启/关闭扣除背景色时上报
    SourceSettingAdvanceClick,  // 点击开启/关闭高级设置时上报
    SourceSettingCaptureMethod,   // 点击切换捕获方式时上报
    SourceSettingWndAreaClick,   // 点击开启/关闭窗口区域模式时上报
    SourceSettingMultiGraphicClick,   // 点击开启/关闭多显卡模式时上报
    SourceSettingAreoClick,     // 点击开启/关闭areo时上报
    SourceSettingSubmitClick,   // 添加素材时，点击确定时上报

    HandonLiveVerificationShow,   //挂播页面验证展示
    HandonLiveVerificationClick,  //挂播页面确认点击

    VtuberUploadFigma,      // 点击上传虚拟形象时上报
    VtuberDownloadFigma,    // 点击下载虚拟形象时上报
    VtuberSetFigmaConfig,   // 设置虚拟形象时上报
    VtuverFaceAlineClick,   //面部校准点击
    VtuverHeadAlineClick,   //头部校准点击
    VtuberPropWindow,       // 虚拟形象属性页初次显示/切换 Tab 时上报
    VtuberSelectMotion,     // 点击“选择动作”时上报
    VtuberSelectKeyCombo,   // 点击“编辑快捷键”或点快捷键输入框时上报
    VtuberStartMotion,      // 快捷键触发虚拟形象动作时上报

    VtuberGiftInteractSetting,  // 虚拟形象管理面板礼物互动设置入口点击时上报
    VtuberGiftInteractSettingChanged,   // 虚拟形象礼物互动设置改变时上报
    VtuberPanelShow,        //虚拟形象管理面板虚拟形象曝光
    VtuberPanelClick,       //虚拟形象管理面板虚拟形象点击

    ProjectionPanelShow,     //直播姬投屏面板曝光
    ProjectionPanelClick,    //直播姬投屏tab点击
    ProjectionIosInstall,    //直播姬有线投屏安装点击
    ProjectionIosSuccess,    //直播姬有线投屏安装成功结果回调

    ProjectionVoiceClick,    //直播姬投屏声音来源点击
    ProjectionAudioClick,    //直播姬投屏音频监听选择点击
    ProjectionSuccessShow,   //直播姬投屏音成功结果回调




    LiveNewRoomBtnClick,    //主播公告点击
    LiveNewRoomSubClick,    //主播公告发布点击

    //语音弹幕打点
    VoiceDanmuSwitchClick,
    VoiceDanmuClick,

    InstallNew,    // 直播姬安装面板安装点击（新）
    InstalledResultNew,//直播姬安装结果回调（新)

    MainNoviceGuide,  //新手引导界面
    MainWndPV,  // 直播姬主页浏览，页面出现时上报
    Install,    // 直播姬安装面板安装点击，点击安装时上报
    InstalledResult,    // 直播姬安装结果自动上报
    UserNotRealNameAuthToast,   // 实名认证toast曝光时上报
    LiveRoomNoCoverToast,   // 上传封面toast曝光时上报
    LiveRoomNoAreaToast,    // 	选择分区toast曝光时上报
    LiveRoomCoverEntryClick,    // 点击封面设置时上报
    LiveRoomAreaEntryClick,     // 点击选择分区时上报
    LiveRoomAreaSubmitClick,    // 直播姬选择分区确认点击
    LiveRoomCoverRefusePV,  // 封面未过审面板曝光
    LiveRoomCoverRefuseUplClick,    // 封面未过审面板点击，封面未过审面板点击时上报
    AppMaterialEntryClick,  // 点击添加素材时上报
    LiveRoomCoverShow,  // 直播姬封面设置按钮曝光时上报

    //PK连麦相关
    PkPanelShow,        //pk面板曝光时上报
    PkPanelRecAnchorShow,  //pk面板推荐pk主播名称出现时上报
    PkPanelRecAnchorClick,  //点击邀请pk时上报
    PkInvitationShow,       //收到他人的pk邀请，面板曝光时上报
    PkInvitationClick,       //收到他人的pk邀请， 点击时上报
    CpmLiveBubbleShow,      //直播推广气泡弹出时上报
    PkPanelEchoHelpShow,    //pk面板回声帮助按钮曝光
    PkPanelEchoHelpClick,   //pk面板回声帮助按钮点击

     //热力风暴相关
    StormProgressShow,      //热力风暴进度条显示
    StormProgressClick,     //热力风暴点击
    StormProgressTracker,   //热力风暴进度上报
    StormApiProgressTracker,//接口获取热力风暴状态
    StormDownLoadTracker,   //热力风暴MP4下载上报
    StormPlayTracker,        //热力风暴播放
    // 录制
    RecordClick,
    CertificationShow,          //直播姬实名认证页曝光
    CertificationResultShow,    //直播姬实名认证页结果曝光
    DmjWindowStatus,           //弹幕姬窗口状态变化曝光
    DmjActiviyTaskSetting,      //弹幕姬活动任务设置页曝光
    DmjRevenueSetting,          //弹幕姬流水设置页曝光
    DmjDanmuSetting,            //弹幕姬弹幕设置页曝光
    DmjActivityWindowEnterance, //活动中心入口点击
    DmjActivityWindowDetails,   //活动中心详情点击
    DmjSettingEnterEffectsClick, //弹幕姬设置进场特效点击

    DmjActivityWindowTask,       //任务入口点击

    LiveAssistantShow,          //直播小助手展示
    LiveAssistantClick,          //点击
    CameraRotateClick,        //镜头逆时针旋转点击
    PkStreamQuality,           //PK连麦，合流质量上报
    //预设素材相关埋点
    PresetMaterialMainViewShow,//直播姬预设页曝光
    PresetMaterialSkipClick,//直播姬预设页跳过点击
    PresetMaterialPrimaryLiveContentClick,//预设页直播一级内容选择
    PresetMaterialSubLiveContentClick,//预设页直播二级内容选择
    PresetMaterialGoToStartLiveClick,//预设页前往开播点击
    PluginSceneClick,           //插件素材入口点击
    SceneHideClick,             //素材隐藏按钮点击
    SceneLockClick,             //素材锁定
    SceneDeleteClick,           //素材删除
    SceneEditClick,             //素材属性设置点击
    LeftToolbarCollapseClick,//直播姬左侧工具栏收起按钮点击
    BrowserRefreshClick,        //插件属性页刷新
    BrowserBackgroundClick,     //插件背景色扣除
    BrowserCustomizedClick,      //背景色自定义
    SingPanelShow,            //唱歌检测提示弹窗曝光
    SingPanelConfirmClick,    //唱歌检测提示弹窗知道了点击
    SingPanelNoMoreClick,     //唱歌检测提示弹窗不再提示点击
    //主播任务一期优化相关埋点
    StopLiveInterceptWindowShow,   //关播拦截窗口显示
    StopLiveInterceptWindowClick,   //关播拦截窗口详情点击
    ColivePresetFunctionPresetClick,        //连麦预设预设 调试点击
    ColivePresetFunctionResetClick,         //连麦预设预设 重置点击
    ColivePresetFunctionCropSave,           //连麦预设预设 保存裁剪参数
    ColivePresetFunctionRecordClick,        //连麦预设预设 录制按钮点击
    ColivePresetFunctionAdviceShow,         //连麦预设预设 弹出建议
    ColivePresetFunctionBtnClick,           //连麦预设预设 按钮点击
    EndLiveDataWindowTaskClick,   //关播页任务卡片点击
    VertCoverShow,
    VertCoverClick,
    VtuberJoinTipDlgShow,        // 虚拟连结提示弹窗展示
    DanmuReadinPanelSwitchClick,  //语音读弹幕面板开关点击
    DanmuReadingCloseClick,       //语音弹幕结束使用
    DanmuReadingClick,            //语音播报弹幕开始播报
    DanmuReadingReceiverClick,    //语音播报弹幕请求
    VoiceBroadcastDelaytimeTrack, //语言弹幕延时表现
    VoiceBroadcastUsageTrack,     //语言弹幕使用功能
   //美颜相关
    BeautyIconClick,
    FilterItemClick,
    StickersViewItemClick,
    BeautyMaterialTabClick,
    BeautyMaterialSecondaryTabClick,
    BeautyMaterialItemClick,
    AllBeautyParamsSaveClick,
    LiveQualityInfoReport,              //开播性能质量体验建设上报
    ThreeVtuberViewShow,                //3d下载页弹出
    TwoVtuberViewShow,                  //2d下载页弹出
    StartThreeVtuberClick,              //启动3Dunity
    StartTwoVtuberClick,                //启动2Dunity
    UnityDownloadResultClick,           //Unity下载结果上报
    TwoUnityDownloadResultClick,        //2D Unity下载结果上报
    TwoUnityStartLive,                  //2D Unity开始直播上报

    //视频连线功能优化
    VideoConnHistoryClick,//连线面板连线记录按钮点击
    VideoConnInvitationClick,    //连线面板主播点击
    VideoConnWaitingPanelShow,//连线等待弹窗曝光
    VideoConnWaitingPanelCancelClick,//连线等待弹窗取消点击
    VideoConnInvitationPanelShow,//收到连线邀请面板曝光
    VideoConnInvitationPanelClick,//收到连线邀请面板点击
    VideoConnFollowClick,//快速关注按钮点击
    VideoConnFollowStatusChange,//关注状态变化
    // 互动玩法&插件
    PluginEntranceShow,     //插件入口曝光
    PluginSetDlgShow,       //插件设置弹窗曝光
    PluginSetSureClick,      //插件设置弹窗确定点击
    //连麦规则优化
    VoiceLinkPanelShow,//连麦面板曝光
    VoiceLinkPanelSwitchClick,//连麦面板开关点击
    VoiceLinkPanelConditionClick,//连麦面板条件点击
    VoiceLinkWaitingPanelShow,//连麦提醒弹窗曝光
    VoiceLinkwaitingPanelClick,//连麦提醒弹窗点击
    VoiceLinkCardEndClick,//连麦中麦位卡挂断点击
    VoiceLinkJoinPanelAudienceClick,//连线面板点击接受申请上报
    //刚进入主界面时运营推广活动弹框
    ActivityPopupsWindowShow,//活动弹框显示
    ActivityPopupsWindowClick,//活动弹框内点击
    //虚拟匹配
    VtuberMatchPanelShow,   //面板曝光
    VtuberMatchBtnClick,    //匹配按钮点击
    VtuberConnWaitShow,     //等待时长
    //一起玩
    PlayTogetherHelpClick,  //功能说明点击
    //直播质量
    StreamingQualityDetailPanelShow,//状态栏直播质量
    StreamingQualityDetailPanelHelpClick,

    FaceFffectSwitchClick,   //脸萌开关
    FaceSourceDownLoadTracker,
    FaceEffectPlayTracker,

    EffectSettingTotalSwitch,   //直播姬动画设置总开关点击
    EffectSettingDetialSwitch,  //直播姬动画设置详细设置点击

    JumpAgreementTrack,// 协议跳转

	  SettingDowanloadClick,
	  CachePathSetClick,
    MP4EffectAdded,
    MP4EffectPlayed,
    MP4EffectDXFailed,

    //多人连线
    MultiVCRoomShow,                //多人连线房间曝光,主界面打开时上报
    MultiVCRoomInviteClick,         //多人连线房间邀请连线点击,点击邀请按钮时上报
    MultiVCInvitePanelShow,          //申请列表面板-打开时上报
    MultiVCApplyPanelJoinClick,      //申请列表面板-申请加入按钮点击时上报
    MultiVCApplyPanelCancelClick,    //申请列表面板-点击取消申请时上报
    MultiVCInvitePanelInviteClick,   //邀请列表面板-点击邀请连线时上报
    MultiVCInvitePanelCancelClick,   //邀请列表面板-点击取消邀请时上报
    MultiVCApplyDialogShow,         //收到他人的多人连线申请弹窗出现时上报
    MultiVCApplyDialogClick,        //多人连线申请弹窗点击按钮时上报

	MultiVCInviteDialogShow,        //收到他人的多人连线邀请弹窗曝光时上报
    MultiVCInviteDialogClick,       //多人连线邀请弹窗点击按钮时上报

    InteractPlayDownload,           //互动玩法下载
    InteractPlayOpen,               //互动玩法使用
    VtuberJoinTrack,                //虚拟连结
    FigmaUploadTrack,               //figma文件上传
    StartVtuberJoinTrack,

    CefJsBridgeCallTrack,  //收到h5的jsbridge调用
    //核心付费用户(大R)弹幕区展示
    DanmuCoreUserSettingClick,//直播姬弹幕互动设置高能用户入场通知开关点击
    DanmuCoreUserModuleClick,//直播姬弹幕姬大R轮播模块状态切换点击
    DanmuCoreUserMessageShow, //直播姬弹幕姬高能用户进场/轮播信息曝光

	VtuberEntranceClick,            //2D|2D入口点击
    QuestionnaireShow,    //问卷曝光
    QuestionnaireClick,   //问卷点击
    //北极星技术埋点track
    //推流CDN相关

    CDNSendPacketFail,    //当前CDN推流发包三次失败上报
    AppUseTimeTracker,           //APP使用时长
    CloseLiveTracker,     //关播埋点
    LaunchDurationTracker,   //launch.duration.track

	AVlinkChannelBaseInfo,   //音视频通话加入频道埋点
	//语聊房
    MultiVoicePreviewShow, //语聊房预览弹窗曝光
    MultiVoicePreviewClick,//语聊房预览弹窗点击
    MultiVoiceJoinApplyPanelShow,//语聊房消息列表曝光
    MultiVoiceApplyPanelInviteClick,//语聊房消息列表去邀请点击
    MultiVoiceJoinApplyPanelClick,//语聊房消息列表反馈按钮点击
    MultiVoiceJoinInvitePanelShow,//语聊房邀请列表曝光
    MultiVoiceJoinInvitePanelClick,//语聊房邀请列表邀请点击
    MulitiVoiceJoinQuitClick,      //语聊房关闭点击
    MulitiVoicePositionClick,       //语聊房单麦位点击
    MulitiVoiceManageClick,          //麦位管理项点击
    MulitiVoiceErrorTrack,
    MulitiVoiceClosedTechTrack,
    //主播任务入口气泡每周提醒一次
    AnchorTaskCenterBubbleShow,
    FansRankClick,         //直播姬粉丝团入口点击
    HotRankClick,         //直播姬榜单入口点击
    HotRankShow,          //直播姬榜单入口曝光
	DanmuGiftPanelClick,
    RTCExceptionInfoTrack,//rtc掉线率埋点
    WebrtcRealtimeTrack,//音视频实时卡顿
    DanmuCoreUserTagShow
};

// 海外数平埋点（技术+业务，先不区分）
enum class LivehimeIntlBehaviorEvent
{
    StartLive = 0,
    LivehimeInstall
};

class EventTrackingService {
public:
    virtual ~EventTrackingService() {}

    // 技术埋点
    virtual RequestProxy ReportLivehimeBehaviorEvent(
        LivehimeBehaviorEvent event_id,
        int64_t mid,
        const std::string& event_msg) = 0;
    virtual RequestProxy ReportLivehimeBehaviorEvent(
        const std::string& event_id,
        const std::string& event_msg) = 0;

    // V2版本的埋点，V2与旧的并存，旧的在业务埋点的地方可以逐步删掉，技术埋点还是继续用旧表
    // 这里的V2指的业务层面的V2，并非埋点协议的V2，以便与以前直接埋到直播姬专属技术埋点表的业务做区分
    virtual RequestProxy ReportLivehimeBehaviorEventV2(
        LivehimeBehaviorEventV2 event_id,
        const base::StringPairs& params) = 0;

    virtual RequestProxy ReportLivehimeBehaviorEventV2(
        const std::string& event_id,
        const base::StringPairs& params) = 0;

    // 数据平台V2版本数据协议的埋点，用的/log/web接口，目前专用于apm数据的上报
    virtual RequestProxy ReportLivehimeApmEventV2(
        const std::string& event_data) = 0;

    // 通过服务端进行埋点
    virtual RequestProxy ReportLivehimeBehaviorEventViaSrv(
        LivehimeViaServerBehaviorEvent event_id,
        const base::StringPairs& params) = 0;

    virtual RequestProxy ReportLivehimeBehaviorEventViaSrvNew(
        LivehimeViaServerBehaviorEventNew event_id,
        const base::StringPairs& params) = 0;

    // 北极星埋点（目前没有埋点重试逻辑，仅仅只是将埋点往北极星logid报而已）
    virtual RequestProxy ReportLivehimeBehaviorEventPolaris(
        LivehimePolarisBehaviorEvent event_id,
        const base::StringPairs& params) = 0;
    virtual RequestProxy ReportLivehimeBehaviorEventPolaris(
        const std::string& event_id,
        const base::StringPairs& params) = 0;

    // 海外数平埋点（具体实现方式待后面数平方确认）
    virtual RequestProxy ReportLivehimeIntlBehaviorEvent(
        LivehimeIntlBehaviorEvent event_id,
        const base::StringPairs& params) = 0;

    virtual RequestProxy ReportLivehimeIntlBehaviorEvent(
        const std::string& event_id,
        const base::StringPairs& params) = 0;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_EVENT_TRACKING_SERVICE_H_
