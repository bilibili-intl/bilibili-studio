#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_TYPES_H_
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_TYPES_H_

#include "base/callback.h"

// 模态框关闭时的执行结果回调，IDOK、IDCANCEL...
typedef base::Callback<void(int, void*)> EndDialogClosure;

// 模态弹窗回调参数结构
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