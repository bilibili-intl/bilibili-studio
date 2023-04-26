#include "json_fill_layout.h"

#include "ui\views\layout\fill_layout.h"

namespace {
  const char property_view_id[] = "layout.view.view_id";
}

namespace views {
  JsonFillLayout::JsonFillLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property)
    :JsonLayout(root, layout_property)
  {
  }

  JsonFillLayout::~JsonFillLayout()
  {

  }

  void JsonFillLayout::InitLayout()
  {
    __super::InitLayout();

    layout_manager_ = new views::FillLayout();
  }

  void JsonFillLayout::InitView()
  {
    __super::InitView();

    int id;
    if (layout_property_->GetInteger(property_view_id, &id))
    {
      views::View *view = root_->GetLayoutView(id);
      DCHECK(view);
      host_->AddChildView(view);
    }
  }
}