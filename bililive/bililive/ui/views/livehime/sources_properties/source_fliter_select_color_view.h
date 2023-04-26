#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_FLITER_SELECT_COLOR_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_FLITER_SELECT_COLOR_VIEW_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

class LivehimeContentLabel;
class SourceFilterBaseView;
class views::View;

class SourceFliterSelectColorView
    :public BililiveWidgetDelegate {

public:
    SourceFliterSelectColorView(SourceFilterBaseView* view);
    ~SourceFliterSelectColorView();

    static void ShowWindow(SourceFilterBaseView* view = nullptr);
    static SkColor GetSelectColor();
protected:
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void OnMouseMoved(const ui::MouseEvent& event) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    void WindowClosing() override;

    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;

private:
    void InitViews();
    void AdjustTipViewPos();
    SkColor GetCursorPosColor();
    void UpdateTipsViewColorVal(const SkColor& color);

    views::View* tip_view_;
    gfx::Rect display_view_rect_;   // 鼠标所在显示器的窗口坐标尺寸
    LivehimeContentLabel* text_label_;
    SourceFilterBaseView* filter_view_;
};

#endif // !BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_FLITER_SELECT_COLOR_VIEW_H_
