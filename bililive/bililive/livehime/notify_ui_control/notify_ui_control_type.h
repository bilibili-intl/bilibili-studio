#pragma once

#include "base/callback.h"

namespace livehime
{
    // UI提示层级优先级
    enum class NotifyUILayer
    {
        LiveRoomCover,
        NewLiveRoomStyle,
        CpmGuide,
        //CpmExpire,
        ActivityView,

        Other,
    };

    // UI提示元素关闭通知回调
    // UI绑定投递任务的时候不需要传，由控制器在执行NotifyUIShowClosure的时候
    // 将控制器自己的OnNotifyUIClose函数传过去，要加入提示层级控制的UI元素应该在自身业务逻辑切实结束时
    // 回调这个NotifyUICloseClosure，以便控制器继续执行下一个层级的UI展示任务
    typedef base::Callback<void(void)> NotifyUICloseClosure;

    // UI提示元素展示函数回调
    typedef base::Callback<void(NotifyUICloseClosure close_callback)> NotifyUIShowClosure;
}
