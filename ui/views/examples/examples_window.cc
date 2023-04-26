// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/examples/examples_window.h"

#include <string>

#include "base/memory/scoped_vector.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/models/combobox_model.h"
#include "ui/base/ui_base_paths.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/label.h"
#include "ui/views/examples/bubble_example.h"
#include "ui/views/examples/button_example.h"
#include "ui/views/examples/checkbox_example.h"
#include "ui/views/examples/combobox_example.h"
#include "ui/views/examples/double_split_view_example.h"
#include "ui/views/examples/label_example.h"
#include "ui/views/examples/link_example.h"
#include "ui/views/examples/menu_example.h"
#include "ui/views/examples/message_box_example.h"
#include "ui/views/examples/multiline_example.h"
#include "ui/views/examples/progress_bar_example.h"
#include "ui/views/examples/radio_button_example.h"
#include "ui/views/examples/scroll_view_example.h"
#include "ui/views/examples/single_split_view_example.h"
#include "ui/views/examples/slider_example.h"
#include "ui/views/examples/tabbed_pane_example.h"
#include "ui/views/examples/table_example.h"
#include "ui/views/examples/text_example.h"
#include "ui/views/examples/textfield_example.h"
#include "ui/views/examples/throbber_example.h"
#include "ui/views/examples/tree_view_example.h"
#include "ui/views/examples/widget_example.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "base/message_loop/message_loop.h"
#include "json_layout_example.h"

#include "ui/views/layout/box_layout.h"
#include "ui/views/controls/button/text_button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/blue_button.h"
#include "data_table_example.h"

namespace views {
namespace examples {

// Model for the examples that are being added via AddExample().
class ComboboxModelExampleList : public ui::ComboboxModel {
 public:
  ComboboxModelExampleList() {}
  virtual ~ComboboxModelExampleList() {}

  // Overridden from ui::ComboboxModel:
  virtual int GetItemCount() const OVERRIDE { return example_list_.size(); }
  virtual string16 GetItemAt(int index) OVERRIDE {
    return UTF8ToUTF16(example_list_[index]->example_title());
  }

  View* GetItemViewAt(int index) {
    return example_list_[index]->example_view();
  }

  void AddExample(ExampleBase* example) {
    example_list_.push_back(example);
  }

 private:
  ScopedVector<ExampleBase> example_list_;

  DISALLOW_COPY_AND_ASSIGN(ComboboxModelExampleList);
};

class ExamplesWindowContents : public WidgetDelegateView,
                               public ComboboxListener {
 public:
  ExamplesWindowContents(Operation operation)
      : combobox_(new Combobox(&combobox_model_)),
        example_shown_(new View),
        status_label_(new Label),
        operation_(operation) {
    instance_ = this;
    combobox_->set_listener(this);
  }
  virtual ~ExamplesWindowContents() {}

  // Prints a message in the status area, at the bottom of the window.
  void SetStatus(const std::string& status) {
    status_label_->SetText(UTF8ToUTF16(status));
  }

  static ExamplesWindowContents* instance() { return instance_; }

 private:
  // Overridden from WidgetDelegateView:
  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE {
    return ASCIIToUTF16("Views Examples");
  }
  virtual View* GetContentsView() OVERRIDE { return this; }
  virtual void WindowClosing() OVERRIDE {
    instance_ = NULL;
    if (operation_ == QUIT_ON_CLOSE)
      base::MessageLoopForUI::current()->Quit();
  }

  // Overridden from View:
  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE {
    if (details.is_add && details.child == this)
      InitExamplesWindow();
  }

  // Overridden from ComboboxListener:
  virtual void OnSelectedIndexChanged(Combobox* combobox) OVERRIDE {
    DCHECK_EQ(combobox, combobox_);
    DCHECK(combobox->selected_index() < combobox_model_.GetItemCount());
    example_shown_->RemoveAllChildViews(false);
    example_shown_->AddChildView(combobox_model_.GetItemViewAt(
        combobox->selected_index()));
    example_shown_->RequestFocus();
    SetStatus(std::string());
    Layout();
  }

  // Creates the layout within the examples window.
  void InitExamplesWindow() {
    AddExamples();

    set_background(Background::CreateStandardPanelBackground());
    GridLayout* layout = new GridLayout(this);
    SetLayoutManager(layout);
    ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, 5);
    column_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1,
                          GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 5);
    layout->AddPaddingRow(0, 5);
    layout->StartRow(0 /* no expand */, 0);
    layout->AddView(combobox_);

    if (combobox_model_.GetItemCount() > 0) {
      layout->StartRow(1, 0);
      example_shown_->SetLayoutManager(new FillLayout());
      example_shown_->AddChildView(combobox_model_.GetItemViewAt(0));
      layout->AddView(example_shown_);
    }

    layout->StartRow(0 /* no expand */, 0);
    layout->AddView(status_label_);
    layout->AddPaddingRow(0, 5);
  }

  // Adds all the individual examples to the combobox model.
  void AddExamples() {
    // Please keep this list in alphabetical order!
    combobox_model_.AddExample(new BubbleExample);
    combobox_model_.AddExample(new ButtonExample);
    combobox_model_.AddExample(new CheckboxExample);
    combobox_model_.AddExample(new ComboboxExample);
    combobox_model_.AddExample(new DoubleSplitViewExample);
    combobox_model_.AddExample(new LabelExample);
    combobox_model_.AddExample(new LinkExample);
    combobox_model_.AddExample(new MenuExample);
    combobox_model_.AddExample(new MessageBoxExample);
    combobox_model_.AddExample(new MultilineExample);
    combobox_model_.AddExample(new ProgressBarExample);
    combobox_model_.AddExample(new RadioButtonExample);
    combobox_model_.AddExample(new ScrollViewExample);
    combobox_model_.AddExample(new SingleSplitViewExample);
    combobox_model_.AddExample(new SliderExample);
    combobox_model_.AddExample(new TabbedPaneExample);
    combobox_model_.AddExample(new TableExample);
    combobox_model_.AddExample(new TextExample);
    combobox_model_.AddExample(new TextfieldExample);
    combobox_model_.AddExample(new ThrobberExample);
    combobox_model_.AddExample(new TreeViewExample);
    combobox_model_.AddExample(new WidgetExample);
    combobox_model_.AddExample(new JsonLayoutExample);
    combobox_model_.AddExample(new DataTableExample);
  }

  static ExamplesWindowContents* instance_;
  ComboboxModelExampleList combobox_model_;
  Combobox* combobox_;
  View* example_shown_;
  Label* status_label_;
  const Operation operation_;

  DISALLOW_COPY_AND_ASSIGN(ExamplesWindowContents);
};

//#include "ExampleForm.h"//----------------------------------------------------------------------------------------------------

class CommonView : public /*views::*/View
{
public:
	explicit CommonView(const wchar_t *pStr)
		: m_strName(pStr)
		, m_bCreated(false){};

	virtual ~CommonView(){}

	virtual void InitCtrlView() = 0;

	const string16& GetViewName() const { return m_strName; }
	View *GetContentView() { return this; }

protected:
	// Overridden from View:
	virtual void ViewHierarchyChanged(
		const ViewHierarchyChangedDetails& details) OVERRIDE{
		View::ViewHierarchyChanged(details);
		// We're not using child == this because a Widget may not be
		// available when this is added to the hierarchy.
		if (details.is_add && GetWidget() && !m_bCreated) {
			m_bCreated = true;
			InitCtrlView();
		}
	}

private:
	string16 m_strName;
	// True if the example view has already been created, or false otherwise.
	bool m_bCreated;
};

class ButtonView : public CommonView, public /*views::*/ButtonListener
{
public:
	ButtonView()
		: CommonView(L"ButtonView")
		, m_btnText(NULL)
		, m_btnLabel(NULL)
		, m_btnImg(NULL){}
	virtual ~ButtonView(){}

	// overridden from CommonViewInfo
	virtual void InitCtrlView() OVERRIDE
	{
		set_background(Background::CreateSolidBackground(SK_ColorBLUE));
		SetLayoutManager(new BoxLayout(BoxLayout::kVertical, 10, 10, 10));

		m_btnText = new TextButton(this, base::string16(L"text"));
		m_btnText->set_focusable(true);
		AddChildView(m_btnText);

		m_btnLabel = new LabelButton(this, base::string16(L"label"));
		m_btnLabel->set_focusable(true);
		AddChildView(m_btnLabel);

		LabelButton *pDisableBtn = new LabelButton(this, base::string16(L"disable"));
		pDisableBtn->SetStyle(Button::STYLE_BUTTON);
		pDisableBtn->SetState(Button::STATE_DISABLED);
		AddChildView(pDisableBtn);

		AddChildView(new BlueButton(this, base::string16(L"blue")));
	}

private:
	// overridden from ButtonListener
	virtual void ButtonPressed(Button* sender, const ui::Event& event) override
	{
		if (sender == m_btnText)
			;
		else if (sender == m_btnLabel)
			;
		else
			;
	}

private:
	TextButton *m_btnText;
	LabelButton *m_btnLabel;
	ImageButton *m_btnImg;
};

class ComboboxModelItemList : public ui::ComboboxModel
{
public:
	ComboboxModelItemList(){}
	virtual ~ComboboxModelItemList(){}

	// Overridden from ui::ComboboxModel
	virtual int GetItemCount() const override { return m_listItem.size(); }
	virtual string16 GetItemAt(int nIndex) override{ return m_listItem[nIndex]->GetViewName(); }

	/*views::*/View *GetItemViewAt(int nIndex){ return m_listItem[nIndex]->GetContentView(); }
	void AddCommonView(CommonView *pView){ m_listItem.push_back(pView); }

private:
	ScopedVector<CommonView> m_listItem;

	DISALLOW_COPY_AND_ASSIGN(ComboboxModelItemList);
};

class ExampleForm : public /*views::*/WidgetDelegateView,
	public /*views::*/ComboboxListener
{
public:
	ExampleForm()
			: m_pCombox(new Combobox(&m_listCmbItems))
			, m_pContent(new View)
			, m_pLabel(new Label)
		{
			m_pCombox->set_listener(this);
		}
	virtual ~ExampleForm(){}

	// overridden from delegate
	virtual bool CanResize() const OVERRIDE{ return true; }
	virtual bool CanMaximize() const override { return true; }
	virtual string16 GetWindowTitle() const override { return L"wh test form"; }
	// 该VIEW将作为ClientView的内容页，WidgetDelegate::CreateClientView时要new ClientView(widget, GetContentsView());这样创建客户区视图
	virtual View *GetContentsView() override { return this; }
	virtual void WindowClosing() override { base::MessageLoopForUI::current()->Quit(); }

	// overridden from view
	virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
	{
		if (details.is_add && details.child == this)
		{
			InitializeForm();

			set_background(/*views::*/Background::CreateStandardPanelBackground());
			/*views::*/GridLayout *layout = new /*views::*/GridLayout(this);
			SetLayoutManager(layout);

			/*views::*/ColumnSet *column = layout->AddColumnSet(0);
			column->AddPaddingColumn(0, 5);
			column->AddColumn(/*views::*/GridLayout::Alignment::FILL, /*views::*/GridLayout::Alignment::FILL, 1, /*views::*/GridLayout::USE_PREF, 0, 0);
			column->AddPaddingColumn(0, 5);

			layout->AddPaddingRow(0, 5);
			layout->StartRow(0, 0);
			layout->AddView(m_pCombox);

			if (m_listCmbItems.GetItemCount())
			{
				layout->StartRow(1, 0);
				m_pContent->SetLayoutManager(new /*views::*/FillLayout);
				m_pContent->AddChildView(m_listCmbItems.GetItemViewAt(0));
				layout->AddView(m_pContent);
			}

			layout->StartRow(0, 0);
			layout->AddView(m_pLabel);
			layout->AddPaddingRow(0, 5);
		}
	}

	// Overridden from ComboboxListener
	virtual void OnSelectedIndexChanged(/*views::*/Combobox* combobox) override
	{
		if (combobox->selected_index() < m_listCmbItems.GetItemCount())
		{
			m_pContent->RemoveAllChildViews(false);
			m_pContent->AddChildView(m_listCmbItems.GetItemViewAt(combobox->selected_index()));
			m_pContent->RequestFocus();
			m_pContent->Layout();

			m_pLabel->SetText(m_listCmbItems.GetItemAt(combobox->selected_index()));
		}
	}

private:
	void InitializeForm()
	{
		m_listCmbItems.AddCommonView(new ButtonView);
	}

private:
	ComboboxModelItemList m_listCmbItems;
	Combobox *m_pCombox;
	View *m_pContent;
	Label *m_pLabel;
};

// static
ExamplesWindowContents* ExamplesWindowContents::instance_ = NULL;

void ShowExamplesWindow(Operation operation) {
	if (ExamplesWindowContents::instance()) {
		ExamplesWindowContents::instance()->GetWidget()->Activate();
	}
	else {
		Widget::CreateWindowWithBounds(new ExamplesWindowContents(operation),
			gfx::Rect(0, 0, 850, 300))->Show();
	}

	//Widget::CreateWindowWithBounds(new ExampleForm, gfx::Rect(0, 0, 500, 500))->Show();
}

void LogStatus(const std::string& string) {
  ExamplesWindowContents::instance()->SetStatus(string);
}

}  // namespace examples
}  // namespace views
