#include "json_combo_layout.h"

#include "ui\views\layout\fill_layout.h"
#include "json_layout.h"

namespace {
  const char property_layout_views[] = "layout.views";
  const char property_layout_view_id[] = "view_id";
}

namespace views {
  JsonComboLayout::JsonComboLayout(
    JsonLayoutView *root,
    const base::DictionaryValue* layout_property)
    :JsonLayout(root, layout_property)
  {
  }

  JsonComboLayout::~JsonComboLayout()
  {

  }

  void JsonComboLayout::InitLayout()
  {
    //__super::InitLayout();

    const base::ListValue* views = NULL;
    if (layout_property_->GetList(property_layout_views, &views))
    {
      const base::DictionaryValue *view = NULL;
      for (size_t i = 0; i < views->GetSize(); i++)
      {
        if (!views->GetDictionary(i, &view))
          continue;

        int view_id = 0;
        if (!view->GetInteger(property_layout_view_id, &view_id))
          continue;

        if (view_id == host_->id())
        {
          JsonLayout *layout_manager = JsonLayout::CreateLayout(root_, view);
          layout_manager->InitLayout();
          layout_manager_ = layout_manager;
          break;
        }
      }
    }

    DCHECK(layout_manager_ != NULL);
  }

  void JsonComboLayout::InitView()
  {
    __super::InitView();

    JsonLayout *layout_manager = (JsonLayout *)layout_manager_;

    const base::ListValue* views = NULL;
    if (layout_property_->GetList(property_layout_views, &views))
    {
      const base::DictionaryValue *view = NULL;
      for (size_t i = 0; i < views->GetSize(); i++)
      {
        if (!views->GetDictionary(i, &view))
          continue;

        int view_id = 0;
        if (!view->GetInteger(property_layout_view_id, &view_id))
          continue;

        if (view_id != host_->id())
          JsonLayout::CreateLayout(root_, view);
        else
          layout_manager->InitView();
      }
    }

  }
}