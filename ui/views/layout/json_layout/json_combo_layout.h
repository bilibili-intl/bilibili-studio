#ifndef UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_COMBO_LAYOUT_H
#define UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_COMBO_LAYOUT_H

#include "json_layout.h"

// for example:
//
// {
//   "view_id":203,
//     "type" : "combo_layout",
//     "layout" :
//   {
//     "views":
//     [
//     {
//       "view_id":1000,
//         "type" : "combo_layout",
//         "layout" :
//       {
//         "views":
//         [
//         {
//           "view_id":1001,
//             "type" : "combo_layout",
//             "layout" :
//           {
//             "views":
//             [
//             {
//               "view_id":1001,
//                 "type" : "box_layout",
//                 "layout" :
//               {
//                 "orientation":1,
//                   "inside_border_horizontal_spacing" : 10,
//                   "inside_border_vertical_spacing" : 10,
//                   "between_child_spacing" : 10,
//                   "spread_blank_space" : true,
//                   "views" : [205, 206]
//               }
//             }
//             ]
//           }
//         },
//         {
//           "view_id":1000,
//           "type" : "box_layout",
//           "layout" :
//           {
//             "orientation":0,
//               "inside_border_horizontal_spacing" : 10,
//               "inside_border_vertical_spacing" : 10,
//               "between_child_spacing" : 10,
//               "spread_blank_space" : true,
//               "views" : [200, 201, 202, 1001]
//           }
//         }
//         ]
//       }
//     },
//     {
//       "view_id":203,
//       "type" : "box_layout",
//       "layout" :
//       {
//         "orientation":0,
//           "inside_border_horizontal_spacing" : 10,
//           "inside_border_vertical_spacing" : 10,
//           "between_child_spacing" : 10,
//           "spread_blank_space" : true,
//           "views" : [207, 208, 1000]
//       }
//     }
//     ]
//   }
// }

namespace views {
  class VIEWS_EXPORT JsonComboLayout
    : public JsonLayout
  {
  public:
    explicit JsonComboLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property);
    virtual ~JsonComboLayout();

    virtual void InitLayout() OVERRIDE;
    virtual void InitView() OVERRIDE;
  };
}
#endif