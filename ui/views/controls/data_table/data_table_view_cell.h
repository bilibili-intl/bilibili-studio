#ifndef UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_CELL_H
#define UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_CELL_H

#include "ui/views/views_export.h"
#include "ui/views/view.h"

namespace views {
  class DataTableView;
  class VIEWS_EXPORT DataTableViewCell
    : public views::View
  {
  public:
    DataTableViewCell(int reuse_id, DataTableView *table);
    virtual ~DataTableViewCell();

    virtual bool NeedsNotificationWhenVisibleBoundsChange() const OVERRIDE;
    virtual void OnVisibleBoundsChanged() OVERRIDE;


    int ReuseID()
    {
      return reuse_id_;
    } 
     
  private:
    int reuse_id_;
    DataTableView *table_;
  };
}  // namespace views

#endif  // UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_SOURCE_H
