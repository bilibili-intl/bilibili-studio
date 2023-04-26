#include "bililive/bililive/ui/views/preview/livehime_preview_view.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_area_utils.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/obs_scene_util.h"
//#include "bililive/bililive/livehime/vtuber/vtuber_pk_ui_presenter.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/preview/livehime_obs_preview_control.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"

#include "grit/generated_resources.h"


namespace
{
    class MainOBSPreviewControl
        : public LivehimeOBSPreviewControl
        , SceneItemMenu
        , obs_proxy_ui::OBSUIProxyObserver
        , base::NotificationObserver
    {
    public:
        MainOBSPreviewControl::MainOBSPreviewControl()
            : LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType::PREVIEW_MAIN)
            , SceneItemMenu(this)
        {
        }

        virtual ~MainOBSPreviewControl() = default;

    protected:
        //override from View
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            __super::ViewHierarchyChanged(details);

            if (details.child == this)
            {
                if (details.is_add)
                {
                    OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);

                    notifation_register_.Add(this,
                                             bililive::NOTIFICATION_BILILIVE_DOGUIDE,
                                             base::NotificationService::AllSources());
                }
                else
                {
                    if (OBSProxyService::GetInstance().obs_ui_proxy())
                    {
                        OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
                    }

                    notifation_register_.RemoveAll();
                }
            }
        }

        // LivehimeOBSPreviewControl
        LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param) override
        {
            MSG msg = { GetNativeControlHWND(), message, w_param, l_param, GetMessageTime() };
            ui::KeyEvent key_e(msg, message == WM_CHAR);

            if (key_e.type() == ui::EventType::ET_KEY_RELEASED)
            {
                if (VK_DELETE == key_e.key_code())
                {
                    bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(),
                                             IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW);
                }
            }

            return __super::OnKeyEvent(message, w_param, l_param);
        }

        // OBSUIProxyObserver
        void OnOBSUIProxyInitialized() override
        {
            fit_to_screen_presenter_.reset(new LivehimeSceneItemFitToScreenPresenter());
            fit_to_screen_presenter_->Start();
        }

        void OnCurrentSceneChanged(int scene_index) override
        {
            CancelDragSceneItem();
        }

        // NotificationObserver
        void Observe(int type,
                     const base::NotificationSource& source,
                     const base::NotificationDetails& details) override
        {
            switch (type)
            {
            case bililive::NOTIFICATION_BILILIVE_DOGUIDE:
            {
                if (0 == details.map_key())
                {
                    hwnd_view_->InvalidateLayout();
                    hwnd_view_->Layout();
                }
            }
                break;
            default:
                break;
            }
        }

    private:
        base::NotificationRegistrar notifation_register_;
        std::unique_ptr<LivehimeSceneItemFitToScreenPresenter> fit_to_screen_presenter_;
    };

    class ProjectorOBSPreviewControl
        : public LivehimeOBSPreviewControl
    {
    public:
        ProjectorOBSPreviewControl::ProjectorOBSPreviewControl()
            : LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType::PREVIEW_PROJECTOR)
        {
            projector_list_.push_back(this);
        }

        virtual ~ProjectorOBSPreviewControl() = default;

    protected:
        // LivehimeOBSPreviewControl
        LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param) override
        {
            MSG msg = { GetNativeControlHWND(), message, w_param, l_param, GetMessageTime() };
            ui::KeyEvent key_e(msg, message == WM_CHAR);

            if (key_e.type() == ui::EventType::ET_KEY_RELEASED)
            {
                if (key_e.key_code() == VK_ESCAPE)
                {
                    GetWidget()->Close();
                }
            }

            return __super::OnKeyEvent(message, w_param, l_param);
        }

        // NativeControl
        void OnDestroy() override
        {
            projector_list_.remove(this);

            __super::OnDestroy();
        }

    public:
        static std::list<ProjectorOBSPreviewControl*> projector_list_;
    };

    std::list<ProjectorOBSPreviewControl*> ProjectorOBSPreviewControl::projector_list_;

    LivehimePreviewView* g_single_instance = nullptr;

    int GetChangeFloatingViewContentSequeId()
    {
        static int auto_inc_seque_id = 0;
        return auto_inc_seque_id++;
    }
}


// LivehimePreviewView
LivehimePreviewView::LivehimePreviewView()
    : weak_ptr_(this)
{
    DCHECK(!g_single_instance);
    g_single_instance = this;
}

LivehimePreviewView::~LivehimePreviewView()
{
    g_single_instance = nullptr;
}

// focus less floating view
bool LivehimePreviewView::SetFloatingViewContent(livehime::PreviewFocusLessFloatingWidgetHierarchy hierarchy,
    views::View* new_content_view, int seque_id)
{
    if (-1 == focus_less_floating_views_[hierarchy].current_seque_id)
    {
        focus_less_floating_views_[hierarchy].floating_view->
            SetContentsView(new_content_view, true);
        focus_less_floating_views_[hierarchy].current_seque_id = seque_id;

        return true;
    }
    else
    {
        NOTREACHED() << "conflicting change request !";
    }
    return false;
}

void LivehimePreviewView::HideFloatingView(livehime::PreviewFocusLessFloatingWidgetHierarchy hierarchy,
    int seque_id, bool delete_content_view)
{
    if ((-1 != seque_id) && (focus_less_floating_views_[hierarchy].current_seque_id == seque_id))
    {
        focus_less_floating_views_[hierarchy].floating_view->
            SetContentsView(nullptr, delete_content_view);
        focus_less_floating_views_[hierarchy].current_seque_id = -1;
    }
    else
    {
        NOTREACHED() << "conflicting change request !";
    }
}

// accept floating view
bool LivehimePreviewView::SetAcceptEventFloatingViewContent(livehime::PreviewAcceptEventFloatingWidgetHierarchy hierarchy,
    views::View* new_content_view, int seque_id, void* floating_view_delegate/* = nullptr*/)
{
    if (-1 == accept_event_floating_views_[hierarchy].current_seque_id)
    {
        accept_event_floating_views_[hierarchy].floating_view->
            SetContentsView(new_content_view, true);
        if (floating_view_delegate)
        {
            accept_event_floating_views_[hierarchy].floating_view->SetFloatingViewDelegate(
                static_cast<BililiveFloatingViewDelegate*>(floating_view_delegate));
        }
        accept_event_floating_views_[hierarchy].current_seque_id = seque_id;

        return true;
    }
    else
    {
        NOTREACHED() << "conflicting change request !";
    }
    return false;
}

void LivehimePreviewView::HideAcceptEventFloatingView(livehime::PreviewAcceptEventFloatingWidgetHierarchy hierarchy,
    int seque_id, bool delete_content_view)
{
    if ((-1 != seque_id) && (accept_event_floating_views_[hierarchy].current_seque_id == seque_id))
    {
        accept_event_floating_views_[hierarchy].floating_view->
            SetContentsView(nullptr, delete_content_view);
        accept_event_floating_views_[hierarchy].current_seque_id = -1;
    }
    else
    {
        NOTREACHED() << "conflicting change request !";
    }
}

void LivehimePreviewView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
        }
        else
        {
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
        }
    }
}

void LivehimePreviewView::InitViews()
{
    obs_preview_control_ = new MainOBSPreviewControl();
    AddChildView(obs_preview_control_);

    obs_preview_control_->SetBackgroundColor(0.082353f, 0.086275f, 0.125490f);  //clrViddupWindowsContent

    // 由Z轴底层向外层逐级创建预览区域的浮动窗口，上级是下级child，下级是上级的owner这样才能保证Z轴层级
    views::View* owner_view = this;
    for (int i = livehime::PreviewFocusLessFloatingWidgetHierarchy::PFLFWH_First;
        i < livehime::PreviewFocusLessFloatingWidgetHierarchy::PFLFWH_Count;
        i++)
    {
        BililiveFloatingView* floating_view = new BililiveFloatingView(nullptr, nullptr, false);
        owner_view->AddChildView(floating_view);
        owner_view = floating_view;

        focus_less_floating_views_[i].floating_view = floating_view;
    }
    // 可操作的悬浮窗口置于信息展示悬浮窗口之上，避免造成穿过信息页面点击操作页带来用户视觉错位
    for (int i = livehime::PreviewAcceptEventFloatingWidgetHierarchy::PAEFWH_First;
        i < livehime::PreviewAcceptEventFloatingWidgetHierarchy::PAEFWH_Count;
        i++)
    {
        BililiveFloatingView* floating_view = new BililiveFloatingView(nullptr, nullptr);
        owner_view->AddChildView(floating_view);
        owner_view = floating_view;

        accept_event_floating_views_[i].floating_view = floating_view;
    }
}

void LivehimePreviewView::Layout()
{
    obs_preview_control_->SetBoundsRect(GetContentsBounds());
    // 悬浮窗口只需要布局Z轴最底层那个即可，其他附于其上的悬浮窗口会一并改变尺寸的
    focus_less_floating_views_[livehime::PFLFWH_First].floating_view->SetBoundsRect(GetContentsBounds());
}

void LivehimePreviewView::OnEnterIntoThirdPartyStreamingMode()
{
    // 第三方推流模式时把预览窗口隐藏，不然窗口尺寸变化时能看到后方的OBS预览画面闪动
    obs_preview_control_->SetVisible(false);
}

void LivehimePreviewView::OnLeaveThirdPartyStreamingMode()
{
    obs_preview_control_->SetVisible(true);
}



// PreviewProjectorView
void PreviewProjectorView::ShowForm(gfx::Rect rect)
{
    for (auto ctrl : ProjectorOBSPreviewControl::projector_list_)
    {
        if (ctrl && ctrl->GetWidget())
        {
            gfx::Point point = ctrl->GetWidget()->GetWorkAreaBoundsInScreen().origin();
            if (rect.origin() == point)
            {
                ctrl->GetWidget()->Activate();
                return;
            }
        }
    }

    PreviewProjectorView* preview_projector_view = new PreviewProjectorView();
    preview_projector_view->ShowWidget();

    SetWindowPos(preview_projector_view->GetWidget()->GetNativeWindow(), NULL,
        rect.x(), rect.y(), rect.width(), rect.height(), SWP_NOZORDER);

    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    HotkeyNotifyView::ShowForm(rb.GetLocalizedString(IDS_ESC_CLOSE_PREVIEW_PROJECTOR),
        gfx::Rect(rect.x(), rect.y(), rect.width(), rect.height() / 2), NULL, NULL);

    SetFocus(preview_projector_view->obs_preview_control()->GetNativeHWND());
}

PreviewProjectorView::PreviewProjectorView()
    : obs_preview_control_(nullptr)
{
}

PreviewProjectorView::~PreviewProjectorView() {
}

void PreviewProjectorView::ViewHierarchyChanged(
    const View::ViewHierarchyChangedDetails& details) {
    if (details.is_add && details.child == this) {
        InitViews();
    }
}

void PreviewProjectorView::InitViews()
{
    SetLayoutManager(new views::FillLayout());

    obs_preview_control_ = new ProjectorOBSPreviewControl();
    AddChildView(obs_preview_control_);
}

void PreviewProjectorView::ShowWidget()
{
    views::Widget* widget = new views::Widget;
    widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    params.remove_standard_frame = true;
    params.delegate = this;

    params.remove_taskbar_icon = true;
    params.double_buffer = true;
    params.opacity = views::Widget::InitParams::INFER_OPACITY;
    params.top_level = true;
    widget->Init(params);
    widget->Show();
}



namespace livehime
{
    int ShowPreviewArea(PreviewFocusLessFloatingWidgetHierarchy hierarchy, views::View* content_view)
    {
        if (g_single_instance)
        {
            int seque_id = GetChangeFloatingViewContentSequeId();
            if (g_single_instance->SetFloatingViewContent(hierarchy, content_view, seque_id))
            {
                return seque_id;
            }
        }
        return -1;
    }

    void HidePreviewArea(PreviewFocusLessFloatingWidgetHierarchy hierarchy, int seque_id, bool delete_content_view)
    {
        if (g_single_instance)
        {
            g_single_instance->HideFloatingView(hierarchy, seque_id, delete_content_view);
        }
    }

    int ShowAcceptEventPreviewArea(PreviewAcceptEventFloatingWidgetHierarchy hierarchy, views::View* content_view,
        void* floating_view_delegate/* = nullptr*/)
    {
        if (g_single_instance)
        {
            int seque_id = GetChangeFloatingViewContentSequeId();
            if (g_single_instance->SetAcceptEventFloatingViewContent(hierarchy, content_view, seque_id, floating_view_delegate))
            {
                return seque_id;
            }
        }
        return -1;
    }

    void HideAcceptEventPreviewArea(PreviewAcceptEventFloatingWidgetHierarchy hierarchy, int seque_id, bool delete_content_view)
    {
        if (g_single_instance)
        {
            g_single_instance->HideAcceptEventFloatingView(hierarchy, seque_id, delete_content_view);
        }
    }
}
