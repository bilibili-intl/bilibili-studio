#include "json_layout.h"
#include "json_fill_layout.h"
#include "base\values.h"
#include "base\file_util.h"
#include "base\json\json_file_value_serializer.h"
#include "base\json\json_string_value_serializer.h"
#include "json_box_layout.h"
#include "json_grid_layout.h"
#include "json_combo_layout.h"

namespace {
  const char property_view_id[] = "view_id";
  const char property_type[] = "type";

  const char fill_layout[] = "fill_layout";
  const char box_layout[] = "box_layout";
  const char grid_layout[] = "grid_layout";
  const char combo_layout[] = "combo_layout";

}

namespace views {
  base::DictionaryValue* JsonLayout::LoadJson(const base::FilePath& json_path)
  {
    if (!base::PathExists(json_path)) {
      return NULL;
    }

    JSONFileValueSerializer serializer(json_path);
    scoped_ptr<base::Value> root(serializer.Deserialize(NULL, NULL));
    if (!root.get()) {
      return NULL;
    }

    if (!root->IsType(base::Value::TYPE_DICTIONARY)) {
      return NULL;
    }

    return static_cast<base::DictionaryValue*>(root.release());
  }

  JsonLayout *JsonLayout::CreateLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property)
  {
    DCHECK(root->id() != 0);
    DCHECK(layout_property != NULL);

    root->AddJsonLayoutView(root->id(), root);

    std::string type;
    if (layout_property->GetString(property_type, &type))
    {
      if (type == fill_layout)
      {
        return new JsonFillLayout(root, layout_property);
      }
      else if (type == box_layout)
      {
        return new JsonBoxLayout(root, layout_property);
      }
      else if (type == grid_layout)
      {
        return new JsonGridLayout(root, layout_property);
      }
      else if (type == combo_layout)
      {
        return new JsonComboLayout(root, layout_property);
      }
      else
      {
        DCHECK(false);
      }
    }

    return NULL;
  }

  base::DictionaryValue* JsonLayout::LoadJson(const std::string &layout_json)
  {
    JSONStringValueSerializer serializer(layout_json);
    scoped_ptr<base::Value> root(serializer.Deserialize(NULL, NULL));
    if (!root.get()) {
      return NULL;
    }

    if (!root->IsType(base::Value::TYPE_DICTIONARY)) {
      return NULL;
    }

    return static_cast<base::DictionaryValue*>(root.release());
  }

  JsonLayout::JsonLayout(
    JsonLayoutView *root,
    const base::DictionaryValue* layout_property)
    : layout_manager_(NULL)
    , root_(root)
    , host_(NULL)
    , layout_property_(layout_property)
  {
    std::string type;
    if (!layout_property_->GetString(property_type, &type))
      DCHECK(false);

    int view_id;
    if (layout_property_->GetInteger(property_view_id, &view_id))
    {
      if (root_->HasJsonLayoutView(view_id))
      {
        host_ = root_->GetJsonLayoutView(view_id);
      }
      else
      {
        host_ = new JsonLayoutView();
        root_->AddJsonLayoutView(view_id, host_);
        host_->layout_ = this;
      }
    }
  }

  void JsonLayout::InitLayout()
  {

  }

  void JsonLayout::InitView()
  {
    host_->SetLayoutManager(this);
  }

  JsonLayout::~JsonLayout()
  {

  }

  void JsonLayout::Installed(views::View* host)
  {
    DCHECK(layout_manager_);
    layout_manager_->Installed(host);
  }

  void JsonLayout::Uninstalled(views::View* host)
  {
    DCHECK(layout_manager_);
    layout_manager_->Uninstalled(host);
  }

  void JsonLayout::Layout(views::View* host)
  {
    DCHECK(layout_manager_);
    layout_manager_->Layout(host);
  }

  gfx::Size JsonLayout::GetPreferredSize(views::View* host)
  {
    DCHECK(layout_manager_);
    return layout_manager_->GetPreferredSize(host);
  }

  int JsonLayout::GetPreferredHeightForWidth(views::View* host, int width)
  {
    DCHECK(layout_manager_);
    return layout_manager_->GetPreferredHeightForWidth(host, width);
  }

  void JsonLayout::ViewAdded(views::View* host, views::View* view)
  {
    DCHECK(layout_manager_);
    layout_manager_->ViewAdded(host, view);
  }

  void JsonLayout::ViewRemoved(views::View* host, views::View* view)
  {
    DCHECK(layout_manager_);
    layout_manager_->ViewRemoved(host, view);
  }
}