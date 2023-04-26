#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_ROUND_BUBBLE_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_ROUND_BUBBLE_H

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

//带下三角的圆角背景图弹出窗口，deactivate非活动窗口时自动关播，可用于弹出菜单
class BililiveRoundBubbleView : public BililiveWidgetDelegate
{
public:
	enum class RoundBubbleType
	{
		Type_RectangleWhite,//长方形白色背景
		Type_SquareWhite,//正方形白色背景
		Type_SquareBlack//正方形黑色背景
	};
	struct LayoutInfo
	{
		int left_margin = 20;//contents_view左边距
		int top_margin = 20;//contents_view上边距
		//int right_margin = 20;//contents_view右边距
		//int bottom_margin = 30;//contents_view下边距
		gfx::Size view_size;
		gfx::Size contents_view_size;
		int popup_y_position_offset = 0;//弹框y轴偏移
	};

	static void ShowWindow(views::View* anchor_view, views::View* contents_view,
		RoundBubbleType bubble_type = RoundBubbleType::Type_RectangleWhite);

	BililiveRoundBubbleView(views::View* contents_view, RoundBubbleType bubble_type);
	virtual ~BililiveRoundBubbleView();
	int GetPopupYPositionOffset() {
		return layout_info_.popup_y_position_offset;
	};

protected:
	// WidgetDelegate
	views::View* GetContentsView() override { return this; }
	ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_NONE; }
	void OnWidgetActivationChanged(views::Widget* widget, bool active)override;
	// views::View
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
	gfx::Size GetPreferredSize() override;
	void OnPaintBackground(gfx::Canvas* canvas)override;
	void Layout()override;

private:
	void InitViews();

private:
	RoundBubbleType bubble_type_ = RoundBubbleType::Type_RectangleWhite;
	LayoutInfo layout_info_;
	views::View* contents_view_ = nullptr;
};



#endif