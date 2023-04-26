#include "bililive/bililive/ui/views/tabarea/tabarea_danmaku_view.h"

#include "base/notification/notification_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/tabarea/tab_pane_danmaku_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/danmaku_search_notification_details.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_toast_view.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"

#include "grit/generated_resources.h"


namespace
{
    enum
    {
        GridInvalidTypeId,
        GridGroupItemTypeId,
    };

    class GridGroupItemView : public GridItemView {
    public:
        GridGroupItemView(GridView* grid_view)
            : GridItemView(GridGroupItemTypeId, grid_view),
            grid_view_(grid_view),
            mouse_enter_(false),
            label_title_(nullptr),
            label_time_(nullptr),
            label_content_(nullptr),
            id_(0){

            set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));

            auto layout_ = new views::GridLayout(this);
            SetLayoutManager(layout_);

            views::ColumnSet *col_set = layout_->AddColumnSet(0);
            col_set->AddPaddingColumn(0, 5);
            col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
            col_set->AddPaddingColumn(0, 10);
            col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
            col_set->AddPaddingColumn(0, 5);

            col_set = layout_->AddColumnSet(1);
            col_set->AddPaddingColumn(0, 5);
            col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::LEADING, 1.0, views::GridLayout::FIXED, 0, 0);
            col_set->AddPaddingColumn(0, 5);

            label_title_ = new LivehimeContentLabel(L"");
            label_title_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label_title_->SetFont(ftTwelve);

            label_time_ = new LivehimeContentLabel(L"");
            label_time_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
            label_time_->SetFont(ftEleven);

            label_content_ = new LivehimeContentLabel(L"");
            label_content_->SetFont(ftTwelve);
            label_content_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

            layout_->AddPaddingRow(0, 5);
            layout_->StartRow(0, 0);
            layout_->AddView(label_title_);
            layout_->AddView(label_time_);

            layout_->AddPaddingRow(0, 5);
            layout_->StartRow(1.0, 1);
            layout_->AddView(label_content_);

            layout_->AddPaddingRow(0, 5);
        }

        static gfx::Size GetItemSize()
        {
            return gfx::Size(kMainWndTabAreaWidth, ftTwelve.GetHeight() * 3);
        }

        void OnMouseEntered(const ui::MouseEvent& event) override {
            mouse_enter_ = true;
            SchedulePaint();
        }

        void OnMouseExited(const ui::MouseEvent& event) override {
            mouse_enter_ = false;
            SchedulePaint();
        }

        bool OnMousePressed(const ui::MouseEvent& event) override {
            __super::OnMousePressed(event);

            if (event.IsLeftMouseButton() && event.GetClickCount() == 2) {
                static_cast<DanmakuGridView*>(grid_view_)->
                    LocateDanmaku(id_);
            }

            return true;
        }

        void OnMouseReleased(const ui::MouseEvent& event) override {
            if (event.IsRightMouseButton()) {
                auto point = event.location();
                ConvertPointToScreen(this, &point);
            }

            __super::OnMouseReleased(event);
        }

        void OnPaintBackground(gfx::Canvas* canvas) override {
            __super::OnPaintBackground(canvas);

            if (mouse_enter_) {
                SkPaint paint;
                paint.setAntiAlias(true);
                paint.setColor(GetColor(ListItemBkHov));
                canvas->DrawRoundRect(GetLocalBounds(), 4, paint);
            }
        }

        static GridItemView* CreateCell(GridView* grid_view) {
            GridGroupItemView* cell = new GridGroupItemView(grid_view);

            return cell;
        }

        void SetText(SkColor crTitle,
            const base::string16& title,
            SkColor crSTime,
            const base::string16& stime,
            SkColor crContent,
            const base::string16& content) {
            label_title_->SetText(title);
            label_title_->SetTextColor(crTitle);

            label_time_->SetText(stime);
            label_time_->SetTextColor(crSTime);

            label_content_->SetText(content);
            label_content_->SetTextColor(crContent);

            InvalidateLayout();
            Layout();
        }

    private:
        // GridItemView
        void BeforePushBackToItemDeque() override { mouse_enter_ = false; }

        gfx::Size GetPreferredSize() override {
            return gfx::Size(kMainWndTabAreaWidth, ftPrimary.GetHeight() * 4);
        }

    private:
        GridView* grid_view_;
        LivehimeContentLabel* label_title_;
        LivehimeContentLabel* label_time_;
        LivehimeContentLabel* label_content_;

        int id_;

        bool mouse_enter_;
    };
}


TabAreaDanmakuView::TabAreaDanmakuView()
    : table_(nullptr)
    , item_index_(0)
    , is_loading_(false)
    , is_searching_(false)
    , is_history_danmaku_(true)
    , bottom_area_view_(nullptr)
    , search_view_(nullptr)
    , label_(nullptr)
    , link_button_(nullptr)
    , danmaku_presenter_(std::make_unique<TabPaneDanmakuPresenterImpl>(this)){}

TabAreaDanmakuView::~TabAreaDanmakuView() {}

void TabAreaDanmakuView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();

            notifation_registrar_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_TABAREA_BLACKLIST_APPENDING,
                base::NotificationService::AllSources());

            notifation_registrar_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_TABAREA_REPORT,
                base::NotificationService::AllSources());

            notifation_registrar_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH,
                base::NotificationService::AllSources());

            notifation_registrar_.Add(this,
                bililive::NOTIFICATION_LIVEHIME_ROOM_ADMIN_ADD,
                base::NotificationService::AllSources());
        } else {
            notifation_registrar_.RemoveAll();
        }
    }
}

int TabAreaDanmakuView::GetNumberOfGroups() {
    return 1;
}

int TabAreaDanmakuView::GetGroupHeaderHeight(int group_index) {
    return 0;
}

int TabAreaDanmakuView::GetItemCountForGroup(int section_index) {
    return cell_list_.size();
}

gfx::Size TabAreaDanmakuView::GetItemSizeForGroup(const gfx::Rect &show_bounds, int section_index) {
    return GridGroupItemView::GetItemSize();
}

int TabAreaDanmakuView::GetGroupHeaderTypeId(int group_index) {
    return GridInvalidTypeId;
}

int TabAreaDanmakuView::GetGroupItemTypeId(int group_index, int item_index) {
    return GridGroupItemTypeId;
}

void TabAreaDanmakuView::UpdateData(GridItemView *item) {
    if (item->type_id() == GridGroupItemTypeId)
    {
        int item_index = item->item_index();
        if (item_index >= 0 && item_index < static_cast <int>(cell_list_.size())) {
            ((GridGroupItemView*)item)->SetText(cell_list_[item_index].crTitleColor,
                cell_list_[item_index].title,
                cell_list_[item_index].crSTime,
                cell_list_[item_index].stime,
                cell_list_[item_index].crContentColor,
                cell_list_[item_index].content);
        }
    }
}

void TabAreaDanmakuView::InitViews() {

    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    {
        search_view_ = new BililiveHideAwareView();

        search_view_->set_background(
            views::Background::CreateSolidBackground(SkColorSetRGB(0xF2, 0xF2, 0xF2)));

        views::GridLayout* layout = new views::GridLayout(search_view_);
        search_view_->SetLayoutManager(layout);

        views::ColumnSet *columnset = layout->AddColumnSet(0);
        columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
        columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

        label_ = new LivehimeContentLabel(L"");
        label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

        link_button_ = new LivehimeLinkButton(this, L"返回");
        link_button_->SetTextColor(views::Button::STATE_NORMAL, clrLivehime);

        layout->AddPaddingRow(1.0f, 0);
        layout->StartRow(0, 0);
        layout->AddView(label_);
        layout->AddView(link_button_, 1, 1, views::GridLayout::FILL, views::GridLayout::FILL, 0, 30);
        layout->AddPaddingRow(1.0f, 0);
    }

    table_ = new DanmakuGridView(this, this);

    table_->RegisterItemType(GridGroupItemTypeId,
        base::Bind(&GridGroupItemView::CreateCell, base::Unretained(table_)));

    bottom_area_view_ = new BottomAreaView(this);

    auto layout_ = new views::GridLayout(this);
    SetLayoutManager(layout_);

    views::ColumnSet *col_set = layout_->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    search_view_->SetVisible(false);

    layout_->StartRow(0, 0);
    layout_->AddView(search_view_);
    layout_->StartRow(1.0, 0);
    layout_->AddView(table_->CreateParentIfNecessary());
    layout_->StartRow(0, 0);
    layout_->AddView(bottom_area_view_);
}

void TabAreaDanmakuView::OnInitDanmaku(
    std::vector<contracts::BarrageDetails> list) {
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&TabAreaDanmakuView::InitDanmakuOnUI,
        base::Unretained(this),
        list));
}

void TabAreaDanmakuView::InitDanmakuOnUI(
    std::vector <contracts::BarrageDetails> list) {
    is_loading_ = false;

    if (list.size() > 0) {
        cell_list_ = list;
        table_->UpdateData(GridView::UpdateDataScrollType::TOBOTTOM);

        history_list_ = list;
    }
}

void TabAreaDanmakuView::LoadDanmaku(int group, int item,
    contracts::UpdateDataType type) {
    item_index_ = item;

    if (!is_history_danmaku_) {
        return;
    }

    if (!is_loading_) {
        is_loading_ = true;
        danmaku_presenter_->LoadDanmaku(type);
    }
}

void TabAreaDanmakuView::OnLoadDanmaku(
    std::vector<contracts::BarrageDetails> list,
    int count,
    contracts::UpdateDataType type) {
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&TabAreaDanmakuView::UpdateDanmakuOnUI,
        base::Unretained(this),
        list, count, type));
}

void TabAreaDanmakuView::UpdateDanmakuOnUI(
    std::vector<contracts::BarrageDetails> list, int count, contracts::UpdateDataType type) {
    is_loading_ = false;

    if (list.size() > 0) {
        cell_list_ = list;
        table_->UpdateData();

        if (type == contracts::UpdateDataType::TOTOP) {
            auto pos = table_->GetGroupItemPosition(0, item_index_ + count);
            table_->CreateParentIfNecessary()->ScrollToPosition(true, pos);
        } else {
            auto pos = table_->GetGroupItemPosition(0, item_index_);
            table_->CreateParentIfNecessary()->ScrollToPosition(true, pos);
        }

        history_list_ = list;
    }
}

void TabAreaDanmakuView::OnLoadStoped() {
    is_loading_ = false;
}


void TabAreaDanmakuView::InitDanmaku() {
    is_loading_ = true;
    danmaku_presenter_->InitDanmaku();
}

void TabAreaDanmakuView::LivehimeHistoryEventTracking() {
    danmaku_presenter_->EventTracking();
}


void TabAreaDanmakuView::Observe(
    int type,
    const base::NotificationSource& source,
    const base::NotificationDetails& details) {

    if (type == bililive::NOTIFICATION_LIVEHIME_TABAREA_REPORT && details.map_key() != 0) {
        auto danmaku_info = reinterpret_cast<const SelectedDanmakuDetails*>(details.map_key());
        danmaku_presenter_->ReportDanmaku(*danmaku_info);
    }

    if (type == bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH
        && details.map_key() != 0) {

        if (is_loading_ || is_searching_) {
            return;
        }

        auto search_info = base::Details<DanmakuSearchDetails>(details);

        int64 time_range = 0;
        auto content = search_info->content;

        if (search_info->time_range == TimeRange::ALL) {
            time_range = 0;
        } else if (search_info->time_range == TimeRange::WEEK) {
            time_range = (base::Time::Now() - base::TimeDelta::FromDays(7)).ToTimeT();
        } else if (search_info->time_range == TimeRange::MONTH) {
            time_range = (base::Time::Now() - base::TimeDelta::FromDays(30)).ToTimeT();
        } else if (search_info->time_range == TimeRange::THREE_MONTH) {
            time_range = (base::Time::Now() - base::TimeDelta::FromDays(90)).ToTimeT();
        }

        int group_index = 0;
        int item_index = 0;
        table_->GetFirstVisibleItemIndex(&group_index, &item_index);
        item_index_ = item_index;

        danmaku_presenter_->SearchDanmaku(content, time_range);

        is_searching_ = true;

        UpdateDanmakuView(0);
    }

    if (type == bililive::NOTIFICATION_LIVEHIME_ROOM_ADMIN_ADD) {
        auto id = reinterpret_cast<int64_t*>(details.map_key());
        if (id) {
            danmaku_presenter_->RoomAdminAdd(*id);
        }
    }
}

void TabAreaDanmakuView::OnSearchDanmaku(std::vector<contracts::BarrageDetails> list) {
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&TabAreaDanmakuView::OnSearchDanmakuOnUI,
        base::Unretained(this), list));
}

void TabAreaDanmakuView::OnSearchDanmakuOnUI(std::vector<contracts::BarrageDetails> list) {
    if (!is_searching_) {
        return;
    }

    cell_list_ = list;
    table_->UpdateData(GridView::UpdateDataScrollType::TOTOP);

    is_searching_ = false;
    is_history_danmaku_ = false;

    UpdateDanmakuView(list.size(), false);
}

void TabAreaDanmakuView::OnLocateDanmaku(std::vector<contracts::BarrageDetails> list) {
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&TabAreaDanmakuView::OnLocateDanmakuOnUI,
        base::Unretained(this), list));
}

void TabAreaDanmakuView::OnLocateDanmakuOnUI(std::vector<contracts::BarrageDetails> list) {
    if (list.size() <= 0) {
        return;
    }

    cell_list_ = list;
    table_->UpdateData(GridView::UpdateDataScrollType::TOTOP);

    history_list_ = list;

    is_history_danmaku_ = true;
    search_view_->SetVisible(false);

    InvalidateLayout();
    Layout();
}

void TabAreaDanmakuView::LocateDanmaku(int id) {
    if (is_history_danmaku_ || id <= 0) {
        return;
    }

    danmaku_presenter_->LocateDanmaku(id);
}

int TabAreaDanmakuView::GetBottomBarHeight() const
{
    return bottom_area_view_->height();
}

void TabAreaDanmakuView::UpdateDanmakuView(int count, bool search) {
    string16 label_text;

    if (search) {
        label_text = L"正在搜索历史弹幕...";
    } else {
        if (count <= 0) {
            label_text = L"没有搜索到目标弹幕...";
        } else {
            label_text = base::StringPrintf(L"搜索到%d条目标弹幕", count);
        }
    }

    label_->SetText(label_text);
    search_view_->SetVisible(true);

    InvalidateLayout();
    Layout();
}


void TabAreaDanmakuView::OnRefreshButtonPressed() {
    if (!is_history_danmaku_ || is_searching_) {
        return;
    }

    RefreshDanmakus();
}

void TabAreaDanmakuView::RefreshDanmakus()
{
    InitDanmaku();
    LivehimeHistoryEventTracking();
}

void TabAreaDanmakuView::OnReportDanmaku(bool success) {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    if (success) {
        ShowTabAreaToast(TabAreaToastType_Default,
            rb.GetLocalizedString(IDS_DANMUKU_REPORT_TOAST_SUCCESS));
    } else {
        ShowTabAreaToast(TabAreaToastType_Warning,
            rb.GetLocalizedString(IDS_DANMUKU_REPORT_TOAST_FAIL));
    }
}

void TabAreaDanmakuView::OnRoomAdminAdd(bool valid, int code, const std::string& error_msg) {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    if (valid && code == 0) {
        ShowTabAreaToast(TabAreaToastType_Default,
            rb.GetLocalizedString(IDS_DANMUKU_ROOM_ADMIN_ADD_TOAST_SUCCESS));

        base::NotificationService::current()->Notify(
            bililive::NOTIFICATION_LIVEHIME_ROOM_ADMIN_APPENDED,
            base::NotificationService::AllSources(),
            base::NotificationService::NoDetails());
    } else if (!valid) {
        ShowTabAreaToast(TabAreaToastType_Warning,
            rb.GetLocalizedString(IDS_DANMUKU_ROOM_ADMIN_ADD_TOAST_FAIL));
    } else {
        ShowTabAreaToast(TabAreaToastType_Warning, base::UTF8ToUTF16(error_msg));
    }
}

void TabAreaDanmakuView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    if (sender == link_button_) {
        cell_list_ = history_list_;
        table_->UpdateData();

        auto pos = table_->GetGroupItemPosition(0, item_index_);
        table_->CreateParentIfNecessary()->ScrollToPosition(true, pos);

        is_searching_ = false;
        is_history_danmaku_ = true;
        search_view_->SetVisible(false);

        InvalidateLayout();
        Layout();
    }
}


// DanmakuGridView
void DanmakuGridView::OnVisibleBoundsNearContentsBoundsTop(
    bool horiz, GridModel::ScrollDirection dir, ScrollReason reason) {
    if (reason == ScrollReason::Wheel && dir == GridModel::ScrollDirection::Forward) {
        int group_index = 0;
        int item_index = 0;
        if (GetFirstVisibleItemIndex(&group_index, &item_index)) {
            danmaku_view_->LoadDanmaku(group_index, item_index,
                contracts::UpdateDataType::TOTOP);
        }
    }
}

void DanmakuGridView::OnVisibleBoundsNearContentsBoundsBottom(
    bool horiz, GridModel::ScrollDirection dir, ScrollReason reason) {
    if (reason == ScrollReason::Wheel && dir == GridModel::ScrollDirection::Backwards) {
        int group_index = 0;
        int item_index = 0;
        if (GetFirstVisibleItemIndex(&group_index, &item_index)) {
            danmaku_view_->LoadDanmaku(group_index, item_index,
                contracts::UpdateDataType::TOBOTTOM);
        }
    }
}

void DanmakuGridView::LocateDanmaku(int id) {
    danmaku_view_->LocateDanmaku(id);
}