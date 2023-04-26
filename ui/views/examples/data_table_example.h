#ifndef UI_VIEWS_EXAMPLES_DATA_TABLE_EXAMPLE_H
#define UI_VIEWS_EXAMPLES_DATA_TABLE_EXAMPLE_H

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/examples/example_base.h"
#include "ui/views/controls/data_table/data_table_view.h"
#include "ui/views/controls/data_table/data_table_view_source.h"

namespace views {
  namespace examples {

    class DataTableExample
      : public ExampleBase
      , public DataTableViewSource{
    public:
      DataTableExample();
      virtual ~DataTableExample();

      // Overridden from ExampleBase:
      virtual void CreateExampleView(View* container) OVERRIDE;


      virtual int GetNumberOfSections();
      virtual DataTableViewCell *GetSectionView(int section_index);
      virtual int GetSectionHeight(int section_index);
      virtual int GetCellCountForSection(int section_index);
      virtual gfx::Size GetCellSizeForSection(int section_index);
      virtual DataTableViewCell *GetCellViewInSection(int section_index, int cell_index);
    private:
      DataTableView *table_;

      DISALLOW_COPY_AND_ASSIGN(DataTableExample);
    };

  }  // namespace examples
}  // namespace views

#endif  // UI_VIEWS_EXAMPLES_DATA_TABLE_EXAMPLE_H
