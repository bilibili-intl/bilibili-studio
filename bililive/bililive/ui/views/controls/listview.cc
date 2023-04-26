#include "listview.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/widget/widget.h"


namespace
{
    const int kDefaultItemWidth = 120 * ui::GetDPIScale();
    const int kDefaultItemHeight = 32 * ui::GetDPIScale();
    const int kDetectScrollThickness = 30 * ui::GetDPIScale();

    class ScrollViewForListView : public ScrollViewWithFloatingScrollbar
    {
    public:
        explicit ScrollViewForListView(ListView *contents)
            : ScrollViewWithFloatingScrollbar(contents,true)
            , list_view_(contents)
        {
        }

    protected:
        bool OnMousePressed(const ui::MouseEvent& event) override
        {
            gfx::Point pt = event.location();
            ConvertPointToTarget(this, list_view_, &pt);
            if (!list_view_->GetLocalBounds().Contains(pt))
            {
                list_view_->ClickOnNonItemArea();
            }
            return __super::OnMousePressed(event);
        }

    private:
        ListView* list_view_;
    };
}


// ListItemView
const char ListItemView::kViewClassName[] = "ListItemView";

ListItemView::ListItemView(ListView *list_view)
    : list_view_(list_view)
    , checked_(false)
    , is_mouse_enter_(false)
{
    DCHECK(list_view);
}

void ListItemView::SetChecked(bool checked)
{
    // ʹ�ô˽ӿ�ֱ��ѡ���б������б�ֻѡ�����ѡ������
    if (checked || (checked_ != checked ))
    {
        list_view_->SetCheck(this, checked, false, CHANGED_BY_API);
    }
}

void ListItemView::SetCheckedInternal(bool checked, ListItemViewChangeReason reason)
{
    if (checked_ != checked)
    {
        checked_ = checked;

        OnSelectedChanged(reason);

        if (list_view_->delegate())
        {
            list_view_->delegate()->OnItemSelectedChanged(reason);
        }

        SchedulePaint();
    }
}

int ListItemView::index() const
{
    return list_view_->GetIndexOf(this);
}

void ListItemView::OnMouseEntered(const ui::MouseEvent& event)
{
    is_mouse_enter_ = true;
    SchedulePaint();
}

void ListItemView::OnMouseExited(const ui::MouseEvent& event)
{
    is_mouse_enter_ = false;
    SchedulePaint();
}

bool ListItemView::OnMousePressed(const ui::MouseEvent& event)
{
    RequestFocus();
    list_view_->SetCheck(this, true, event.IsControlDown());

    if (event.IsOnlyLeftMouseButton())
    {
        return list_view_->OnItemViewPressed(this, event);
    }
    return false;
}

bool ListItemView::OnMouseDragged(const ui::MouseEvent& event)
{
    // ���itemview����ContextMenu���Ҽ�����ǿ��Լ������¼�·�ɵģ�
    // ��ʹ�����OnMousePresssed���ص���false���ο�View::ProcessMousePressed
    if (event.IsOnlyLeftMouseButton())
    {
        return list_view_->OnItemViewDragged(this, event);
    }
    return false;
}

void ListItemView::OnMouseReleased(const ui::MouseEvent& event)
{
    list_view_->OnItemViewReleased(this, event);
}

void ListItemView::OnPaintBackground(gfx::Canvas* canvas)
{
    SkColor clr = list_view_->item_background_color(is_mouse_enter_ ?
                                                    views::Button::STATE_HOVERED :
                                                    views::Button::STATE_NORMAL);
    clr = checked() ? list_view_->item_background_color(views::Button::STATE_PRESSED) : clr;
    canvas->DrawColor(clr);
}



// ListView
const char ListView::kViewClassName[] = "ListView";

ListView::ListView(ListViewController *controller)
    : controller_(controller)
    , allow_multiple_select_(false)
    , allow_item_drag_move_(false)
    , item_size_(kDefaultItemWidth, kDefaultItemHeight)
    , current_drag_item_index_(-1)
    , initial_drag_item_index_(-1)
    , is_item_drag_(false)
    , cancel_drag_item_(true)
    , drag_item_view_(nullptr)
{
    set_focusable(true);
    set_focus_border(nullptr);

    scroll_view_ = new ScrollViewForListView(this);
    scroll_view_->set_hide_horizontal_scrollbar(true);

    for (int i = 0; i < views::Button::ButtonState::STATE_COUNT; i++)
    {
        item_bk_clrs_[i] = SK_ColorWHITE;
    }
    item_bk_clrs_[views::Button::STATE_HOVERED] =
        GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_FocusedMenuItemBackgroundColor);
    item_bk_clrs_[views::Button::STATE_PRESSED] =
        GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_TableSelectionBackgroundFocused);
}

ListView::~ListView()
{
}

void ListView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
    }
}

void ListView::Layout()
{
    SetSize(gfx::Size(std::max(Container()->width(), item_size_.width()), item_size_.height() * child_count()));

    int y_offset = 0;
    for (int i = 0, count = child_count(); i < count; i++)
    {
        child_at(i)->SetBounds(0, y_offset, width(), item_size_.height());
        y_offset += item_size_.height();
    }
}

void ListView::PaintChildren(gfx::Canvas* canvas)
{
    if (is_item_drag_ && drag_item_view_)
    {
        bool found_view = false;
        // �ɵ��������Ϊ������ʱ������Ļ������ȼ��ߣ�˳�����ˣ������ƶ�Ч���ã�
        // �ɸ��������Ϊ������ʱ���ڻ������ȼ����������ƶ�Ч���ֻ������ĺ�С��һ�������ڶ���
        // ���Ե����קʱʼ�����Ż��Ʊ���ק�ʹ��ͼ��ʼ�մ������
        for (int i = 0, count = child_count(); i < count; ++i)
        {
            if (child_at(i) == drag_item_view_)
            {
                found_view = true;
            }
            else
            {
                if (!child_at(i)->layer())
                {
                    child_at(i)->Paint(canvas);
                }
            }
        }
        if (found_view && !drag_item_view_->layer())
        {
            drag_item_view_->Paint(canvas);
        }
    }
    else
    {
        __super::PaintChildren(canvas);
    }
}

bool ListView::OnKeyPressed(const ui::KeyEvent& event)
{
    if (event.key_code() == ui::KeyboardCode::VKEY_ESCAPE)
    {
        // ȡ����ק����
        cancel_drag_item_ = true;
        if (is_item_drag_)
        {
            is_item_drag_ = false;

            if (-1 != GetIndexOf(drag_item_view_))
            {
                drag_item_view_->RequestFocus();
            }
            else
            {
                NOTREACHED() << "unbelievable!";
            }

            // ��ק���������������������²���
            if (initial_drag_item_index_ != current_drag_item_index_)
            {
                if (animation_ && animation_->IsAnimating())
                {
                    animation_->Cancel();
                }
                SetItemViewIndex(drag_item_view_, initial_drag_item_index_, ListItemView::CHANGED_BY_USER);
                Layout();
            }
            drag_item_view_ = nullptr;
        }
        return true;
    }
    return __super::OnKeyPressed(event);
}

void ListView::AddChildView(View* view)
{
    __super::AddChildView(view);
}

void ListView::AddChildViewAt(View* view, int index)
{
    __super::AddChildViewAt(view, index);
}

void ListView::OnBoundsAnimatorDone(views::BoundsAnimator* animator)
{
}

void ListView::SetEnabledItemDragMove(bool enabled)
{
    if (allow_item_drag_move_ != enabled)
    {
        allow_item_drag_move_ = enabled;

        if (allow_item_drag_move_)
        {
            animation_.reset(new views::BoundsAnimator(this));
            animation_->AddObserver(this);
        }
        else
        {
            if (animation_)
            {
                animation_.reset(nullptr);

                // ֹͣ����֮��ȷ������item����ȷlayout�ˣ���ֹ�����ŵ�һ��ͱ�ͣ�˵���item��ûŲ��λ������
                Layout();
            }
        }
    }
}

void ListView::SetItemBackgroundColor(views::Button::ButtonState state, SkColor color, bool repaint/* = false*/)
{
    item_bk_clrs_[state] = color;
    if (repaint)
    {
        SchedulePaint();
    }
}

SkColor ListView::item_background_color(views::Button::ButtonState state) const
{
    return item_bk_clrs_[state];
}

std::vector<int> ListView::GetSelectedItems() const
{
    std::vector<int> vct;
    for (int i = 0, count = child_count(); i < count; i++)
    {
        if (((ListItemView*)child_at(i))->checked())
        {
            vct.push_back(i);
        }
    }
    return vct;
}

std::vector<ListItemView*> ListView::GetItemViews(bool desc/* = false*/) const
{
    std::vector<ListItemView*> vct;
    if (!desc)
    {
        for (int i = 0, count = child_count(); i < count; i++)
        {
            //if (IsListItemView(child_at(i)))
            {
                vct.push_back((ListItemView*)child_at(i));
            }
        }
    }
    else
    {
        for (int i = child_count() - 1; i >= 0; i--)
        {
            //if (IsListItemView(child_at(i)))
            {
                vct.push_back((ListItemView*)child_at(i));
            }
        }
    }
    return vct;
}

void ListView::SetItemSize(const gfx::Size& size, bool relayout /*= false*/)
{
    DCHECK(size.height() > 0);

    item_size_ = size;
    if (relayout)
    {
        Layout();
    }
}

void ListView::AddItemView(ListItemView *itemview, bool relayout/* = true*/)
{
    if ((itemview->parent() == this) || (itemview->list_view() != this))
    {
        return;
    }

    AddItemViewAt(itemview, child_count(), relayout);
}

void ListView::AddItemViewAt(ListItemView* view, int index, bool relayout/* = true*/)
{
    DCHECK(IsListItemView(view));

    AddChildViewAt(view, index);

    if(relayout)
    {
        InvalidateLayout();
        PreferredSizeChanged();
    }
}

void ListView::SetItemViewIndex(ListItemView* view, int new_index,
                                ListItemView::ListItemViewChangeReason reason/* = ListItemView::CHANGED_BY_API*/)
{
    int old_index = GetIndexOf(view);
    if(old_index != new_index)
    {
        __super::ReorderChildView(view, new_index);

        if (controller_)
        {
            // ����new_index����Ч���жϣ�views::View���ܻ�ı�view��ʵ�������������»�ȡһ��
            new_index = GetIndexOf(view);
            if (new_index != old_index)
            {
                controller_->OnItemsOrderChanged(reason);
            }
        }
    }
}

int ListView::GetFirstSelectedItemPosition() const
{
    int sel = -1;
    for (int i = 0, count = child_count(); i < count; i++)
    {
        //if (IsListItemView(child_at(i)))
        {
            if (((ListItemView*)child_at(i))->checked())
            {
                sel = i;
                break;
            }
        }
    }
    return sel;
}

int ListView::FindItem(_In_ const LVFINDITEMINFO* pFindInfo, _In_ int nStart /*= -1*/) const
{
    DCHECK(controller_);

    if(controller_)
    {
        nStart = std::max(0, nStart);
        for (int i = nStart, count = child_count(); i < count; i++)
        {
            //if (IsListItemView(child_at(i)))
            {
                if (controller_->FindListItem(pFindInfo, (ListItemView*)child_at(i)))
                {
                    return i;
                }
            }
        }
    }
    return -1;
}

int ListView::GetNextSelectedItem(_Inout_ int& pos) const
{
    DCHECK(pos >= 0);

    int sel = -1;
    for (int i = pos + 1, count = child_count(); i < count; i++)
    {
        //if (IsListItemView(child_at(i)))
        {
            if (((ListItemView*)child_at(i))->checked())
            {
                sel = i;
                break;
            }
        }
    }
    return sel;
}

UINT ListView::GetSelectedCount() const
{
    int nums = 0;
    for (int i = 0, count = child_count(); i < count; i++)
    {
        //if (IsListItemView(child_at(i)))
        {
            if (((ListItemView*)child_at(i))->checked())
            {
                nums++;
            }
        }
    }
    return nums;
}

bool ListView::GetCheck(_In_ int nItem) const
{
    if (nItem >= 0 && nItem < child_count())
    {
        //if (IsListItemView(child_at(nItem)))
        {
            return ((ListItemView*)child_at(nItem))->checked();
        }
    }
    return false;
}

void ListView::SetCheck(_In_ int nItem, _In_ bool fCheck /*= TRUE*/, bool only_one_checkd/* = true*/,
                        ListItemView::ListItemViewChangeReason reason/* = ListItemView::CHANGED_BY_API*/)
{
    if (nItem >= 0 && nItem < child_count())
    {
        //if (IsListItemView(child_at(nItem)))
        {
            ((ListItemView*)child_at(nItem))->SetCheckedInternal(fCheck, reason);
        }
    }

    bool mutil_sel = (!only_one_checkd && allow_multiple_select_);
    // ������Ҫѡ�е��ؼ��������ѡ�ľͰ��������ѡ
    if (fCheck && !mutil_sel)
    {
        for (int i = 0, count = child_count(); i < count; i++)
        {
            if (i != nItem)
            {
                //if (IsListItemView(child_at(nItem)))
                {
                    ((ListItemView*)child_at(i))->SetCheckedInternal(false, reason);
                }
            }
        }
    }
}

void ListView::SetCheck(ListItemView *item_view, _In_ bool fCheck /*= true*/,
                        bool is_control_down/* = false*/,
                        ListItemView::ListItemViewChangeReason reason/* = ListItemView::CHANGED_BY_USER*/)
{
    if (item_view->parent() == this && item_view->list_view() == this)
    {
        int index = GetIndexOf(item_view);
        if (-1 != index)
        {
            SetCheck(index, fCheck, !is_control_down, reason);
        }
    }
}

bool ListView::GetItemRect(_In_ int nItem, _Out_ gfx::Rect &lpRect) const
{
    bool item_valid = (nItem >= 0 && nItem < child_count());
    lpRect = GetItemRect(nItem);
    return item_valid;
}

gfx::Rect ListView::GetItemRect(int nItem) const
{
    return gfx::Rect(0, nItem * item_size_.height(), width(), item_size_.height());
}

void ListView::ClickOnNonItemArea()
{
    SetCheck(-1, true, true, ListItemView::CHANGED_BY_USER);
    RequestFocus();
}

int ListView::GetItemCount() const
{
    return child_count();
}

bool ListView::OnItemViewPressed(const ListItemView* item_view, const ui::MouseEvent& event)
{
    // ���������ͬʱ�϶�
    if(enabled_item_drag_move() && GetSelectedCount() == 1)
    {
        gfx::Point pt(event.location());
        ConvertPointToTarget(item_view, this, &pt);
        cancel_drag_item_ = false;
        drag_item_view_ = const_cast<ListItemView*>(item_view);
        initial_drag_item_index_ = GetIndexOf(item_view);
        current_drag_item_index_ = initial_drag_item_index_;
        last_item_drag_point_ = pt;
        return true;
    }
    return false;
}

bool ListView::OnItemViewDragged(const ListItemView* item_view, const ui::MouseEvent& event)
{
    if (!enabled_item_drag_move() || (GetSelectedCount() != 1))
    {
        return false;
    }

    if (cancel_drag_item_)
    {
        return false;
    }

    if (!is_item_drag_)
    {
        is_item_drag_ = true;
        RequestFocus();
    }
    gfx::Point pt(event.location());
    ConvertPointToTarget(item_view, this, &pt);
    // ��ȷ����ק������ƶ�����
    bool down_direction = true;
    down_direction = (pt.y() - last_item_drag_point_.y() >= 0);
    // ��ȷ����ǰ������ڵ�������item���Լ�����������
    int nItem = pt.y() / item_size_.height();
    // ��ק�����ܵ��ķ�Χ�����ڵ�ǰ������ռ�ݵķ�Χ
    nItem = std::min(nItem, child_count() - 1);
    nItem = std::max(0, nItem);
    if(nItem != current_drag_item_index_)
    {
        gfx::Rect hover_rect = GetItemRect(nItem);
        gfx::Rect current_rect = GetItemRect(current_drag_item_index_);
        // ��������ƶ�������Ƿ񴩹���ǰhover���������߾���item��Ų���Ƿ�����
        // ��ֻ���жϵ�ǰ�����뱻��ק�����/�±߾��Ƿ񳬹���ߵ�һ���������Ƿ���ҪŲ��
        // �ҹ���hover����һ��ű��hover�����������丽������
        bool do_move = false;
        if (down_direction)
        {
            do_move = (pt.y() - current_rect.bottom() >= item_size_.height() / 2);
            nItem = do_move ? nItem : nItem - 1;
        }
        else
        {
            do_move = (current_rect.y() - pt.y() >= item_size_.height() / 2);
            nItem = do_move ? nItem : nItem + 1;
        }
        if (do_move)
        {
            DCHECK(nItem != current_drag_item_index_);
            // ʵʱ�������߼�˳�򣬱�����ʱ���߼�˳���UI˳�������ȫһ�µ�
            SetItemViewIndex(const_cast<ListItemView*>(item_view), nItem, ListItemView::CHANGED_BY_USER);
            // ��ֻ��Ҫ��Ӱ�쵽�ķ�Χ�е����Ƶ����Ӧ��λ�ü���
            // ����һ�μ�¼��ճ����ͱ���ճ����֮���������շ���ƽ��
            int min_index = std::min(nItem, current_drag_item_index_);
            int max_index = std::max(nItem, current_drag_item_index_);
            if (animation_)
            {
                for (int i = min_index; i <= max_index; i++)
                {
                    gfx::Rect item_rect;
                    if (GetItemRect(i, item_rect))
                    {
                        animation_->AnimateViewTo(child_at(i), item_rect);
                    }
                }
            }

            current_drag_item_index_ = nItem;
        }
    }

    last_item_drag_point_ = pt;

    // �ϵ�ͷ/β��ʱ���Զ�����
    if (Container()->vertical_scroll_bar()->visible())
    {
        gfx::Rect vsb_rect = GetVisibleBounds();
        if (down_direction)
        {
            if (pt.y() >= vsb_rect.bottom() - kDetectScrollThickness)
            {
                ScrollRectToVisible(gfx::Rect(vsb_rect.x(),
                                              std::min(vsb_rect.y() + item_size_.height(), height() - vsb_rect.height()),
                                              vsb_rect.width(), vsb_rect.height()));
            }
        }
        else
        {
            if (pt.y() <= vsb_rect.y() + kDetectScrollThickness)
            {
                ScrollRectToVisible(gfx::Rect(vsb_rect.x(),
                                              std::max(vsb_rect.y() - item_size_.height(), 0),
                                              vsb_rect.width(), vsb_rect.height()));
            }
        }
    }

    return true;
}

void ListView::OnItemViewReleased(const ListItemView* item_view, const ui::MouseEvent& event)
{
    if (cancel_drag_item_)
    {
        return;
    }

    if (is_item_drag_)
    {
        is_item_drag_ = false;

        if(-1 != GetIndexOf(drag_item_view_))
        {
            drag_item_view_->RequestFocus();
        }
        else
        {
            NOTREACHED() << "unbelievable!";
        }

        drag_item_view_ = nullptr;

        // ��ק���������������������²���
        if(initial_drag_item_index_ != current_drag_item_index_)
        {
            if (animation_ && animation_->IsAnimating())
            {
                animation_->Cancel();
                Layout();
            }
        }
    }
}

ListItemView* ListView::DeleteItem(_In_ int nItem)
{
    ListItemView *view = GetItem(nItem);
    if (view)
    {
        RemoveChildView(view);
        PreferredSizeChanged();
    }
    return view;
}

void ListView::DeleteAllItems(bool delete_children)
{
    RemoveAllChildViews(delete_children);

    PreferredSizeChanged();
}

ListItemView* ListView::GetItem(int nItem)
{
    if (nItem >= 0 && nItem < child_count())
    {
        views::View* view = child_at(nItem);
        if (IsListItemView(view))
        {
            return (ListItemView*)view;
        }
    }
    return nullptr;
}

bool ListView::IsListItemView(const views::View* view)
{
    DCHECK(view);

    return (strcmp(view->GetClassName(), ListItemView::kViewClassName) == 0);
}
