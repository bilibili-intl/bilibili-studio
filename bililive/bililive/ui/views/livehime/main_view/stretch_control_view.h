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
	bool is_button_can_clicked_ = false;//����ť�Ƿ���Ե�����տ�ʼʱ�ǲ����Ե���ģ��ȴ���������������ɺ�ſ��ԣ���Ȼ������������ָʾģ�鲻��
	//CommonMessageBridge* parent_bridge_ = nullptr;//�븸��BililiveOBSViewͨ��
	views::View* main_view_ = nullptr;//BililiveOBSView����
	views::View* tabarea_view_ = nullptr;//��๤��������view
	views::View* tabarea_dummy_view_ = nullptr;//��๤��������view��BililiveOBSView�еİ�װ����
	StretchImageButton* stretch_button_ = nullptr;
	base::OneShotTimer<StretchControlView> can_clicked_timer_;//is_button_can_clicked_ ������Чֵ���׷���
	base::RepeatingTimer<StretchControlView> timer_;//���λ�ü�鶨ʱ��
};


#endif