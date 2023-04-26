#ifndef UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_SOURCE_H
#define UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_SOURCE_H

#include "ui/views/views_export.h"
#include "ui/gfx/size.h"

namespace views {
  class DataTableViewCell;
  class VIEWS_EXPORT DataTableViewSource
  {
  public:
    virtual int GetNumberOfSections() = 0;
    virtual DataTableViewCell *GetSectionView(int section_index) = 0;
    virtual int GetSectionHeight(int section_index) = 0;
    virtual int GetCellCountForSection(int section_index) = 0;
    virtual gfx::Size GetCellSizeForSection(int section_index) = 0;
    virtual DataTableViewCell *GetCellViewInSection(int section_index, int cell_index) = 0;
  };

}  // namespace views

#endif  // UI_VIEWS_CONTROLS_DATA_TABLE_DATA_TABLE_VIEW_SOURCE_H
