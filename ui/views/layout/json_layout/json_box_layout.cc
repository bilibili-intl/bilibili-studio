#include "json_box_layout.h"

#include "ui\views\layout\fill_layout.h"
#include "ui\views\layout\box_layout.h"

namespace {
  const char property_layout_orientation[] = "layout.orientation";
  const char property_layout_inside_border_horizontal_spacing[] = "layout.inside_border_horizontal_spacing";
  const char property_layout_inside_border_vertical_spacing[] = "layout.inside_border_vertical_spacing";
  const char property_layout_between_child_spacing[] = "layout.between_child_spacing";
  const char property_layout_spread_blank_space[] = "layout.spread_blank_space";
  const char property_layout_views[] = "layout.views";
}

namespace views {
  JsonBoxLayout::JsonBoxLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property)
    :JsonLayout(root, layout_property)
  {
  }

  JsonBoxLayout::~JsonBoxLayout()
  {

  }

  void JsonBoxLayout::InitLayout()
  {
    __super::InitLayout();

    int orientation = 0;
    int inside_border_horizontal_spacing = 0;
    int inside_border_vertical_spacing = 0;
    int between_child_spacing = 0;
    layout_property_->GetInteger(
      property_layout_orientation, &orientation);
    layout_property_->GetInteger(
      property_layout_inside_border_horizontal_spacing,
      &inside_border_horizontal_spacing);
    layout_property_->GetInteger(
      property_layout_inside_border_vertical_spacing,
      &inside_border_vertical_spacing);
    layout_property_->GetInteger(
      property_layout_between_child_spacing,
      &between_child_spacing);

    views::BoxLayout* layout_manager = new views::BoxLayout(
      (views::BoxLayout::Orientation)orientation,
      inside_border_horizontal_spacing,
      inside_border_vertical_spacing,
      between_child_spacing);

    bool spread_blank_space;
    if (layout_property_->GetBoolean(
      property_layout_spread_blank_space,
      &spread_blank_space))
    {
      layout_manager->set_spread_blank_space(spread_blank_space);
    }

    layout_manager_ = layout_manager;
  }

  void JsonBoxLayout::InitView()
  {
    __super::InitView();

    const base::ListValue* view_list = NULL;
    if (layout_property_->GetList(property_layout_views, &view_list))
    {
      for (size_t i = 0; i < view_list->GetSize(); i++)
      {
        int view_id;
        if (!view_list->GetInteger(i, &view_id))
          continue;

        views::View *view = root_->GetLayoutView(view_id);
        DCHECK(view);
        host_->AddChildView(view);
      }
    }
  }
}