#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_MAIN_VIEW_STRETCH_CONTROL_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_MAIN_VIEW_STRETCH_CONTROL_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/controls/image_view.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/preset_material/preset_material_common.h"
#include "ui/views/widget\widget_delegate.h"


class PopupLeftToolViddupView :public views::WidgetDelegateView
{
public:
	static void ShowForm(views::View* anchor_view, views::View* content_view, views::View* detect_view);
	static void HideForm();
	static void CloseForm();
	static views::Widget* GetWidgetSingleInstance();

protected:
	explicit PopupLeftToolViddupView(views::View* content_view);
	virtual ~PopupLeftToolViddupView();

protected:
	views::View* GetContentsView() override { return this; }
};

class StretchControlViddupView :public views::View,
	public views::ButtonListener
{
public:
	class StretchImageButton :public BililiveImageButton
	{
	public:
		StretchImageButton(StretchControlViddupView* parent_view);
		~StretchImageButton() = default;

	protected:
		// CustomButton
		void StateChanged() override;

	private:
			StretchControlViddupView* parent_view_;
	};

	void ImageButtonStateChanged(bool is_hovered);
	
	StretchControlViddupView(views::View* main_view,views::View* tabarea_view, views::View* tabarea_dummy_view);
	~StretchControlViddupView();

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
	bool is_button_can_clicked_ = false;
	//CommonMessageBridge* parent_bridge_ = nullptr;
	views::View* main_view_ = nullptr;
	views::View* tabarea_view_ = nullptr;
	views::View* tabarea_dummy_view_ = nullptr;
	StretchImageButton* stretch_button_ = nullptr;
	base::OneShotTimer<StretchControlViddupView> can_clicked_timer_;//is_button_can_clicked_ Set a valid value pocket scheme
	base::RepeatingTimer<StretchControlViddupView> timer_;//Mouse position check timer
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_MAIN_VIEW_STRETCH_CONTROL_VIEW_H_