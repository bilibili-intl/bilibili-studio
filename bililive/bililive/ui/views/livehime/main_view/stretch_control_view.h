#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STRETCH_CONTROL_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STRETCH_CONTROL_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/controls/image_view.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/preset_material/preset_material_common.h"
#include "ui/views/widget\widget_delegate.h"

class PopupLeftToolView :public views::WidgetDelegateView
{
public:
	static void ShowForm(views::View* anchor_view, views::View* content_view, views::View* detect_view);
	static void HideForm();
	static void CloseForm();
	static views::Widget* GetWidgetSingleInstance();

protected:
	explicit PopupLeftToolView(views::View* content_view);
	virtual ~PopupLeftToolView();

protected:
	views::View* GetContentsView() override { return this; }

};



class StretchControlView :public views::View,
	public views::ButtonListener
{
public:
	class StretchImageButton :public BililiveImageButton
	{
	public:
		StretchImageButton(StretchControlView* parent_view);
		~StretchImageButton() = default;
	protected:
		// CustomButton
		void StateChanged() override;
	private:
		StretchControlView* parent_view_;
	};

	void ImageButtonStateChanged(bool is_hovered);

    StretchControlView(views::View* main_view,views::View* tabarea_view, views::View* tabarea_dummy_view);
    ~StretchControlView();

	void Quit();
	void SetStretchButtonCanClicked(bool can_clicked) { is_button_can_clicked_ = can_clicked; }

protected:
	// views::View,
	virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
	//ButtonListener
	void ButtonPressed(views::Button* sender, const ui::Event& event)override;
private:
	void InitView();
	void StartDoCheckMouse();
	void StopDoCheckMouse();
	void OnCheckMouse();
	void OnStretchButtonCanClicked();
private:
	bool is_stretch_ = true;
	bool is_button_can_clicked_ = false;//收起按钮是否可以点击，刚开始时是不可以点击的，等待新人引导界面完成后才可以，不然新人引导界面指示模块不对
	//CommonMessageBridge* parent_bridge_ = nullptr;//与父类BililiveOBSView通信
	views::View* main_view_ = nullptr;//BililiveOBSView父类
	views::View* tabarea_view_ = nullptr;//左侧工具栏整个view
	views::View* tabarea_dummy_view_ = nullptr;//左侧工具栏整个view在BililiveOBSView中的包装父类
	StretchImageButton* stretch_button_ = nullptr;
	base::OneShotTimer<StretchControlView> can_clicked_timer_;//is_button_can_clicked_ 设置有效值兜底方案
	base::RepeatingTimer<StretchControlView> timer_;//鼠标位置检查定时器
};


#endif