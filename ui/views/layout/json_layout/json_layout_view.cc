#include "json_layout_view.h"
#include "json_layout.h"

namespace views {
  JsonLayoutView::JsonLayoutView()
    : layout_(NULL)
  {

  }

  JsonLayoutView::~JsonLayoutView()
  {

  }

  void JsonLayoutView::AddJsonLayoutView(int id, JsonLayoutView *view)
  {
    if (view->id() != 0)
      DCHECK(view->id() == id);
    else
      view->set_id(id);

    if (json_layout_views_.find(id) != json_layout_views_.end()
      && json_layout_views_[id] != view)
    {
      DCHECK(false);
    }

    json_layout_views_[id] = view;
  }

  void JsonLayoutView::AddLayoutView(int id, views::View *view)
  {
    if (layout_views_.find(id) != layout_views_.end()
      && layout_views_[id] != view)
    {
      DCHECK(false);
    }

    layout_views_[id] = view;
  }

  views::View *JsonLayoutView::GetLayoutView(int id)
  {
    if (HasJsonLayoutView(id))
      return GetJsonLayoutView(id);

    DCHECK(layout_views_.find(id) != layout_views_.end());
    return layout_views_[id];
  }

  bool JsonLayoutView::HasLayoutView(int id)
  {
    if (HasJsonLayoutView(id))
      return true;

    return layout_views_.find(id) != layout_views_.end();
  }

  JsonLayoutView* JsonLayoutView::GetJsonLayoutView(int id)
  {
    DCHECK(json_layout_views_.find(id) != json_layout_views_.end());
    return json_layout_views_[id];
  }

  bool JsonLayoutView::HasJsonLayoutView(int id)
  {
    return json_layout_views_.find(id) != json_layout_views_.end();
  }

  void JsonLayoutView::SetJsonLayoutString(const std::string &layout_json)
  {
    layout_ = JsonLayout::CreateLayout(this, JsonLayout::LoadJson(layout_json));
  }

  void JsonLayoutView::SetJsonLayoutPath(const base::FilePath& json_path)
  {
    layout_ = JsonLayout::CreateLayout(this, JsonLayout::LoadJson(json_path));
  }

  void JsonLayoutView::SetLayoutDictionary(const base::DictionaryValue* layout_property)
  {
    layout_ = JsonLayout::CreateLayout(this, layout_property);
  }

  void JsonLayoutView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
  {
    if (details.is_add && details.child == this)
    {
      layout_->InitLayout();
      layout_->InitView();
    }
  }
}