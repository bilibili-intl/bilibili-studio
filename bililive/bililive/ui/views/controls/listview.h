#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_LISTVIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_LISTVIEW_H

#include "bililive_floating_scroll_view.h"

#include "ui/gfx/text_constants.h"
#include "ui/views/view.h"
#include "ui/views/animation/bounds_animator.h"
#include "ui/views/animation/bounds_animator_observer.h"
#include "ui/views/controls/button/button.h"

namespace
{
    class ScrollViewForListView;
}

class ListView;

// �б������
class ListItemView : public views::View
{
public:
    enum ListItemViewChangeReason
    {
        CHANGED_BY_USER,
        CHANGED_BY_API,
    };

    static const char kViewClassName[];

    explicit ListItemView(ListView *list_view);
    virtual ~ListItemView() = default;

    void SetChecked(bool checked);
    bool checked() const { return checked_; }
    int index() const;

protected:
    // View
    const char* GetClassName() const override{ return kViewClassName; };
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    virtual void OnSelectedChanged(ListItemViewChangeReason reason) {}

    bool IsMouseEntered() const { return is_mouse_enter_; }
    ListView* list_view() const { return list_view_; }

private:
    void SetCheckedInternal(bool checked, ListItemViewChangeReason reason);

private:
    friend class ListView;
    ListView *list_view_;
    bool checked_;
    bool is_draging_;
    bool is_mouse_enter_;

    DISALLOW_COPY_AND_ASSIGN(ListItemView);
};

struct LVFINDITEMINFO
{
    int type = 0;
    std::string atext;
    base::string16 wtext;
    int64 value = 0;
    void* param = nullptr;
};

// �ص��Ϳ�����
class ListViewController
{
public:
    virtual void OnItemSelectedChanged(ListItemView::ListItemViewChangeReason reason) {}
    virtual void OnItemsOrderChanged(ListItemView::ListItemViewChangeReason reason) {}

    virtual bool FindListItem(const LVFINDITEMINFO* pFindInfo, ListItemView* item_view) = 0;
};

// Ҫ������ק�ƶ�ʱ�б�������һ���ƶ���Ч�����־ͱ�������ק��ʱ������ϸı�View��˳��
// �������ִӵڶ��ο�ʼ��������ƽ�ƣ����м������ſ����б���Viewʱ�� ���UI˳����߼�˳��᲻�ϵ�
// ��������ʹ�����ڻ�ȡָ��UI˳��ε�Viewʵ�����ϱ�úܸ��ӣ����ǲ��ò����ô�����ԭ����Y�᷽�����
// �ķ�ʽ����һ�Ƚ�UI˳���и�View��Rect��ȷ������Ҫ�ƶ���Viewʵ�����ϣ���ʱ������
// ����View�Ļ���˳���ǰ���View���丸View�е��߼�˳������һ���Ƶģ����Ե����߼�˳��������View�ƶ�����UI
// ˳�������λ��ʱ�����View˲�Ƶ�Ŀ��λ�õ�Ч�����޷�����һ��˳�����ƶ�Ч����

// �������Ҫʵʱ���б����ƶ��ſ�Ч������ֻ��Ҫ������ͷ�ʱ�ı���˳������קʱ����һ����קͼ����
// hover���ϱ�ʾ������λ�ü��ɣ�

// �б�View����GridView��ͬ��ListView���������б���View������Ҫ������Model��֧���б����������
class ListView
    : public views::View
    , public views::BoundsAnimatorObserver
{
public:
    static const char kViewClassName[];

    explicit ListView(ListViewController *controller);
    virtual ~ListView();

    ScrollViewWithFloatingScrollbar* Container() { return scroll_view_; }

    ListViewController* delegate() const { return controller_; }
    std::vector<int> GetSelectedItems() const;
    std::vector<ListItemView*> GetItemViews(bool desc = false) const;

    void SetAllowMultipleSelect(bool allow) { allow_multiple_select_ = allow; }
    bool allow_multiple_select() const { return allow_multiple_select_; }
    bool IsItemDragging() const { return is_item_drag_; }

    void AddItemView(ListItemView *itemview, bool relayout = true);
    void AddItemViewAt(ListItemView* view, int index, bool relayout = true);
    void SetItemViewIndex(ListItemView* view, int new_index, 
                          ListItemView::ListItemViewChangeReason reason = ListItemView::CHANGED_BY_API);

    void SetEnabledItemDragMove(bool enabled);
    bool enabled_item_drag_move() const { return allow_item_drag_move_; }

    void SetItemBackgroundColor(views::Button::ButtonState state, SkColor color, bool repaint = false);
    SkColor item_background_color(views::Button::ButtonState state) const;

    void SetItemSize(const gfx::Size& size, bool relayout = false);
    gfx::Size item_size() const { return item_size_; }

    // CListCtrl Begin ------------------------------------------
    int GetFirstSelectedItemPosition() const;

    // Finds an item in the control matching the specified criteria.
    int FindItem(_In_ const LVFINDITEMINFO* pFindInfo, _In_ int nStart = -1) const;

    // Finds the next selected item, after a previous call
    // to GetFirstSelectedItemPosition().
    int GetNextSelectedItem(_Inout_ int& pos) const;

    // Retrieves the number of selected items in the control.
    UINT GetSelectedCount() const;

    // Retrieves the checked state of a particular item. Only useful
    // on controls with the LVS_EX_CHECKBOXES style.
    bool GetCheck(_In_ int nItem) const;

    // Sets the checked state of a particular item. Only useful
    // on controls with the LVS_EX_CHECKBOXES style.
    void SetCheck(_In_ int nItem, _In_ bool fCheck = true, bool only_one_checkd = true,
                  ListItemView::ListItemViewChangeReason reason = ListItemView::CHANGED_BY_API);

    bool GetItemRect(_In_ int nItem, _Out_ gfx::Rect &lpRect) const;

    // Retrieves the number of items in the control.
    int GetItemCount() const;

    // Retrieves a description of a particular item in the control.
    ListItemView* GetItem(int nItem);

    // Removes a single item from the control.
    ListItemView* DeleteItem(_In_ int nItem);

    // Removes all items from the control.
    void DeleteAllItems(bool delete_children);
    // CListCtrl End ---------------------------------------------

    // View
    void Layout() override;

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void PaintChildren(gfx::Canvas* canvas) override;
    const char* GetClassName() const override { return kViewClassName; };
    bool OnKeyPressed(const ui::KeyEvent& event) override;
    // Adds |view| as a child of this view, optionally at |index|.
    void AddChildView(View* view);
    void AddChildViewAt(View* view, int index);

    // BoundsAnimatorObserver
    void OnBoundsAnimatorProgressed(views::BoundsAnimator* animator) override {}
    void OnBoundsAnimatorDone(views::BoundsAnimator* animator) override;

private:
    static bool IsListItemView(const views::View* view);
    void SetCheck(ListItemView *item_view, _In_ bool fCheck = true, 
                  bool is_control_down = false, 
                  ListItemView::ListItemViewChangeReason reason = ListItemView::CHANGED_BY_USER);
    gfx::Rect GetItemRect(int nItem) const;
    void ClickOnNonItemArea();

    // item view
    bool OnItemViewPressed(const ListItemView* item_view, const ui::MouseEvent& event);
    bool OnItemViewDragged(const ListItemView* item_view, const ui::MouseEvent& event);
    void OnItemViewReleased(const ListItemView* item_view, const ui::MouseEvent& event);

private:
    friend class ListItemView;
    friend class ScrollViewForListView;
    ListViewController *controller_;
    ScrollViewWithFloatingScrollbar *scroll_view_;
    bool allow_multiple_select_;
    bool allow_item_drag_move_;
    scoped_ptr<views::BoundsAnimator> animation_;
    SkColor item_bk_clrs_[views::Button::ButtonState::STATE_COUNT];
    gfx::Size item_size_;

    bool cancel_drag_item_;
    bool is_item_drag_;
    ListItemView* drag_item_view_;
    int initial_drag_item_index_;// ����ק�����������ֵ
    int current_drag_item_index_;// ����ק�ǰ������ֵ
    gfx::Point last_item_drag_point_;// ����ק�ƶ�ʱ����������꣬��ListViewΪ�������ϵ

    DISALLOW_COPY_AND_ASSIGN(ListView);
};

#endif