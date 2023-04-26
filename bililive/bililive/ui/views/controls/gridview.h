#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_GRIDVIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_GRIDVIEW_H

#include "bililive_floating_scroll_view.h"

#include "base/memory/weak_ptr.h"

#include "ui/views/view.h"
#include "ui/gfx/text_constants.h"

class GridView;

// �б�����࣬�������������Ļ���
class GridItemView : public views::View
{
public:
    GridItemView(int type_id, GridView *grid_view);
    virtual ~GridItemView() = default;

    int type_id() const{ return type_id_; }
    int group_index() const{ return group_index_; }
    int item_index() const{ return item_index_; }

protected:
    // call by GridView
    virtual void BeforePushBackToItemDeque(){}

private:
    void SetIndexInfo(bool is_header, int group_index, int item_index = -1);

private:
    friend class GridView;
    GridView *grid_view_;
    int type_id_;       // �б�������ID
    bool is_header_;
    int group_index_;   // �б��ǰ������������
    int item_index_;    // �б����ڵ�ǰ���е�����

    DISALLOW_COPY_AND_ASSIGN(GridItemView);
};

// GridView����Դ
class GridModel
{
public:
    enum ScrollDirection
    {
        NoMove,
        Forward,
        Backwards,
    };

    virtual int GetNumberOfGroups() = 0;
    virtual int GetGroupHeaderHeight(int group_index) = 0;
    virtual int GetItemCountForGroup(int group_index) = 0;
    virtual gfx::Size GetItemSizeForGroup(const gfx::Rect& show_bounds, int group_index) = 0;
    virtual int GetGroupHeaderTypeId(int group_index) = 0; // ����group_index��Ӧ����header��typeid
    virtual int GetGroupItemTypeId(int group_index, int item_index) = 0; // ����group_index&item_index��Ӧ���������typeid
    virtual void UpdateData(GridItemView *item) = 0;// ��item������ȡ��ʱ�ᴥ���˻ص���item��������ˢ��
    virtual bool ShowGroupHeaderWhenDataEmpty(int group_index) { return false; }// ���б�����Ϊ0��ʱ���Ƿ���ʾ��ͷ

    // ����������ȸ�/��
    virtual bool EnableGroupItemDifferentSize(int group_index) { return false; }
    virtual gfx::Size GetItemSizeForEnableGroupItemDifferentSize(
        const gfx::Rect &show_bounds, int group_index, int item_index) { return gfx::Size(); }

    // �������ӽ�����ײ�ʱ����֪ͨ
    virtual void OnVisibleBoundsNearContentsBoundsTop(bool horiz, ScrollDirection dir, ScrollReason reason) {}
    virtual void OnVisibleBoundsNearContentsBoundsBottom(bool horiz, ScrollDirection dir, ScrollReason reason) {}
};

// �������б�View
// ����ʵ�ʵ�ҵ�����󣬵�ǰGridViewֻ֧�ִ�ֱ�������ݲ�֧��ˮƽ������
// ֻ֧�ֵȿ���б����֧�ָ���ȸߣ����ȸߵ������ͬһ����ʱ����v_align_�������뷽ʽ
class GridView
    : public views::View
{
    // �ɼ������Ϣ
    struct ItemInfo
    {
        ItemInfo(bool is_header, int group, int item = -1)
            : is_header_(is_header)
            , group_index_(group)
            , item_index_(item)
            , view_(nullptr)
        {
        }

        ItemInfo()
            : is_header_(false)
            , group_index_(-1)
            , item_index_(-1)
            , view_(nullptr)
        {
        }

        bool operator==(const ItemInfo& _Right) const
        {
            return (this->is_header_ == _Right.is_header_ &&
                this->group_index_ == _Right.group_index_ &&
                this->item_index_ == _Right.item_index_/* &&
                this->bounds_ == _Right.bounds_*/);
        }

        bool is_header_;
        int group_index_;   // �б��ǰ������������
        int item_index_;    // �б����ڵ�ǰ���е�����
        gfx::Rect bounds_;

        GridItemView *view_;
    };

public:
    static const char kViewClassName[];

    enum UpdateDataScrollType
    {
        NOSCROLL,   // ������ָ�ӿ������걣�ֲ��䣬����ָ��������ֲ��䣬����һ���ɼ�����ֲ��䣨ͬһgroup_index_��item_index_��
        TOTOP,
        TOBOTTOM,
    };

    typedef base::Callback<GridItemView*(void)> ItemConstructor;
    typedef std::deque<GridItemView*> ItemDeque;

    explicit GridView(GridModel* model);
    virtual ~GridView();

    // ע���б���view��������
    void RegisterItemType(int type_id, ItemConstructor item_constructor);

    // Returns a new ScrollView that contains the receiver.
    //is_narrow�Ƿ���խ�Ĺ�����
    ScrollViewWithFloatingScrollbar* CreateParentIfNecessary(bool is_bar_narrow = false);

    // ����item�ĸ���
    void LockWindowUpdate(bool lock);
    bool IsLockWindowUpdate() const;
    // ����GridView�Ĺ������򣬽�h��v����֮һ�������������򶼿ɹ�
    //void SetScrollDirection(bool horiz);

    // ����Դ������ɾ֮��Ӧ����һ������ˢ�º����²���
    void UpdateData(UpdateDataScrollType scrolltype = NOSCROLL);
    // ����Դ������ĳһ����������ݺ�Ӧ�����һ�����ˢ��
    // ��ӵ�ǰ����ʹ�õ�itemviews�н��б���ƥ�䣬�ҵ��������UpdataData��������ˢ��
    void UpdateGroupHeaderData(int group_index);
    void UpdateGroupItemData(int group_index, int item_index);
    // �ϲ�ɵ���������getbounds�õ�ĳ�������Ȼ�����ScrollView::ScrollToPosition����ָ���ط�
    int GetGroupHeaderPosition(int group_index);
    int GetGroupItemPosition(int group_index, int item_index);
    gfx::Rect GetGroupBounds(int group_index);
    std::vector<gfx::Rect> GetAllGroupBounds();
    void SetDetectThickness(int thickness);
    // ��ȡ��һ���ɼ�����/������
    bool GetFirstVisibleItemIndex(int *group_index, int *item_index);
    bool GetLastVisibleItemIndex(int *group_index, int *item_index);

    // Assigns a new model to the table view, detaching the old one if present.
    // If |model| is NULL, the table view cannot be used after this call. This
    // should be called in the containing view's destructor to avoid destruction
    // issues when the model needs to be deleted before the table.
    void SetModel(GridModel* model);
    GridModel* model() const { return model_; }

    base::WeakPtr<GridView> GetWeakPtr();

    GridItemView* FindVisibleItemView(bool is_header, int group_index, int item_index);
protected:
    // View
    virtual bool NeedsNotificationWhenVisibleBoundsChange() const OVERRIDE{ return true; }
    virtual void OnVisibleBoundsChanged() OVERRIDE;
    virtual void Layout() OVERRIDE;
    virtual void VisibilityChanged(View* starting_from, bool is_visible) OVERRIDE;
    virtual const char* GetClassName() const OVERRIDE{ return kViewClassName; };

    // �������ӽ�����ײ�ʱ����֪ͨ
    virtual void OnVisibleBoundsNearContentsBoundsTop(bool horiz, GridModel::ScrollDirection dir, ScrollReason reason);
    virtual void OnVisibleBoundsNearContentsBoundsBottom(bool horiz, GridModel::ScrollDirection dir, ScrollReason reason);

private:
    void CheckItemVisibleBounds(GridItemView* item, bool check_visible = true);
    std::vector<ItemInfo> GridView::GetVisibleItems(const gfx::Rect &visible_bounds);
    // �����ö�����ȡ��һ��view��Ȼ��Ϊ��������ݣ�֮���ٲ��ֵ�������
    GridItemView* GetItemView(int type_id);

    // ��ȡһ�����ȫ��ϸ�ڣ�����ֵΪ���������߶ȣ�
    // �βη��أ�ͷ���߶ȡ������ܹ��ж�����ܹ��ּ��С�һ�ж����ÿ�е��иߡ�
    // �������POSITION���������ͷ���Ͻ�λ�ã�
    int GetGroupDetails(const gfx::Rect &show_bounds, int group_index,
        OUT int* out_header_height = nullptr,                     // ͷ���߶�
        OUT int* out_item_count = nullptr,                        // �����ܹ��ж�����
        OUT int* out_item_count_per_line = nullptr,               // һ�ж�����
        OUT int* out_line_count = nullptr,                        // �ܹ��ּ���
        OUT std::vector<gfx::Rect>* out_line_rect_vct = nullptr,  // ÿ�е�bounds���и�Ϊ������ߵ����ֵ��
        OUT std::vector<gfx::Rect>* out_item_rect_vct = nullptr   // �������bounds���������ͷ���Ͻ�λ�ã�
    );

private:
    friend class GridItemView;
    GridModel* model_;
    bool horiz_scroll_;
    gfx::HorizontalAlignment group_h_align_ = gfx::ALIGN_CENTER;// ���ˮƽ���뷽ʽ������ÿ�е��б����ǽ�����
    gfx::VerticalAlignment v_align_ = gfx::ALIGN_TOP;// �����б���Ĵ�ֱ���뷽ʽ
    ScrollViewWithFloatingScrollbar *scroll_view_;
    bool lock_window_update_;
    int detect_border_thickness_;
    gfx::Rect last_visible_bounds_;

    std::map<int, ItemConstructor> item_constructor_map_;   // �б��������map������id-���ʹ�������
    std::map<int, ItemDeque> item_reuse_map_;               // �����б���map������id-����ʵ��
    std::vector<ItemInfo> visible_items_;                   // �ɼ������Ϣ

    base::WeakPtrFactory<GridView> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(GridView);
};

#endif