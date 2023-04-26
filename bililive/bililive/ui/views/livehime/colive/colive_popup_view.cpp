#include "bililive/bililive/ui/views/livehime/colive/colive_popup_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget.h"

#include "base/notification/notification_observer.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"


namespace
{
    class PanelView
        : public BililiveWidgetDelegate
        , public BililiveWidgetDelegateObserver
    {
    public:
        explicit PanelView(
            BililiveWidgetDelegate* main_window,
            views::Widget* relate_widget,
            bililive::PanelType panel_type, View *content_view, int view_width)
            : main_window_(main_window)
            , relate_pk_operate_widget_(relate_widget)
            , panel_type_(panel_type)
            , frame_width_(view_width)
            , content_view_(content_view)
        {
        }

        void AdjustPosition()
        {
            gfx::Rect rect = relate_pk_operate_widget_->GetWindowBoundsInScreen();

            if (panel_type_ == bililive::PanelType::Right)
            {
                rect.set_x(rect.right() - frame_width_);
            }
            rect.set_width(frame_width_);

            gfx::Rect cur_rect = GetWidget()->GetWindowBoundsInScreen();
            if (cur_rect != rect)
            {
                GetWidget()->SetBounds(rect);
            }
        };

    protected:
        // WidgetDelegate
        View* GetContentsView() override
        {
            return this;
        }

        // View
        void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {
                if (details.is_add)
                {
                    //set_background(views::Background::CreateSolidBackground(SkColorSetRGB(0x34, 0x34, 0x34)));

                    SetLayoutManager(new views::FillLayout());
                    if (content_view_)
                    {
                        AddChildView(content_view_);
                    }

                    relate_pk_operate_widget_->AddObserver(this);
                    main_window_->AddObserver(this);
                }
            }
        };

        gfx::Size GetPreferredSize() override
        {
            gfx::Size size;
            size.SetSize(frame_width_, relate_pk_operate_widget_->GetWindowBoundsInScreen().height());

            return size;
        };

        // WidgetObserver
        void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override
        {
            AdjustPosition();
        }

        void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override
        {
            if (widget == relate_pk_operate_widget_)
            {
                AdjustPosition();
            }
        }

        void OnWidgetDestroying(views::Widget* widget) override
        {
            relate_pk_operate_widget_->RemoveObserver(this);
            main_window_->RemoveObserver(this);

            __super::OnWidgetDestroying(widget);
        }

        // BililiveWidgetDelegateObserver
        void OnWidgetEnabledChanged(BililiveWidgetDelegate *widget_delegate, BOOL enabled) override
        {
            ::EnableWindow(GetWidget()->GetNativeView(), enabled);
        }

    private:
        BililiveWidgetDelegate* main_window_;
        views::Widget* relate_pk_operate_widget_;
        bililive::PanelType panel_type_;
        int frame_width_;
        views::View *content_view_;
    };
}


namespace bililive
{
    bool ShowPanelView(
        BililiveWidgetDelegate* main_window,
        views::Widget* pk_operate_widget,
        PanelType panel_type, views::View* content_view, int view_width)
    {
        DCHECK(main_window && pk_operate_widget && content_view);
        if (main_window && pk_operate_widget && content_view)
        {
            gfx::Rect rect = pk_operate_widget->GetWindowBoundsInScreen();
            rect.set_width(view_width);

            PanelView *delegate_view = new PanelView(
                main_window, pk_operate_widget, panel_type, content_view, view_width);

            views::Widget *widget = new views::Widget();
            BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget);
            native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
            views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
            params.native_widget = native_widget;
            params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
            params.parent = pk_operate_widget->GetNativeView();
            params.bounds = rect;   // 给一个初始够大的尺寸，防止 content_view 内部 gridview 计算时 DCHECK

            // TYPE_WINDOW_FRAMELESS 导致窗口为 WS_POPUP 风格。这种情况下需要使用
            // FRAME_TYPE_FORCE_CUSTOM 以避免一些问题，参见 native_widget 中的 GetClientAreaInsets 方法。
            BililiveWidgetDelegate::ShowWidget(delegate_view, widget, params, false, true);

            delegate_view->AdjustPosition();

            widget->Show();
            widget->Activate();
            return true;
        }

        return false;
    }
}
