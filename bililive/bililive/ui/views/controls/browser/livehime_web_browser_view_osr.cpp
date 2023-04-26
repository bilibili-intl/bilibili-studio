#include "livehime_web_browser_view_osr.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "ui/gfx/canvas.h"
#include "ui/views/painter.h"
#include "cef/cef_core/include/internal/cef_types.h"
#include "skia/include/effects/SkGradientShader.h"

namespace
{
    //gfx::ImageSkia                 g_image;
    //std::mutex                     g_mutex;
};

LivehimeWebBrowserViewOSR::LivehimeWebBrowserViewOSR(
    cef_proxy::client_handler_type browser_type,
    const gfx::Rect& browser_rect,
    int fps,
    const std::string& init_url,
    LivehimeWebBrowserDelegate* delegate,
    const std::string& post_data) :
    browser_type_(browser_type),
    url_(init_url),
    delegate_(delegate),
    post_data_(post_data),
    browser_rect_(browser_rect),
    fps_(fps),
    weakptr_factory_(this)
{
    InitViews();
}

LivehimeWebBrowserViewOSR::~LivehimeWebBrowserViewOSR()
{
    is_deleted_ = true;
    UninitViews();
}

void LivehimeWebBrowserViewOSR::NativeControlIgnoreViewVisibilityChanged(bool ignore, bool visible)
{
    native_ctrl_visible_ignore_view_visibility_changed_ = ignore;
    SetVisible(visible);
}

cef_proxy::browser_bind_data* LivehimeWebBrowserViewOSR::browser_bind_data() const
{
    if (bind_data_) {
        return bind_data_.get();
    }
    return nullptr;
}

const std::string& LivehimeWebBrowserViewOSR::GetURL()
{
    return url_;
}

cef_proxy::client_handler_type LivehimeWebBrowserViewOSR::GetBrowserType()
{
    return browser_type_;
}

const std::string& LivehimeWebBrowserViewOSR::GetPostData()
{
    return post_data_;
}

cef_proxy::browser_bind_data* LivehimeWebBrowserViewOSR::GetBindData()
{
    if (bind_data_) {
        return bind_data_.get();
    }
    return nullptr;
}

void LivehimeWebBrowserViewOSR::ResetBindData(__int64 id, cef_proxy::client_handler_type type)
{
    bind_data_.reset(new cef_proxy::browser_bind_data(id, type));
}

void LivehimeWebBrowserViewOSR::LoadURL(const std::string& url)
{
    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->LoadUrl(*GetBindData(), url);
    }
}

void LivehimeWebBrowserViewOSR::BrowserResize(const gfx::Rect& rect)
{
    browser_rect_ = rect;

    RECT rt;
    rt.left = browser_rect_.x();
    rt.right = browser_rect_.right();
    rt.top = browser_rect_.y();
    rt.bottom = browser_rect_.bottom();

    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->ResizeBrowser(*GetBindData(), rt);
    }
}

base::EventStatus LivehimeWebBrowserViewOSR::WillProcessEvent(const gfx::NativeEvent& event)
{
    if (receive_key_event_) {
        if (event.message == WM_KEYDOWN || event.message == WM_SYSKEYDOWN) {
            OnKeyPressed(event);
        }
    }

    return base::EventStatus::EVENT_CONTINUE;
}

void LivehimeWebBrowserViewOSR::DidProcessEvent(const gfx::NativeEvent& event)
{
}

void LivehimeWebBrowserViewOSR::OnLoadCefProxyCore(bool succeed)
{
    if (succeed)
    {
        ResetBindData(CefProxyWrapper::GenerateBindDataId(), GetBrowserType());

        std::string url = GetURL();

        base::Closure cb = base::Bind(&LivehimeWebBrowserViewOSR::ClosureFunction, weakptr_factory_.GetWeakPtr());
        cef_proxy::cef_proxy_osr_onpaint callback = std::bind(&LivehimeWebBrowserViewOSR::CefPaintCallBackOnWorkThread, this, cb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        if (GetBindData()) {
            CefProxyWrapper::GetInstance()->CreateOsrBrowser(*GetBindData(), browser_rect_, fps_, callback, url, GetPostData());
        }
    }
    else
    {
        OnWebBrowserCoreInvalid();
        LOG(WARNING) << "LivehimeWebBrowserViewOSR::OnLoadCefProxyCore Failed";
    }
}

void LivehimeWebBrowserViewOSR::OnPaint(gfx::Canvas* canvas)
{
    __super::OnPaint(canvas);

    if (paint_mutex.try_lock())
    {
        canvas->DrawImageInt(paint_image, 0, 0);
        paint_mutex.unlock();
    }
}

bool LivehimeWebBrowserViewOSR::OnMousePressed(const ui::MouseEvent& event)
{
    __super::OnMousePressed(event);

    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    cef_proxy::MouseButtonType type{ cef_proxy::MouseButtonType::MBT_LEFT };

    if (event.IsLeftMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_LEFT;
    }
    else if (event.IsRightMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_RIGHT;
    }
    else if (event.IsMiddleMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_MIDDLE;
    }

    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseClick(*GetBindData(), cef_proxy_mouse_event, type, false, 1);
    }
    return true;
}

bool LivehimeWebBrowserViewOSR::OnMouseDragged(const ui::MouseEvent& event)
{
    __super::OnMouseDragged(event);

    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    cef_proxy::MouseButtonType type{ cef_proxy::MouseButtonType::MBT_LEFT };

    if (event.IsLeftMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_LEFT;
    }
    else if (event.IsRightMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_RIGHT;
    }
    else if (event.IsMiddleMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_MIDDLE;
    }
    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseMove(*GetBindData(), cef_proxy_mouse_event, false);
    }
    return true;
}

void LivehimeWebBrowserViewOSR::OnMouseReleased(const ui::MouseEvent& event)
{
    __super::OnMouseReleased(event);

    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    cef_proxy::MouseButtonType type{ cef_proxy::MouseButtonType::MBT_LEFT };

    if (event.IsLeftMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_LEFT;
    }
    else if (event.IsRightMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_RIGHT;
    }
    else if (event.IsMiddleMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_MIDDLE;
    }
    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseClick(*GetBindData(), cef_proxy_mouse_event, type, true, 1);
    }
}

void LivehimeWebBrowserViewOSR::OnMouseCaptureLost()
{
    __super::OnMouseCaptureLost();
}

void LivehimeWebBrowserViewOSR::OnMouseMoved(const ui::MouseEvent& event)
{
    __super::OnMouseMoved(event);

    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    cef_proxy::MouseButtonType type{ cef_proxy::MouseButtonType::MBT_LEFT };

    if (event.IsLeftMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_LEFT;
    }
    else if (event.IsRightMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_RIGHT;
    }
    else if (event.IsMiddleMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_MIDDLE;
    }
    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseMove(*GetBindData(), cef_proxy_mouse_event, false);
    }
}

void LivehimeWebBrowserViewOSR::OnMouseEntered(const ui::MouseEvent& event)
{
    __super::OnMouseEntered(event);

    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    cef_proxy::MouseButtonType type{ cef_proxy::MouseButtonType::MBT_LEFT };

    if (event.IsLeftMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_LEFT;
    }
    else if (event.IsRightMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_RIGHT;
    }
    else if (event.IsMiddleMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_MIDDLE;
    }
    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseMove(*GetBindData(), cef_proxy_mouse_event, false);
    }
}

void LivehimeWebBrowserViewOSR::OnMouseExited(const ui::MouseEvent& event)
{
    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    cef_proxy::MouseButtonType type{ cef_proxy::MouseButtonType::MBT_LEFT };

    if (event.IsLeftMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_LEFT;
    }
    else if (event.IsRightMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_RIGHT;
    }
    else if (event.IsMiddleMouseButton())
    {
        type = cef_proxy::MouseButtonType::MBT_MIDDLE;
    }
    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseMove(*GetBindData(), cef_proxy_mouse_event, true);
    }
}

bool IsKeyDown(WPARAM wparam) {
    return (GetKeyState(wparam) & 0x8000) != 0;
}

bool IsKeyChar(WPARAM wparam, LPARAM lparam) {
    // 0--9
    if (wparam >= ui::KeyboardCode::VKEY_0 &&
        wparam <= ui::KeyboardCode::VKEY_9) {
        return true;
    }

    // a-z
    if (wparam >= 'a' && wparam <= 'z') {
        return true;
    }

    // A-Z
    if (wparam >= ui::KeyboardCode::VKEY_A &&
        wparam <= ui::KeyboardCode::VKEY_Z) {
        return true;
    }

    // VK_NUMPAD0-VK_NUMPAD9
    if (wparam >= ui::KeyboardCode::VKEY_NUMPAD0 &&
        wparam <= ui::KeyboardCode::VKEY_NUMPAD9) {
        return true;
    }

    return false;
}

int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
    int modifiers = 0;
    if (IsKeyDown(VK_SHIFT))
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (IsKeyDown(VK_CONTROL))
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (IsKeyDown(VK_MENU))
        modifiers |= EVENTFLAG_ALT_DOWN;

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    if (::GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;

    switch (wparam) {
    case VK_RETURN:
        if ((lparam >> 16) & KF_EXTENDED)
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
        //case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        if (!((lparam >> 16) & KF_EXTENDED))
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
        modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_SHIFT:
        if (IsKeyDown(VK_LSHIFT))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (IsKeyDown(VK_RSHIFT))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_CONTROL:
        if (IsKeyDown(VK_LCONTROL))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (IsKeyDown(VK_RCONTROL))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_MENU:
        if (IsKeyDown(VK_LMENU))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (IsKeyDown(VK_RMENU))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_LWIN:
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_RWIN:
        modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    }

    return modifiers;
}

void LivehimeWebBrowserViewOSR::OnKeyPressed(const gfx::NativeEvent& event)
{
    cef_proxy::KeyEvent cef_proxy_key_event = {};
    cef_proxy_key_event.modifiers = GetCefKeyboardModifiers(
        event.wParam, event.lParam);

    if (IsKeyChar(event.wParam, event.lParam)) {
        // A-Z
        if (event.wParam >= ui::KeyboardCode::VKEY_A &&
            event.wParam <= ui::KeyboardCode::VKEY_Z) {
            if (!(::GetKeyState(VK_CAPITAL) & 1)) {
                cef_proxy_key_event.windows_key_code = event.wParam + 32;
            }
            else {
                cef_proxy_key_event.windows_key_code = event.wParam;
            }
        }
        else if (event.wParam >= ui::KeyboardCode::VKEY_NUMPAD0 &&
            event.wParam <= ui::KeyboardCode::VKEY_NUMPAD9) {
            cef_proxy_key_event.windows_key_code = event.wParam - 48;
        }
        else {
            cef_proxy_key_event.windows_key_code = event.wParam;
        }
    }
    else {
        cef_proxy_key_event.windows_key_code = event.wParam;
    }

    cef_proxy_key_event.native_key_code = event.lParam;
    cef_proxy_key_event.is_system_key =
        event.message == WM_SYSCHAR ||
        event.message == WM_SYSKEYDOWN ||
        event.message == WM_SYSKEYUP;

    if (IsKeyChar(event.wParam, event.lParam)) {
        cef_proxy_key_event.type = cef_proxy::KeyEventTpye::KEYEVENT_CHAR;
    }
    else if (event.message == WM_KEYDOWN ||
        event.message == WM_SYSKEYDOWN) {
        cef_proxy_key_event.type = cef_proxy::KeyEventTpye::KEYEVENT_RAWKEYDOWN;
    }
    else if (event.message == WM_KEYUP ||
        event.message == WM_SYSKEYUP) {
        cef_proxy_key_event.type = cef_proxy::KeyEventTpye::KEYEVENT_KEYUP;
    }

    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendKeyClick(*GetBindData(), cef_proxy_key_event);
    }
}

bool LivehimeWebBrowserViewOSR::GetString(HIMC imc,
    WPARAM lparam,
    int type,
    std::wstring& result) {
    if (!(lparam & type))
        return false;

    LONG string_size = ::ImmGetCompositionString(imc, type, NULL, 0);
    if (string_size <= 0)
        return false;

    // For trailing NULL - ImmGetCompositionString excludes that.
    string_size += sizeof(WCHAR);
    std::vector<wchar_t> buffer(string_size);
    ::ImmGetCompositionString(imc, type, &buffer[0], string_size);
    result = &buffer[0];
    return true;
}

bool LivehimeWebBrowserViewOSR::GetResult(HWND hwnd, LPARAM lparam, std::wstring& result) {
    bool ret = false;
    HIMC imc = ::ImmGetContext(hwnd);
    if (imc) {
        ret = GetString(imc, lparam, GCS_RESULTSTR, result);
        ::ImmReleaseContext(hwnd, imc);
    }

    return ret;
}

bool LivehimeWebBrowserViewOSR::OnIMEMessage(const MSG event, LRESULT* result) {
    if (!this->visible()) {
        return false;
    }

    //LOG(INFO) << "LivehimeWebBrowserViewOSR::OnIMEMessage: wParam="
    //	<< event.wParam << ", lParam=" << event.lParam;

  switch (event.message) {
  case WM_IME_STARTCOMPOSITION:
      break;
  case WM_IME_COMPOSITION:
      if (GetBindData()) {
          std::wstring cTextStr;
          if (GetResult(this->GetWidget()->GetNativeView(), event.lParam, cTextStr)) {
              LOG(INFO) << "LivehimeWebBrowserViewOSR::OnIMEMessage: Text=" << cTextStr;
              CefProxyWrapper::GetInstance()->ImeCommitText(*GetBindData(), cTextStr, INT_MAX, INT_MAX, 0);
          }
      }
      break;
  case WM_IME_ENDCOMPOSITION:
      if (GetBindData()) {
          CefProxyWrapper::GetInstance()->ImeCancelComposition(*GetBindData());
      }
      break;
  default:
      return false;
  }

  return true;
}

bool LivehimeWebBrowserViewOSR::OnMouseWheel(const ui::MouseWheelEvent& event)
{
    __super::OnMouseWheel(event);

    cef_proxy::MouseEvent cef_proxy_mouse_event;
    cef_proxy_mouse_event.x = event.x();
    cef_proxy_mouse_event.y = event.y();
    cef_proxy_mouse_event.modifiers = 0;

    if (GetBindData()) {
        CefProxyWrapper::GetInstance()->SendMouseWheel(*GetBindData(), cef_proxy_mouse_event, event.x_offset(), event.y_offset());
    }
    return true;
}

void LivehimeWebBrowserViewOSR::OnSetTokenCookiesCompleted(bool success)
{
    if (delegate_)
    {
        delegate_->OnSetTokenCookiesCompleted(success);
    }
}

void LivehimeWebBrowserViewOSR::OnWebBrowserCoreInvalid()
{
    if (delegate_)
    {
        delegate_->OnWebBrowserCoreInvalid();
    }
}

void LivehimeWebBrowserViewOSR::OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data, const std::string& msg_name, const cef_proxy::calldata* data)
{
    // 在这里要判断一下这个特定业务模块的通知是不是此webview的
    if (bind_data_ && (*bind_data_ == bind_data))
    {
        if (!handler_details_)
        {
            using namespace std::placeholders;
            handler_details_ = std::make_unique<CefProxyMsgHandleExtraDetails>();
            handler_details_->widget = GetWidget();
            handler_details_->NativeControlIgnoreViewVisibilityChanged =
                std::bind(&LivehimeWebBrowserViewOSR::NativeControlIgnoreViewVisibilityChanged, this, _1, _2);
            handler_details_->OnCefKeyEvent =
                std::bind(&LivehimeWebBrowserViewOSR::OnCefKeyEvent, this, _1, _2);
        }
        CefProxyMsgHandle(delegate_, handler_details_.get(), bind_data, msg_name, GetURL(), data);
    }

}

void LivehimeWebBrowserViewOSR::OnCefKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event)
{
    if (key_event.type == cef_proxy::KEYEVENT_RAWKEYDOWN)
    {
        // 按键按下且不是在页面内的可编辑区域内按下的就走ESC处理流程
        if (key_event.windows_key_code == VK_ESCAPE && !key_event.focus_on_editable_field)
        {
            OnEscapeKeyEvent(pre_key_event, key_event);
        }
    }
}

void LivehimeWebBrowserViewOSR::OnEscapeKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event)
{
}

void LivehimeWebBrowserViewOSR::InitViews()
{
    CefProxyWrapper::GetInstance()->AddObserver(this);
    if (CefProxyWrapper::GetInstance()->IsValid())
    {
        base::MessageLoopForUI::current()->AddObserver(this);
        base::MessageLoop::current()->PostTask(
            FROM_HERE, base::Bind(&LivehimeWebBrowserViewOSR::OnLoadCefProxyCore, weakptr_factory_.GetWeakPtr(), true));
    }
    else
    {
        // 如果CEF模块当前无效，且底层是已经完成整个初始化流程了的话，
        // 那么CEF模块在当前程序运行周期就是不可用的了
        if (CefProxyWrapper::GetInstance()->IsAlreadyInitFinished())
        {
            // 直接在控件上就显示WebCore加载失败的提示，方便直观的问题查看
            OnWebBrowserCoreInvalid();
        }
        LOG(WARNING) << "LivehimeWebBrowserViewOSR::InitViews Failed";
    }
}

void LivehimeWebBrowserViewOSR::UninitViews()
{
    if (bind_data_&& bind_data_.get()){
        CefProxyWrapper::GetInstance()->CloseBrowser(*bind_data_.get());
    }
    base::MessageLoopForUI::current()->RemoveObserver(this);
    CefProxyWrapper::GetInstance()->RemoveObserver(this);
}

void LivehimeWebBrowserViewOSR::ClosureFunction()
{
    SchedulePaint();
}

void LivehimeWebBrowserViewOSR::CefPaintCallBackOnWorkThread(base::Closure cb, const void* data, int width, int height)
{
    if (is_deleted_)
    {
        return;
    }
    SkBitmap bitmap_ref;
    bitmap_ref.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap_ref.setPixels(const_cast<void*>(data));
    //SkBitmap 只是引用内存，所以deep copy 遍保存副本
    SkBitmap bitmap;
    bitmap_ref.deepCopyTo(&bitmap, SkBitmap::kARGB_8888_Config);

    paint_mutex.lock();
    paint_image = gfx::ImageSkia::CreateFrom1xBitmap(bitmap);
    paint_mutex.unlock();

    BililiveThread::PostTask(
        BililiveThread::UI, FROM_HERE,
        cb);
}
