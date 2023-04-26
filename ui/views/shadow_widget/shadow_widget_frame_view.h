#ifndef UI_VIEWS_SHADOW_WIDGET_SHADOW_WIDGET_FRAME_VIEW_H
#define UI_VIEWS_SHADOW_WIDGET_SHADOW_WIDGET_FRAME_VIEW_H

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "ui/gfx/insets.h"
#include "ui/views/window/non_client_view.h"

namespace views {

class ShadowWidgetBorder;

// The non-client frame view of bubble-styled widgets.
class VIEWS_EXPORT ShadowWidgetFrameView : public NonClientFrameView {
 public:
  // Internal class name.
  static const char kViewClassName[];

  explicit ShadowWidgetFrameView();
  virtual ~ShadowWidgetFrameView();

  // NonClientFrameView overrides:
  virtual gfx::Rect GetBoundsForClientView() const OVERRIDE;
  virtual gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const OVERRIDE;
  virtual int NonClientHitTest(const gfx::Point& point) OVERRIDE;
  virtual void GetWindowMask(const gfx::Size& size,
                             gfx::Path* window_mask) OVERRIDE;
  virtual void ResetWindowControls() OVERRIDE;
  virtual void UpdateWindowIcon() OVERRIDE;
  virtual void UpdateWindowTitle() OVERRIDE;

  // View overrides:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void Layout() OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;

  // Use bubble_border() and SetBubbleBorder(), not border() and set_border().
  ShadowWidgetBorder* bubble_border() const { return bubble_border_; }
  void SetBubbleBorder(ShadowWidgetBorder* border);

  gfx::Rect GetUpdatedWindowBounds(const gfx::Rect& content_rect);

 protected:
  // Returns the bounds for the monitor showing the specified |rect|.
  // This function is virtual to support testing environments.
  virtual gfx::Rect GetMonitorBounds(const gfx::Rect& rect);

 private:
  FRIEND_TEST_ALL_PREFIXES(BubbleFrameViewTest, GetBoundsForClientView);

  // The bubble border.
  ShadowWidgetBorder* bubble_border_;

  DISALLOW_COPY_AND_ASSIGN(ShadowWidgetFrameView);
};

}  // namespace views

#endif  // UI_VIEWS_SHADOW_WIDGET_SHADOW_WIDGET_FRAME_VIEW_H
