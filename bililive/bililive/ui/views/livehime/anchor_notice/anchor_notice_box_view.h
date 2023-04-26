#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_ANCHOR_NOTICE_BOX_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_ANCHOR_NOTICE_BOX_VIEW_H_

#include "bililive/bililive/ui/views/controls/gridview.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/secret/public/live_streaming_service.h"

class LivehimeListStateBannerView;

class AnchorNoticeBoxView
    : BililiveWidgetDelegate
    , GridModel
{
    struct NoticeInfo
    {
        NoticeInfo() = default;

        gfx::Size item_size;
    };

public:
    static void ShowWindow(views::Widget *parent, int last_unread_count);
    static bool IsShowing();

protected:
    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget *widget) override;
    void WindowClosing() override;
    void OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view) override;

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // GridModel
    int GetNumberOfGroups() override;
    int GetGroupHeaderHeight(int group_index) override;
    int GetItemCountForGroup(int group_index) override;
    gfx::Size GetItemSizeForGroup(const gfx::Rect &show_bounds, int group_index) override;
    int GetGroupHeaderTypeId(int group_index) override; // 返回group_index对应的组header的typeid
    int GetGroupItemTypeId(int group_index, int item_index) override; // 返回group_index&item_index对应的组中项的typeid
    void UpdateData(GridItemView *item) override;// 当item被重新取出时会触发此回调对item进行数据刷新
    // 允许组内项不等高/宽
    bool EnableGroupItemDifferentSize(int group_index) override;
    gfx::Size GetItemSizeForEnableGroupItemDifferentSize(
        const gfx::Rect &show_bounds, int group_index, int item_index) override;
    void OnVisibleBoundsNearContentsBoundsBottom(bool horiz, ScrollDirection dir, ScrollReason reason) override;

private:
    explicit AnchorNoticeBoxView(int last_unread_count);
    ~AnchorNoticeBoxView();

    void InitViews();
    void UninitViews();

private:
    LivehimeListStateBannerView* status_view_ = nullptr;
    GridView* gridview_ = nullptr;
    std::vector<NoticeInfo> notices_;

    int64 last_min_cursor_ = 0;

    base::WeakPtrFactory<AnchorNoticeBoxView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(AnchorNoticeBoxView);
};

#endif