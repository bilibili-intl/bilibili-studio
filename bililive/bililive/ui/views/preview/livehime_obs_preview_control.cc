#include "bililive/bililive/ui/views/preview/livehime_obs_preview_control.h"

#include <windowsx.h>

#include "ui/base/events/event_utils.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/screen.h"
#include "ui/views/controls/native/native_view_host.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"

#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"


    // LRESULT OnMessageRangeHandlerEX(UINT uMsg, WPARAM wParam, LPARAM lParam)
#define OBSPREVIEW_MESSAGE_RANGE_HANDLER_EX(msgFirst, msgLast, func) \
    if(uMsg >= msgFirst && uMsg <= msgLast) \
    { \
        lResult = func(uMsg, wParam, lParam); \
        bHandled = TRUE; \
    }

// LRESULT OnMessageHandlerEX(UINT uMsg, WPARAM wParam, LPARAM lParam)
#define OBSPREVIEW_MESSAGE_HANDLER_EX(msg, func) \
    if(uMsg == msg) \
    { \
        lResult = func(uMsg, wParam, lParam); \
        bHandled = TRUE; \
    }


// -*- LivehimeOBSPreviewControl -*-
LivehimeOBSPreviewControl::LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType preview_type)
    : preview_type_(preview_type)
    , obs_preview_(nullptr)
    , class_name_(L"LivehimeChildWindow")
{
    set_focusable(true);
    set_focus_border(nullptr);
}

LivehimeOBSPreviewControl::~LivehimeOBSPreviewControl()
{
}

void LivehimeOBSPreviewControl::SetClearOnly(bool clear_only) {
    is_clear_only_ = clear_only;
    if (obs_preview_) {
        obs_preview_->SetClearOnly(clear_only);
    }
}

void LivehimeOBSPreviewControl::SetRenderEnabled(bool enabled) {
    is_render_enabled_ = enabled;
    if (obs_preview_) {
        obs_preview_->OnOBSPreviewControlVisibility(enabled);
    }
}

void LivehimeOBSPreviewControl::SetBackgroundColor(float r, float g, float b)
{
    if (obs_preview_)
    {
        obs_preview_->SetBackgroundColor(r, g, b);
    }
}

void LivehimeOBSPreviewControl::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    __super::ViewHierarchyChanged(details);

    if (details.child == this)
    {
        if(details.is_add)
        {
            InitViews();
        }
        else
        {
            UninitViews();
        }
    }
}

void LivehimeOBSPreviewControl::InitViews()
{
    DCHECK(!obs_preview_);
    obs_preview_ = OBSProxyService::GetInstance().obs_ui_proxy()->ConnectOBSPReview(this, preview_type_);
    obs_preview_->SetClearOnly(is_clear_only_);
}

void LivehimeOBSPreviewControl::UninitViews()
{
    if (obs_preview_)
    {
        obs_preview_->OnOBSPreviewControlDestorying();

        OBSProxyService::GetInstance().obs_ui_proxy()->DisonnectOBSPReview(this);

        obs_preview_ = nullptr;
    }
}

void LivehimeOBSPreviewControl::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    if (obs_preview_)
    {
        obs_preview_->OnOBSPreviewControlSizeChanged(bounds());
    }
}

void LivehimeOBSPreviewControl::VisibilityChanged(View* starting_from, bool is_visible) {
    __super::VisibilityChanged(starting_from, is_visible);

    if (obs_preview_) {
        BOOL is_window_visible = ::IsWindowVisible(GetNativeControlHWND());
        obs_preview_->OnOBSPreviewControlVisibility(
            is_window_visible != 0 && is_visible && is_render_enabled_);
    }
}

gfx::NativeCursor LivehimeOBSPreviewControl::GetCursor(const ui::MouseEvent& event)
{
    if (obs_preview_ &&
        (obs_proxy_ui::PreviewType::PREVIEW_MAIN == preview_type_ /*||
            obs_proxy_ui::PreviewType::PREVIEW_COLIVE == preview_type_*/))
    {
        return ::LoadCursor(nullptr, MAKEINTRESOURCE(obs_preview_->GetHTComponent(event.location())));
    }
    return __super::GetCursor(event);
}

HWND LivehimeOBSPreviewControl::CreateNativeControl(HWND parent_container)
{
    static bool bRegister = false;
    if (!bRegister) {
        WNDCLASS wndclass;
        wndclass.style = CS_DBLCLKS; // 窗口类型0为缺省类型，2021/10/29改成CS_DBLCLKS，接受鼠标双击事件，以便弹出属性设置界面
        wndclass.lpfnWndProc = LivehimeObsPreviewWndProc; // 定义窗口处理函数
        wndclass.cbClsExtra = 0; // 窗口类无扩展
        wndclass.cbWndExtra = 0; // 窗口实例无扩展
        wndclass.hInstance = ::GetModuleHandle(nullptr); // 当前实例句柄
        wndclass.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);// 窗口的最小化图标为缺省图标
        wndclass.hCursor = ::LoadCursor(nullptr, IDC_ARROW); // 窗口采用箭头光标
        wndclass.hbrBackground = static_cast<HBRUSH>(::GetStockObject(NULL_BRUSH)); // 窗口背景
        wndclass.lpszMenuName = nullptr; // 窗口无菜单
        wndclass.lpszClassName = class_name_.c_str(); // 窗口类名
        if (::RegisterClass(&wndclass)) {
            bRegister = true;
        }
    }

    DWORD dwStyle = WS_VISIBLE | WS_CHILD;
    auto hWnd = ::CreateWindowExW(
        GetAdditionalExStyle(), class_name_.c_str(), L"",
        dwStyle, 0, 0, 0, 0, parent_container, nullptr, GetModuleHandle(nullptr), this);

    return hWnd;
}

void LivehimeOBSPreviewControl::OnDestroy()
{
    UninitViews();

    ::UnregisterClassW(class_name_.c_str(), GetModuleHandle(nullptr));
}

LRESULT LivehimeOBSPreviewControl::OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    switch (uMsg) {
    case WM_WINDOWPOSCHANGED:
        if (obs_preview_) {
            BOOL is_window_visible = ::IsWindowVisible(GetNativeControlHWND());
            obs_preview_->OnOBSPreviewControlVisibility(
                is_window_visible != 0 && visible() && is_render_enabled_);
        }
        break;

    default:
        break;
    }

    return __super::OnReflectMsg(uMsg, wParam, lParam, bHandled);
}

LRESULT LivehimeOBSPreviewControl::OnMouseRange(UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
    case WM_SETCURSOR:
    {
        POINT point = gfx::Screen::GetScreenFor(GetNativeControlHWND())->GetCursorScreenPoint().ToPOINT();
        ::ScreenToClient(GetNativeControlHWND(), &point);
        ui::MouseEvent mouse_event(ui::EventType::ET_MOUSE_MOVED,
                                   gfx::Point(point), gfx::Point(point),
                                   ui::EventFlags::EF_NONE);
        ::SetCursor(GetCursor(mouse_event));
        return TRUE;
    }
        break;
    case WM_LBUTTONDOWN:
    {
        RequestFocus();
        ::SetCapture(GetNativeControlHWND());
    }
        break;
    case WM_LBUTTONUP:
    {
        if (::GetCapture() == GetNativeControlHWND())
        {
            ::ReleaseCapture();
        }
    }
        break;
    default:
        break;
    }

    if(obs_preview_)
    {
        MSG msg = { GetNativeControlHWND(), message, w_param, l_param, GetMessageTime(),
                { LOWORD(l_param), HIWORD(l_param)} };
        POINT point = { GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) };

        if ((message != WM_MOUSEWHEEL) && (message != WM_MOUSEHWHEEL))
        {
            // 直接以MSG构造MouseEvent的话框架会自己进行dip转换，
            // 而针对预览区的鼠标事件处理我们一律采用的是设备坐标，所以直接用设备坐标来构造
            ui::MouseEvent mouse_event(ui::EventTypeFromNative(msg),
                                       gfx::Point(point), gfx::Point(point),
                                       ui::EventFlagsFromNative(msg));

            obs_preview_->OnOBSPreviewControlMouseEvent(&mouse_event);
        }
        else
        {
            // MouseWheel的鼠标坐标是以屏幕左上角为坐标原点的，
            // 新增的预览区鼠标滚轮事件处理是以预览区为坐标参照系，采用设备坐标，
            // 此时需要ScreeenToClient
            ::ScreenToClient(GetNativeControlHWND(), &point);

            ui::MouseEvent mouse_event(ui::EventType::ET_MOUSEWHEEL,
                                       gfx::Point(point), gfx::Point(point),
                                       ui::EventFlagsFromNative(msg));

            int zDelta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
            int xDelta = 0;
            int yDelta = 0;
            if (message == WM_MOUSEHWHEEL)
            {
                xDelta = -zDelta;
            }
            else
            {
                yDelta = zDelta;
            }

            ui::MouseWheelEvent mouse_wheel_event(mouse_event, xDelta, yDelta);
            obs_preview_->OnOBSPreviewControlMouseWheelEvent(&mouse_wheel_event);
        }
    }

    return S_OK;
}

LRESULT LivehimeOBSPreviewControl::OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param)
{
    if (obs_preview_)
    {
        MSG msg = { GetNativeControlHWND(), message, w_param, l_param, GetMessageTime() };
        ui::KeyEvent key_event(msg, message == WM_CHAR);

        if (key_event.type() == ui::EventType::ET_KEY_PRESSED)
        {
            if (VK_ESCAPE == key_event.key_code())
            {
                CancelDragSceneItem();
            }
        }

        obs_preview_->OnOBSPreviewControlKeyEvent(&key_event);
    }
    return S_OK;
}

LRESULT LivehimeOBSPreviewControl::OnFocusEvent(UINT message, WPARAM w_param, LPARAM l_param)
{
    if (obs_preview_)
    {
        obs_preview_->OnOBSPreviewControlFocusEvent(message == WM_SETFOCUS);
    }
    return S_OK;
}

void LivehimeOBSPreviewControl::CancelDragSceneItem()
{
    if (obs_preview_)
    {
        gfx::Point pt = gfx::Screen::GetScreenFor(GetNativeControlHWND())->GetCursorScreenPoint();
        OnMouseRange(WM_LBUTTONUP, 0, MAKELPARAM(pt.x(), pt.y()));
    }
}

HWND LivehimeOBSPreviewControl::GetNativeHWND()
{
    return GetNativeControlHWND();
}

gfx::Rect LivehimeOBSPreviewControl::GetViewRect()
{
    return bounds();
}

LRESULT CALLBACK LivehimeOBSPreviewControl::MessageHandler(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    BOOL bHandled = FALSE;

    switch (uMsg) {
    case WM_RBUTTONUP:
    {
        // 直接走默认的右键处理过程，不然不会响应右键菜单弹出WM_CONTEXTMENU消息
        bHandled = TRUE;
        return DefWindowProcW(GetNativeControlHWND(), uMsg, wParam, lParam);
    }
    case WM_LBUTTONDBLCLK:
    {
        if (preview_type_ == obs_proxy_ui::PreviewType::PREVIEW_MAIN)
        {
            //双击弹出属性设置界面
            //LOG(INFO) << "MessageHandler WM_LBUTTONDBLCLK";
            int command_id = CommandBililiveLivehime::IDC_LIVEHIME_SCENE_ITEM_PROPERTY;
            base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_SCENE_ITEM_CMD_EXECUTED,
                base::NotificationService::AllSources(),
                base::Details<int>(&command_id));
        }
        return DefWindowProcW(GetNativeControlHWND(), uMsg, wParam, lParam);
    }break;
    case WM_KILLFOCUS:
    {
        // 拖拽过程中丢失焦点就取消拖拽
        CancelDragSceneItem();

        return lResult;
    }
    case WM_ERASEBKGND:
        return 1;

    default:
        break;
    }

    // Range handlers must go first!
    OBSPREVIEW_MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseRange);
    OBSPREVIEW_MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE, WM_NCXBUTTONDBLCLK, OnMouseRange);

    // Mouse events.
    OBSPREVIEW_MESSAGE_HANDLER_EX(WM_SETCURSOR, OnMouseRange);
    OBSPREVIEW_MESSAGE_HANDLER_EX(WM_MOUSELEAVE, OnMouseRange);
    OBSPREVIEW_MESSAGE_HANDLER_EX(WM_NCMOUSELEAVE, OnMouseRange);

    // Key events.
    OBSPREVIEW_MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST, WM_KEYLAST, OnKeyEvent);

    // focus
    OBSPREVIEW_MESSAGE_HANDLER_EX(WM_SETFOCUS, OnFocusEvent);
    OBSPREVIEW_MESSAGE_HANDLER_EX(WM_KILLFOCUS, OnFocusEvent);

    if (bHandled) {
        return lResult;
    } else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK LivehimeOBSPreviewControl::LivehimeObsPreviewWndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 在 WM_CREATE 产生并返回之前，GetNativeControlHWND() 获取不到窗口句柄
    if (uMsg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto control = reinterpret_cast<LivehimeOBSPreviewControl*>(cs->lpCreateParams);
        if (!control) {
            DCHECK(false);
            return FALSE;
        }

        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(control));
        return control->MessageHandler(hWnd, uMsg, wParam, lParam);
    }

    auto control = reinterpret_cast<LivehimeOBSPreviewControl*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (control) {
        return control->MessageHandler(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}