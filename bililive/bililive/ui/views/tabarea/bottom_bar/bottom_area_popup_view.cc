#include "bottom_area_popup_view.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/widget/widget_observer.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_inside_frame_titlebar.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_view.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/danmaku_search_view.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/global_banned_view.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "grit/theme_resources.h"
#include "grit/generated_resources.h"


namespace
{
    class TabAreaBottomBarFunctionView
        : public BililiveWidgetDelegate
        , public base::NotificationObserver
    {
    public:
        TabAreaBottomBarFunctionView(TabareaBottomFunction function, views::View *content_view)
            : content_view_(content_view)
            , function_(function)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();
            switch (function_)
            {
            case TabareaBottomFunction_Banned:
                caption_ = rb.GetLocalizedString(IDS_DANMUKU_BANNED_TITLE);
                break;
            case TabareaBottomFunction_NameList:
                caption_ = rb.GetLocalizedString(IDS_DANMUKU_NAMELIST_WINDOW_TITLE);
                break;
            case TabareaBottomFunction_Search:
                caption_ = rb.GetLocalizedString(IDS_DANMUKU_SEARCH_TITLE);
                break;
            default:
                break;
            }
        }

        void AdjustPosition()
        {
            //gfx::Rect tab_rect = GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view()->GetMiddleAreaBounds();
            //tab_rect = GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view()->ConvertRectToWidget(tab_rect);
            //int cy = GetWidget()->GetRootView()->GetHeightForWidth(tab_rect.width());
            //cy = std::min(cy, tab_rect.height());
            //gfx::Rect bounds(tab_rect.x(), tab_rect.bottom() - cy, tab_rect.width(), cy);
            //GetWidget()->SetBounds(bounds);
        };

    protected:
        // BililiveWidgetDelegate
        void OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view) override
        {
            BililiveFrameView* frame_view = reinterpret_cast<BililiveFrameView*>(non_client_frame_view);
            frame_view->SetTitlebar(new LivehimeInsideFrameTitleBar(caption_));
            frame_view->SetShadowFrame(false);
            //frame_view->SetBorderColor(clrWindowsBorder);
            frame_view->set_border(views::Border::CreateSolidSidedBorder(1, 1, 0, 1, GetColor(LivehimeColorType::WindowBorder)));
            frame_view->SetEnableDragMove(false);
            frame_view->SetBackgroundColor(GetColor(WindowClient));
        }

        views::View* GetContentsView() override
        {
            return this;
        }

        ui::ModalType GetModalType() const override
        {
            return ui::ModalType::MODAL_TYPE_CHILD;
        };

        // View
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    SetLayoutManager(new views::FillLayout());
                    if (content_view_)
                    {
                        AddChildView(content_view_);
                    }

                    GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->AddObserver(this);
                    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_BANNED_CLOSE,
                                             base::NotificationService::AllSources());
                    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_CLOSE,
                                             base::NotificationService::AllSources());
                    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_CLOSE,
                                             base::NotificationService::AllSources());
                }
            }
        };

        gfx::Size GetPreferredSize() override
        {
            int cx = kMainWndTabAreaWidth - 2;
            gfx::Size size(cx, cx);
            if (content_view_)
            {
                size.SetSize(cx, content_view_->GetHeightForWidth(cx));
            }
            return size;
        };

        void ChildPreferredSizeChanged(View* child) override
        {
            AdjustPosition();
        };

        void OnThemeChanged() override
        {
            __super::OnThemeChanged();

            BililiveFrameView* frame_view = reinterpret_cast<BililiveFrameView*>(GetWidget()->non_client_view()->frame_view());
            frame_view->SetBackgroundColor(GetColor(WindowClient));
        }

        // WidgetObserver
        void OnWidgetDestroying(views::Widget* widget) override
        {
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->RemoveObserver(this);
            if (widget->widget_delegate() == this)
            {
                notifation_register_.RemoveAll();

                // 关闭通知
                base::NotificationService::current()->Notify(
                    kTabareaCloseTable.at(function_),
                    base::NotificationService::AllSources(),
                    base::NotificationService::NoDetails());
            }

            __super::OnWidgetDestroying(widget);
        };

        void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override
        {
            if (widget == GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget())
            {
                AdjustPosition();
            }

            __super::OnWidgetBoundsChanged(widget, new_bounds);
        };

        // NotificationObserver
        void Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details) override
        {
            if (details.map_key() != 0)
            {
                switch (type)
                {
                case bililive::NOTIFICATION_LIVEHIME_TABAREA_BANNED_CLOSE:
                    if (function_ == TabareaBottomFunction_Banned)
                    {
                        GetWidget()->Close();
                    }
                    break;
                case bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_CLOSE:
                    if (function_ == TabareaBottomFunction_NameList)
                    {
                        GetWidget()->Close();
                    }
                    break;
                case bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_CLOSE:
                    if (function_ == TabareaBottomFunction_Search)
                    {
                        GetWidget()->Close();
                    }
                    break;
                default:
                    break;
                }
            }
        }

    private:
        TabareaBottomFunction function_;
        views::View *content_view_;
        base::NotificationRegistrar notifation_register_;
    };
}

const std::map<TabareaBottomFunction, bililive::NotificationType> kTabareaShowTable
{
    { TabareaBottomFunction_Banned, bililive::NOTIFICATION_LIVEHIME_TABAREA_BANNED_SHOW },
    { TabareaBottomFunction_NameList, bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_SHOW },
    { TabareaBottomFunction_Search, bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_SHOW }
};

const std::map<TabareaBottomFunction, bililive::NotificationType> kTabareaCloseTable
{
    { TabareaBottomFunction_Banned, bililive::NOTIFICATION_LIVEHIME_TABAREA_BANNED_CLOSE },
    { TabareaBottomFunction_NameList, bililive::NOTIFICATION_LIVEHIME_TABAREA_NAMELIST_CLOSE },
    { TabareaBottomFunction_Search, bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH_CLOSE }
};

bool ShowTabAreaBottomBarFunctionView(TabareaBottomFunction function)
{
    views::View *content_view = nullptr;

    switch (function)
    {
    case TabareaBottomFunction_Banned:
        content_view = new GlobalBannedView();
        break;
    case TabareaBottomFunction_NameList:
        //content_view = new BlacklistView();
        break;
    case TabareaBottomFunction_Search:
        content_view = new DanmakuSearchView();
        break;
    default:
        break;
    }

    if (content_view)
    {
        TabAreaBottomBarFunctionView *delegate_view = new TabAreaBottomBarFunctionView(function, content_view);
        views::Widget *widget = new views::Widget();
        BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.native_widget = native_widget;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.parent = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView();
        params.child = true;
        BililiveWidgetDelegate::ShowWidget(delegate_view, widget, params, false);

        delegate_view->AdjustPosition();

        widget->Show();
        widget->Activate();

        // 开启通知
        base::NotificationService::current()->Notify(
            kTabareaShowTable.at(function),
            base::NotificationService::AllSources(),
            base::NotificationService::NoDetails());
    }

    return content_view != nullptr;
}
