#include "gridview.h"

#include "ui/base/win/dpi.h"
#include "ui/views/widget/widget.h"


namespace
{
    const int kDetectBorderThickness_ = 10 * ui::GetDPIScale();
}


// GridItemView
GridItemView::GridItemView(int type_id, GridView *grid_view)
    : type_id_(type_id)
    , grid_view_(grid_view)
    , is_header_(false)
    , group_index_(-1)
    , item_index_(-1)
{
}

void GridItemView::SetIndexInfo(bool is_header, int group_index, int item_index /*= -1*/)
{
    is_header_ = is_header;
    group_index_ = group_index;
    item_index_ = item_index;
}


// GridView
const char GridView::kViewClassName[] = "GridView";

GridView::GridView(GridModel* model)
    : model_(model)
    , scroll_view_(nullptr)
    , horiz_scroll_(false)
    , lock_window_update_(false)
    , detect_border_thickness_(kDetectBorderThickness_)
    , weakptr_factory_(this)
{
    DCHECK(model);
}

GridView::~GridView()
{
}

void GridView::RegisterItemType(int type_id, ItemConstructor item_constructor)
{
    DCHECK(item_constructor_map_.find(type_id) == item_constructor_map_.end());
    item_constructor_map_[type_id] = item_constructor;
}

//void GridView::SetGridModel::ScrollDirection(bool horiz)
//{
//    horiz_scroll_ = horiz;
//    if (scroll_view_)
//    {
//        scroll_view_->set_hide_horizontal_scrollbar(!horiz_scroll_);
//    }
//    UpdateData();
//}

void GridView::SetModel(GridModel* model)
{
    DCHECK(model);
    model_ = model;
}

base::WeakPtr<GridView> GridView::GetWeakPtr()
{
    return weakptr_factory_.GetWeakPtr();
}

ScrollViewWithFloatingScrollbar* GridView::CreateParentIfNecessary(bool is_bar_narrow)
{
    if (!scroll_view_)
    {
        scroll_view_ = new ScrollViewWithFloatingScrollbar(this, is_bar_narrow);
        scroll_view_->set_hide_horizontal_scrollbar(!horiz_scroll_);
    }
    return scroll_view_;
}

void GridView::UpdateData(UpdateDataScrollType scrolltype/* = NOSCROLL*/)
{
    gfx::Rect visible_bounds = GetVisibleBounds();

    // 所谓的NOSCROLL控件只能保证对于控件而言更新数据之后继续保持之前第一个可见项的
    // 组索引、项索引、项类型这三个参数与更新前的第一个可见项的这三个参数一致，
    // 而不是保证上层看到的内容保持不变
    bool valid = false;
    int y_offset = 0;
    ItemInfo item;
    if (scrolltype == NOSCROLL)
    {
        if (!visible_items_.empty())
        {
            item = visible_items_.front();
            y_offset = item.view_->bounds().y() - visible_bounds.y();
            valid = true;
        }
    }

    // 遍历上次的全部可见项，不需要检测是否可见，全部放回重用队列
    for each (GridView::ItemInfo var in visible_items_)
    {
        CheckItemVisibleBounds(var.view_, false);
    }
    // 重新计算尺寸，重新布局可视区
    visible_items_.clear();
    if (scroll_view_)
    {
        scroll_view_->InvalidateLayout();
        ((views::View*)scroll_view_)->Layout();
    }
    else
    {
        Layout();
    }

    gfx::Rect new_visible_bounds;
    switch (scrolltype)
    {
    case NOSCROLL:
    {
        int position = 0;
        if (valid)
        {
            if (item.is_header_)
            {
                position = GetGroupHeaderPosition(item.group_index_);
            }
            else
            {
                position = GetGroupItemPosition(item.group_index_, item.item_index_);
            }
            position -= y_offset;
            position = std::max(position, 0);
            position = std::min(position, height() - visible_bounds.height());
        }
        new_visible_bounds.SetRect(0, position, visible_bounds.width(), visible_bounds.height());
    }
        break;
    case TOTOP:
        new_visible_bounds.SetRect(0, 0, visible_bounds.width(), visible_bounds.height());
        break;
    case TOBOTTOM:
        new_visible_bounds.SetRect(0, height() - visible_bounds.height(), visible_bounds.width(), visible_bounds.height());
        break;
    default:
        break;
    }
    if (visible_bounds != new_visible_bounds)
    {
        ScrollRectToVisible(new_visible_bounds);
    }
    else
    {
        OnVisibleBoundsChanged();
    }
}

void GridView::UpdateGroupHeaderData(int group_index)
{
    GridItemView *view = FindVisibleItemView(true, group_index, -1);
    DCHECK(view);
    if (view)
    {
        model_->UpdateData(view);
    }
}

void GridView::UpdateGroupItemData(int group_index, int item_index)
{
    GridItemView *view = FindVisibleItemView(false, group_index, item_index);
    DCHECK(view);
    if (view)
    {
        model_->UpdateData(view);
    }
}

GridItemView* GridView::FindVisibleItemView(bool is_header, int group_index, int item_index)
{
    GridItemView *view = nullptr;
    for each (GridView::ItemInfo var in visible_items_)
    {
        if (is_header)
        {
            if (var.is_header_ == is_header &&
                var.group_index_ == group_index)
            {
                view = var.view_;
                break;
            }
        }
        else
        {
            if (var.group_index_ == group_index &&
                var.item_index_ == item_index)
            {
                view = var.view_;
                break;
            }
        }
    }
    return view;
}

void GridView::Layout()
{
    if (!parent())
    {
        return;
    }

    gfx::Rect par_cnt_bounds = parent()->GetContentsBounds();
    if (par_cnt_bounds.IsEmpty())
    {
        return;
    }

    // 计算并设置自己的尺寸以便ScrollView显示滚动条
    gfx::Size size;
    if (!horiz_scroll_)
    {
        // 遍历所有组->取组中所有项数->根据项尺寸和父scrollview尺寸计算每行能显示几个项以及需要几行才能显示完
        int total_height = 0;
        for (int group_index = 0; group_index < model_->GetNumberOfGroups(); group_index++)
        {
            total_height += GetGroupDetails(par_cnt_bounds, group_index);
        }
        size.SetSize(par_cnt_bounds.width(), total_height);
    }
    SetSize(size);
}

void GridView::OnVisibleBoundsChanged()
{
    if (IsLockWindowUpdate())
    {
        return;
    }

    // 视口改变之后需要重新计算获知哪些item该被显示在可视区
    gfx::Rect visible_bounds = GetVisibleBounds();
    gfx::Rect local_bounds = GetContentsBounds();
    if (!horiz_scroll_)
    {
        // 为视口即将贴顶或贴底给上层提供一个触边通知时机
        // 在触边通知中就能更新数据是最好的，否则就必须在确实更新数据之后调一遍UpdateData，
        // 这样会再次计算一遍可见项，影响布局效率
        GridModel::ScrollDirection dir = GridModel::ScrollDirection::NoMove;
        if (visible_bounds.y() - last_visible_bounds_.y() > 0)
        {
            dir = GridModel::Backwards;
        }
        else if (visible_bounds.y() - last_visible_bounds_.y() < 0)
        {
            dir = GridModel::Forward;
        }
        ScrollReason reason = ScrollReason::NoMove;
        if (scroll_view_)
        {
            reason = scroll_view_->last_scroll_reason();
        }
        if (visible_bounds.y() - local_bounds.y() <= detect_border_thickness_)
        {
            OnVisibleBoundsNearContentsBoundsTop(false, dir, reason);
        }
        if (local_bounds.bottom() - visible_bounds.bottom() <= detect_border_thickness_)
        {
            OnVisibleBoundsNearContentsBoundsBottom(false, dir, reason);
        }

        // 根据当前可视区获取该被显示的项的信息（header/item、bounds、index）
        // 若此次改变的范围所该被显示的项与上一次改变该被显示的项一致就没必要再次布局了
        std::vector<GridView::ItemInfo> visible_items = std::move(GetVisibleItems(visible_bounds));
        if (visible_items != visible_items_)
        {
            // 遍历上次的全部可见项，已经不可见则将view放回重用队列
            for each (GridView::ItemInfo var in visible_items_)
            {
                CheckItemVisibleBounds(var.view_);
            }

            for (std::vector<GridView::ItemInfo>::iterator v_iter, iter = visible_items.begin(); iter != visible_items.end(); ++iter)
            {
                // 新范围的可见项和旧的重叠的不需再获取
                v_iter = std::find_if(visible_items_.begin(), visible_items_.end(),
                    [iter](GridView::ItemInfo &var)->bool {
                    return (var == *iter);
                });
                if (v_iter != visible_items_.end())
                {
                    // 因为最后会将最新的可见列表直接visible_items赋给缓存visible_items_
                    // 所以在跳过获取view的项把缓存里的view记录下来，不然后面直接拷贝的时候缓存列表中的view_就会是null的
                    iter->view_ = v_iter->view_;

                    // 组/项索引都一样但是bounds变了说明应该是前面有项发生了增删而没有调用UpdateData
                    DCHECK(iter->bounds_ == v_iter->bounds_);
                }
                else
                {
                    // 将组索引/项索引传给数据源，由数据源根据索引返回该项的view类型id，然后在根据此id
                    // 到重用队列中取或根据注册的对应类型view的创建函数去创建一个新的view
                    int type_id = -1;
                    if (iter->is_header_)
                    {
                        type_id = model_->GetGroupHeaderTypeId(iter->group_index_);
                    }
                    else
                    {
                        type_id = model_->GetGroupItemTypeId(iter->group_index_, iter->item_index_);
                    }
                    iter->view_ = GetItemView(type_id);

                    // header可以为null，项不行
                    if (!iter->is_header_)
                    {
                        DCHECK(iter->view_);
                    }

                    // 从重用队列里拿的view要刷新一遍它最新的数据
                    if (iter->view_)
                    {
                        iter->view_->SetIndexInfo(iter->is_header_, iter->group_index_, iter->item_index_);
                        model_->UpdateData(iter->view_);
                    }
                }

                if(iter->view_)
                {
                    iter->view_->SetBoundsRect(iter->bounds_);
                }
            }

            visible_items_ = visible_items;
        }
        else
        {
            DCHECK(true);
        }
    }

    last_visible_bounds_ = visible_bounds;
}

std::vector<GridView::ItemInfo> GridView::GetVisibleItems(const gfx::Rect &visible_bounds)
{
    std::vector<ItemInfo> visible_items;
    if (visible_bounds.IsEmpty())
    {
        return visible_items;
    }

    int last_bottom = GetContentsBounds().y();
    for (int group_index = 0; group_index < model_->GetNumberOfGroups(); group_index++)
    {
        int header_height = 0;
        int item_count = 0;
        int item_count_per_line = 0;
        int line_count = 0;
        std::vector<gfx::Rect> line_rect_vct;
        std::vector<gfx::Rect> item_rect_vct;

        int group_height = GetGroupDetails(visible_bounds, group_index,
            &header_height, &item_count, &item_count_per_line, &line_count, &line_rect_vct, &item_rect_vct);

        // 整组的高度为0就继续下一组
        if (0 == group_height)
        {
            continue;
        }

        // group_index组的整个区域（头加全部项）的底部的y坐标都还未进入可视区的顶部y坐标就继续遍历下一个组
        if (last_bottom + group_height <= visible_bounds.y())
        {
            last_bottom += group_height;
            continue;
        }

        // 取所有行的最大宽度值（当前只允许等宽项，所以直接乘即可）
        /*auto max_iter = std::max(line_rect_vct.begin(), line_rect_vct.end(),
            [](const gfx::Rect& elem1, const gfx::Rect& elem2)->bool {
                return elem1.width() < elem2.width();
            });
        int line_max_width = max_iter->width();*/
        int line_max_width = item_count_per_line *
            model_->GetItemSizeForGroup(visible_bounds, group_index).width();

        // 根据组横向对齐类型决定可视区中该组的X轴偏移（当前的做法是组中每行的项是紧挨着的）
        int offset_x = visible_bounds.x();
        switch (group_h_align_)
        {
        case gfx::ALIGN_LEFT:
            offset_x = visible_bounds.x();
            break;
        case gfx::ALIGN_CENTER:
            offset_x = (visible_bounds.width() - line_max_width) / 2;
            break;
        case gfx::ALIGN_RIGHT:
            offset_x = visible_bounds.right() - line_max_width;
            break;
        default:
            break;
        }

        // 先计算该组的header的区域，header已经进入可视区了就加入到可见项列表中
        int group_origin_y = last_bottom;
        gfx::Rect header_bounds(offset_x, group_origin_y, line_max_width, header_height);
        if (visible_bounds.Intersects(header_bounds))
        {
            ItemInfo header(true, group_index);
            header.bounds_ = header_bounds;
            visible_items.push_back(header);
        }

        // 累加y轴偏移，头部区域一旦超出可视区就不必再继续计算了
        // 即当前的可视区只能显示到header，不能继续往下显示item了
        last_bottom += header_height;
        if (last_bottom >= visible_bounds.bottom())
        {
            break;
        }

        // 逐行遍历，将进入可视区的第一行的行索引记录下来
        int first_visible_line = 0;
        // 从可视区顶部到header底部还有空隙的话则计算该空隙能容纳多少行，
        // 容不下的那行就是已经开始进入可视区的行索引了，从那行开始遍历item项
        int hb_to_vby_thickness = visible_bounds.y() - last_bottom;
        if (hb_to_vby_thickness > 0)
        {
            for (size_t line_index = 0; line_index < line_rect_vct.size(); line_index++)
            {
                gfx::Rect line_bounds(offset_x, group_origin_y + line_rect_vct[line_index].y(),
                    line_rect_vct[line_index].width(), line_rect_vct[line_index].height());
                if (visible_bounds.Intersects(line_bounds))
                {
                    // 找到了第一个可见行
                    first_visible_line = line_index;
                    break;
                }
                else
                {
                    // 被遍历到的行还不可见，则累加行高y轴偏移，继续遍历下一行
                    last_bottom += line_rect_vct[line_index].height();
                }
            }
        }

        DCHECK(last_bottom <= visible_bounds.bottom()) << "should not happen !";

        // 从第一个可见行的第一个item开始遍历
        gfx::Rect item_bounds;
        int item_index = first_visible_line * item_count_per_line;
        for (int line_index = first_visible_line; line_index < (int)line_rect_vct.size(); line_index++)
        {
            for (int line_item_index = 0;
                line_item_index < item_count_per_line && item_index < item_count;
                line_item_index++, item_index++)
            {
                // 计算该item的区域
                item_bounds = gfx::Rect(
                    offset_x + item_rect_vct[item_index].x(),
                    group_origin_y + item_rect_vct[item_index].y(),
                    item_rect_vct[item_index].width(),
                    item_rect_vct[item_index].height());
                // item已经进入可视区就加入到可见项列表中
                if (visible_bounds.Intersects(item_bounds))
                {
                    ItemInfo item(false, group_index, item_index);
                    item.bounds_ = item_bounds;
                    visible_items.push_back(item);
                }
            }

            // 累加y轴，一旦超出可视区就不必再继续计算了
            last_bottom += line_rect_vct[line_index].height();
            if (last_bottom >= visible_bounds.bottom())
            {
                break;
            }
        }

        // 一旦超出可视区就不必再继续计算了
        if (last_bottom >= visible_bounds.bottom())
        {
            break;
        }
    }

    return visible_items;
}

GridItemView* GridView::GetItemView(int type_id)
{
    GridItemView *view = nullptr;

    // 重用队列里没有这个类型的实例了就用RegisterItemType注册的构建函数创建一个新实例
    if (item_reuse_map_.find(type_id) == item_reuse_map_.end() || item_reuse_map_[type_id].empty())
    {
        if (item_constructor_map_.find(type_id) != item_constructor_map_.end())
        {
            view = item_constructor_map_[type_id].Run();
            DCHECK(view);
            AddChildView(view);
        }
    }
    else
    {
        view = item_reuse_map_[type_id].front();
        item_reuse_map_[type_id].pop_front();
        DCHECK(view->parent() == this);
    }

    if (view)
    {
        view->SetVisible(true);
    }

    return view;
}

int GridView::GetGroupDetails(const gfx::Rect &show_bounds, int group_index,
    OUT int* out_header_height /*= nullptr*/, /* 头部高度 */
    OUT int* out_item_count /*= nullptr*/, /* 组内总共有多少项 */
    OUT int* out_item_count_per_line /*= nullptr*/, /* 一行多少项 */
    OUT int* out_line_count /*= nullptr*/, /* 总共分几行 */
    OUT std::vector<gfx::Rect>* out_line_rect_vct /*= nullptr*/, /* 每行的bounds（行高为行内项高的最大值） */
    OUT std::vector<gfx::Rect>* out_item_rect_vct /*= nullptr /* 各个项的POSITION（相对于组头左上角位置） */
)
{
    if (!horiz_scroll_)
    {
        int header_height = model_->GetGroupHeaderHeight(group_index);
        const int item_count = model_->GetItemCountForGroup(group_index);
        const gfx::Size item_size = model_->GetItemSizeForGroup(show_bounds, group_index);
        DCHECK(!item_size.IsEmpty());
        // 一行的宽度可以容纳多少项，外层要保证当前容器行宽大于等于一项的宽度，否则就没法计算布局了
        const int item_count_per_line = std::floor(show_bounds.width() / item_size.width());
        DCHECK(item_count_per_line > 0);
        // 当前的容器宽度要把组内的项都布局完需要多少行
        const int line_count = std::ceil(item_count * 1.0 / item_count_per_line);

        // 计算全部组内项布局完需要的高度，如果组内项是等高的，那么总行数直接乘以项高即可，
        // 如果组内项是不等高的，那么要一行一行的计算行高，行高取行内项高度的最大值
        std::vector<gfx::Rect> line_rect_vct;
        std::vector<gfx::Rect> item_rect_vct;
        int total_item_height = 0;

        // 当组中没有项的时候看看是否需要显示组头
        if ((0 == item_count) && !model_->ShowGroupHeaderWhenDataEmpty(group_index))
        {
            // 列表为空的同时又不需要显示头的就不再计算一大堆东西了
            header_height = 0;
        }
        else
        {
            if (model_->EnableGroupItemDifferentSize(group_index))
            {
                gfx::Size diff_item_size;
                int line_item_max_height = 0;
                int item_index = 0;
                int x = 0;
                int y = header_height;
                std::vector<gfx::Size> line_item_size_vct;
                for (int line_index = 0; line_index < line_count; line_index++)
                {
                    line_item_max_height = 0;
                    line_item_size_vct.clear();

                    // 计算当前行高，取行内项高最大值
                    for (int line_item_index = 0;
                        line_item_index < item_count_per_line && item_index < item_count;
                        line_item_index++, item_index++)
                    {
                        diff_item_size = model_->GetItemSizeForEnableGroupItemDifferentSize(
                            show_bounds, group_index, item_index);
                        // 允许不等高但不允许不等宽
                        DCHECK(!diff_item_size.IsEmpty() && item_size.width() == diff_item_size.width());
                        // 行高取行内项高最大值
                        line_item_max_height = std::max(line_item_max_height, diff_item_size.height());
                        // 缓存行内各项的尺寸，省得再调model_->GetItemSizeForEnableGroupItemDifferentSize
                        line_item_size_vct.push_back(diff_item_size);
                    }

                    // 根据当前行高和GridView列表项的垂直对齐方式计算行内各项的位置
                    int item_y = 0;
                    for (size_t i = 0; i < line_item_size_vct.size(); i++)
                    {
                        x = i * item_size.width();

                        switch (v_align_)
                        {
                        case gfx::ALIGN_TOP:
                            item_y = y;
                            break;
                        case gfx::ALIGN_VCENTER:
                            item_y = y + (line_item_max_height - line_item_size_vct[i].height()) / 2;
                            break;
                        case gfx::ALIGN_BOTTOM:
                            item_y = y + line_item_max_height - line_item_size_vct[i].height();
                            break;
                        default:
                            break;
                        }

                        item_rect_vct.push_back(gfx::Rect(gfx::Point(x, item_y), line_item_size_vct[i]));
                    }

                    // 记录每行的bounds
                    line_rect_vct.push_back(gfx::Rect(0, y, item_count_per_line * item_size.width(), line_item_max_height));

                    // 累加列表项总行高
                    total_item_height += line_item_max_height;

                    // 下一行的Y坐标为上次的Y坐标加上当前行的行高
                    y += line_item_max_height;
                }
            }
            else
            {
                int item_index = 0;
                int x = 0;
                int y = header_height;
                for (int line_index = 0; line_index < line_count; line_index++)
                {
                    // 计算每项的位置
                    for (int line_item_index = 0;
                        line_item_index < item_count_per_line && item_index < item_count;
                        line_item_index++, item_index++)
                    {
                        x = line_item_index * item_size.width();

                        item_rect_vct.push_back(gfx::Rect(gfx::Point(x, y), item_size));
                    }

                    // 记录每行的bounds
                    line_rect_vct.push_back(gfx::Rect(0, y, item_count_per_line * item_size.width(), item_size.height()));

                    // 累加列表项总行高
                    total_item_height += item_size.height();

                    // 下一行的Y坐标为上次的Y坐标加上当前行的行高
                    y += item_size.height();
                }
            }
        }

        // 全部组内项布局完需要的高度和组头需要的高度就是这个组的总高度
        int group_height = header_height + total_item_height;

        // 将计算的结果返给调用处，调用处需要再进一步处理逻辑时就不需要再算一遍了
        if (out_header_height)
        {
            *out_header_height = header_height;
        }
        if (out_item_count)
        {
            *out_item_count = item_count;
        }
        if (out_item_count_per_line)
        {
            *out_item_count_per_line = item_count_per_line;
        }
        if (out_line_count)
        {
            *out_line_count = line_count;
        }
        if (out_line_rect_vct)
        {
            *out_line_rect_vct = std::move(line_rect_vct);
        }
        if (out_item_rect_vct)
        {
            *out_item_rect_vct = std::move(item_rect_vct);
        }

        return group_height;
    }
    else
    {
        NOTREACHED() << "gridview no support horizontal now !";
    }
    return 0;
}

void GridView::CheckItemVisibleBounds(GridItemView* item, bool check_visible/* = true*/)
{
    if (!item)
    {
        return;
    }

    DCHECK(item->parent() == this);

    bool reuse = false;
    if (check_visible)
    {
        gfx::Rect rect = item->GetVisibleBounds();
        if (rect.IsEmpty())
        {
            reuse = true;
        }
    }
    else
    {
        reuse = true;
    }
    if (reuse)
    {
        // 提供一个时机通知view即将被回收了
        item->BeforePushBackToItemDeque();

        item->SetVisible(false);
        item_reuse_map_[item->type_id()].push_back(item);
    }
}

bool GridView::IsLockWindowUpdate() const
{
    if (lock_window_update_)
    {
        return true;
    }
    const views::Widget *widget = GetWidget();
    if (widget)
    {
        if (widget->IsMinimized() || !widget->IsVisible())
        {
            return true;
        }
    }
    return false;
}

void GridView::LockWindowUpdate(bool lock)
{
    // 从锁定变成解锁应重新布局一遍可视区
    bool required_layout = false;
    if (!lock && lock_window_update_)
    {
        required_layout = true;
    }

    lock_window_update_ = lock;

    if (required_layout)
    {
        OnVisibleBoundsChanged();
    }
}

int GridView::GetGroupHeaderPosition(int group_index)
{
    gfx::Rect cnt_bounds = GetContentsBounds();
    int group_header_y = cnt_bounds.y();
    for (int i = 0; i < model_->GetNumberOfGroups() && i < group_index; i++)
    {
        group_header_y += GetGroupDetails(cnt_bounds, i);
    }
    return group_header_y;
}

int GridView::GetGroupItemPosition(int group_index, int item_index)
{
    gfx::Rect cnt_bounds = GetContentsBounds();
    if (!horiz_scroll_)
    {
        // 拿到指定组的起始Y坐标
        int group_header_y = GetGroupHeaderPosition(group_index);
        // 拿到指定组的项区域信息
        std::vector<gfx::Rect> item_rect_vct;
        int item_count = 0;
        GetGroupDetails(cnt_bounds, group_index, nullptr, &item_count, nullptr, nullptr, nullptr, &item_rect_vct);
        DCHECK(item_rect_vct.size() == static_cast<size_t>(item_count));
        if (item_index >= 0 && item_index < item_count)
        {
            return group_header_y + item_rect_vct[item_index].y();
        }
    }
    else
    {
        NOTREACHED() << "gridview no support horizontal now !";
    }
    return 0;
}

gfx::Rect GridView::GetGroupBounds(int group_index)
{
    gfx::Rect group_bounds;
    gfx::Rect cnt_bounds = GetContentsBounds();
    int group_header_y = cnt_bounds.y();
    for (int i = 0; i < model_->GetNumberOfGroups() && i <= group_index; i++)
    {
        int group_height = GetGroupDetails(cnt_bounds, i);
        if (i == group_index)
        {
            group_bounds.SetRect(cnt_bounds.x(), group_header_y, cnt_bounds.width(), group_height);
        }
        group_header_y += group_height;
    }
    return group_bounds;
}

std::vector<gfx::Rect> GridView::GetAllGroupBounds()
{
    std::vector<gfx::Rect> bounds_vct;

    gfx::Rect cnt_bounds = GetContentsBounds();
    int group_header_y = cnt_bounds.y();
    int group_height = 0;
    for (int group_index = 0; group_index < model_->GetNumberOfGroups(); group_index++)
    {
        group_height = GetGroupDetails(cnt_bounds, group_index);
        bounds_vct.push_back(gfx::Rect(cnt_bounds.x(), group_header_y, cnt_bounds.width(), group_height));
        group_header_y += group_height;
    }

    return bounds_vct;
}

void GridView::VisibilityChanged(View* starting_from, bool is_visible)
{
    if (visible())
    {
        OnVisibleBoundsChanged();
    }
}

void GridView::SetDetectThickness(int thickness)
{
    detect_border_thickness_ = thickness;
}

bool GridView::GetFirstVisibleItemIndex(int *group_index, int *item_index)
{
    if (!visible_items_.empty())
    {
        ItemInfo &item = visible_items_[0];
        if (group_index)
        {
            *group_index = item.group_index_;
        }
        if (item_index)
        {
            *item_index = item.item_index_;
        }
    }
    return !visible_items_.empty();
}

bool GridView::GetLastVisibleItemIndex(int *group_index, int *item_index) {
    if (!visible_items_.empty())
    {
        ItemInfo &item = visible_items_.back();
        if (group_index)
        {
            *group_index = item.group_index_;
        }
        if (item_index)
        {
            *item_index = item.item_index_;
        }
    }
    return !visible_items_.empty();
}

void GridView::OnVisibleBoundsNearContentsBoundsTop(bool horiz, GridModel::ScrollDirection dir, ScrollReason reason)
{
    model_->OnVisibleBoundsNearContentsBoundsTop(horiz, dir, reason);
}

void GridView::OnVisibleBoundsNearContentsBoundsBottom(bool horiz, GridModel::ScrollDirection dir, ScrollReason reason)
{
    model_->OnVisibleBoundsNearContentsBoundsBottom(horiz, dir, reason);
}
