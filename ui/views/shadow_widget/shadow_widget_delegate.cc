#include "ui\views\shadow_widget\shadow_widget_delegate.h"
#include "shadow_widget_frame_view.h"
#include "shadow_widget_border.h"
#include "ui\views\widget\widget_delegate.h"
#include "ui\views\widget\widget.h"

namespace views
{
  views::ShadowClientView::ShadowClientView(views::ShadowWidgetBorder::Shadow shadow_type)
    : host_widget_(NULL)
    , shadow_type_(shadow_type)
  {

  }

  views::ShadowClientView::~ShadowClientView()
  {

  }

  void ShadowClientView::SetHostWidget(views::Widget *widget)
  {
    host_widget_ = widget;
    host_widget_->AddObserver(this);
  }

  views::NonClientFrameView *views::ShadowClientView::CreateNonClientFrameView(views::Widget *widget)
  {
    views::ShadowWidgetFrameView *frame = new views::ShadowWidgetFrameView();
    frame->SetBubbleBorder(new views::ShadowWidgetBorder(
      shadow_type_,
      SkColorSetARGB(0x00, 0x00, 0x00, 0x00)));
    return frame;
  }

  views::View *views::ShadowClientView::GetContentsView()
  {
    return this;
  }

  void views::ShadowClientView::OnWidgetClosing(Widget* widget)
  {
    host_widget_->RemoveObserver(this);
    GetWidget()->Close();
  }

  void views::ShadowClientView::OnWidgetCreated(Widget* widget)
  {
  }

  void views::ShadowClientView::OnWidgetDestroying(Widget* widget)
  {

  }

  void views::ShadowClientView::OnWidgetDestroyed(Widget* widget)
  {
  }

  void views::ShadowClientView::OnWidgetVisibilityChanged(Widget* widget, bool visible)
  {
    if (visible)
    {
      GetWidget()->Show();
    }
    else
    {
      GetWidget()->Hide();
    }
  }

  void views::ShadowClientView::OnWidgetActivationChanged(Widget* widget, bool active)
  {
    widget->StackAbove(host_widget_->GetNativeView());
  }

  void views::ShadowClientView::OnWidgetBoundsChanged(Widget* widget, const gfx::Rect& new_bounds)
  {
    if (widget->IsMaximized() || widget->IsMinimized())
    {
      GetWidget()->Hide();
    }
    else
    {
      if (!GetWidget()->IsVisible())
      {
        GetWidget()->ShowInactive();
      }
      
      gfx::Rect bounds = GetWidget()->non_client_view()->GetWindowBoundsForClientBounds(new_bounds);
      GetWidget()->SetBounds(bounds);
    }
  }

  views::ShadowClientView * CreateShadowWidget(gfx::NativeView parent_widget)
  {
    views::ShadowClientView* delegate_view = new views::ShadowClientView(views::ShadowWidgetBorder::BIG_SHADOW);
    views::Widget *widget = new views::Widget();
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW);
    params.parent = parent_widget;
    params.accept_events = false;
    params.can_activate = false;
    params.remove_standard_frame = true;
    params.remove_taskbar_icon = true;
    params.delegate = delegate_view;
    params.opacity = Widget::InitParams::TRANSLUCENT_WINDOW;
    widget->Init(params);
    widget->Show();

    return delegate_view;
  }

}