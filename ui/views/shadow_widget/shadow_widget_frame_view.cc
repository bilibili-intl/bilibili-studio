#include "ui/views/shadow_widget/shadow_widget_frame_view.h"

#include <algorithm>

#include "grit/ui_resources.h"
#include "ui/base/hit_test.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/path.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/skia_util.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/window/client_view.h"
#include "shadow_widget_border.h"

namespace
{

  // Padding, in pixels, for the title view, when it exists.
  const int kTitleTopInset = 12;
  const int kTitleLeftInset = 19;
  const int kTitleBottomInset = 12;

  // Get the |vertical| or horizontal screen overflow of the |window_bounds|.
  int GetOffScreenLength(const gfx::Rect &monitor_bounds,
                         const gfx::Rect &window_bounds,
                         bool vertical)
  {
    if (monitor_bounds.IsEmpty() || monitor_bounds.Contains(window_bounds))
    {
      return 0;
    }

    //  window_bounds
    //  +-------------------------------+
    //  |             top               |
    //  |      +----------------+       |
    //  | left | monitor_bounds | right |
    //  |      +----------------+       |
    //  |            bottom             |
    //  +-------------------------------+
    if (vertical)
      return std::max(0, monitor_bounds.y() - window_bounds.y()) +
             std::max(0, window_bounds.bottom() - monitor_bounds.bottom());
    return std::max(0, monitor_bounds.x() - window_bounds.x()) +
           std::max(0, window_bounds.right() - monitor_bounds.right());
  }

}  // namespace

namespace views
{

  // static
  const char ShadowWidgetFrameView::kViewClassName[] = "ShadowFrameView";

  ShadowWidgetFrameView::ShadowWidgetFrameView()
    : bubble_border_(NULL)
  {

  }

  ShadowWidgetFrameView::~ShadowWidgetFrameView() {}

  gfx::Rect ShadowWidgetFrameView::GetBoundsForClientView() const
  {
    gfx::Rect client_bounds = GetLocalBounds();
    client_bounds.Inset(GetInsets());
    client_bounds.Inset(bubble_border_->GetInsets());
    return client_bounds;
  }

  gfx::Rect ShadowWidgetFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect &client_bounds) const
  {
    return const_cast<ShadowWidgetFrameView *>(this)->GetUpdatedWindowBounds(client_bounds);
  }

  int ShadowWidgetFrameView::NonClientHitTest(const gfx::Point &point)
  {
    return HTNOWHERE;
  }

  void ShadowWidgetFrameView::GetWindowMask(const gfx::Size &size,
      gfx::Path *window_mask)
  {
    // NOTE: this only provides implementations for the types used by dialogs.
    if (bubble_border_->shadow() != BubbleBorder::SMALL_SHADOW &&
        bubble_border_->shadow() != BubbleBorder::NO_SHADOW_OPAQUE_BORDER)
    {
      return;
    }

    // Use a window mask roughly matching the border in the image assets.
    static const int kBorderStrokeSize = 1;
    static const SkScalar kCornerRadius = SkIntToScalar(6);
    const gfx::Insets border_insets = bubble_border_->GetInsets();
    SkRect rect = { SkIntToScalar(border_insets.left() - kBorderStrokeSize),
                    SkIntToScalar(border_insets.top() - kBorderStrokeSize),
                    SkIntToScalar(size.width() - border_insets.right() +
                                  kBorderStrokeSize),
                    SkIntToScalar(size.height() - border_insets.bottom() +
                                  kBorderStrokeSize)
                  };
    if (bubble_border_->shadow() == BubbleBorder::NO_SHADOW_OPAQUE_BORDER)
    {
      window_mask->addRoundRect(rect, kCornerRadius, kCornerRadius);
    }
    else
    {
      static const int kBottomBorderShadowSize = 2;
      rect.fBottom += SkIntToScalar(kBottomBorderShadowSize);
      window_mask->addRect(rect);
    }
  }

  void ShadowWidgetFrameView::ResetWindowControls()
  {

  }

  void ShadowWidgetFrameView::UpdateWindowIcon()
  {

  }

  void ShadowWidgetFrameView::UpdateWindowTitle()
  {

  }

  gfx::Size ShadowWidgetFrameView::GetPreferredSize()
  {
    const gfx::Size client(GetWidget()->client_view()->GetPreferredSize());
    gfx::Rect client_rect(client.width(), client.height());
    gfx::Size size(GetUpdatedWindowBounds(client_rect).size());
    int title_bar_width = GetInsets().width() + border()->GetInsets().width();
    size.SetToMax(gfx::Size(title_bar_width, 0));
    return size;
  }

  void ShadowWidgetFrameView::Layout()
  {
    gfx::Rect bounds(GetLocalBounds());
    bounds.Inset(border()->GetInsets());
    // Small additional insets yield the desired 10px visual close button insets.
    bounds.Inset(0, 0, 0 + 1, 0);
  }

  const char *ShadowWidgetFrameView::GetClassName() const
  {
    return kViewClassName;
  }

  void ShadowWidgetFrameView::SetBubbleBorder(ShadowWidgetBorder *border)
  {
    bubble_border_ = border;
    set_border(bubble_border_);

    set_background(new views::ShadowWidgetBackground(border));
  }

  gfx::Rect ShadowWidgetFrameView::GetUpdatedWindowBounds(const gfx::Rect &content_rect)
  {
    return bubble_border_->GetBounds(content_rect);
  }

  gfx::Rect ShadowWidgetFrameView::GetMonitorBounds(const gfx::Rect &rect)
  {
    return gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(
             rect.CenterPoint()).work_area();
  }

}  // namespace views