#ifndef BILILIVE_PUBLIC_BILILIVE_BILILIVE_NOTIFICATION_TYPES_H_
#define BILILIVE_PUBLIC_BILILIVE_BILILIVE_NOTIFICATION_TYPES_H_

#include "base/notification/notification_types.h"

namespace bililive
{
    enum NotificationType
    {
        NOTIFICATION_BILILIVE_START = base::NOTIFICATION_CONTENT_END + 1,

        NOTIFICATION_APP_TERMINATING,

        NOTIFICATION_EXTENSIONS_READY,
        NOTIFICATION_BILILIVE_THEME_CHANGED,

        NOTIFICATION_BILILIVE_DANMAKU_MSG,
        NOTIFICATION_BILILIVE_DANMAKU_STATUS,  /*1 for started, 0 for end and -1 for start error(detail type : int)*/
        NOTIFICATION_BILILIVE_AUDIENCES_COUNT,
        NOTIFICATION_BILILIVE_ATTENTION_COUNT,// 关注数有变化，各模块自行重载
        NOTIFICATION_BILILIVE_WIDGET_MESSAGE,
        NOTIFICATION_BILILIVE_DOGUIDE,
        NOTIFICATION_BILILIVE_DOGUIDE_CFG_CHECKED,  // 开播引导配置程序启动检测已完毕

        NOTIFICATION_BILILIVE_ROOM_LOCK,
        NOTIFICATION_BILILIVE_ROOM_END,
        NOTIFICATION_BILILIVE_STUDIO_END,
        NOTIFICATION_BILILIVE_END_WIN_TASK_VIEW_CLOSE,

        NOTIFICATION_BILILIVE_STREAMING_STATUS,   // 推流状态
        NOTIFICATION_BILILIVE_AUDIODEVICE_ACTIVATE,   // 音频设备添加
        NOTIFICATION_BILILIVE_AUDIODEVICE_DEACTIVATE,   // 音频设备移除

        NOTIFICATION_LIVEHIME_USERINFO_VIEW_SHOW,
        NOTIFICATION_LIVEHIME_USERINFO_VIEW_CLOSE,

        NOTIFICATION_LIVEHIME_RECORDING_STARTING,
        NOTIFICATION_LIVEHIME_START_RECORDING,
        NOTIFICATION_LIVEHIME_RECORDING_STOPPING,
        NOTIFICATION_LIVEHIME_STOP_RECORDING,
        NOTIFICATION_LIVEHIME_RECORDING_ERROR,

        NOTIFICATION_LIVEHIME_STREAMING_FEEDBACK,
        NOTIFICATION_LIVEHIME_STREAM_ENCODED_QP,

        NOTIFICATION_LIVEHIME_COLIVE_JOIN_CHANNEL_SUCCEEDED,   // 加入声网频道（连麦房间）成功
        NOTIFICATION_LIVEHIME_COLIVE_STARTING_ERROR,           // 连麦开始时（加入频道或开始推流时）出错
        NOTIFICATION_LIVEHIME_COLIVE_USER_JOINED,              // 有主播加入了房间
        NOTIFICATION_LIVEHIME_COLIVE_USER_OFFLINE,             // 有主播从房间离开
        NOTIFICATION_LIVEHIME_COLIVE_LEAVE_CHANNEL,            // 已离开声网频道（连麦房间）
        NOTIFICATION_LIVEHIME_COLIVE_PRE_REMOTE_VIDEO_DISPLAY, // 准备显示对方画面
        NOTIFICATION_LIVEHIME_COLIVE_CONNECTION_INTERRUPTED,   // 与连麦服务器断开，会自动重试
        NOTIFICATION_LIVEHIME_COLIVE_CONNECTION_LOST,          // 与连麦服务器断开，会自动重试
        NOTIFICATION_LIVEHIME_COLIVE_CONNECTION_BANNED,        // 被连麦服务器禁止连接
        NOTIFICATION_LIVEHIME_COLIVE_STREAM_PUBLISHED,         // 通知主播推流成功
        NOTIFICATION_LIVEHIME_COLIVE_STREAM_MESSAGE_ERROR,     // 接收对方数据流消息错误
        NOTIFICATION_LIVEHIME_COLIVE_SDK_ERROR,                // 连麦 SDK 出错
        NOTIFICATION_LIVEHIME_COLIVE_AUDIO_LEVEL,              // 监听语聊房用户声音变化

        NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_DISABLE,           // 禁用系统音捕捉
        NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_ENABLE,            // 启用系统音捕捉

        NOTIFICATION_LIVEHIME_DYNAMIC_DATA,        //动态配置下载(数据)
        NOTIFICATION_LIVEHIME_DYNAMIC_FILE,        //动态配置下载(文件)

        NOTIFICATION_LIVEHIME_TABAREA_BANNED_SHOW,
        NOTIFICATION_LIVEHIME_TABAREA_BANNED_CLOSE,
        NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_SHOW,
        NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_CLOSE,
        NOTIFICATION_LIVEHIME_TABAREA_SEARCH_SHOW,
        NOTIFICATION_LIVEHIME_TABAREA_SEARCH_CLOSE,
        NOTIFICATION_LIVEHIME_TABAREA_BLACKLIST_APPENDING, // 将添加指定用户到黑名单
        NOTIFICATION_LIVEHIME_TABAREA_BLACKLIST_APPENDED, // 指定用户已被添加到黑名单
        NOTIFICATION_LIVEHIME_TABAREA_REPORT,             // 举报弹幕

        //NOTIFICATION_LIVEHIME_SCENE_HAS_SELECTED_ITEM,     // 当前场景中是否有源项被选中
        NOTIFICATION_LIVEHIME_SCENE_ITEM_CMD_EXECUTED,     // 与场景源项操作相关的命令执行

        NOTIFICATION_LIVEHIME_DANMAKU_RELOADPREF,       // 弹幕姬重新加载配置
        NOTIFICATION_LIVEHIME_TABAREA_SEARCH,           // 搜索弹幕
        NOTIFICATION_LIVEHIME_ROOM_ADMIN_ADD,
        NOTIFICATION_LIVEHIME_ROOM_ADMIN_APPENDED,

        NOTIFICATION_LIVEHIME_GIFTBOX_OPEN,             // 礼物盒子打开了
        NOTIFICATION_LIVEHIME_GIFTBOX_CLOSE,             // 礼物盒子关闭了
        NOTIFICATION_LIVEHIME_GIFTBOX_NEWGIFT,             // 新到礼物统计完了

        NOTIFICATION_LIVEHIME_UPDATE_AVATAR,
        NOTIFICATION_LIVEHIME_UPDATE_SAN,
        NOTIFICATION_LIVEHIME_UPDATE_CAPTAIN,
        NOTIFICATION_LIVEHIME_GET_AREALIST_FAIL,
        NOTIFICATION_LIVEHIME_GET_AREALIST_SUCCESS,
        NOTIFICATION_LIVEHIME_ROOMINFO_UPDATING,        // 房间信息更新中，即将调接口之前
        NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_SUCCESS,  // 房间信息更新成功
        NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_FAIL,     // 房间信息更新失败

        NOTIFICATION_LIVEHIME_PARTITION_BUTTON_SELECTED,    // 分区按钮选中
        NOTIFICATION_LIVEHIME_PARTITION_TAG_BUTTON_SELECTED,    // 分区TAG按钮选中
        NOTIFICATION_LIVEHIME_PARTITION_TAG_BUTTON_UNSELECTED,    // 分区TAG按钮反选

        NOTIFICATION_LIVEHIME_VCONN_RECORD_PANEL_SHOW,
        NOTIFICATION_LIVEHIME_VCONN_RECORD_PANEL_CLOSE,
        NOTIFICATION_LIVEHIME_VCONN_CANDIDATE_PANEL_SHOW,
        NOTIFICATION_LIVEHIME_VCONN_CANDIDATE_PANEL_CLOSE,
        NOTIFICATION_LIVEHIME_VCONN_SETTINGS_PANEL_SHOW,
        NOTIFICATION_LIVEHIME_VCONN_SETTINGS_PANEL_CLOSE,
        NOTIFICATION_LIVEHIME_VCONN_INVITE_USER,

        NOTIFICATION_LIVEHIME_VIDEO_PK_SETTINGS_PANEL_SHOW,
        NOTIFICATION_LIVEHIME_VIDEO_PK_SETTINGS_PANEL_CLOSE,
        NOTIFICATION_LIVEHIME_VIDEO_PK_INITIAL_PANEL_CLOSE,
        NOTIFICATION_LIVEHIME_VIDEO_PK_TOAST_CLOSE,
        NOTIFICATION_LIVEHIME_VIDEO_PK_GUIDE_BUBBLE_CLOSE,

        NOTIFICATION_LIVEHIME_LOADING_ROOMINFO,         // 加载用户房间信息ing
        NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_FAILED,     // 加载用户房间信息失败
        NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS,    // 房间信息拉取完成，需重新加载房间相关信息
        NOTIFICATION_LIVEHIME_MAYBE_USER_CONFUSED,      // 用户点了日志或反馈客服，此时可以把一些信息写到日志里
        NOTIFICATION_LIVEHIME_LIVEROOM_NOT_GRANT,       // 直播间尚未开通
        NOTIFICATION_LIVEHIME_LIVEROOM_NO_COVER,        // 直播间没封面
        NOTIFICATION_LIVEHIME_LIVEROOM_NO_AREA,         // 直播间没分区
        NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_FAILED,   // 创建直播间失败
        NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_SUCCESS,  // 创建直播间成功

        NOTIFICATION_LIVEHIME_DISABLE_START_LIVE,       // 禁用开始直播按钮（后续有多种情况需要使用这个通知应该通过参数来区分状况）
        NOTIFICATION_LIVEHIME_ENABLE_START_LIVE,        // 启用开始直播按钮（后续有多种情况需要使用这个通知应该通过参数来区分状况）
        NOTIFICATION_LIVEHIME_DISABLE_PARTITION_BTN,    // 禁用顶部工具栏分区按钮
        NOTIFICATION_LIVEHIME_ENABLE_PARTITION_BTN,     // 启用顶部工具栏分区按钮

        NOTIFICATION_LIVEHIME_SET_TAGS,

        NOTIFICATION_LIVEHIME_LOTTERY_STATUS_CHANGED_R,  //收到H5的状态变化
        NOTIFICATION_LIVEHIME_LOTTERY_STATUS_CHANGED_S,  //发给H5的状态变化
        NOTIFICATION_LIVEHIME_LOTTERY_ANCHOR_LOT_CHECKSTATUS,  //收到的状态变更广播
        NOTIFICATION_LIVEHIME_LOTTERY_ANCHOR_LOT_END,  //收到的状态变更广播
        NOTIFICATION_LIVEHIME_LOTTERY_ANCHOR_LOT_AWARD,  //收到的状态变更广播

        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_INIT,   //初始化
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_UPDATE,   //点击左右箭头
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_NEW,    //收到新来的广播
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_DELETE, //删除或有item到期
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_REPORT, //举报成功
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_DETAILS, //点击item
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_CLOSE,   //itemview关闭
        NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_TIMEREPEAT, //计时器行为

        NOTIFICATION_LIVEHIME_ANCHOR_PROMOTION_UPDATE,
        NOTIFICATION_LIVEHIME_ANCHOR_PROMOTION_URL,

        NOTIFICATION_LIVEHIME_CPM_LAST_EFFECT,      // 最近一次星光投放效果通知
        NOTIFICATION_LIVEHIME_CPM_STARLIGHT_TRADING,// 星光币变更通知
        NOTIFICATION_LIVEHIME_CPM_NEW_REAL_TIME_ORDER, //新增实时订单

        NOTIFICATION_LIVEHIME_RANK_POPUP_WINDOW,    //开始弹主界面的层级弹窗

        NOTIFICATION_LIVEHIME_VIDEO_PK_BY_THIRD_PARTY_TIPS, //第三方指定pk强提示

        NOTIFICATION_LIVEHIME_VIDEO_PK_GOLD_PK_INVITING,  //赏金赛邀请
        NOTIFICATION_LIVEHIME_VIDEO_PK_GOLD_PK_MATCHING,  //赏金赛匹配

        NOTIFICATION_BILILIVE_END,
        NOTIFICATION_LIVEHIME_DANMAKU_VOICE_PLAY_STATE,
        NOTIFICATION_LIVEHIME_DANMAKU_UI_PANINT,

        NOTIFICATION_LIVEHIME_VIDEO_UPDATE_REMOTE_ANCHOR_VOICE,//关闭远端主播声音

        NOTIFICATION_LIVEHIME_TALK_TOPIC_UPDATE,            //参与话题变化
        NOTIFICATION_LIVEHIME_FIFTER_BROADCAST,            //供应H5订阅广播使用

        NOTIFICATION_LIVEHIME_WATCHED_CHANGE,            // 看过

        NOTIFICATION_LIVEHIME_VOICE_BROADCAST,          //开启语音弹幕功能
        NOTIFICATION_LIVEHIME_MODULE_ENTRANCE,          //模块加载完成
        NOTIFICATION_LIVEHIME_MULTI_VIDEO_CONN_STATUS_CHANGED,  //多人视频连线状态改变,开始or结束
    };

}   // namespace bililive

#endif  // BILILIVE_PUBLIC_BILILIVE_BILILIVE_NOTIFICATION_TYPES_H_
