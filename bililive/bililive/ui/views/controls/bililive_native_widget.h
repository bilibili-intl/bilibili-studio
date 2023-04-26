#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_NATIVEWIDGETWIN_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_NATIVEWIDGETWIN_H

#include "ui/views/widget/widget.h"
#include "ui/views/widget/native_widget_win.h"
#include "ui/views/win/hwnd_message_handler.h"
#include "ui/views/controls/messagebox/message_box.h"


class BililiveNativeWidgetWin : public views::NativeWidgetWin
{
public:
    enum WidgetOperate
    {
        WO_NONE,
        WO_CLOSE,
        WO_HIDE,
    };

    explicit BililiveNativeWidgetWin(views::internal::NativeWidgetDelegate* delegate);
    virtual ~BililiveNativeWidgetWin() {}

    // 失去焦点后如何处理窗口
    void set_inactive_operate(WidgetOperate io) { inactive_operate_ = io; }
    WidgetOperate inactive_operate() const { return inactive_operate_; }

    // Escape如何处理窗口
    void set_escape_operate(WidgetOperate wo) { escape_operate_ = wo; }
    WidgetOperate escape_operate() const { return escape_operate_; }

    void set_insure_show_corner(views::InsureShowCorner isc) { isc_ = isc; }

protected:
    // Overridden from HWNDMessageHandlerDelegate:
    bool PreHandleMSG(UINT message,
        WPARAM w_param,
        LPARAM l_param,
        LRESULT *result) override;
    bool GetClientAreaInsets(gfx::Insets *insets) const override;
    void HandleFrameChanged() override;
    
private:
    WidgetOperate inactive_operate_;
    WidgetOperate escape_operate_;
    views::InsureShowCorner isc_;
};

#endif
