
#ifndef CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_COMMON_UNITTEST_H_
#define CHROME_BROWSER_UI_WINDOW_SIZER_WINDOW_SIZER_COMMON_UNITTEST_H_

#include <vector>

#include "base/logging.h"
#include "chrome/browser/ui/window_sizer/window_sizer.h"
#include "chrome/test/base/test_browser_window.h"
#include "content/public/test/test_browser_thread.h"
#include "ui/gfx/rect.h"

static const gfx::Rect p1024x768(0, 0, 1024, 768);
static const gfx::Rect p1280x1024(0, 0, 1280, 1024);
static const gfx::Rect p1600x1200(0, 0, 1600, 1200);
static const gfx::Rect p1680x1050(0, 0, 1680, 1050);
static const gfx::Rect p1920x1200(0, 0, 1920, 1200);

static const gfx::Rect left_s1024x768(-1024, 0, 1024, 768);

static const gfx::Rect right_s1024x768(1024, 0, 1024, 768);

static const gfx::Rect top_s1024x768(0, -768, 1024, 768);

static const gfx::Rect bottom_s1024x768(0, 768, 1024, 768);

static const gfx::Rect bottom_s1600x1200(0, 1200, 1600, 1200);

static const gfx::Rect taskbar_bottom_work_area(0, 0, 1024, 734);
static const gfx::Rect taskbar_top_work_area(0, 34, 1024, 734);
static const gfx::Rect taskbar_left_work_area(107, 0, 917, 768);
static const gfx::Rect taskbar_right_work_area(0, 0, 917, 768);

extern int kWindowTilePixels;

class TestMonitorInfoProvider : public MonitorInfoProvider {
 public:
  TestMonitorInfoProvider();
  virtual ~TestMonitorInfoProvider();

  void AddMonitor(const gfx::Rect& bounds, const gfx::Rect& work_area);

  virtual gfx::Rect GetPrimaryDisplayWorkArea() const OVERRIDE;

  virtual gfx::Rect GetPrimaryDisplayBounds() const OVERRIDE;

  virtual gfx::Rect GetMonitorWorkAreaMatching(
      const gfx::Rect& match_rect) const OVERRIDE;

 private:
  size_t GetMonitorIndexMatchingBounds(const gfx::Rect& match_rect) const;

  std::vector<gfx::Rect> monitor_bounds_;
  std::vector<gfx::Rect> work_areas_;

  DISALLOW_COPY_AND_ASSIGN(TestMonitorInfoProvider);
};

class TestStateProvider : public WindowSizer::StateProvider {
 public:
  TestStateProvider();
  virtual ~TestStateProvider() {}

  void SetPersistentState(const gfx::Rect& bounds,
                          const gfx::Rect& work_area,
                          ui::WindowShowState show_state,
                          bool has_persistent_data);
  void SetLastActiveState(const gfx::Rect& bounds,
                          ui::WindowShowState show_state,
                          bool has_last_active_data);

  virtual bool GetPersistentState(
      gfx::Rect* bounds,
      gfx::Rect* saved_work_area,
      ui::WindowShowState* show_state) const OVERRIDE;
  virtual bool GetLastActiveWindowState(
      gfx::Rect* bounds,
      ui::WindowShowState* show_state) const OVERRIDE;

 private:
  gfx::Rect persistent_bounds_;
  gfx::Rect persistent_work_area_;
  bool has_persistent_data_;
  ui::WindowShowState persistent_show_state_;

  gfx::Rect last_active_bounds_;
  bool has_last_active_data_;
  ui::WindowShowState last_active_show_state_;

  DISALLOW_COPY_AND_ASSIGN(TestStateProvider);
};


enum Source { DEFAULT, LAST_ACTIVE, PERSISTED, BOTH };

void GetWindowBoundsAndShowState(const gfx::Rect& monitor1_bounds,
                                 const gfx::Rect& monitor1_work_area,
                                 const gfx::Rect& monitor2_bounds,
                                 const gfx::Rect& bounds,
                                 const gfx::Rect& work_area,
                                 ui::WindowShowState show_state_persisted,
                                 ui::WindowShowState show_state_last,
                                 Source source,
                                 const Browser* browser,
                                 const gfx::Rect& passed_in,
                                 gfx::Rect* out_bounds,
                                 ui::WindowShowState* out_show_state);

void GetWindowBounds(const gfx::Rect& monitor1_bounds,
                     const gfx::Rect& monitor1_work_area,
                     const gfx::Rect& monitor2_bounds,
                     const gfx::Rect& bounds,
                     const gfx::Rect& work_area,
                     Source source,
                     const Browser* browser,
                     const gfx::Rect& passed_in,
                     gfx::Rect* out_bounds);

ui::WindowShowState GetWindowShowState(
    ui::WindowShowState show_state_persisted,
    ui::WindowShowState show_state_last,
    Source source,
    const Browser* browser,
    const gfx::Rect& display_config);

#endif  
