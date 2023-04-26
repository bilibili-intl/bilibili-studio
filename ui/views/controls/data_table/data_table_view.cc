#include "data_table_view.h"

#include "ui/views/controls/scroll_view.h"
#include "ui/views/widget/widget.h"

#include "base/strings/stringprintf.h"
#include "base/bind.h"

namespace views
{
  DataTableView::DataTableView(DataTableViewSource *data_source)
    : data_source_(data_source)
    , scroll_view_(NULL)
    , lock_visible_bounds_change_(false)
  {
    DCHECK(data_source_);
  }

  DataTableView::~DataTableView()
  {

  }

  void DataTableView::LockVisibleBoundsChanged(bool lock)
  {
      lock_visible_bounds_change_ = lock;
      if (!lock)
      {
          if (scroll_view_)
          {
              scroll_view_->Layout();
          }
          OnVisibleBoundsChanged();
      }
  }

  void DataTableView::RegisterCellClass(int reuse_id, CellConstructor cell_constructor)
  {
    DCHECK(cell_constructor_map_.find(reuse_id) == cell_constructor_map_.end());
    cell_constructor_map_[reuse_id] = cell_constructor;
  }

  views::DataTableViewCell* DataTableView::GetReusableCell(int reuse_id)
  {
    views::DataTableViewCell* cell = NULL;
    if (cell_reuse_map_.find(reuse_id) == cell_reuse_map_.end()
      || cell_reuse_map_[reuse_id].empty())
    {
      DCHECK(cell_constructor_map_.find(reuse_id) != cell_constructor_map_.end());
      cell = cell_constructor_map_[reuse_id].Run();
      AddChildView(cell);
    }

    if (!cell)
    {
      cell = cell_reuse_map_[reuse_id].front();
      cell_reuse_map_[reuse_id].pop_front();
      DCHECK(cell->parent() == this);
    }

    if (!cell->visible())
    {
      cell->SetVisible(true);
    }

    return cell;
  }

  void DataTableView::InvalidateCells()
  {
    std::map<DataTableViewCell*, std::pair<int, int>> cell_map = cell_map_;
    for (std::map<DataTableViewCell*, std::pair<int, int>>::iterator it = cell_map.begin();
      it != cell_map.end(); ++it)
    {
      //PushBackToCellDeque(it->first);
        DataTableViewCell* cell = it->first;
        std::map<DataTableViewCell*, std::pair<int, int>>::iterator cell_map_it = cell_map_.find(cell);
        if (cell_map_it != cell_map_.end())
        {
            int section_index = cell_map_[cell].first;
            int cell_index = cell_map_[cell].second;
            std::map<int, std::vector<int>>::iterator it = visible_items_.find(section_index);
            if (it != visible_items_.end())
            {
                std::vector<int>::iterator cell_it = std::find(it->second.begin(), it->second.end(), cell_index);
                if (cell_it != it->second.end())
                {
                    it->second.erase(cell_it);
                } else
                {
                    DCHECK(cell_index == -1);
                }

                if (it->second.empty())
                {
                    visible_items_.erase(it);
                }
            } else
            {
                DCHECK(false);
            }

            cell_map_.erase(cell_map_it);
        } else
        {
            DCHECK(false);
        }

        DCHECK(cell->parent() == this);

        cell_reuse_map_[cell->ReuseID()].push_back(cell);
    }
    DCHECK(visible_items_.size() == 0);
  }

  void DataTableView::PushBackToCellDeque(DataTableViewCell* cell)
  {
      if (!ShouldReLayoutDataTableCell())
      {
          return;
      }

    std::map<DataTableViewCell*, std::pair<int, int>>::iterator cell_map_it = cell_map_.find(cell);
    if (cell_map_it != cell_map_.end())
    {
      int section_index = cell_map_[cell].first;
      int cell_index = cell_map_[cell].second;
      std::map<int, std::vector<int>>::iterator it = visible_items_.find(section_index);
      if (it != visible_items_.end())
      {
        std::vector<int>::iterator cell_it = std::find(it->second.begin(), it->second.end(), cell_index);
        if (cell_it != it->second.end())
        {
          it->second.erase(cell_it);
        }
        else
        {
          DCHECK(cell_index == -1);
        }

        if (it->second.empty())
        {
          visible_items_.erase(it);
        }
      }
      else
      {
          DCHECK(false);
      }

      cell_map_.erase(cell_map_it);
    }
    else
    {
        DCHECK(false);
    }

    DCHECK(cell->parent() == this);

      cell->SetVisible(false);
      cell_reuse_map_[cell->ReuseID()].push_back(cell);
  }

  bool DataTableView::NeedsNotificationWhenVisibleBoundsChange() const
  {
    return true;
  }

  void DataTableView::OnVisibleBoundsChanged()
  {
      if (!ShouldReLayoutDataTableCell())
      {
          return;
      }

      const int nDetectBottom = 300;
      gfx::Rect visible_bounds = GetVisibleBounds();
      gfx::Rect local_bounds = GetLocalBounds();
      if (local_bounds.bottom() - visible_bounds.bottom() < nDetectBottom)
      {
          OnVisibleBoundsNearLocalBoundsBottom();
      }

    std::map<int, std::vector<int>> visible_item = GetVisibleItemsAndCalcRect();
    if (visible_item == visible_items_)
    {
      return;
    }

    {
      std::map<DataTableViewCell*, std::pair<int, int>> cell_map = cell_map_;
      std::map<DataTableViewCell*, std::pair<int, int>>::iterator cell_map_it = cell_map.begin();
      while (cell_map_it != cell_map.end())
      {
        (*cell_map_it).first->OnVisibleBoundsChanged();
        cell_map_it++;
      }
    }

    for (int i = 0; i < data_source_->GetNumberOfSections(); i++)
    {
      if (visible_item.find(i) == visible_item.end())
        continue;

      for (std::vector<int>::iterator it = visible_item[i].begin(); it != visible_item[i].end();
        ++it)
      {
        if (visible_items_.find(i) != visible_items_.end()
          && std::find(visible_items_[i].begin(), visible_items_[i].end(), *it) != visible_items_[i].end())
        {
          continue;
        }

        if (*it == -1)
        {
          DataTableViewCell* section_view = data_source_->GetSectionView(i);
          section_view->SetBoundsRect(visible_item_rect_[std::make_pair(i, -1)]);
          cell_map_[section_view] = std::make_pair(i, -1);
        }
        else
        {
          DCHECK(visible_item_rect_.find(std::make_pair(i, *it)) != visible_item_rect_.end());

          views::DataTableViewCell *cell = data_source_->GetCellViewInSection(i, *it);
          cell->SetBoundsRect(visible_item_rect_[std::make_pair(i, *it)]);
          cell_map_[cell] = std::make_pair(i, *it);
        }
      }
    }

    visible_items_ = visible_item;
  }

  std::map<int, std::vector<int>> DataTableView::GetVisibleItemsAndCalcRect()
  {
    std::map<int, std::vector<int>> visible_items;
    gfx::Rect visible_bounds = GetVisibleBounds();
    if (visible_bounds.IsEmpty())
    {
      return visible_items;
    }

    visible_item_rect_.clear();

    int top_layout_area = 0;
    for (int i = 0; i < data_source_->GetNumberOfSections();i++)
    {
      gfx::Size cell_size = data_source_->GetCellSizeForSection(i);
      int section_height = data_source_->GetSectionHeight(i);
      int cell_cout_per_line = floor(visible_bounds.width() / cell_size.width());
      int cell_count = data_source_->GetCellCountForSection(i);
      int cell_lines = ceil(cell_count * 1.0 / cell_cout_per_line);
      int total_cell_height = cell_lines * cell_size.height();
      int total_section_height = total_cell_height + section_height;

      if (top_layout_area + total_section_height <= visible_bounds.y())
      {
        top_layout_area += total_section_height;
        continue;
      }

      if (top_layout_area >= visible_bounds.bottom())
      {
        break;
      }

	  static const int scroll_cx = scroll_view_->vertical_scroll_bar()->GetLayoutSize();
      int offset_x = (width() - scroll_cx - cell_cout_per_line*cell_size.width()) / 2;
      //visible_item_rect_[std::make_pair(i, -1)] = gfx::Rect(0, top_layout_area, width(), section_height);
      visible_item_rect_[std::make_pair(i, -1)] = gfx::Rect(offset_x, top_layout_area, cell_cout_per_line*cell_size.width(), section_height);
      if (top_layout_area < visible_bounds.bottom()
        && (top_layout_area + section_height) > visible_bounds.y())
      {
        visible_items[i].push_back(-1);
      }

      top_layout_area += section_height;
      if (top_layout_area >= visible_bounds.bottom())
        break;

      int start_cell_line = std::max<int>(0, floor((visible_bounds.y() - top_layout_area) / cell_size.height()));
      top_layout_area += start_cell_line*cell_size.height();
      if (top_layout_area >= visible_bounds.bottom())
        break;

      for (int j = start_cell_line * cell_cout_per_line; j < cell_count; j++)
      {
        visible_items[i].push_back(j);
        visible_item_rect_[std::make_pair(i, j)] =
            gfx::Rect(offset_x + (j%cell_cout_per_line) * cell_size.width(), top_layout_area, cell_size.width(), cell_size.height());
            //gfx::Rect((j%cell_cout_per_line) * cell_size.width(), top_layout_area, cell_size.width(), cell_size.height());
        if ((j + 1) % cell_cout_per_line == 0 || j == cell_count - 1)
        {
          top_layout_area += cell_size.height();
          if (top_layout_area >= visible_bounds.bottom())
            break;
        }
      }

      if (top_layout_area >= visible_bounds.bottom())
        break;
    }

    return visible_items;
  }

  views::ScrollView* DataTableView::CreateParentIfNecessary()
  {
      if (!scroll_view_)
      {
          scroll_view_ = ScrollView::CreateScrollViewWithBorder();
          scroll_view_->set_border(nullptr);
          scroll_view_->SetContents(this);
          scroll_view_->set_hide_horizontal_scrollbar(true);
      }
      return scroll_view_;
  }

  gfx::Size DataTableView::GetPreferredSize()
  {
    return gfx::Size(width(), height());
  }

  void DataTableView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
      if (details.child == this && details.is_add == false)
      {
          LockVisibleBoundsChanged(true);
      }
  }

  void DataTableView::AutoSetContentSize()
  {
    gfx::Rect parent_bounds = parent()->bounds();
    if (parent_bounds.IsEmpty())
    {
      return;
    }

    int total_height = 0;
    for (int i = 0; i < data_source_->GetNumberOfSections(); i++)
    {
      gfx::Size cell_size = data_source_->GetCellSizeForSection(i);
      int section_height = data_source_->GetSectionHeight(i);
      int cell_cout_per_line = floor(parent_bounds.width() / cell_size.width());
      int cell_lines = ceil(data_source_->GetCellCountForSection(i) * 1.0 / cell_cout_per_line);
      int total_cell_height = cell_lines * cell_size.height();
      int total_section_height = total_cell_height + section_height;

      total_height += total_section_height;
    }

    SetSize(gfx::Size(scroll_view_->GetContentsBounds().width(), total_height));

    if (!ShouldReLayoutDataTableCell())
    {
        return;
    }
    std::map<std::pair<int, int>, gfx::Rect> last_visible_item_rect = visible_item_rect_;
    if (GetVisibleItemsAndCalcRect() != visible_items_
      || last_visible_item_rect != visible_item_rect_)
    {
      std::map<DataTableViewCell*, std::pair<int, int>> cell_map = cell_map_;
      for (std::map<DataTableViewCell*, std::pair<int, int>>::iterator it = cell_map.begin();
        it != cell_map.end(); ++it)
      {
        PushBackToCellDeque(it->first);
      }
      DCHECK(visible_items_.size() == 0);
    }
  }

  void DataTableView::Layout()
  {
    AutoSetContentSize();
  }

  void DataTableView::VisibilityChanged(View* starting_from, bool is_visible)
  {
      if (is_visible && GetWidget())
      {
          OnVisibleBoundsChanged();
      }
      __super::VisibilityChanged(starting_from, is_visible);
  }

  bool DataTableView::ShouldReLayoutDataTableCell()
  {
      if (lock_visible_bounds_change_ || GetWidget()->IsMinimized())
      {
          return false;
      }
      return true;
  }

}
