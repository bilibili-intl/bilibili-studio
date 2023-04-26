#ifndef UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_FILL_LAYOUT_H
#define UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_FILL_LAYOUT_H

#include "json_layout.h"

// for example:
// 
//   {
//     "view_id":203,
//       "type" : "fill_layout",
//       "layout" :
//     {
//       "view":{"view_id":200}
//     }
//   }

namespace views {
  class VIEWS_EXPORT JsonFillLayout
    : public JsonLayout
  {
  public:
    explicit JsonFillLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property);
    virtual ~JsonFillLayout();

    virtual void InitLayout() OVERRIDE;
    virtual void InitView() OVERRIDE;
  };
}
#endif