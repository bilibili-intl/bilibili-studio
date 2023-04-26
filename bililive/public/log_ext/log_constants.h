#pragma once

#include "base/strings/stringprintf.h"

// Scope	事件	参数	时机
// 基础信息（全生命周期）
// blink_event_app_launch	            机型、系统版本、应用版本 | 启动时
// blink_event_net_status	            运营商、网络（wifi、4G、3G、无网络） | 网络状态发生变化时
// blink_event_enterforeground          进入前台
// blink_event_enterbackground          进入后台
// blink_event_page_change	            类名、H5 Url | 页面切换
// blink_event_app_interrupt	        打断原因（电话） | 应用被打断
// 
// 开播
// blink_event_live_start	            开播类型	开始直播
// blink_event_live_stop	            关播原因（用户操作、非用户操作）	结束直播
// blink_event_clarity_change	        切换原因（手动、自动）、分辨率、码率、帧率	清晰度切换
// 
// 推流
// blink_event_stream_start	        推流地址、分辨率、码率、类型（RTMP、SRT）	开始推流
// blink_event_stream_success		    推流成功
// blink_event_stream_preview_pause	预览暂停
// blink_event_stream_preview_resume	预览恢复
// blink_event_stream_stop		        停止推流
// blink_event_stream_fail	            错误码	推流异常
// blink_event_stream_speed_except		推流速率小于50kbps（第一次低于这个值打印）
// blink_event_stream_pushurl_change	切换原因（网络切换、开播中断）	推流地址变化
// 
// 连麦
// blink_event_link_start	            连麦业务类型	开始连麦流程
// blink_event_link_accept		        接收连麦
// blink_event_link_stop		        停止连麦
// blink_event_link_fail	            错误码	连麦失败
// blink_event_link_except	            异常原因（电话终端、前后台 、弱网）	连麦异常
// 
// 美颜
// blink_event_beauty_start	        全量美颜参数（美颜、美妆、滤镜、贴纸、曝光）	开播的时候
// blink_event_beauty_change	        全量美颜参数（美颜、美妆、滤镜、贴纸、曝光）	调整美颜生效
// 
// 录屏 / 投屏
// blink_event_recordscreen_start	    麦克风开关（？）	开始录屏
// blink_event_recordscreen_stop		停止录屏
// blink_event_recordscreen_fail	    失败原因	录屏失败
// blink_event_transferscreen_start	投屏参数（分辨率、码率、帧率）	开始投屏
// blink_event_transferscreen_stop		停止投屏
// blink_event_transferscreen_except	错误码	投屏异常



namespace app_log {

    extern const char kLogAppLaunch[];
    extern const char kLogNetStatus[];
    extern const char kLogEnterForeground[];
    extern const char kLogEnterBackground[];

    extern const char kLogLiveStart[];
    extern const char kLogLiveStop[];
    extern const char kLogClarityChange[];

    extern const char kLogStreamStart[];
    extern const char kLogStreamSuccess[];
    extern const char kLogStreamStop[];
    extern const char kLogStreamFail[];
    extern const char kLogStreamSpeedExcept[];
    extern const char kLogStreamPushurlChange[];

    extern const char kLogLinkStart[];
    extern const char kLogLinkAccept[];
    extern const char kLogLinkStop[];
    extern const char kLogLinkFail[];
    extern const char kLogLinkExcept[];

    extern const char kLogBeautyStart[];
    extern const char kLogBeautyChange[];

    extern const char kLogRecordScreenStart[];
    extern const char kLogRecordScreenStop[];
    extern const char kLogRecordScreenFail[];
    extern const char kLogTransferScreenStart[];
    extern const char kLogTransferScreenStop[];
    extern const char kLogTransferScreenExcept[];
}

