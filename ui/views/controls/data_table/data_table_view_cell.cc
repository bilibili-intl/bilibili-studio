#include "data_table_view_cell.h"
#include "data_table_view.h"
#include "..\label.h"
#include "base\strings\stringprintf.h"

namespace views
{
  DataTableViewCell::DataTableViewCell(int reuse_id, DataTableView *table)
    : reuse_id_(reuse_id)
    , table_(table)
  {
    table_->AddChildView(this);
  }

  DataTableViewCell::~DataTableViewCell()
  {
  }

  void DataTableViewCell::OnVisibleBoundsChanged()
  {
    if (GetVisibleBounds().IsEmpty() && visible())
    {
      //Label *lable = (Label *)child_at(0);
      //std::wstring output = L"PushBackToCellDeque:"+lable->text();
      //OutputDebugStringW(output.c_str());
      //OutputDebugStringW(L"\n");

      table_->PushBackToCellDeque(this);
    }
  }

  bool DataTableViewCell::NeedsNotificationWhenVisibleBoundsChange() const
  {
    return false;
  }
}
