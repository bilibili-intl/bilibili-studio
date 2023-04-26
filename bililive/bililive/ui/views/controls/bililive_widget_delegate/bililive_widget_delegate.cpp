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

    // ����ʹ��windowsϵͳapi���е�ϵͳ���������������ǣ�������#32770��
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
        EnumChildWndType_TopSysModal,// Ѱ��ָ�������ϵ���������ϵͳ����
        EnumChildWndType_TopOwnedModal,// Ѱ��ָ�������ϵ����������������Լ��Ļ��Ƶ�����ģ̬��
        EnumChildWndType_TopModal,// Ѱ��ָ�������ϵ�������㣬�����ֵ�������
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
                // ��ѯowner���������Ǹ���Դ���ڲ��Ҵ�������ϵͳ������Ĵ���
                if (::GetWindow(hwnd, GW_OWNER) == eunm_param->source_hwnd)
                {
                    wchar_t name[MAX_CLASS_NAME] = { 0 };
                    if (::GetClassName(hwnd, name, MAX_CLASS_NAME))
                    {
                        if (wcscmp(name, kSystemDialogBoxClassName) == 0)
                        {
                            eunm_param->target_hwnd = hwnd;// �ȼ�¼��������ҵ���child
                            eunm_param->source_hwnd = hwnd;// ������ҵ���childΪowner������child
                            ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, param);
                            return FALSE;
                        }
                    }
                }
            }
            	break;
            case EnumChildWndType::EnumChildWndType_TopOwnedModal:
            {
                // ��ѯowner���������Ǹ���Դ���ڣ������ҵ��Ĵ���Ϊowner������child
                if (::GetWindow(hwnd, GW_OWNER) == eunm_param->source_hwnd)
                {
                    // ����ֻ�ں�ģ̬����ͨ�ĵ�������
                    BililiveWidgetDelegate* widget_delegate = GetWidgetDelegate(hwnd);
                    if(widget_delegate && widget_delegate->GetModalType() != ui::MODAL_TYPE_NONE)
                    {
                        eunm_param->target_hwnd = hwnd;// �ȼ�¼��������ҵ���child
                        eunm_param->source_hwnd = hwnd;// ������ҵ���childΪowner������child
                        ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsCallback, param);
                        return FALSE;
                    }
                }
            }
                break;
            case EnumChildWndType::EnumChildWndType_TopModal:
            {
                // ��ѯowner���������Ǹ���Դ���ڣ������ҵ��Ĵ���Ϊowner������child
                bool found = false;
                if (::GetWindow(hwnd, GW_OWNER) == eunm_param->source_hwnd)
                {
                    BililiveWidgetDelegate* widget_delegate = GetWidgetDelegate(hwnd);
                    if (widget_delegate)
                    {
                        // ����ֻ�ں�ģ̬����ͨ�ĵ�������
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
                            // ����ֻ�ں�ģ̬����ͨ�ĵ�������
                            if (wcscmp(name, kSystemDialogBoxClassName) == 0)
                            {
                                found = true;
                            }
                        }
                    }
                    if (found)
                    {
                        eunm_param->target_hwnd = hwnd;// �ȼ�¼��������ҵ���child
                        eunm_param->source_hwnd = hwnd;// ������ҵ���childΪowner������child
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
        // ģ̬����ʱ���������������ģ̬��������ģ̬����ʱowner������disable�ģ������disable״̬�Ĵ�������ж�
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

    // ������owner����
    if (params.parent)
    {
        // �ϸ���У�������ui::CenterAndSizeWindow�����Ķ�������ˮƽ����
        ui::CenterAndSizeWindowEx(
            params.parent, widget->GetNativeView(),
            widget->GetWindowBoundsInScreen().size());
    }

    // ��ǰ���ֵ�����ͳһ���ñ�������ͽ�������ʾ
    views::InsureWidgetVisible(widget, anchor_type);
}

void BililiveWidgetDelegate::DoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
                                           views::Widget::InitParams params,
                                           const EndDialogSignalHandler* handler/* = nullptr*/)
{
    DCHECK(delegate_view->GetModalType() != ui::MODAL_TYPE_NONE);

    // ֻҪ�ǲ������ǵĻ�����ʾ�Ĵ��ڣ���ֻ������һ������F����������ΪGW_OWNER���ҽ��ܵ�һ��MODAL��C��
    // ��Ҫ��F���ٵ��´�N���´�N������C��ΪGW_OWNER������
    // ��Ϊ������ͬһ�������ϵ�����㼶��ͬ��ģ̬����ô������һ��ģ̬��ر�ʱ��ܻ���HWNDMessageHandler��
    // ����GW_OWNER��ʹ�ܣ��ͽ�����GW_OWNER���ϻ���ģ̬������ȴ�ӽ���̬�ص�ʹ��̬����UI�������ҵ����⡣
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

        // ��ģ̬��ʱӦ�ð�owner���ڻ�ԭ
        RestoreWindow(params.parent);
    }

    // ��GW_OWNER������ʾ��͸���ڸǴ��ڣ��������Ա�ʶ���ڴ��ڵ���ģ̬���״̬��
    if (top_modal_delegate)
    {
        top_modal_delegate->ShowMaskWidget(delegate_view->show_mask_hwnd_when_domodal());
    }

    // ����ģ̬���ر�֪ͨ�ص�
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

    // ��ģ̬����ֻ�ܻ��������浯
    if (!obs_core_not_init)
    {
        if (root_owner == GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView())
        {
            std::shared_ptr<int> result = std::make_shared<int>(0);
            delegate_view->SetRealModalResult(result);
            DoModalWidget(delegate_view, widget, params);
            // ����RunLooop
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
        // ����RunLooop
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

    //// ����ȷ����Ҫ���Ĵ�������֮����������ǰ�����ģ̬��
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
    //    // �ҵ���Ҫ���Ĵ��壬���������Ƿ���ϵͳ����
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

// ��⵱ǰĿ��ģ̬�����Ƿ��Ѵ��ڣ������ظ��򿪣����ز�ѯ����Ŀ��ģ̬����
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
            // �ر�owner��ģ̬���ڴ�
            ancestor_delegate->ShowMaskWidget(false);
        }
        // �ָ�owner����
        if (!::IsIconic(hwnd_owner))
        {
            ::SetWindowPos(hwnd_owner, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
            ::SetForegroundWindow(hwnd_owner);
        }

        // ���ûص�֪ͨ�ϲ�ģ̬�������
        if (!modal_end_dialog_handler_.closure.is_null())
        {
            base::MessageLoop::current()->PostTask(
                FROM_HERE, base::Bind(BililiveEndDialog,
                                      modal_end_dialog_handler_,
                                      GetResultCode()));
        }

        // ����ģ̬���˳���ǰǶ�׵�RunLooop
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

            // Ϊ�˷�ֹһ�������ϵ��˶��ģ̬����Ȼ����ײ���Ǹ���رյ��¸�owner���ڱ�Ϊʹ��״̬
            // �����ϻ�����ģ̬����ȴ�ܲ�����������֣��������ʹ��̬�ٴ��жϣ����������ϻ���ģ̬��
            // �����½������
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
        "�����ʹ������ò�Ҫֱ�Ӹı�ģ̬�ص��������ݣ���Ϊ��������ǵ�����ʱ���ɵ��ò㴫�����ģ�"\
        "ֱ�Ӹı��������������Ҫȷ�����������ݽṹ�ͱ�������ݽṹһ�£����򽫵���ģ̬�ص�ʱ��data�����ݽ�������";
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
