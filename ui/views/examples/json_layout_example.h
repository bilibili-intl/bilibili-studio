// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_EXAMPLES_JSON_LAYOUT_WIDGET_EXAMPLE_H_
#define UI_VIEWS_EXAMPLES_JSON_LAYOUT_WIDGET_EXAMPLE_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/examples/example_base.h"
#include "ui/views/widget/widget.h"

namespace views {
namespace examples {

// WidgetExample demonstrates how to create a popup widget.
class JsonLayoutExample : public ExampleBase, public ButtonListener {
 public:
   JsonLayoutExample();
   virtual ~JsonLayoutExample();

  // Overridden from ExampleBase:
  virtual void CreateExampleView(View* container) OVERRIDE;

 private:
  // Construct a button with the specified |label| and |tag| in |container|.
  void BuildButton(View* container, const std::string& label, int tag);

  // Construct a Widget for |sender|, initialize with |params|, and call Show().
  void ShowWidget(View* sender, Widget::InitParams params);

  // Overridden from ButtonListener:
  virtual void ButtonPressed(Button* sender, const ui::Event& event) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(JsonLayoutExample);
};

}  // namespace examples
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_JSON_LAYOUT_WIDGET_EXAMPLE_H_
