#include "ui/views/examples/data_table_example.h"

#include "base/strings/utf_string_conversions.h"
#include "ui/views/controls/link.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/view.h"
#include "base/strings/stringprintf.h"
#include "base/bind.h"

namespace views {
  namespace examples {
    class TestCell
      : public DataTableViewCell
    {
    public:
      TestCell(DataTableView *table)
        : DataTableViewCell(100, table)
      {}

      virtual ~TestCell()
      {

      }

      static DataTableViewCell* CreateCell(DataTableView *table)
      {
        TestCell *cell = new TestCell(table);
        cell->set_background(views::Background::CreateSolidBackground(SkColorSetRGB(200, rand() % 255, rand() % 255)));
        cell->SetLayoutManager(new FillLayout);
        cell->AddChildView(new Label(L""));

        return cell;
      }

      void SetText(int section_index, int cell_index)
      {
        Label *label = (Label *)child_at(0);
        label->SetText(base::StringPrintf(L"section:%d, cell:%d", section_index, cell_index));
      }
    };

    class TestSectionCell
      : public DataTableViewCell
    {
    public:
      TestSectionCell(DataTableView *table)
        : DataTableViewCell(200, table)
      {}

      virtual ~TestSectionCell()
      {

      }

      static DataTableViewCell* CreateCell(DataTableView *table)
      {
        TestSectionCell *cell = new TestSectionCell(table);
        cell->set_background(views::Background::CreateSolidBackground(SkColorSetRGB(200, rand() % 255, rand() % 255)));
        cell->SetLayoutManager(new FillLayout);
        cell->AddChildView(new Label(L""));

        return cell;
      }

      void OnVisibleBoundsChanged()
      {
        __super::OnVisibleBoundsChanged();
      }

      void SetText(int section_index)
      {
        Label *label = (Label *)child_at(0);
        label->SetText(base::StringPrintf(L"section:%d", section_index));
      }
    };

    DataTableExample::DataTableExample() : ExampleBase("DataTableExample") {
    }

    DataTableExample::~DataTableExample() {
    }

    void DataTableExample::CreateExampleView(View* container) {
      table_ = new DataTableView(this);
      
      table_->RegisterCellClass(100, base::Bind(&TestCell::CreateCell, base::Unretained(table_)));
      table_->RegisterCellClass(200, base::Bind(&TestSectionCell::CreateCell, base::Unretained(table_)));

      container->SetLayoutManager(new FillLayout);
      container->AddChildView(table_->CreateParentIfNecessary());
    }

    int DataTableExample::GetNumberOfSections()
    {
      return 20;
    }

    DataTableViewCell * DataTableExample::GetSectionView(int section_index)
    {
      TestSectionCell *cell = (TestSectionCell *)table_->GetReusableCell(200);
      cell->SetText(section_index);

      return cell;
    }

    int DataTableExample::GetSectionHeight(int section_index)
    {
      return 12 + section_index;
    }

    int DataTableExample::GetCellCountForSection(int section_index)
    {
      return 10;
    }

    gfx::Size DataTableExample::GetCellSizeForSection(int section_index)
    {
      return gfx::Size(200 + section_index, 200 + section_index);
    }

    views::DataTableViewCell * DataTableExample::GetCellViewInSection(int section_index, int cell_index)
    {
      TestCell *cell = (TestCell *)table_->GetReusableCell(100);
      cell->SetText(section_index, cell_index);

      return cell;
    }

  }  // namespace examples
}  // namespace views
