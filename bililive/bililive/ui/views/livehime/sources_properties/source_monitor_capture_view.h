#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MONITOR_CAPTURE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MONITOR_CAPTURE_VIEW_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "ui/gfx/display.h"
#include "ui/views/controls/button/button.h"

namespace
{
    class CaptureViewport;
}

class BililiveLabel;

class MonitorCaptureView
    : public BililiveWidgetDelegate,
      public views::ButtonListener
{
public:
    static void ShowForm(EndDialogClosure *capture_result_callback = nullptr);
    void SetCaptureResultCallback(EndDialogClosure* capture_result_callback);
protected:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;
    void WindowClosing() override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    void OnMouseMoved(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    bool OnKeyPressed(const ui::KeyEvent& event) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    MonitorCaptureView();
    ~MonitorCaptureView();

    void AdjustViewport(gfx::Rect* capture_rect = nullptr);
    void ApplyCapture();
    void AdjustTipView();

private:
    friend class CaptureViewport;
    CaptureViewport* viewport_view_ = nullptr;// 框选view
    views::View* operate_view_ = nullptr;// 确认/取消view
    views::View* tip_view_ = nullptr;// 截屏操作提示view
    BililiveLabel* tip_rect_label_ = nullptr;
    BililiveLabel* rect_label_ = nullptr;
    bool valid_capture_ = false;// 当前框选捕获区是否有效（可能正在拖拽也可能已经框选完毕了）
    bool drag_size_now_ = false;// 是否正处于拖拽中
    gfx::Display capture_display_;// 当前鼠标所在的显示器屏幕坐标尺寸
    gfx::Rect display_map_view_rect_;// 鼠标所在显示器的窗口坐标尺寸
    bool can_drag_ = false;// 是否允许进行拖拽选取
    gfx::Point down_pt_;// 左键按下时的坐标，窗口坐标系
    gfx::Rect capture_rect_;// 截取区域的窗口坐标尺寸，最终确认截取时要转为屏幕坐标尺寸
    gfx::Rect hovered_wnd_rect_;// 鼠标点所指向的窗口的区域，当前窗口坐标系
    EndDialogClosure capture_result_callback_ ;//截屏结果回调
    DISALLOW_COPY_AND_ASSIGN(MonitorCaptureView);
};

#endif