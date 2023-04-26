#ifndef UI_VIEWS_SHADOW_WIDGET_SHADOW_WIDGET_DELEGATE_H
#define UI_VIEWS_SHADOW_WIDGET_SHADOW_WIDGET_DELEGATE_H

#include "shadow_widget_border.h"
#include "ui\views\widget\widget_delegate.h"
#include "ui\views\widget\widget_observer.h"

namespace gfx
{
  class Rect;
}

namespace views
{
  class VIEWS_EXPORT ShadowClientView
    : public views::WidgetDelegateView
    , public views::WidgetObserver
  {
  public:
    explicit ShadowClientView(views::ShadowWidgetBorder::Shadow shadow_type);

    virtual ~ShadowClientView();

    void SetHostWidget(views::Widget *widget);

    // views::WidgetDelegateView
    virtual views::NonClientFrameView *CreateNonClientFrameView(views::Widget *widget) OVERRIDE;
    virtual views::View *GetContentsView() OVERRIDE;

    // views::WidgetOberver
    virtual void OnWidgetClosing(Widget* widget) OVERRIDE;
    virtual void OnWidgetCreated(Widget* widget) OVERRIDE;
    virtual void OnWidgetDestroying(Widget* widget) OVERRIDE;
    virtual void OnWidgetDestroyed(Widget* widget) OVERRIDE;
    virtual void OnWidgetVisibilityChanged(Widget* widget, bool visible) OVERRIDE;
    virtual void OnWidgetActivationChanged(Widget* widget, bool active)OVERRIDE;
    virtual void OnWidgetBoundsChanged(Widget* widget,
      const gfx::Rect& new_bounds)OVERRIDE;

  private:
    views::Widget *host_widget_;
    views::ShadowWidgetBorder::Shadow shadow_type_;
  };

  VIEWS_EXPORT views::ShadowClientView *CreateShadowWidget(gfx::NativeView parent_widget);
}

#endif  // UI_VIEWS_SHADOW_WIDGET_SHADOW_WIDGET_DELEGATE_H