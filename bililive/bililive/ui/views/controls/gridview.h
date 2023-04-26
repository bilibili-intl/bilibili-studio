#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_GRIDVIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_GRIDVIEW_H

#include "bililive_floating_scroll_view.h"

#include "base/memory/weak_ptr.h"

#include "ui/views/view.h"
#include "ui/gfx/text_constants.h"

class GridView;

// 列表项基类，即组项和数据项的基类
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
    int type_id_;       // 列表项类型ID
    bool is_header_;
    int group_index_;   // 列表项当前从属的组索引
    int item_index_;    // 列表项在当前组中的索引

    DISALLOW_COPY_AND_ASSIGN(GridItemView);
};

// GridView数据源
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
    virtual int GetGroupHeaderTypeId(int group_index) = 0; // 返回group_index对应的组header的typeid
    virtual int GetGroupItemTypeId(int group_index, int item_index) = 0; // 返回group_index&item_index对应的组中项的typeid
    virtual void UpdateData(GridItemView *item) = 0;// 当item被重新取出时会触发此回调对item进行数据刷新
    virtual bool ShowGroupHeaderWhenDataEmpty(int group_index) { return false; }// 当列表项数为0的时候是否显示组头

    // 允许组内项不等高/宽
    virtual bool EnableGroupItemDifferentSize(int group_index) { return false; }
    virtual gfx::Size GetItemSizeForEnableGroupItemDifferentSize(
        const gfx::Rect &show_bounds, int group_index, int item_index) { return gfx::Size(); }

    // 滚动到接近顶或底部时进行通知
    virtual void OnVisibleBoundsNearContentsBoundsTop(bool horiz, ScrollDirection dir, ScrollReason reason) {}
    virtual void OnVisibleBoundsNearContentsBoundsBottom(bool horiz, ScrollDirection dir, ScrollReason reason) {}
};

// 无限项列表View
// 根据实际的业务需求，当前GridView只支持垂直滚动，暂不支持水平滚动，
// 只支持等宽的列表项，但支持各项不等高，不等高的项布局在同一行中时根据v_align_决定对齐方式
class GridView
    : public views::View
{
    // 可见项的信息
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
        int group_index_;   // 列表项当前从属的组索引
        int item_index_;    // 列表项在当前组中的索引
        gfx::Rect bounds_;

        GridItemView *view_;
    };

public:
    static const char kViewClassName[];

    enum UpdateDataScrollType
    {
        NOSCROLL,   // 并不是指视口区坐标保持不变，而是指看到的项保持不变，即第一个可见的项保持不变（同一group_index_、item_index_）
        TOTOP,
        TOBOTTOM,
    };

    typedef base::Callback<GridItemView*(void)> ItemConstructor;
    typedef std::deque<GridItemView*> ItemDeque;

    explicit GridView(GridModel* model);
    virtual ~GridView();

    // 注册列表项view构建函数
    void RegisterItemType(int type_id, ItemConstructor item_constructor);

    // Returns a new ScrollView that contains the receiver.
    //is_narrow是否是窄的滚动条
    ScrollViewWithFloatingScrollbar* CreateParentIfNecessary(bool is_bar_narrow = false);

    // 锁定item的更新
    void LockWindowUpdate(bool lock);
    bool IsLockWindowUpdate() const;
    // 设置GridView的滚动方向，仅h和v其中之一，不能两个方向都可滚
    //void SetScrollDirection(bool horiz);

    // 数据源数据增删之后应进行一遍数据刷新和重新布局
    void UpdateData(UpdateDataScrollType scrolltype = NOSCROLL);
    // 数据源更新了某一数据项的数据后应针对这一项进行刷新
    // 会从当前正在使用的itemviews中进行遍历匹配，找到后调用其UpdataData进行数据刷新
    void UpdateGroupHeaderData(int group_index);
    void UpdateGroupItemData(int group_index, int item_index);
    // 上层可调用这两个getbounds得到某项的坐标然后调用ScrollView::ScrollToPosition滚到指定地方
    int GetGroupHeaderPosition(int group_index);
    int GetGroupItemPosition(int group_index, int item_index);
    gfx::Rect GetGroupBounds(int group_index);
    std::vector<gfx::Rect> GetAllGroupBounds();
    void SetDetectThickness(int thickness);
    // 获取第一条可见项组/项索引
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

    // 滚动到接近顶或底部时进行通知
    virtual void OnVisibleBoundsNearContentsBoundsTop(bool horiz, GridModel::ScrollDirection dir, ScrollReason reason);
    virtual void OnVisibleBoundsNearContentsBoundsBottom(bool horiz, GridModel::ScrollDirection dir, ScrollReason reason);

private:
    void CheckItemVisibleBounds(GridItemView* item, bool check_visible = true);
    std::vector<ItemInfo> GridView::GetVisibleItems(const gfx::Rect &visible_bounds);
    // 从重用队列中取出一个view，然后为其填充数据，之后再布局到可视区
    GridItemView* GetItemView(int type_id);

    // 获取一个组的全部细节，返回值为该组的整体高度；
    // 形参返回：头部高度、组内总共有多少项、总共分几行、一行多少项、每行的行高、
    // 各个项的POSITION（相对于组头左上角位置）
    int GetGroupDetails(const gfx::Rect &show_bounds, int group_index,
        OUT int* out_header_height = nullptr,                     // 头部高度
        OUT int* out_item_count = nullptr,                        // 组内总共有多少项
        OUT int* out_item_count_per_line = nullptr,               // 一行多少项
        OUT int* out_line_count = nullptr,                        // 总共分几行
        OUT std::vector<gfx::Rect>* out_line_rect_vct = nullptr,  // 每行的bounds（行高为行内项高的最大值）
        OUT std::vector<gfx::Rect>* out_item_rect_vct = nullptr   // 各个项的bounds（相对于组头左上角位置）
    );

private:
    friend class GridItemView;
    GridModel* model_;
    bool horiz_scroll_;
    gfx::HorizontalAlignment group_h_align_ = gfx::ALIGN_CENTER;// 组的水平对齐方式，组内每行的列表项是紧挨的
    gfx::VerticalAlignment v_align_ = gfx::ALIGN_TOP;// 行内列表项的垂直对齐方式
    ScrollViewWithFloatingScrollbar *scroll_view_;
    bool lock_window_update_;
    int detect_border_thickness_;
    gfx::Rect last_visible_bounds_;

    std::map<int, ItemConstructor> item_constructor_map_;   // 列表项构建函数map，类型id-类型创建函数
    std::map<int, ItemDeque> item_reuse_map_;               // 重用列表项map，类型id-类型实例
    std::vector<ItemInfo> visible_items_;                   // 可见项的信息

    base::WeakPtrFactory<GridView> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(GridView);
};

#endif