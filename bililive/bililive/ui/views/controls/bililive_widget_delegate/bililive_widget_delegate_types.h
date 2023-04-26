#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_TYPES_H_
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_TYPES_H_

#include "base/callback.h"

// ģ̬��ر�ʱ��ִ�н���ص���IDOK��IDCANCEL...
typedef base::Callback<void(int, void*)> EndDialogClosure;

// ģ̬�����ص������ṹ
struct EndDialogSignalHandler
{
    enum class DataType
    {
        DataType_Unknown = 0,
        DataType_SceneItemCmdHandlerParam
    };
    EndDialogClosure closure;
    DataType data_type = DataType::DataType_Unknown;
    void* data = nullptr;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_H_