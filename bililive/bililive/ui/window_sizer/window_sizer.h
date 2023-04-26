
#ifndef CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_H_
#define CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/ui/host_desktop.h"
#include "ui/base/ui_base_types.h"
#include "ui/gfx/rect.h"

class Browser;

class MonitorInfoProvider {
 public:
  virtual ~MonitorInfoProvider() {}

  virtual gfx::Rect GetPrimaryDisplayWorkArea() const = 0;

  virtual gfx::Rect GetPrimaryDisplayBounds() const = 0;

  virtual gfx::Rect GetMonitorWorkAreaMatching(
      const gfx::Rect& match_rect) const = 0;
};

class WindowSizer {
 public:
  class StateProvider;

  WindowSizer(StateProvider* state_provider, const Browser* browser);

  WindowSizer(StateProvider* state_provider,
              MonitorInfoProvider* monitor_info_provider,
              const Browser* browser);

  virtual ~WindowSizer();

  class StateProvider {
   public:
    virtual ~StateProvider() {}

    virtual bool GetPersistentState(gfx::Rect* bounds,
                                    gfx::Rect* work_area,
                                    ui::WindowShowState* show_state) const = 0;

    virtual bool GetLastActiveWindowState(
        gfx::Rect* bounds,
        ui::WindowShowState* show_state) const = 0;
  };

  void DetermineWindowBoundsAndShowState(
      const gfx::Rect& specified_bounds,
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const;

  static void GetBrowserWindowBoundsAndShowState(
      const std::string& app_name,
      const gfx::Rect& specified_bounds,
      const Browser* browser,
      gfx::Rect* window_bounds,
      ui::WindowShowState* show_state);

  static gfx::Point GetDefaultPopupOrigin(const gfx::Size& size,
                                          chrome::HostDesktopType type);

  static const int kDesktopBorderSize;

  static const int kMaximumWindowWidth;

  static const int kWindowTilePixels;

#if defined(USE_ASH)
  static int GetForceMaximizedWidthLimit();
#endif

 private:
  enum Edge { TOP, LEFT, BOTTOM, RIGHT };

  bool GetLastWindowBounds(gfx::Rect* bounds,
                           ui::WindowShowState* show_state) const;

  bool GetSavedWindowBounds(gfx::Rect* bounds,
                            ui::WindowShowState* show_state) const;

  void GetDefaultWindowBounds(gfx::Rect* default_bounds) const;
#if defined(USE_ASH)
  void GetDefaultWindowBoundsAsh(gfx::Rect* default_bounds) const;
#endif

  void AdjustBoundsToBeVisibleOnMonitorContaining(
      const gfx::Rect& other_bounds,
      const gfx::Rect& saved_work_area,
      gfx::Rect* bounds) const;

#if defined(USE_ASH)
  bool GetBoundsOverrideAsh(gfx::Rect* bounds_in_screen,
                            ui::WindowShowState* show_state) const;
#endif

  ui::WindowShowState GetWindowDefaultShowState() const;

  scoped_ptr<StateProvider> state_provider_;
  scoped_ptr<MonitorInfoProvider> monitor_info_provider_;

  const Browser* browser_;

  DISALLOW_COPY_AND_ASSIGN(WindowSizer);
};

#endif  
