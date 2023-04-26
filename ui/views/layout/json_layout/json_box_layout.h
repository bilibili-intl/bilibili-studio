#ifndef UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_BOX_LAYOUT_H
#define UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_BOX_LAYOUT_H

#include "json_layout.h"

// for example:
//
//   {
//     "view_id":203,
//       "type" : "box_layout",
//       "layout" :
//     {
//       "orientation":0,
//         "inside_border_horizontal_spacing" : 10,
//         "inside_border_vertical_spacing" : 10,
//         "between_child_spacing" : 10,
//         "spread_blank_space" : true,
//         "views" : [200, 201, 202, 204, 205, 206, 207, 208]
//     }
// 
//   }

namespace views {
  class VIEWS_EXPORT JsonBoxLayout
    : public JsonLayout
  {
  public:
    explicit JsonBoxLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property);
    virtual ~JsonBoxLayout();

    virtual void InitLayout() OVERRIDE;
    virtual void InitView() OVERRIDE;
  };
}
#endif