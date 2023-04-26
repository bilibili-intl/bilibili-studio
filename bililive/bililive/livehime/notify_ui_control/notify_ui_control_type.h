#pragma once

#include "base/callback.h"

namespace livehime
{
    // UI��ʾ�㼶���ȼ�
    enum class NotifyUILayer
    {
        LiveRoomCover,
        NewLiveRoomStyle,
        CpmGuide,
        //CpmExpire,
        ActivityView,

        Other,
    };

    // UI��ʾԪ�عر�֪ͨ�ص�
    // UI��Ͷ�������ʱ����Ҫ�����ɿ�������ִ��NotifyUIShowClosure��ʱ��
    // ���������Լ���OnNotifyUIClose��������ȥ��Ҫ������ʾ�㼶���Ƶ�UIԪ��Ӧ��������ҵ���߼���ʵ����ʱ
    // �ص����NotifyUICloseClosure���Ա����������ִ����һ���㼶��UIչʾ����
    typedef base::Callback<void(void)> NotifyUICloseClosure;

    // UI��ʾԪ��չʾ�����ص�
    typedef base::Callback<void(NotifyUICloseClosure close_callback)> NotifyUIShowClosure;
}
