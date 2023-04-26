#ifndef UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_LAYOUT_H
#define UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_LAYOUT_H

#include "ui\views\layout\layout_manager.h"
#include "ui\views\view.h"
#include "base\values.h"
#include "json_layout_view.h"

namespace views {

  class VIEWS_EXPORT JsonLayout
    : public views::LayoutManager
  {
  public:
    static base::DictionaryValue* LoadJson(const base::FilePath& json_path);
    static base::DictionaryValue* LoadJson(const std::string &layout_json);
    static JsonLayout *CreateLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property);

  public:
    explicit JsonLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property);
    virtual ~JsonLayout();

    virtual void InitLayout();
    virtual void InitView();

    // views::LayoutManager
    virtual void Installed(views::View* host) OVERRIDE;
    virtual void Uninstalled(views::View* host) OVERRIDE;
    virtual void Layout(views::View* host) OVERRIDE;
    virtual gfx::Size GetPreferredSize(views::View* host) OVERRIDE;
    virtual int GetPreferredHeightForWidth(views::View* host, int width) OVERRIDE;
    virtual void ViewAdded(views::View* host, views::View* view) OVERRIDE;
    virtual void ViewRemoved(views::View* host, views::View* view) OVERRIDE;

  protected:
    views::LayoutManager *layout_manager_;
    JsonLayoutView *root_;
    JsonLayoutView *host_;
    const base::DictionaryValue *layout_property_;
  };
}
#endif