// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/examples/json_layout_example.h"

#include "base/strings/utf_string_conversions.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/dialog_delegate.h"
#include "../layout/json_layout/json_layout_view.h"
#include "../layout/fill_layout.h"
#include "../controls/button/blue_button.h"

namespace views {
namespace examples {

namespace {

  std::string combolayout_test1 = "\
  {\
    \"view_id\":203,\
      \"type\" : \"combo_layout\",\
      \"layout\" :\
    {\
      \"views\":\
      [\
      {\
        \"view_id\":1000,\
          \"type\" : \"combo_layout\",\
          \"layout\" :\
        {\
          \"views\":\
          [\
          {\
            \"view_id\":1001,\
              \"type\" : \"combo_layout\",\
              \"layout\" :\
            {\
              \"views\":\
              [\
              {\
                \"view_id\":1001,\
                  \"type\" : \"box_layout\",\
                  \"layout\" :\
                {\
                  \"orientation\":1,\
                    \"inside_border_horizontal_spacing\" : 10,\
                    \"inside_border_vertical_spacing\" : 10,\
                    \"between_child_spacing\" : 10,\
                    \"spread_blank_space\" : true,\
                    \"views\" : [205, 206]\
                }\
              }\
              ]\
            }\
          },\
          {\
            \"view_id\":1000,\
            \"type\" : \"box_layout\",\
            \"layout\" :\
            {\
              \"orientation\":0,\
                \"inside_border_horizontal_spacing\" : 10,\
                \"inside_border_vertical_spacing\" : 10,\
                \"between_child_spacing\" : 10,\
                \"spread_blank_space\" : true,\
                \"views\" : [200, 201, 202, 1001]\
            }\
          }\
          ]\
        }\
      },\
      {\
        \"view_id\":203,\
        \"type\" : \"box_layout\",\
        \"layout\" :\
        {\
          \"orientation\":0,\
            \"inside_border_horizontal_spacing\" : 10,\
            \"inside_border_vertical_spacing\" : 10,\
            \"between_child_spacing\" : 10,\
            \"spread_blank_space\" : true,\
            \"views\" : [207, 208, 1000]\
        }\
      }\
      ]\
    }\
  }\
  ";

  std::string combolayout_test2 = "\
  {\
   \"view_id\":203,\
   \"type\" : \"combo_layout\",\
   \"layout\" :\
  {\
    \"views\":\
    [\
    {\
      \"view_id\":1000,\
        \"type\" : \"grid_layout\",\
        \"layout\" :\
      {\
        \"insets\":{\"top\":0, \"left\" : 0, \"bottom\" : 0, \"right\" : 0},\
          \"minimum_size\" : {\"width\":300, \"height\" : 500},\
          \"columnset\" :\
          [\
        {\
          \"column_set_id\":0,\
            \"columns\" :\
            [\
          {\
            \"h_align\":3,\
              \"v_align\" : 3,\
              \"resize_percent\" : 0.3,\
              \"size_type\" : 0,\
              \"fixed_width\" : 0,\
              \"min_width\" : 0\
          },\
          {\
            \"h_align\":3,\
            \"v_align\" : 3,\
            \"resize_percent\" : 0.3,\
            \"size_type\" : 0,\
            \"fixed_width\" : 0,\
            \"min_width\" : 0\
          }\
            ]\
        }\
          ],\
            \"viewset\":\
          [\
          {\
            \"column_set_id\":0,\
              \"rows\" :\
              [\
            {\
              \"vertical_resize\":0.0,\
                \"views\" : [\
                  200,\
                    201\
                ]\
            },\
            {\
              \"vertical_resize\":0.0,\
              \"views\" : [\
                208,\
                  204\
              ]\
            }\
              ]\
          }\
          ]\
      }\
    },\
    {\
      \"view_id\":203,\
      \"type\" : \"grid_layout\",\
      \"layout\" :\
      {\
        \"insets\":{\"top\":0, \"left\" : 0, \"bottom\" : 0, \"right\" : 0},\
          \"minimum_size\" : {\"width\":300, \"height\" : 500},\
          \"columnset\" :\
          [\
        {\
          \"column_set_id\":0,\
            \"columns\" :\
            [\
          {\
            \"h_align\":3,\
              \"v_align\" : 3,\
              \"resize_percent\" : 0.3,\
              \"size_type\" : 0,\
              \"fixed_width\" : 0,\
              \"min_width\" : 0\
          },\
          {\
            \"h_align\":3,\
            \"v_align\" : 3,\
            \"resize_percent\" : 0.3,\
            \"size_type\" : 0,\
            \"fixed_width\" : 0,\
            \"min_width\" : 0\
          }\
            ]\
        }\
          ],\
            \"viewset\":\
          [\
          {\
            \"column_set_id\":0,\
              \"rows\" :\
              [\
            {\
              \"vertical_resize\":0.0,\
                \"views\" : [\
                  205,\
                    206\
                ]\
            },\
            {\
              \"vertical_resize\":0.0,\
              \"views\" : [\
                207, 1000\
              ]\
            }\
              ]\
          }\
          ]\
      }\
    }\
    ]\
  }\
  }";

  std::string combolayout_test3 = "\
  {\
  \"view_id\":203,\
  \"type\" : \"combo_layout\",\
  \"layout\" :\
  {\
    \"views\":\
    [\
    {\
      \"view_id\":1000,\
        \"type\" : \"grid_layout\",\
        \"layout\" :\
      {\
        \"insets\":{\"top\":0, \"left\" : 0, \"bottom\" : 0, \"right\" : 0},\
          \"minimum_size\" : {\"width\":300, \"height\" : 500},\
          \"columnset\" :\
          [\
        {\
          \"column_set_id\":0,\
            \"columns\" :\
            [\
          {\
            \"h_align\":3,\
              \"v_align\" : 3,\
              \"resize_percent\" : 0.3,\
              \"size_type\" : 0,\
              \"fixed_width\" : 0,\
              \"min_width\" : 0\
          },\
          {\
            \"h_align\":3,\
            \"v_align\" : 3,\
            \"resize_percent\" : 0.3,\
            \"size_type\" : 0,\
            \"fixed_width\" : 0,\
            \"min_width\" : 0\
          }\
            ]\
        }\
          ],\
            \"viewset\":\
          [\
          {\
            \"column_set_id\":0,\
              \"rows\" :\
              [\
            {\
              \"vertical_resize\":0.0,\
                \"views\" : [\
                  200,\
                    201\
                ]\
            },\
            {\
              \"vertical_resize\":1,\
              \"views\" : [\
                208,\
                  204\
              ]\
            }\
              ]\
          }\
          ]\
      }\
    },\
    {\
      \"view_id\":203,\
      \"type\" : \"fill_layout\",\
      \"layout\" :\
      {\
        \"view\":{\"view_id\":1000}\
      }\
    }\
    ]\
  }\
}\
";

  std::string combolayout_test4 = "\
  {\
  \"view_id\":203,\
  \"type\" : \"combo_layout\",\
  \"layout\" :\
  {\
    \"views\":\
    [\
    {\
      \"view_id\":1000,\
        \"type\" : \"grid_layout\",\
        \"layout\" :\
      {\
        \"insets\":{\"top\":0, \"left\" : 0, \"bottom\" : 0, \"right\" : 0},\
          \"minimum_size\" : {\"width\":300, \"height\" : 500},\
          \"columnset\" :\
          [\
        {\
          \"column_set_id\":0,\
            \"columns\" :\
            [\
          {\
            \"h_align\":3,\
              \"v_align\" : 3,\
              \"resize_percent\" : 0.3,\
              \"size_type\" : 0,\
              \"fixed_width\" : 0,\
              \"min_width\" : 0\
          },\
          {\
            \"h_align\":3,\
            \"v_align\" : 3,\
            \"resize_percent\" : 0.3,\
            \"size_type\" : 0,\
            \"fixed_width\" : 0,\
            \"min_width\" : 0\
          }\
            ]\
        }\
          ],\
            \"viewset\":\
          [\
          {\
            \"column_set_id\":0,\
              \"rows\" :\
              [\
            {\
              \"vertical_resize\":0.0,\
                \"views\" : [\
                  200,\
                    201\
                ]\
            },\
            {\
              \"vertical_resize\":0.0,\
              \"views\" : [\
                208,\
                  204\
              ]\
            }\
              ]\
          }\
          ]\
      }\
    },\
    {\
      \"view_id\":203,\
      \"type\" : \"box_layout\",\
      \"layout\" :\
      {\
        \"orientation\":0,\
          \"inside_border_horizontal_spacing\" : 10,\
          \"inside_border_vertical_spacing\" : 10,\
          \"between_child_spacing\" : 10,\
          \"spread_blank_space\" : true,\
          \"views\" : [202, 1000, 205, 206, 207]\
      }\
    }\
    ]\
  }\
}\
";

  std::string gridlayout_test = "\
  {\
    \"view_id\":203,\
      \"type\" : \"grid_layout\",\
      \"layout\" :\
    {\
      \"insets\":{\"top\":0, \"left\" : 0, \"bottom\" : 0, \"right\" : 0},\
        \"minimum_size\" : {\"width\":300, \"height\" : 500},\
        \"columnset\" :\
        [\
      {\
        \"column_set_id\":0,\
          \"columns\" :\
          [\
        {\
          \"h_align\":3,\
            \"v_align\" : 3,\
            \"resize_percent\" : 0.3,\
            \"size_type\" : 0,\
            \"fixed_width\" : 0,\
            \"min_width\" : 0\
        },\
        {\
          \"resize_percent\":0,\
          \"width\" : 100\
        },\
        {\
          \"h_align\":3,\
          \"v_align\" : 3,\
          \"resize_percent\" : 0.3,\
          \"size_type\" : 0,\
          \"fixed_width\" : 0,\
          \"min_width\" : 0\
        },\
        {\
          \"resize_percent\":0,\
          \"width\" : 100\
        },\
        {\
          \"h_align\":3,\
          \"v_align\" : 3,\
          \"resize_percent\" : 0.4,\
          \"size_type\" : 0,\
          \"fixed_width\" : 0,\
          \"min_width\" : 0\
        },\
        {\
          \"resize_percent\":0,\
          \"width\" : 100\
        }\
          ]\
      },\
      {\
        \"column_set_id\":1,\
        \"columns\" :\
        [\
        {\
          \"h_align\":3,\
            \"v_align\" : 3,\
            \"resize_percent\" : 0,\
            \"size_type\" : 0,\
            \"fixed_width\" : 100,\
            \"min_width\" : 0\
        },\
        {\
          \"resize_percent\":0,\
          \"width\" : 400\
        },\
        {\
          \"h_align\":3,\
          \"v_align\" : 3,\
          \"resize_percent\" : 1,\
          \"size_type\" : 0,\
          \"fixed_width\" : 0,\
          \"min_width\" : 0\
        }\
        ]\
      }\
        ],\
          \"viewset\":\
        [\
        {\
          \"column_set_id\":0,\
            \"rows\" :\
            [\
          {\
            \"vertical_resize\":0.0,\
              \"views\" : [\
                200,\
                  0,\
                  202\
              ]\
          },\
          {\
            \"vertical_resize\":0.0,\
            \"size\" : 10\
          },\
          {\
            \"vertical_resize\":0.0,\
            \"views\" : [\
            {\
              \"view_id\":204,\
                \"h_align\" : 0,\
                \"v_align\" : 0,\
                \"pref_width\" : 0,\
                \"pref_height\" : 0\
            },\
            {\
              \"view_id\":205,\
              \"h_align\" : 1,\
              \"v_align\" : 0,\
              \"pref_width\" : 0,\
              \"pref_height\" : 0\
            },\
            {\
              \"view_id\":206,\
              \"h_align\" : 2,\
              \"v_align\" : 0,\
              \"pref_width\" : 0,\
              \"pref_height\" : 0\
            }\
            ]\
          },\
          {\
            \"vertical_resize\":0.0,\
            \"size\" : 10\
          }\
            ]\
        },\
        {\
          \"column_set_id\":1,\
          \"rows\" :\
          [\
          {\
            \"vertical_resize\":0.0,\
              \"views\" : [\
                207,\
                  208\
              ]\
          }\
          ]\
        }\
        ]\
    }\
  }";

  class LayoutTestWindowContents
    : public views::DialogDelegateView
    , public views::ButtonListener
  {
  public:
    LayoutTestWindowContents(std::string layout_json)
      : layout_json_(layout_json)
    {
    }
    virtual ~LayoutTestWindowContents() {}

  private:
    // Overridden from WidgetDelegateView:
    virtual string16 GetWindowTitle() const OVERRIDE{
      return ASCIIToUTF16("Layout test");
    }
    virtual View* GetContentsView() OVERRIDE{ return this; }

    // Overridden from View:
    virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE{
      if (details.is_add && details.child == this)
      InitExamplesWindow();
    }

    // Creates the layout within the examples window.
    void InitExamplesWindow() {
      AddExamplesViews();

      set_background(views::Background::CreateStandardPanelBackground());
    }

    virtual void ButtonPressed(views::Button* sender, const ui::Event& event)
    {

    }

    void AddExamplesViews() {
      view_ = new JsonLayoutView();
      SetLayoutManager(new views::FillLayout());
      view_->set_id(203);

      views::View *view = new views::BlueButton(this, L"200");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(200, view);

      view = new views::BlueButton(this, L"201");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(201, view);

      view = new views::BlueButton(this, L"202");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(202, view);

      view = new views::BlueButton(this, L"204");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(204, view);

      view = new views::BlueButton(this, L"205");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(205, view);

      view = new views::BlueButton(this, L"206");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(206, view);

      view = new views::BlueButton(this, L"207");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(207, view);

      view = new views::BlueButton(this, L"208");
      view->set_background(views::Background::CreateSolidBackground(255, 0, 0, 255));
      view_->AddLayoutView(208, view);

      view_->SetJsonLayoutString(layout_json_);
      AddChildView(view_);
    }

  private:
    JsonLayoutView *view_;
    std::string layout_json_;

    DISALLOW_COPY_AND_ASSIGN(LayoutTestWindowContents);
  };

}  // namespace

JsonLayoutExample::JsonLayoutExample() : ExampleBase("JsonLayoutWidget") {
}

JsonLayoutExample::~JsonLayoutExample() {
}

void JsonLayoutExample::CreateExampleView(View* container) {
  container->SetLayoutManager(new BoxLayout(BoxLayout::kHorizontal, 0, 0, 10));
  BuildButton(container, "GridLayout widget", 1);
  BuildButton(container, "ComboLayout widget1", 2);
  BuildButton(container, "ComboLayout widget2", 3);
  BuildButton(container, "ComboLayout widget3", 4);
  BuildButton(container, "ComboLayout widget4", 5);
}

void JsonLayoutExample::BuildButton(View* container,
                                const std::string& label,
                                int tag) {
  LabelButton* button = new LabelButton(this, ASCIIToUTF16(label));
  button->set_focusable(true);
  button->set_tag(tag);
  container->AddChildView(button);
}

void JsonLayoutExample::ShowWidget(View* sender, Widget::InitParams params) {
  // Setup shared Widget heirarchy and bounds parameters.
  params.parent = sender->GetWidget()->GetNativeView();
  params.bounds = gfx::Rect(sender->GetBoundsInScreen().CenterPoint(),
                            gfx::Size(300, 200));

  Widget* widget = new Widget();
  widget->Init(params);

  // If the Widget has no contents by default, add a view with a 'Close' button.
  if (!widget->GetContentsView()) {
    View* contents = new View();
    contents->SetLayoutManager(new BoxLayout(BoxLayout::kHorizontal, 0, 0, 0));
    contents->set_background(Background::CreateSolidBackground(SK_ColorGRAY));
    widget->SetContentsView(contents);
  }

  widget->Show();
}

void JsonLayoutExample::ButtonPressed(Button* sender, const ui::Event& event) {

  switch (sender->tag())
  {
  case 1:
  {
          DialogDelegate::CreateDialogWidget(new LayoutTestWindowContents(gridlayout_test), NULL,
            sender->GetWidget()->GetNativeView())->Show();
  }
    break;

  case 2:
  {

          DialogDelegate::CreateDialogWidget(new LayoutTestWindowContents(combolayout_test1), NULL,
            sender->GetWidget()->GetNativeView())->Show();
  }
    break;

  case 3:
  {
          DialogDelegate::CreateDialogWidget(new LayoutTestWindowContents(combolayout_test2), NULL,
            sender->GetWidget()->GetNativeView())->Show();
  }
    break;

  case 4:
  {
          DialogDelegate::CreateDialogWidget(new LayoutTestWindowContents(combolayout_test3), NULL,
            sender->GetWidget()->GetNativeView())->Show();
  }
    break;

  case 5:
  {
          DialogDelegate::CreateDialogWidget(new LayoutTestWindowContents(combolayout_test4), NULL,
            sender->GetWidget()->GetNativeView())->Show();
  }
    break;

  default:
    break;
  }
 
}

}  // namespace examples
}  // namespace views
