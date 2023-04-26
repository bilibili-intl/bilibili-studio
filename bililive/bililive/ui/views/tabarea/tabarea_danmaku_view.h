#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_VIEW_H_

#include "bililive/bililive/ui/views/tabarea/bottom_bar/bottom_area_view.h"
#include "bililive/bililive/livehime/tabarea/tab_pane_danmaku_contract.h"
#include "bililive/bililive/ui/views/controls/gridview.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "ui/views/controls/data_table/data_table_view.h"
#include "ui/views/controls/data_table/data_table_view_source.h"


class TabAreaDanmakuView:
    public views::View,
    public GridModel,
    public contracts::TabPaneDanmakuView,
    public base::NotificationObserver,
    public BottomAreaDelegate,
    public views::ButtonListener {
public:
    TabAreaDanmakuView();

    ~TabAreaDanmakuView();

    void LoadDanmaku(int group, int item, contracts::UpdateDataType type);

    void RefreshDanmakus();

    void LocateDanmaku(int id);

    int GetBottomBarHeight() const;

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // GridModel
    int GetNumberOfGroups() override;
    int GetGroupHeaderHeight(int group_index) override;
    int GetItemCountForGroup(int group_index) override;
    gfx::Size GetItemSizeForGroup(const gfx::Rect &show_bounds, int group_index) override;
    int GetGroupHeaderTypeId(int group_index) override;
    int GetGroupItemTypeId(int group_index, int item_index) override;
    void UpdateData(GridItemView *item) override;

    //presenter
    void OnInitDanmaku(
        std::vector<contracts::BarrageDetails> list) override;

    void OnLoadDanmaku(
        std::vector<contracts::BarrageDetails> list,
        int count,
        contracts::UpdateDataType type) override;

    void OnLoadStoped() override;

    void OnSearchDanmaku(std::vector<contracts::BarrageDetails> list) override;

    void OnLocateDanmaku(std::vector<contracts::BarrageDetails> list) override;

    // BottomAreaDelegate
    void OnRefreshButtonPressed() override;

    void OnReportDanmaku(bool success) override;

    void OnRoomAdminAdd(bool valid, int code, const std::string& error_msg) override;

    void InitDanmakuOnUI(
        std::vector<contracts::BarrageDetails> list);

    void UpdateDanmakuOnUI(
        std::vector<contracts::BarrageDetails> list, int count, contracts::UpdateDataType type);

    void OnSearchDanmakuOnUI(std::vector<contracts::BarrageDetails> list);

    void OnLocateDanmakuOnUI(std::vector<contracts::BarrageDetails> list);

    void InitViews();

    // NotificationObserver
    void Observe(
        int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

private:
    void InitDanmaku();

    void LivehimeHistoryEventTracking();

    void UpdateDanmakuView(int count, bool search = true);

private:
    int item_index_;  //首个可见cell的index
    bool update_;
    bool is_loading_;
    bool is_searching_;
    bool is_history_danmaku_;
    LivehimeContentLabel* label_;
    LivehimeLinkButton*   link_button_;
    GridView *table_;
    BottomAreaView *bottom_area_view_;
    views::View* search_view_;
    std::vector<contracts::BarrageDetails> cell_list_;
    std::vector<contracts::BarrageDetails> history_list_;
    std::unique_ptr<contracts::TabPaneDanmakuPresenter> danmaku_presenter_;
    base::NotificationRegistrar notifation_registrar_;
    DISALLOW_COPY_AND_ASSIGN(TabAreaDanmakuView);
};


class DanmakuContextMenu;

class DanmakuGridView : public GridView {
public:
    DanmakuGridView(GridModel* model, TabAreaDanmakuView* danmaku_view) :
        GridView(model),
        danmaku_view_(danmaku_view){}

    ~DanmakuGridView() {}

    void OnVisibleBoundsNearContentsBoundsTop(
        bool horiz, GridModel::ScrollDirection dir, ScrollReason reason) override;

    void OnVisibleBoundsNearContentsBoundsBottom(
        bool horiz, GridModel::ScrollDirection dir, ScrollReason reason) override;

    void LocateDanmaku(int id);

private:
    TabAreaDanmakuView* danmaku_view_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TAB_PANE_DANMAKU_VIEW_H_