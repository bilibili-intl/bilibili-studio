#ifndef UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_LAYOUT_VIEW_H
#define UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_LAYOUT_VIEW_H

#include "ui\views\view.h"
#include "base\values.h"

namespace views {
  class JsonLayout;

  class VIEWS_EXPORT JsonLayoutView
    : public views::View
  {
  public:
    explicit JsonLayoutView();
    virtual ~JsonLayoutView();

    void SetJsonLayoutString(const std::string &layout_json);
    void SetJsonLayoutPath(const base::FilePath& json_path);
    void SetLayoutDictionary(const base::DictionaryValue* layout_property);

    void AddJsonLayoutView(int id, JsonLayoutView *view);
    JsonLayoutView *GetJsonLayoutView(int id);
    bool HasJsonLayoutView(int id);

    void AddLayoutView(int id, views::View *view);
    views::View *GetLayoutView(int id);
    bool HasLayoutView(int id);

    // views::View
    virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE;

  private:
    std::map<int, JsonLayoutView*> json_layout_views_;
    std::map<int, views::View*> layout_views_;

  protected:
    JsonLayout *layout_;

    friend class JsonLayout;
  };
}
#endif