#include "json_grid_layout.h"

#include "ui\views\layout\grid_layout.h"

namespace {
  const char property_layout_insets_top[] = "layout.insets.top";
  const char property_layout_insets_left[] = "layout.insets.left";
  const char property_layout_insets_bottom[] = "layout.insets.bottom";
  const char property_layout_insets_right[] = "layout.insets.right";

  const char property_layout_minimum_size_width[] = "layout.minimum_size.width";
  const char property_layout_minimum_size_height[] = "layout.minimum_size.height";

  const char property_layout_columnset[] = "layout.columnset";
  const char columnset_column_set_id[] = "column_set_id";
  const char columnset_columns[] = "columns";
  const char columnset_columns_h_align[] = "h_align";
  const char columnset_columns_v_align[] = "v_align";
  const char columnset_columns_resize_percent[] = "resize_percent";
  const char columnset_columns_size_type[] = "size_type";
  const char columnset_columns_fixed_width[] = "fixed_width";
  const char columnset_columns_min_width[] = "min_width";
  const char columnset_columns_width[] = "width";

  const char property_layout_viewset[] = "layout.viewset";
  const char rowset_rows[] = "rows";
  const char rowset_rows_vertical_resize[] = "vertical_resize";
  const char rowset_rows_views[] = "views";
  const char rowset_rows_size[] = "size";

  const char rowset_rows_view_view_id[] = "view_id";
  const char rowset_rows_view_h_align[] = "h_align";
  const char rowset_rows_view_v_align[] = "v_align";
  const char rowset_rows_view_pref_width[] = "pref_width";
  const char rowset_rows_view_pref_height[] = "pref_height";
}

namespace views {
  JsonGridLayout::JsonGridLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property)
    :JsonLayout(root, layout_property)
  {
  }

  JsonGridLayout::~JsonGridLayout()
  {

  }

  void JsonGridLayout::InitLayout()
  {
    __super::InitLayout();

    layout_manager_ = new views::GridLayout(host_);

    SetLayoutInsets();

    SetLayoutMinSize();

    CreateLayoutColumnSet();
  }

  void JsonGridLayout::InitView()
  {
    __super::InitView();

    CreateViewSet();
  }

  void JsonGridLayout::SetLayoutInsets()
  {
    views::GridLayout *layout_manager = (views::GridLayout *)layout_manager_;

    int insets_top = 0;
    int insets_left = 0;
    int insets_bottom = 0;
    int insets_right = 0;

    layout_property_->GetInteger(property_layout_insets_top, &insets_top);
    layout_property_->GetInteger(property_layout_insets_left, &insets_left);
    layout_property_->GetInteger(property_layout_insets_bottom, &insets_bottom);
    layout_property_->GetInteger(property_layout_insets_right, &insets_right);
    layout_manager->SetInsets(insets_top, insets_left, insets_bottom, insets_right);
  }

  void JsonGridLayout::SetLayoutMinSize()
  {
    views::GridLayout *layout_manager = (views::GridLayout *)layout_manager_;

    int minimum_size_width = 0;
    int minimum_size_height = 0;

    layout_property_->GetInteger(property_layout_minimum_size_width, &minimum_size_width);
    layout_property_->GetInteger(property_layout_minimum_size_height, &minimum_size_height);

    layout_manager->set_minimum_size(gfx::Size(minimum_size_width, minimum_size_height));
  }

  void JsonGridLayout::CreateLayoutColumnSet()
  {
    views::GridLayout *layout_manager = (views::GridLayout *)layout_manager_;

    const base::ListValue* column_set_list = NULL;
    if (layout_property_->GetList(property_layout_columnset, &column_set_list))
    {
      for (size_t i = 0; i < column_set_list->GetSize(); i++)
      {
        const base::DictionaryValue *column_set = NULL;
        if (!column_set_list->GetDictionary(i, &column_set))
          continue;

        int column_set_id;
        if (!column_set->GetInteger(columnset_column_set_id, &column_set_id))
          continue;

        views::ColumnSet* control_column_set = layout_manager->AddColumnSet(column_set_id);

        const base::ListValue* columns = NULL;
        if (column_set->GetList(columnset_columns, &columns))
        {
          for (size_t j = 0; j < columns->GetSize(); j++)
          {
            const base::DictionaryValue *column = NULL;
            if (!columns->GetDictionary(j, &column))
              continue;

            CreateColumn(control_column_set, column);
          }
        }
      }
    }
  }

  void JsonGridLayout::CreateColumn(views::ColumnSet* column_set, const base::DictionaryValue *column)
  {
    int h_align = 3;
    int v_align = 3;
    double resize_percent = 0;
    int size_type = 1;
    int fixed_width = 0;
    int min_width = 0;
    int width = 0;

    bool r_h_align = column->GetInteger(columnset_columns_h_align, &h_align);
    bool r_v_align = column->GetInteger(columnset_columns_v_align, &v_align);
    bool r_resize_percent = column->GetDouble(columnset_columns_resize_percent, &resize_percent);
    bool r_size_type = column->GetInteger(columnset_columns_size_type, &size_type);
    bool r_fixed_width = column->GetInteger(columnset_columns_fixed_width, &fixed_width);
    bool r_min_width = column->GetInteger(columnset_columns_min_width, &min_width);
    bool r_width = column->GetInteger(columnset_columns_width, &width);

    if (r_h_align || r_v_align || r_size_type || r_min_width || r_fixed_width)
    {
      column_set->AddColumn(
        (views::GridLayout::Alignment)h_align,
        (views::GridLayout::Alignment)v_align,
        resize_percent,
        (views::GridLayout::SizeType)size_type,
        fixed_width,
        min_width);
    }
    else
    {
      column_set->AddPaddingColumn(resize_percent, width);
    }
  }

  void JsonGridLayout::CreateViewSet()
  {
    views::GridLayout *layout_manager = (views::GridLayout *)layout_manager_;

    const base::ListValue* view_set_list = NULL;
    if (layout_property_->GetList(property_layout_viewset, &view_set_list))
    {
      for (size_t i = 0; i < view_set_list->GetSize(); i++)
      {
        const base::DictionaryValue *row_set = NULL;
        if (!view_set_list->GetDictionary(i, &row_set))
          continue;

        int column_set_id;
        if (!row_set->GetInteger(columnset_column_set_id, &column_set_id))
          continue;

        const base::ListValue *rows = NULL;
        if (!row_set->GetList(rowset_rows, &rows))
          continue;

        for (size_t j = 0; j < rows->GetSize(); j++)
        {
          const base::DictionaryValue *row = NULL;
          if (!rows->GetDictionary(j, &row))
            continue;

          CreateRow(column_set_id, row);
        }
      }
    }
  }

  void JsonGridLayout::CreateRow(int column_set_id, const base::DictionaryValue *row)
  {
    views::GridLayout *layout_manager = (views::GridLayout *)layout_manager_;

    double vertical_resize = 0;
    row->GetDouble(rowset_rows_vertical_resize, &vertical_resize);

    const base::ListValue *views = NULL;
    if (!row->GetList(rowset_rows_views, &views))
    {
      int size = 0;
      row->GetInteger(rowset_rows_size, &size);
      layout_manager->AddPaddingRow(vertical_resize, size);
      return;
    }

    layout_manager->StartRow(vertical_resize, column_set_id);

    for (size_t i = 0; i < views->GetSize(); i++)
    {
      int view_id = 0;
      if (views->GetInteger(i, &view_id))
      {
        if (view_id == 0)
        {
          layout_manager->SkipColumns(1);
          continue;
        }
        views::View *view = root_->GetLayoutView(view_id);
        DCHECK(view);
        layout_manager->AddView(view);
      }

      const base::DictionaryValue *view = NULL;
      if (views->GetDictionary(i, &view))
      {
        int view_id = 0;
        int h_align = 0;
        int v_align = 0;
        int pref_width = 0;
        int pref_height = 0;

        if (!view->GetInteger(rowset_rows_view_view_id, &view_id))
          continue;

        view->GetInteger(rowset_rows_view_h_align, &h_align);
        view->GetInteger(rowset_rows_view_v_align, &v_align);
        view->GetInteger(rowset_rows_view_pref_width, &pref_width);
        view->GetInteger(rowset_rows_view_pref_height, &pref_height);

        views::View *view = root_->GetLayoutView(view_id);
        DCHECK(view);
        layout_manager->AddView(view, 1, 1,
          (views::GridLayout::Alignment)h_align,
          (views::GridLayout::Alignment)v_align,
          pref_width, pref_height);
      }
    }
  }
}