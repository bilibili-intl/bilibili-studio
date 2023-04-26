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

    // ��ν��NOSCROLL�ؼ�ֻ�ܱ�֤���ڿؼ����Ը�������֮���������֮ǰ��һ���ɼ����
    // �������������������������������������ǰ�ĵ�һ���ɼ��������������һ�£�
    // �����Ǳ�֤�ϲ㿴�������ݱ��ֲ���
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

    // �����ϴε�ȫ���ɼ������Ҫ����Ƿ�ɼ���ȫ���Ż����ö���
    for each (GridView::ItemInfo var in visible_items_)
    {
        CheckItemVisibleBounds(var.view_, false);
    }
    // ���¼���ߴ磬���²��ֿ�����
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

    // ���㲢�����Լ��ĳߴ��Ա�ScrollView��ʾ������
    gfx::Size size;
    if (!horiz_scroll_)
    {
        // ����������->ȡ������������->������ߴ�͸�scrollview�ߴ����ÿ������ʾ�������Լ���Ҫ���в�����ʾ��
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

    // �ӿڸı�֮����Ҫ���¼����֪��Щitem�ñ���ʾ�ڿ�����
    gfx::Rect visible_bounds = GetVisibleBounds();
    gfx::Rect local_bounds = GetContentsBounds();
    if (!horiz_scroll_)
    {
        // Ϊ�ӿڼ������������׸��ϲ��ṩһ������֪ͨʱ��
        // �ڴ���֪ͨ�о��ܸ�����������õģ�����ͱ�����ȷʵ��������֮���һ��UpdateData��
        // �������ٴμ���һ��ɼ��Ӱ�첼��Ч��
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

        // ���ݵ�ǰ��������ȡ�ñ���ʾ�������Ϣ��header/item��bounds��index��
        // ���˴θı�ķ�Χ���ñ���ʾ��������һ�θı�ñ���ʾ����һ�¾�û��Ҫ�ٴβ�����
        std::vector<GridView::ItemInfo> visible_items = std::move(GetVisibleItems(visible_bounds));
        if (visible_items != visible_items_)
        {
            // �����ϴε�ȫ���ɼ���Ѿ����ɼ���view�Ż����ö���
            for each (GridView::ItemInfo var in visible_items_)
            {
                CheckItemVisibleBounds(var.view_);
            }

            for (std::vector<GridView::ItemInfo>::iterator v_iter, iter = visible_items.begin(); iter != visible_items.end(); ++iter)
            {
                // �·�Χ�Ŀɼ���;ɵ��ص��Ĳ����ٻ�ȡ
                v_iter = std::find_if(visible_items_.begin(), visible_items_.end(),
                    [iter](GridView::ItemInfo &var)->bool {
                    return (var == *iter);
                });
                if (v_iter != visible_items_.end())
                {
                    // ��Ϊ���Ὣ���µĿɼ��б�ֱ��visible_items��������visible_items_
                    // ������������ȡview����ѻ������view��¼��������Ȼ����ֱ�ӿ�����ʱ�򻺴��б��е�view_�ͻ���null��
                    iter->view_ = v_iter->view_;

                    // ��/��������һ������bounds����˵��Ӧ����ǰ�����������ɾ��û�е���UpdateData
                    DCHECK(iter->bounds_ == v_iter->bounds_);
                }
                else
                {
                    // ��������/��������������Դ��������Դ�����������ظ����view����id��Ȼ���ڸ��ݴ�id
                    // �����ö�����ȡ�����ע��Ķ�Ӧ����view�Ĵ�������ȥ����һ���µ�view
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

                    // header����Ϊnull�����
                    if (!iter->is_header_)
                    {
                        DCHECK(iter->view_);
                    }

                    // �����ö������õ�viewҪˢ��һ�������µ�����
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

        // ����ĸ߶�Ϊ0�ͼ�����һ��
        if (0 == group_height)
        {
            continue;
        }

        // group_index�����������ͷ��ȫ����ĵײ���y���궼��δ����������Ķ���y����ͼ���������һ����
        if (last_bottom + group_height <= visible_bounds.y())
        {
            last_bottom += group_height;
            continue;
        }

        // ȡ�����е������ֵ����ǰֻ����ȿ������ֱ�ӳ˼��ɣ�
        /*auto max_iter = std::max(line_rect_vct.begin(), line_rect_vct.end(),
            [](const gfx::Rect& elem1, const gfx::Rect& elem2)->bool {
                return elem1.width() < elem2.width();
            });
        int line_max_width = max_iter->width();*/
        int line_max_width = item_count_per_line *
            model_->GetItemSizeForGroup(visible_bounds, group_index).width();

        // ���������������;����������и����X��ƫ�ƣ���ǰ������������ÿ�е����ǽ����ŵģ�
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

        // �ȼ�������header������header�Ѿ�����������˾ͼ��뵽�ɼ����б���
        int group_origin_y = last_bottom;
        gfx::Rect header_bounds(offset_x, group_origin_y, line_max_width, header_height);
        if (visible_bounds.Intersects(header_bounds))
        {
            ItemInfo header(true, group_index);
            header.bounds_ = header_bounds;
            visible_items.push_back(header);
        }

        // �ۼ�y��ƫ�ƣ�ͷ������һ�������������Ͳ����ټ���������
        // ����ǰ�Ŀ�����ֻ����ʾ��header�����ܼ���������ʾitem��
        last_bottom += header_height;
        if (last_bottom >= visible_bounds.bottom())
        {
            break;
        }

        // ���б�����������������ĵ�һ�е���������¼����
        int first_visible_line = 0;
        // �ӿ�����������header�ײ����п�϶�Ļ������ÿ�϶�����ɶ����У�
        // �ݲ��µ����о����Ѿ���ʼ������������������ˣ������п�ʼ����item��
        int hb_to_vby_thickness = visible_bounds.y() - last_bottom;
        if (hb_to_vby_thickness > 0)
        {
            for (size_t line_index = 0; line_index < line_rect_vct.size(); line_index++)
            {
                gfx::Rect line_bounds(offset_x, group_origin_y + line_rect_vct[line_index].y(),
                    line_rect_vct[line_index].width(), line_rect_vct[line_index].height());
                if (visible_bounds.Intersects(line_bounds))
                {
                    // �ҵ��˵�һ���ɼ���
                    first_visible_line = line_index;
                    break;
                }
                else
                {
                    // �����������л����ɼ������ۼ��и�y��ƫ�ƣ�����������һ��
                    last_bottom += line_rect_vct[line_index].height();
                }
            }
        }

        DCHECK(last_bottom <= visible_bounds.bottom()) << "should not happen !";

        // �ӵ�һ���ɼ��еĵ�һ��item��ʼ����
        gfx::Rect item_bounds;
        int item_index = first_visible_line * item_count_per_line;
        for (int line_index = first_visible_line; line_index < (int)line_rect_vct.size(); line_index++)
        {
            for (int line_item_index = 0;
                line_item_index < item_count_per_line && item_index < item_count;
                line_item_index++, item_index++)
            {
                // �����item������
                item_bounds = gfx::Rect(
                    offset_x + item_rect_vct[item_index].x(),
                    group_origin_y + item_rect_vct[item_index].y(),
                    item_rect_vct[item_index].width(),
                    item_rect_vct[item_index].height());
                // item�Ѿ�����������ͼ��뵽�ɼ����б���
                if (visible_bounds.Intersects(item_bounds))
                {
                    ItemInfo item(false, group_index, item_index);
                    item.bounds_ = item_bounds;
                    visible_items.push_back(item);
                }
            }

            // �ۼ�y�ᣬһ�������������Ͳ����ټ���������
            last_bottom += line_rect_vct[line_index].height();
            if (last_bottom >= visible_bounds.bottom())
            {
                break;
            }
        }

        // һ�������������Ͳ����ټ���������
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

    // ���ö�����û��������͵�ʵ���˾���RegisterItemTypeע��Ĺ�����������һ����ʵ��
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
    OUT int* out_header_height /*= nullptr*/, /* ͷ���߶� */
    OUT int* out_item_count /*= nullptr*/, /* �����ܹ��ж����� */
    OUT int* out_item_count_per_line /*= nullptr*/, /* һ�ж����� */
    OUT int* out_line_count /*= nullptr*/, /* �ܹ��ּ��� */
    OUT std::vector<gfx::Rect>* out_line_rect_vct /*= nullptr*/, /* ÿ�е�bounds���и�Ϊ������ߵ����ֵ�� */
    OUT std::vector<gfx::Rect>* out_item_rect_vct /*= nullptr /* �������POSITION���������ͷ���Ͻ�λ�ã� */
)
{
    if (!horiz_scroll_)
    {
        int header_height = model_->GetGroupHeaderHeight(group_index);
        const int item_count = model_->GetItemCountForGroup(group_index);
        const gfx::Size item_size = model_->GetItemSizeForGroup(show_bounds, group_index);
        DCHECK(!item_size.IsEmpty());
        // һ�еĿ�ȿ������ɶ�������Ҫ��֤��ǰ�����п���ڵ���һ��Ŀ�ȣ������û�����㲼����
        const int item_count_per_line = std::floor(show_bounds.width() / item_size.width());
        DCHECK(item_count_per_line > 0);
        // ��ǰ���������Ҫ�����ڵ����������Ҫ������
        const int line_count = std::ceil(item_count * 1.0 / item_count_per_line);

        // ����ȫ�������������Ҫ�ĸ߶ȣ�����������ǵȸߵģ���ô������ֱ�ӳ�����߼��ɣ�
        // ����������ǲ��ȸߵģ���ôҪһ��һ�еļ����иߣ��и�ȡ������߶ȵ����ֵ
        std::vector<gfx::Rect> line_rect_vct;
        std::vector<gfx::Rect> item_rect_vct;
        int total_item_height = 0;

        // ������û�����ʱ�򿴿��Ƿ���Ҫ��ʾ��ͷ
        if ((0 == item_count) && !model_->ShowGroupHeaderWhenDataEmpty(group_index))
        {
            // �б�Ϊ�յ�ͬʱ�ֲ���Ҫ��ʾͷ�ľͲ��ټ���һ��Ѷ�����
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

                    // ���㵱ǰ�иߣ�ȡ����������ֵ
                    for (int line_item_index = 0;
                        line_item_index < item_count_per_line && item_index < item_count;
                        line_item_index++, item_index++)
                    {
                        diff_item_size = model_->GetItemSizeForEnableGroupItemDifferentSize(
                            show_bounds, group_index, item_index);
                        // �����ȸߵ��������ȿ�
                        DCHECK(!diff_item_size.IsEmpty() && item_size.width() == diff_item_size.width());
                        // �и�ȡ����������ֵ
                        line_item_max_height = std::max(line_item_max_height, diff_item_size.height());
                        // �������ڸ���ĳߴ磬ʡ���ٵ�model_->GetItemSizeForEnableGroupItemDifferentSize
                        line_item_size_vct.push_back(diff_item_size);
                    }

                    // ���ݵ�ǰ�иߺ�GridView�б���Ĵ�ֱ���뷽ʽ�������ڸ����λ��
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

                    // ��¼ÿ�е�bounds
                    line_rect_vct.push_back(gfx::Rect(0, y, item_count_per_line * item_size.width(), line_item_max_height));

                    // �ۼ��б������и�
                    total_item_height += line_item_max_height;

                    // ��һ�е�Y����Ϊ�ϴε�Y������ϵ�ǰ�е��и�
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
                    // ����ÿ���λ��
                    for (int line_item_index = 0;
                        line_item_index < item_count_per_line && item_index < item_count;
                        line_item_index++, item_index++)
                    {
                        x = line_item_index * item_size.width();

                        item_rect_vct.push_back(gfx::Rect(gfx::Point(x, y), item_size));
                    }

                    // ��¼ÿ�е�bounds
                    line_rect_vct.push_back(gfx::Rect(0, y, item_count_per_line * item_size.width(), item_size.height()));

                    // �ۼ��б������и�
                    total_item_height += item_size.height();

                    // ��һ�е�Y����Ϊ�ϴε�Y������ϵ�ǰ�е��и�
                    y += item_size.height();
                }
            }
        }

        // ȫ�������������Ҫ�ĸ߶Ⱥ���ͷ��Ҫ�ĸ߶Ⱦ����������ܸ߶�
        int group_height = header_height + total_item_height;

        // ������Ľ���������ô������ô���Ҫ�ٽ�һ�������߼�ʱ�Ͳ���Ҫ����һ����
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
        // �ṩһ��ʱ��֪ͨview������������
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
    // ��������ɽ���Ӧ���²���һ�������
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
        // �õ�ָ�������ʼY����
        int group_header_y = GetGroupHeaderPosition(group_index);
        // �õ�ָ�������������Ϣ
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
