#ifndef UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_H
#define UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_H

#include "data_table_view_source.h"
#include "data_table_view_cell.h"

#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/callback.h"

#include "ui/views/view.h"
#include "ui/views/views_export.h"
#include "ui/views/controls/scroll_view.h"


namespace views {
  class VIEWS_EXPORT DataTableView
    : public views::View
  {
  public:
    typedef base::Callback<DataTableViewCell*(void)> CellConstructor;
    typedef std::deque<DataTableViewCell*> CellDeque;

    explicit DataTableView(DataTableViewSource *data_source);
    virtual ~DataTableView();

    ScrollView* CreateParentIfNecessary();

    void RegisterCellClass(int reuse_id, CellConstructor cell_constructor);
    DataTableViewCell* GetReusableCell(int reuse_id);

    void InvalidateCells();

    virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details);
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual bool NeedsNotificationWhenVisibleBoundsChange() const OVERRIDE;
    virtual void OnVisibleBoundsChanged() OVERRIDE;
    virtual void Layout() OVERRIDE;
    virtual void VisibilityChanged(View* starting_from, bool is_visible) OVERRIDE;
    virtual void OnVisibleBoundsNearLocalBoundsBottom(){}

    void LockVisibleBoundsChanged(bool lock);

  protected:
    void AutoSetContentSize();
    void PushBackToCellDeque(DataTableViewCell* cell);
    std::map<int, std::vector<int>> GetVisibleItemsAndCalcRect();

    bool ShouldReLayoutDataTableCell();

  private:
    ScrollView* scroll_view_;

    std::map<int, CellConstructor> cell_constructor_map_;
    std::map<int, CellDeque> cell_reuse_map_;

    DataTableViewSource *data_source_;

    std::map<DataTableViewCell*, std::pair<int, int>> cell_map_;//std::pair<int, int>,first->group index, second->group header(-1) or item(>=0)
    std::map<int, std::vector<int>> visible_items_;// key->group index, value->group header(-1) or item(>=0)
    std::map<std::pair<int, int>, gfx::Rect> visible_item_rect_;//std::pair<int, int>,first->group index, second->group header(-1) or item(>=0)

    bool lock_visible_bounds_change_;

    friend class DataTableViewCell;
  };

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_H
