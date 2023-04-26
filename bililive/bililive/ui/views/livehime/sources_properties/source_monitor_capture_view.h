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
    CaptureViewport* viewport_view_ = nullptr;// ��ѡview
    views::View* operate_view_ = nullptr;// ȷ��/ȡ��view
    views::View* tip_view_ = nullptr;// ����������ʾview
    BililiveLabel* tip_rect_label_ = nullptr;
    BililiveLabel* rect_label_ = nullptr;
    bool valid_capture_ = false;// ��ǰ��ѡ�������Ƿ���Ч������������קҲ�����Ѿ���ѡ����ˣ�
    bool drag_size_now_ = false;// �Ƿ���������ק��
    gfx::Display capture_display_;// ��ǰ������ڵ���ʾ����Ļ����ߴ�
    gfx::Rect display_map_view_rect_;// ���������ʾ���Ĵ�������ߴ�
    bool can_drag_ = false;// �Ƿ����������קѡȡ
    gfx::Point down_pt_;// �������ʱ�����꣬��������ϵ
    gfx::Rect capture_rect_;// ��ȡ����Ĵ�������ߴ磬����ȷ�Ͻ�ȡʱҪתΪ��Ļ����ߴ�
    gfx::Rect hovered_wnd_rect_;// ������ָ��Ĵ��ڵ����򣬵�ǰ��������ϵ
    EndDialogClosure capture_result_callback_ ;//��������ص�
    DISALLOW_COPY_AND_ASSIGN(MonitorCaptureView);
};

#endif