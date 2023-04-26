#include "bililive_native_widget.h"



BililiveNativeWidgetWin::BililiveNativeWidgetWin(views::internal::NativeWidgetDelegate* delegate)
    : views::NativeWidgetWin(delegate)
    , inactive_operate_(WO_NONE)
    , escape_operate_(WO_CLOSE)
    , isc_(views::ISC_NEARBY)
{
}

bool BililiveNativeWidgetWin::PreHandleMSG(UINT message,
    WPARAM w_param,
    LPARAM l_param,
    LRESULT *result)
{
    switch (message)
    {
    case WM_ACTIVATE:
    {
        if (LOWORD(w_param) == WA_INACTIVE)
        {
            views::Widget *widget = GetWidget();
            if (widget && widget->IsVisible())
            {
                switch (inactive_operate_)
                {
                case WO_CLOSE:
                    widget->Close();
                    break;
                case WO_HIDE:
                    widget->Hide();
                    break;
                default:
                    break;
                }
            }
        }
    }
        break;
    case WM_KEYDOWN:
        if (w_param == VK_ESCAPE)
        {
            views::Widget *widget = GetWidget();
            if (widget && widget->IsVisible())
            {
                switch (escape_operate_)
                {
                case WO_CLOSE:
                    widget->Close();
                    break;
                case WO_HIDE:
                    widget->Hide();
                    break;
                default:
                    break;
                }
            }
        }
        break;
    case WM_DISPLAYCHANGE:
    {
        views::InsureWidgetVisible(GetWidget(), isc_);
    }
        break;
    default:
        break;
    }
    return views::NativeWidgetWin::PreHandleMSG(message, w_param, l_param, result);
}

bool BililiveNativeWidgetWin::GetClientAreaInsets(gfx::Insets *insets) const
{
    // 这里指定非客户区大小。
    // 返回 false 表示由底层指定大小。如果返回 false，insets 必须指定全0，否则底层会 DCHECK。
    // POPUP 窗口需要返回 false 以防止一些问题（比如打开窗口时闪现一个预料之外的颜色）
    if (IsUsingCustomFrame()) {
        return false;
    } else {
        insets->Set(0, 0, -1, 0);
        return true;
    }
}

void BililiveNativeWidgetWin::HandleFrameChanged() {

}