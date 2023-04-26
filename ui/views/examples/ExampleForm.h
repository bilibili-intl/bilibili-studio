#pragma once
//#include "base/time/time.h"
//#include "base/memory/scoped_ptr.h"
//#include <string>
//#include "ui/views/widget/widget_delegate.h"
//#include "ui/views/controls/combobox/combobox_listener.h"
//#include "ui/views/controls/combobox/combobox.h"
//#include "ui/views/controls/label.h"
//#include "ui/views/view.h"
////#include "base/strings/utf_string_conversions.h"
//#include "ui/base/models/combobox_model.h"
//#include "base/memory/scoped_vector.h"
//#include "ui/views/controls/button/text_button.h"
//#include "ui/views/controls/button/label_button.h"
//#include "ui/views/controls/button/image_button.h"
//#include "ui/views/controls/button/blue_button.h"
//#include "ui/views/layout/box_layout.h"
//#include "ui/views/layout/grid_layout.h"
//#include "ui/views/layout/fill_layout.h"
#include "base/memory/scoped_vector.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/models/combobox_model.h"
#include "ui/base/ui_base_paths.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/label.h"
//#include "ui/views/examples/bubble_example.h"
#include "ui/views/examples/button_example.h"
//#include "ui/views/examples/checkbox_example.h"
#include "ui/views/examples/combobox_example.h"
//#include "ui/views/examples/double_split_view_example.h"
//#include "ui/views/examples/label_example.h"
//#include "ui/views/examples/link_example.h"
//#include "ui/views/examples/menu_example.h"
//#include "ui/views/examples/message_box_example.h"
//#include "ui/views/examples/multiline_example.h"
//#include "ui/views/examples/progress_bar_example.h"
//#include "ui/views/examples/radio_button_example.h"
//#include "ui/views/examples/scroll_view_example.h"
//#include "ui/views/examples/single_split_view_example.h"
//#include "ui/views/examples/slider_example.h"
//#include "ui/views/examples/tabbed_pane_example.h"
//#include "ui/views/examples/table_example.h"
//#include "ui/views/examples/text_example.h"
//#include "ui/views/examples/textfield_example.h"
//#include "ui/views/examples/throbber_example.h"
//#include "ui/views/examples/tree_view_example.h"
#include "ui/views/examples/widget_example.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "base/message_loop/message_loop.h"
//#include "json_layout_example.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/blue_button.h"


namespace views {
	//class CommonView : public /*views::*/View
	//{
	//public:
	//	explicit CommonView(const wchar_t *pStr)
	//		: m_strName(pStr){};

	//	virtual ~CommonView(){}

	//	virtual void InitCtrlView() = 0;

	//	const string16& GetViewName() const { return m_strName; }
	//	/*views::*/View *GetContentView() { return this; }

	//private:
	//	string16 m_strName;
	//};

	//class ComboboxModelItemList : public ui::ComboboxModel
	//{
	//public:
	//	ComboboxModelItemList();
	//	virtual ~ComboboxModelItemList();

	//	// Overridden from ui::ComboboxModel
	//	virtual int GetItemCount() const override { m_listItem.size(); }
	//	virtual string16 GetItemAt(int nIndex) override{ return m_listItem[nIndex]->GetViewName(); }

	//	/*views::*/View *GetItemViewAt(int nIndex){ return m_listItem[nIndex]->GetContentView(); }
	//	void AddCommonView(CommonView *pView){ m_listItem.push_back(pView); }

	//private:
	//	ScopedVector<CommonView> m_listItem;

	//	DISALLOW_COPY_AND_ASSIGN(ComboboxModelItemList);
	//};

	//class ButtonView : public CommonView, public /*views::*/ButtonListener
	//{
	//public:
	//	ButtonView()
	//		: CommonView(L"ButtonView")
	//		, m_btnText(NULL)
	//		, m_btnLabel(NULL)
	//		, m_btnImg(NULL){}
	//	virtual ~ButtonView(){}

	//	// overridden from CommonViewInfo
	//	virtual void InitCtrlView() OVERRIDE
	//	{
	//		set_background(/*views::*/Background::CreateSolidBackground(SK_ColorBLUE));
	//		SetLayoutManager(new /*views::*/BoxLayout(/*views::*/BoxLayout::kVertical, 10, 10, 10));

	//		m_btnText = new /*views::*/TextButton(this, base::string16(L"text"));
	//		m_btnText->set_focusable(true);
	//		AddChildView(m_btnText);

	//		m_btnLabel = new /*views::*/LabelButton(this, base::string16(L"label"));
	//		m_btnLabel->set_focusable(true);
	//		AddChildView(m_btnLabel);

	//		/*views::*/LabelButton *pDisableBtn = new /*views::*/LabelButton(this, base::string16(L"disable"));
	//		pDisableBtn->SetStyle(/*views::*/Button::STYLE_BUTTON);
	//		pDisableBtn->SetState(/*views::*/Button::STATE_DISABLED);
	//		AddChildView(pDisableBtn);

	//		AddChildView(new /*views::*/BlueButton(this, base::string16(L"blue")));
	//	}

	//		// overridden from view
	//		virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
	//	{
	//		InitCtrlView();
	//	}

	//private:
	//	// overridden from ButtonListener
	//	virtual void ButtonPressed(/*views::*/Button* sender, const ui::Event& event) override
	//	{
	//		if (sender == m_btnText)
	//			;
	//		else if (sender == m_btnLabel)
	//			;
	//		else
	//			;
	//	}

	//private:
	//	/*views::*/TextButton *m_btnText;
	//	/*views::*/LabelButton *m_btnLabel;
	//	/*views::*/ImageButton *m_btnImg;
	//};

	//class ExampleForm : public /*views::*/WidgetDelegateView,
	//	public /*views::*/ComboboxListener
	//{
	//public:
	//	ExampleForm();
	//	virtual ~ExampleForm();

	//	// overridden from delegate
	//	virtual bool CanResize() const OVERRIDE{ return true; }
	//	virtual bool CanMaximize() const override { return true; }
	//	virtual string16 GetWindowTitle() const override { return L"wh test form"; }
	//	virtual View *GetContentsView() override { return this; }
	//	virtual void WindowClosing() override { base::MessageLoopForUI::current()->Quit(); }

	//	// overridden from view
	//	virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
	//	{
	//		if (details.is_add && details.child == this)
	//		{
	//			InitializeForm();

	//			set_background(/*views::*/Background::CreateStandardPanelBackground());
	//			/*views::*/GridLayout *layout = new /*views::*/GridLayout(this);
	//			SetLayoutManager(layout);

	//			/*views::*/ColumnSet *column = layout->AddColumnSet(0);
	//			column->AddPaddingColumn(0, 5);
	//			column->AddColumn(/*views::*/GridLayout::Alignment::FILL, /*views::*/GridLayout::Alignment::FILL, 1, /*views::*/GridLayout::USE_PREF, 0, 0);
	//			column->AddPaddingColumn(0, 5);

	//			layout->AddPaddingRow(0, 5);
	//			layout->StartRow(0, 0);
	//			layout->AddView(m_pCombox);

	//			if (m_listCmbItems.GetItemCount())
	//			{
	//				layout->StartRow(1, 0);
	//				m_pContent->SetLayoutManager(new /*views::*/FillLayout);
	//				m_pContent->AddChildView(m_listCmbItems.GetItemViewAt(0));
	//				layout->AddView(m_pContent);
	//			}

	//			layout->StartRow(0, 0);
	//			layout->AddView(m_pLabel);
	//			layout->AddPaddingRow(0, 5);
	//		}
	//	}

	//	// Overridden from ComboboxListener
	//	virtual void OnSelectedIndexChanged(/*views::*/Combobox* combobox) override
	//	{
	//		if (combobox->selected_index() < m_listCmbItems.GetItemCount())
	//		{
	//			m_pContent->RemoveAllChildViews(false);
	//			m_pContent->AddChildView(m_listCmbItems.GetItemViewAt(combobox->selected_index()));
	//			m_pContent->RequestFocus();
	//			m_pContent->Layout();

	//			m_pLabel->SetText(m_listCmbItems.GetItemAt(combobox->selected_index()));
	//		}
	//	}

	//private:
	//	void InitializeForm()
	//	{
	//		m_listCmbItems.AddCommonView(new ButtonView);
	//	}

	//private:
	//	ComboboxModelItemList m_listCmbItems;
	//	/*views::*/Combobox *m_pCombox;
	//	/*views::*/View *m_pContent;
	//	/*views::*/Label *m_pLabel;
	//};

}