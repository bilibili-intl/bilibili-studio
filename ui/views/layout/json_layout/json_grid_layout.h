#ifndef UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_GRID_LAYOUT_H
#define UI_VIEWS_LAYOUT_JSON_LAYOUT_JSON_GRID_LAYOUT_H

#include "json_layout.h"
#include "ui\views\layout\grid_layout.h"

// for example:
//
// {
//   "view_id":203,
//     "type" : "grid_layout",
//     "layout" :
//   {
//     "insets":{"top":0, "left" : 0, "bottom" : 0, "right" : 0},
//       "minimum_size" : {"width":300, "height" : 500},
//       "columnset" :
//       [
//     {
//       "column_set_id":0,
//         "columns" :
//         [
//       {
//         "h_align":3,
//           "v_align" : 3,
//           "resize_percent" : 0.3,
//           "size_type" : 0,
//           "fixed_width" : 0,
//           "min_width" : 0
//       },
//       {
//         "resize_percent":0,
//         "width" : 100
//       },
//       {
//         "h_align":3,
//         "v_align" : 3,
//         "resize_percent" : 0.3,
//         "size_type" : 0,
//         "fixed_width" : 0,
//         "min_width" : 0
//       },
//       {
//         "resize_percent":0,
//         "width" : 100
//       },
//       {
//         "h_align":3,
//         "v_align" : 3,
//         "resize_percent" : 0.4,
//         "size_type" : 0,
//         "fixed_width" : 0,
//         "min_width" : 0
//       },
//       {
//         "resize_percent":0,
//         "width" : 100
//       }
//         ]
//     },
//     {
//       "column_set_id":1,
//       "columns" :
//       [
//       {
//         "h_align":3,
//           "v_align" : 3,
//           "resize_percent" : 0,
//           "size_type" : 0,
//           "fixed_width" : 100,
//           "min_width" : 0
//       },
//       {
//         "resize_percent":0,
//         "width" : 400
//       },
//       {
//         "h_align":3,
//         "v_align" : 3,
//         "resize_percent" : 1,
//         "size_type" : 0,
//         "fixed_width" : 0,
//         "min_width" : 0
//       }
//       ]
//     }
//       ],
//         "viewset":
//       [
//       {
//         "column_set_id":0,
//           "rows" :
//           [
//         {
//           "vertical_resize":0.0,
//             "views" : [
//               200,
//                 0,
//                 202
//             ]
//         },
//         {
//           "vertical_resize":0.0,
//           "size" : 10
//         },
//         {
//           "vertical_resize":0.0,
//           "views" : [
//           {
//             "view_id":204,
//               "h_align" : 0,
//               "v_align" : 0,
//               "pref_width" : 0,
//               "pref_height" : 0
//           },
//           {
//             "view_id":205,
//             "h_align" : 1,
//             "v_align" : 0,
//             "pref_width" : 0,
//             "pref_height" : 0
//           },
//           {
//             "view_id":206,
//             "h_align" : 2,
//             "v_align" : 0,
//             "pref_width" : 0,
//             "pref_height" : 0
//           }
//           ]
//         },
//         {
//           "vertical_resize":0.0,
//           "size" : 10
//         }
// 
//           ]
//       },
//       {
//         "column_set_id":1,
//         "rows" :
//         [
//         {
//           "vertical_resize":0.0,
//             "views" : [
//               207,
//                 208
//             ]
//         }
//         ]
//       }
//       ]
//   }
// }

namespace views {
  class VIEWS_EXPORT JsonGridLayout
    : public JsonLayout
  {
  public:
    explicit JsonGridLayout(JsonLayoutView *root, const base::DictionaryValue* layout_property);
    virtual ~JsonGridLayout();

    virtual void InitLayout() OVERRIDE;
    virtual void InitView() OVERRIDE;

  private:
    void SetLayoutInsets();
    void SetLayoutMinSize();
    void CreateLayoutColumnSet();
    void CreateColumn(views::ColumnSet* column_set, const base::DictionaryValue *column);
    void CreateViewSet();
    void CreateRow(int column_set_id, const base::DictionaryValue *row);
  };
}
#endif