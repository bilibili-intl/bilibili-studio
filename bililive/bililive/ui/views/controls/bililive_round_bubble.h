#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_ROUND_BUBBLE_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_ROUND_BUBBLE_H

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

//�������ǵ�Բ�Ǳ���ͼ�������ڣ�deactivate�ǻ����ʱ�Զ��ز��������ڵ����˵�
class BililiveRoundBubbleView : public BililiveWidgetDelegate
{
public:
	enum class RoundBubbleType
	{
		Type_RectangleWhite,//�����ΰ�ɫ����
		Type_SquareWhite,//�����ΰ�ɫ����
		Type_SquareBlack//�����κ�ɫ����
	};
	struct LayoutInfo
	{
		int left_margin = 20;//contents_view��߾�
		int top_margin = 20;//contents_view�ϱ߾�
		//int right_margin = 20;//contents_view�ұ߾�
		//int bottom_margin = 30;//contents_view�±߾�
		gfx::Size view_size;
		gfx::Size contents_view_size;
		int popup_y_position_offset = 0;//����y��ƫ��
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