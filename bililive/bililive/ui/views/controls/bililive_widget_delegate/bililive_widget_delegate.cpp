#include "bililive_widget_delegate.h"
#include "bililive_mask_widget.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_shadow_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/uploader/bililive_uploader_type.h"
#include "bililive/public/bililive/bililive_process.h"

#include "base/run_loop.h"

#include "ui/base/win/dpi.h"
#include "ui/base/win/hwnd_util.h"
#include "ui/views/focus/accelerator_handler.h"

#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"


namespace
{
    void RunModalLoop()
    {
        base::MessageLoopForUI* loop = base::MessageLoopForUI::current();
        base::MessageLoopForUI::ScopedNestableTaskAllower allow_nested(loop);
        views::AcceleratorHandler accelerator_handler;
        base::RunLoop run_loop(&accelerator_handler);
        auto quit_inner_loop = run_loop.QuitClosure();
        run_loop.Run();
        quit_inner_loop.Run();
    }

    void QuitModalLoop()
    {
        base::MessageLoopForUI::current()->Quit();
    }

    const char* const kBililiveWidgetDelegateWndProcKey = "__BILILIVE_WIDGETDELEGATE_WNDPROC__";

    // 我们使用windows系统api进行的系统弹窗，其类名都是（？）“#32770”
    // https://docs.microsoft.com/zh-cn/windows/desktop/winmsg/about-window-classes
    const wchar_t* const kSystemDialogBoxClassName = L"#32770";

    BililiveWidgetDelegate* GetWidgetDelegate(gfx::NativeView native_view)
    {
        views::Widget *par_widget = views::Widget::GetWidgetForNativeWindow(native_view);
        if (par_widget)
        {
            views::WidgetDelegate* widget_delegate = par_widget->widget_delegate();
            if (widget_delegate && strcmp(BililiveWidgetDelegate::kWidgetDelegateClassName,
                widget_delegate->GetWidgetDelegateClassName()) == 0)
            {
                return (BililiveWidgetDelegate*)widget_delegate;
            }
        }
        return nullptr;
    }

    void RestoreWindow(HWND hwnd)
    {
        if (!::IsWindowVisible(hwnd))
        {
            ::ShowWindow(hwnd, SW_SHOW);
        }
        if (::IsIconic(hwnd))
        {
            ::SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
        }
        ::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        ::SetForegroundWindow(hwnd);
    }

    enum class EnumChildWndType
    {
        EnumChildWndType_NoImpl,
        EnumChildWndType_TopSysModal,// 寻找指定窗口上弹出的最顶层的系统弹窗
        EnumChildWndType_TopOwnedModal,// 寻找指定窗口上弹出的最顶层的以我们自己的机制弹出的模态框
        EnumChildWndType_TopModal,// 寻找指定窗口上弹出的最顶层，不区分弹出机制
    };

    struct EnumChildWndParam
    {
        EnumChildWndType enum_type = EnumChildWndType::EnumChildWndType_NoImpl;
        HWND source_hwnd = nullptr;
        HWND target_hwnd = nullptr;
    };

    BOOL CALLBACK EnumThreadWindowsCallback(HWND hwnd, LPARAM param)
    {
        EnumChildWndParam *eunm_param = (EnumChildWndParam*)param;
        if (eunm_param)
        {
            switch (eunm_param->enum_type)
            {
            case EnumChildWndType::EnumChildWndType_TopSysModal:
            {
                // 查询owner窗口是我们给的源窗口并且窗口类是系统弹窗类的窗口
                if (::GetWindow(hwnd, GW_OWNER) == eunm_param->source_hwnd)
                {
                    wchar_t name[MAX_CLASS_NAME] = { 0 };
                    if (::GetClassName(hwnd, name, MAX_CLASS_NAME))
                    {
                        if (wcscmp(name, kSystemDialogBoxClassName) == 0)
                        {
                            eunm_param->target_hwnd = hwnd;// 先记录下来这个找到的child
                            eunm_param->source_hwnd = hwnd;// 以这个找到的child为owner再找其child
                            ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, param);
                            return FALSE;
                        }
                    }
                }
            }
            	break;
            case EnumChildWndType::EnumChildWndType_TopOwnedModal:
            {
                // 查询owner窗口是我们给的源窗口，则以找到的窗口为owner再找其child
                if (::GetWindow(hwnd, GW_OWNER) == eunm_param->source_hwnd)
                {
                    // 我们只在乎模态框，普通的弹窗不管
                    BililiveWidgetDelegate* widget_delegate = GetWidgetDelegate(hwnd);
                    if(widget_delegate && widget_delegate->GetModalType() != ui::MODAL_TYPE_NONE)
                    {
                        eunm_param->target_hwnd = hwnd;// 先记录下来这个找到的child
                        eunm_param->source_hwnd = hwnd;// 以这个找到的child为owner再找其child
                        ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, param);
                        return FALSE;
                    }
                }
            }
                break;
            case EnumChildWndType::EnumChildWndType_TopModal:
            {
                // 查询owner窗口是我们给的源窗口，则以找到的窗口为owner再找其child
                bool found = false;
                if (::GetWindow(hwnd, GW_OWNER) == eunm_param->source_hwnd)
                {
                    BililiveWidgetDelegate* widget_delegate = GetWidgetDelegate(hwnd);
                    if (widget_delegate)
                    {
                        // 我们只在乎模态框，普通的弹窗不管
                        if(widget_delegate->GetModalType() != ui::MODAL_TYPE_NONE)
                        {
                            found = true;
                        }
                    }
                    else
                    {
                        wchar_t name[MAX_CLASS_NAME] = { 0 };
                        if (::GetClassName(hwnd, name, MAX_CLASS_NAME))
                        {
                            // 我们只在乎模态框，普通的弹窗不管
                            if (wcscmp(name, kSystemDialogBoxClassName) == 0)
                            {
                                found = true;
                            }
                        }
                    }
                    if (found)
                    {
                        eunm_param->target_hwnd = hwnd;// 先记录下来这个找到的child
                        eunm_param->source_hwnd = hwnd;// 以这个找到的child为owner再找其child
                        ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, param);
                        return FALSE;
                    }
                }
            }
                break;
            default:
                NOTREACHED() << "invalid enum windows type!";
                break;
            }
        }
        return eunm_param != nullptr;
    }

    void BililiveEndDialog(const EndDialogSignalHandler& handler, int result_code)
    {
        if (!handler.closure.is_null())
        {
            handler.closure.Run(result_code, handler.data);
        }
    }
}


char BililiveWidgetDelegate::kWidgetDelegateClassName[] = "BililiveWidgetDelegate";

views::Widget* g_current_widget = nullptr;

LRESULT CALLBACK BililiveWidgetDelegate::BililiveWidgetDelegateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BililiveWidgetDelegate* widget_delegate = static_cast<BililiveWidgetDelegate*>(
        ui::ViewProp::GetValue(hwnd, kBililiveWidgetDelegateWndProcKey));
    DCHECK(widget_delegate);
    WNDPROC original_handler = widget_delegate->original_handler_;
    DCHECK(original_handler);

    switch (msg)
    {
    case WM_PAINT:
    {
        if (DanmakuhimeDataHandler::FilterPaintMessage(hwnd))
        {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);

            ::EndPaint(hwnd, &ps);
            return 0;
        }
    }
    break;

    case WM_ENABLE:
    {
        widget_delegate->CheckStatus();
    }
        break;
    case WM_SETCURSOR:
    {
        // 模态弹窗时点击父窗体则闪动模态弹窗自身；模态弹窗时owner窗体是disable的，仅针对disable状态的窗体进行判断
        if (!::IsWindowEnabled(hwnd))
        {
            if ((HIWORD(lParam) == WM_LBUTTONDOWN ) || (HIWORD(lParam) == WM_RBUTTONDOWN))
            {
                FlashModalWidget(hwnd);
            }
        }
    }
        break;
    case WM_DESTROY:
    {
        ui::SetWindowProc(hwnd, reinterpret_cast<WNDPROC>(original_handler));
        widget_delegate->prop_.reset();
    }
        break;
    case WM_SYSCOMMAND:
    {
        if (wParam == SC_MINIMIZE)
        {
            RECT rect = { 0 };
            ::GetWindowRect(hwnd, &rect);
            widget_delegate->origin_before_minimize_in_screen_ = gfx::Rect(rect).origin();
        }
    }
        break;
    default:
        break;
    }
    return ::CallWindowProc(reinterpret_cast<WNDPROC>(original_handler), hwnd, msg, wParam, lParam);
}

void BililiveWidgetDelegate::ShowWidget(
    BililiveWidgetDelegate *delegate_view, views::Widget *widget,
    views::Widget::InitParams params, bool bShow/* = true*/, bool use_custom_shadow/* = false*/,
    views::InsureShowCorner anchor_type/* = views::InsureShowCorner::ISC_NEARBY*/)
{
    if (!params.native_widget)
    {
        params.native_widget = new BililiveNativeWidgetWin(widget);
    }
    params.delegate = delegate_view;
    widget->AddObserver(delegate_view);
    widget->set_frame_type(
        use_custom_shadow ? views::Widget::FRAME_TYPE_FORCE_CUSTOM : views::Widget::FRAME_TYPE_FORCE_NATIVE);

    delegate_view->init_by_specified_way_ = true;
    widget->Init(params);
    if (bShow)
    {
        widget->Show();
        widget->Activate();
    }
    else
    {
        widget->Hide();
    }

    // 居中于owner窗口
    if (params.parent)
    {
        // 严格居中，而不是ui::CenterAndSizeWindow那样的顶部对齐水平居中
        ui::CenterAndSizeWindowEx(
            params.parent, widget->GetNativeView(),
            widget->GetWindowBoundsInScreen().size());
    }

    // 当前各种弹出框统一采用被隐藏则就近贴屏显示
    views::InsureWidgetVisible(widget, anchor_type);
}

void BililiveWidgetDelegate::DoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
                                           views::Widget::InitParams params,
                                           const EndDialogSignalHandler* handler/* = nullptr*/)
{
    DCHECK(delegate_view->GetModalType() != ui::MODAL_TYPE_NONE);

    // 只要是采用我们的机制显示的窗口，都只允许在一个窗口F上以自身作为GW_OWNER弹且仅能弹一个MODAL框C，
    // 若要在F上再弹新窗N则新窗N必须以C作为GW_OWNER弹出。
    // 因为若允许同一个窗体上弹多个层级相同的模态框那么在其中一个模态框关闭时框架会在HWNDMessageHandler中
    // 将其GW_OWNER窗使能，就将导致GW_OWNER窗上还有模态框但自身却从禁用态回到使能态导致UI交互混乱的问题。
    BililiveWidgetDelegate* top_modal_delegate = nullptr;
    if (params.parent)
    {
        top_modal_delegate = GetWidgetDelegate(params.parent);
        if (top_modal_delegate)
        {
            HWND top_modal_hwnd = top_modal_delegate->GetTopModalWnd();
            if(top_modal_hwnd)
            {
                params.parent = top_modal_hwnd;
                top_modal_delegate = GetWidgetDelegate(top_modal_hwnd);
            }
        }

        // 弹模态窗时应该把owner窗口还原
        RestoreWindow(params.parent);
    }

    // 在GW_OWNER窗上显示半透明遮盖窗口，用以明显标识窗口处于弹出模态框的状态中
    if (top_modal_delegate)
    {
        top_modal_delegate->ShowMaskWidget(delegate_view->show_mask_hwnd_when_domodal());
    }

    // 设置模态窗关闭通知回调
    if (handler)
    {
        delegate_view->SetEndDialogSignalHandler(*handler);
    }

    ShowWidget(delegate_view, widget, params, false);
    widget->Show();
    g_current_widget = widget;
}

int BililiveWidgetDelegate::RealDoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
                       views::Widget::InitParams params, bool obs_core_not_init)
{
    DCHECK(delegate_view->GetModalType() != ui::MODAL_TYPE_NONE);
    if(!obs_core_not_init)
    DCHECK(params.parent);
    HWND root_owner = params.parent;
    HWND owner = nullptr;
    do
    {
        owner = ::GetWindow(root_owner, GW_OWNER);
        if (owner)
        {
            root_owner = owner;
        }
    } while (owner != nullptr);

    // 真模态弹框只能基于主界面弹
    if (!obs_core_not_init)
    {
        if (root_owner == GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView())
        {
            std::shared_ptr<int> result = std::make_shared<int>(0);
            delegate_view->SetRealModalResult(result);
            DoModalWidget(delegate_view, widget, params);
            // 开启RunLooop
            RunModalLoop();
            return *result;
        }
        else
        {
            NOTREACHED() << "real modal window must popup base on owner == main_view";
        }
    }
    else
    {
        std::shared_ptr<int> result = std::make_shared<int>(0);
        delegate_view->SetRealModalResult(result);
        DoModalWidget(delegate_view, widget, params);
        // 开启RunLooop
        RunModalLoop();
        return *result;
    }

    return 0;
}

bool BililiveWidgetDelegate::IsWidgetInModalNow(gfx::NativeView hwnd)
{
    BililiveWidgetDelegate* widget_delegate = GetWidgetDelegate(hwnd);
    if (widget_delegate)
    {
        return widget_delegate->IsInModalNow();
    }
    return false;
}

void BililiveWidgetDelegate::FlashModalWidget(gfx::NativeView hwnd)
{
    BililiveWidgetDelegate* widget_delegate = GetWidgetDelegate(hwnd);
    if (widget_delegate)
    {
        HWND top_modal_hwnd = widget_delegate->GetTopModalWnd();
        if (top_modal_hwnd)
        {
            BililiveWidgetDelegate* top_modal_delegate = GetWidgetDelegate(top_modal_hwnd);
            if (top_modal_delegate)
            {
                if (!top_modal_delegate->IsFlashFrameDisabled())
                {
					top_modal_delegate->GetWidget()->Activate();
					top_modal_delegate->FlashFrame();
                }
            }
            else
            {
                ::SetForegroundWindow(top_modal_hwnd);

                FLASHWINFO fi = { 0 };
                fi.cbSize = sizeof(FLASHWINFO);
                fi.dwFlags = FLASHW_CAPTION | FLASHW_TIMER;
                fi.hwnd = top_modal_hwnd;
                fi.uCount = 6;
                fi.dwTimeout = 100;
                FlashWindowEx(&fi);
            }
            ::MessageBeep(MB_ICONWARNING);
        }
    }

    //// 若明确给出要闪的窗口则闪之，否则闪当前的最顶层模态框
    //BililiveWidgetDelegate *flash_widget_delegate = nullptr;
    //if (invoke_view)
    //{
    //    BililiveWidgetDelegate *bililive_delegate = GetWidgetDelegate(invoke_view);
    //    if (bililive_delegate)
    //    {
    //        BililiveWidgetDelegate *last_modal_delegate = bililive_delegate->last_model_widget_delegate();
    //        if (last_modal_delegate)
    //        {
    //            flash_widget_delegate = last_modal_delegate;
    //        }
    //        else
    //        {
    //            flash_widget_delegate = bililive_delegate;
    //        }
    //    }
    //}
    //else
    //{
    //    if (!model_views_.empty())
    //    {
    //        flash_widget_delegate = model_views_.top();
    //    }
    //}
    //if (flash_widget_delegate)
    //{
    //    // 找到了要闪的窗体，看看其上是否有系统弹窗
    //    HWND system_modal_hwnd = flash_widget_delegate->GetSystemModalWnd();
    //    if (system_modal_hwnd)
    //    {
    //        ::SetForegroundWindow(system_modal_hwnd);

    //        FLASHWINFO fi = { 0 };
    //        fi.cbSize = sizeof(FLASHWINFO);
    //        fi.dwFlags = FLASHW_CAPTION | FLASHW_TIMER;
    //        fi.hwnd = system_modal_hwnd;
    //        fi.uCount = 6;
    //        fi.dwTimeout = 100;
    //        FlashWindowEx(&fi);
    //    }
    //    else
    //    {
    //        flash_widget_delegate->FlashFrame();
    //    }
    //    ::MessageBeep(MB_ICONWARNING);
    //}
}

// 检测当前目标模态窗口是否已存在，避免重复打开，返回查询到的目标模态窗口
views::Widget* BililiveWidgetDelegate::GetCurrentWidget() {
    return g_current_widget;
}

BililiveWidgetDelegate::BililiveWidgetDelegate(const gfx::ImageSkia& skia, const base::string16 &caption,
    int TitleBarButtonCombination/* = TBB_CLOSE*/)
    : skia_(skia)
    , caption_(caption)
    , titlebar_button_combination_(TitleBarButtonCombination)
{
    InitParam();
}

BililiveWidgetDelegate::BililiveWidgetDelegate()
    : titlebar_button_combination_(TBB_CLOSE)
{
    InitParam();
}

void BililiveWidgetDelegate::InitParam()
{
    modal_result_ = std::make_shared<int>(0);
    init_by_specified_way_ = false;
    cache_maxmin_size_ = true;
    show_mask_hwnd_when_domodal_ = false;
    widget_enabled_ = TRUE;

    BililiveWidgetDelegate::OnThemeChanged();
}

BililiveWidgetDelegate::~BililiveWidgetDelegate()
{
}

views::NonClientFrameView *BililiveWidgetDelegate::CreateNonClientFrameView(views::Widget *widget)
{
    DCHECK(init_by_specified_way_) << "BililiveWidgetDelegate's Widget must be Init by ShowWindow";

    BililiveFrameView* frame_view = new BililiveFrameView(&skia_, GetWindowTitle(), titlebar_button_combination_);

    frame_view->SetCacheMaxminSize(cache_maxmin_size_);

    set_frame_details(frame_view);

    OnCreateNonClientFrameView(frame_view);

    return frame_view;
}

void BililiveWidgetDelegate::OnWidgetCreated(views::Widget* widget)
{
    if(widget->widget_delegate() == this)
    {
        widget_enabled_ = ::IsWindowEnabled(widget->GetNativeView());

        // We subclass the hwnd so we get the interested messages.
        original_handler_ = ui::SetWindowProc(
            widget->GetNativeView(), &BililiveWidgetDelegate::BililiveWidgetDelegateWndProc);
        prop_.reset(new ui::ViewProp(widget->GetNativeView(), kBililiveWidgetDelegateWndProcKey, this));
    }
}

void BililiveWidgetDelegate::OnWidgetDestroying(views::Widget* widget)
{
    if (widget->widget_delegate() == this)
    {
        OnClosing();
    }
}

void BililiveWidgetDelegate::OnWidgetDestroyed(views::Widget* widget)
{
    if (widget->widget_delegate() == this)
    {
        GetWidget()->RemoveObserver(this);
        g_current_widget = nullptr;
    }
}

void BililiveWidgetDelegate::OnClosing()
{
    /*GetWidget()->RemoveObserver(this);*/

    if (GetModalType() != ui::MODAL_TYPE_NONE)
    {
        HWND hwnd_owner = ::GetWindow(GetWidget()->GetNativeView(), GW_OWNER);
        BililiveWidgetDelegate* ancestor_delegate = GetWidgetDelegate(hwnd_owner);
        if (ancestor_delegate)
        {
            // 关闭owner的模态遮掩窗
            ancestor_delegate->ShowMaskWidget(false);
        }
        // 恢复owner焦点
        if (!::IsIconic(hwnd_owner))
        {
            ::SetWindowPos(hwnd_owner, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            ::SetForegroundWindow(hwnd_owner);
        }

        // 调用回调通知上层模态操作结果
        if (!modal_end_dialog_handler_.closure.is_null())
        {
            base::MessageLoop::current()->PostTask(
                FROM_HERE, base::Bind(BililiveEndDialog,
                                      modal_end_dialog_handler_,
                                      GetResultCode()));
        }

        // 是真模态就退出当前嵌套的RunLooop
        if (real_modal_)
        {
            QuitModalLoop();
        }
    }
}

void BililiveWidgetDelegate::OnThemeChanged()
{
    set_background(views::Background::CreateSolidBackground(GetColor(WindowClient)));

    if (GetWidget() && GetWidget()->non_client_view())
    {
        views::NonClientFrameView *frame_view = GetWidget()->non_client_view()->frame_view();
        if (strcmp(frame_view->GetClassName(), BililiveFrameView::kDevViewClassName) == 0)
        {
            set_frame_details((BililiveFrameView*)frame_view);
        }
    }
}

void BililiveWidgetDelegate::set_frame_details(BililiveFrameView* frame_view)
{
    frame_view->SetBackgroundColor(GetColor(WindowTitle));
}

void BililiveWidgetDelegate::SetCacheMaxminSize(bool cache)
{
    cache_maxmin_size_ = cache;
    if (GetWidget() && GetWidget()->non_client_view() && GetWidget()->non_client_view()->frame_view())
    {
        if (strcmp(GetWidget()->non_client_view()->frame_view()->GetClassName(), BililiveFrameView::kDevViewClassName) == 0)
        {
            ((BililiveFrameView*)GetWidget()->non_client_view()->frame_view())->SetCacheMaxminSize(cache_maxmin_size_);
        }
    }
}

void BililiveWidgetDelegate::FlashFrame()
{
    if (GetWidget()->non_client_view())
    {
        views::NonClientFrameView *frame_view = GetWidget()->non_client_view()->frame_view();
        if (strcmp(frame_view->GetClassName(), BililiveFrameView::kDevViewClassName) == 0)
        {
            GetWidget()->Activate();
            ((BililiveFrameView*)frame_view)->FlashFrame();
        }
    }
}

void BililiveWidgetDelegate::SetWindowTitle(const base::string16& title)
{
    caption_ = title;
    if (GetWidget())
    {
        GetWidget()->UpdateWindowTitle();
    }
}

void BililiveWidgetDelegate::AddObserver(BililiveWidgetDelegateObserver *obs)
{
    observers_.AddObserver(obs);
}

void BililiveWidgetDelegate::RemoveObserver(BililiveWidgetDelegateObserver *obs)
{
    observers_.RemoveObserver(obs);
}

void BililiveWidgetDelegate::CheckStatus()
{
    if (GetWidget() && ::IsWindow(GetWidget()->GetNativeView()))
    {
        if (widget_enabled_ != ::IsWindowEnabled(GetWidget()->GetNativeView()))
        {
            widget_enabled_ = !widget_enabled_;

            FOR_EACH_OBSERVER(BililiveWidgetDelegateObserver, observers_,
                              OnWidgetEnabledChanged(this, widget_enabled_));

            // 为了防止一个窗口上弹了多层模态窗，然而最底层的那个框关闭导致根owner窗口变为使能状态
            // 而其上还存留模态框但其却能操作的情况出现，在这里对使能态再次判断，若发现其上还有模态窗
            // 则重新将其禁用
            if (widget_enabled_)
            {

            }
        }
    }
}

// modal
HWND BililiveWidgetDelegate::GetSystemModalWnd()
{
    EnumChildWndParam param;
    param.enum_type = EnumChildWndType::EnumChildWndType_TopSysModal;
    param.source_hwnd = GetWidget()->GetNativeView();
    ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, (LPARAM)&param);
    return param.target_hwnd;
}

HWND BililiveWidgetDelegate::GetOwnedTopModalWnd()
{
    EnumChildWndParam param;
    param.enum_type = EnumChildWndType::EnumChildWndType_TopOwnedModal;
    param.source_hwnd = GetWidget()->GetNativeView();
    ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, (LPARAM)&param);
    return param.target_hwnd;
}

HWND BililiveWidgetDelegate::GetTopModalWnd()
{
    EnumChildWndParam param;
    param.enum_type = EnumChildWndType::EnumChildWndType_TopModal;
    param.source_hwnd = GetWidget()->GetNativeView();
    ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, (LPARAM)&param);
    return param.target_hwnd;
}

bool BililiveWidgetDelegate::IsInSystemModalNow()
{
    return GetSystemModalWnd() != nullptr;
}

bool BililiveWidgetDelegate::IsInOwnedModalNow()
{
    return GetOwnedTopModalWnd() != nullptr;
}

bool BililiveWidgetDelegate::IsInModalNow()
{
    return GetTopModalWnd() != nullptr;
}

void BililiveWidgetDelegate::ShowMaskWidget(bool show)
{
    if (show)
    {
        if (!modal_status_mask_widget_)
        {
            modal_status_mask_widget_ = BililiveMaskWidget::ShowMaskWidget(GetWidget()->GetNativeView());
        }
    }
    else
    {
        if (modal_status_mask_widget_)
        {
            modal_status_mask_widget_->Close();
            modal_status_mask_widget_ = nullptr;
        }
    }
}

void BililiveWidgetDelegate::SetEndDialogSignalHandler(const EndDialogSignalHandler& handler)
{
    modal_end_dialog_handler_ = handler;
}

void BililiveWidgetDelegate::SetResultCode(int result_code)
{
    DCHECK(GetModalType() != ui::MODAL_TYPE_NONE);
    *modal_result_ = result_code;
}

int BililiveWidgetDelegate::GetResultCode() const
{
    DCHECK(GetModalType() != ui::MODAL_TYPE_NONE);
    return *modal_result_;
}

void BililiveWidgetDelegate::SetResultData(void* data)
{
    DCHECK(GetModalType() != ui::MODAL_TYPE_NONE);
    DCHECK(nullptr == modal_end_dialog_handler_.data) <<
        "常规的使用中最好不要直接改变模态回调关联数据，因为这个数据是弹窗的时候由调用层传进来的，"\
        "直接改变这个关联数据需要确保传进的数据结构和变更的数据结构一致，否则将导致模态回调时对data的数据解析错误。";
    modal_end_dialog_handler_.data = data;
}

void* BililiveWidgetDelegate::GetResultData() const
{
    DCHECK(GetModalType() != ui::MODAL_TYPE_NONE);
    return modal_end_dialog_handler_.data;
}

void BililiveWidgetDelegate::SetRealModalResult(const std::shared_ptr<int>& modal_result)
{
    DCHECK(GetModalType() != ui::MODAL_TYPE_NONE);
    real_modal_ = true;
    modal_result_ = modal_result;
}



// BilibiliUploaderWidgetDelegate
BilibiliUploaderWidgetDelegate::BilibiliUploaderWidgetDelegate()
    : is_transparent_(true)
{
}

BilibiliUploaderWidgetDelegate::BilibiliUploaderWidgetDelegate(gfx::ImageSkia skia, const base::string16 &caption)
    : BililiveWidgetDelegate(skia, caption)
{
    set_transparent(true);
}

views::NonClientFrameView *BilibiliUploaderWidgetDelegate::CreateNonClientFrameView(views::Widget *widget)
{
    views::NonClientFrameView *frame = nullptr;
    if (!is_transparent())
    {
        BililiveFrameView *b_frame = new BililiveFrameView(&skia_, caption_);
        b_frame->SetBorderColor(bililive_uploader::clrFrameBorder);
        b_frame->SetBackgroundColor(bililive_uploader::clrPink);
        frame = b_frame;
    }
    else
    {
        BililiveFrameTitleBarView *title_bar_ = new BililiveFrameTitleBarView(&skia_, caption_, TBB_CLOSE);
        title_bar_->set_background(views::Background::CreateSolidBackground(bililive_uploader::clrPink));
        frame = new BiliShadowFrameView(title_bar_);
    }
    return frame;
}
