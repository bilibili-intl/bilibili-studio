#include "anchor_notice_box_view.h"

#include <regex>
#include <shellapi.h>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/ext/callable_callback.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "net/base/escape.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_list_state_banner.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

using namespace bililive;

namespace
{
    const int kMinItemHeigth = GetLengthByDPIScale(100);

    base::string16 GetTimeGapString(const base::string16& time_at)
    {
        base::string16 date = time_at.substr(0, 10);

        base::Time::Exploded exploded = { 0 };
        int conver_count = _snwscanf(time_at.c_str(), time_at.length(), L"%d-%d-%d %d:%d:%d",
            &exploded.year, &exploded.month, &exploded.day_of_month, &exploded.hour, &exploded.minute, &exploded.second);
        if (conver_count == 6)
        {
            base::Time record_time = base::Time::FromLocalExploded(exploded);
            base::Time now_time = base::Time::Now();
            base::TimeDelta delta = now_time - record_time;
            bililive::TimeSpan span(delta);
            if (span.GetDays() > 6 * 30)
            {
                //date = date;
            }
            else if (span.GetDays() > 30 && span.GetDays() <= 6 * 30)
            {
                date = base::StringPrintf(L"%d个月前", span.GetDays() / 30);
            }
            else if (span.GetDays() >= 1 && span.GetDays() <= 30)
            {
                date = base::StringPrintf(L"%d天前", span.GetDays());
            }
            else if (span.GetTotalHours() >= 1 && span.GetDays() < 1)
            {
                date = base::StringPrintf(L"%d小时前", span.GetTotalHours());
            }
            else if (span.GetTotalHours() < 1)
            {
                date = L"刚刚";
            }
        }

        return date;
    }

    enum
    {
        NoticeItemTypeId = 1,
    };

    class NoticeItemView
        : public GridItemView
        , views::ButtonListener
    {
        struct HREF_DETAILS
        {
            std::wstring text;
            std::wstring href;
            std::wstring link_url;
        };

    public:
        static GridItemView* CreateCell(GridView* grid_view)
        {
            GridItemView* cell = new NoticeItemView(grid_view);
            return cell;
        }

        explicit NoticeItemView(GridView* grid_view)
            : GridItemView(NoticeItemTypeId, grid_view)
            , weak_ptr_factory_(this)
        {
            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

            column_set = layout->AddColumnSet(1);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

            title_label_ = new LivehimeTitleLabel();
            title_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            title_label_->SetMultiLine(true);

            time_label_ = new LivehimeTipLabel();

            layout->AddPaddingRow(0, kPaddingRowHeightForGroupTips);
            layout->StartRow(0, 0);
            layout->AddView(title_label_);
            layout->AddView(time_label_);

            content_view_ = new views::View();

            layout->AddPaddingRow(0, kPaddingRowHeightForGroupTips);
            layout->StartRow(0, 1);
            layout->AddView(content_view_);
            layout->AddPaddingRow(0, kPaddingRowHeightForGroupTips);

            set_border(views::Border::CreateSolidSidedBorder(0, 0, 1, 0, GetColor(CtrlBorderNor)));
        }

    protected:
        // view
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            int tag = sender->tag();
            if (tag >= 0 && tag < (int)href_details_.size())
            {
                if (!href_details_[tag].link_url.empty())
                {
                    ShellExecuteW(0, L"open", bililive::FastForwardChangeEnv(href_details_[tag].link_url).c_str(), 0, 0, SW_SHOW);
                }
            }
        }

    private:
        void RelayoutContentView()
        {
            content_view_->RemoveAllChildViews(true);

            views::GridLayout *grid_layout = new views::GridLayout(content_view_);
            content_view_->SetLayoutManager(grid_layout);

            views::ColumnSet *column_set = grid_layout->AddColumnSet(0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
            column_set = grid_layout->AddColumnSet(1);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

            for (size_t i= 0; i < href_details_.size(); ++i)
            {
                LivehimeContentLabel* text_label = new LivehimeContentLabel(href_details_[i].text);
                text_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
                text_label->SetMultiLine(true);

                grid_layout->StartRow(0, 0);
                grid_layout->AddView(text_label);

                if (!href_details_[i].href.empty())
                {
                    LivehimeLinkButton* link_button = new LivehimeLinkButton(this, href_details_[i].href);
                    link_button->SetTextColor(views::Button::STATE_NORMAL, clrLinkButtonPressed);
                    link_button->set_tag(i);

                    grid_layout->StartRow(0, 1);
                    grid_layout->AddView(link_button);
                }
            }
            content_view_->InvalidateLayout();
        }

    private:
        BililiveLabel *title_label_ = nullptr;
        BililiveLabel *time_label_ = nullptr;
        views::View* content_view_ = nullptr;

        std::vector<HREF_DETAILS> href_details_;

        base::WeakPtrFactory<NoticeItemView> weak_ptr_factory_;
    };

    AnchorNoticeBoxView* g_single_instance = nullptr;
}

void AnchorNoticeBoxView::ShowWindow(views::Widget *parent, int last_unread_count)
{
    if (!g_single_instance)
    {
        views::Widget *widget_ = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.parent = parent->GetNativeView();

        g_single_instance = new AnchorNoticeBoxView(last_unread_count);
        DoModalWidget(g_single_instance, widget_, params);
    }
    else
    {
        if (g_single_instance->GetWidget())
        {
            g_single_instance->GetWidget()->Activate();
        }
    }
}

bool AnchorNoticeBoxView::IsShowing()
{
    return !!g_single_instance;
}

AnchorNoticeBoxView::AnchorNoticeBoxView(int last_unread_count)
    : BililiveWidgetDelegate(gfx::ImageSkia(),
        0 == last_unread_count ? L"系统通知" : base::StringPrintf(L"系统通知（%d）", last_unread_count))
    , gridview_(nullptr)
    , weakptr_factory_(this)
{
}

AnchorNoticeBoxView::~AnchorNoticeBoxView()
{
    g_single_instance = nullptr;

}

views::NonClientFrameView* AnchorNoticeBoxView::CreateNonClientFrameView(views::Widget *widget)
{
    return __super::CreateNonClientFrameView(widget);
}

void AnchorNoticeBoxView::WindowClosing()
{
    UninitViews();
}

void AnchorNoticeBoxView::OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view)
{

}

void AnchorNoticeBoxView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
        }
        else
        {
            UninitViews();
        }
    }
}

void AnchorNoticeBoxView::InitViews()
{
    status_view_ = new LivehimeListStateBannerView;

    gridview_ = new GridView(this);
    gridview_->RegisterItemType(NoticeItemTypeId,
        base::Bind(&NoticeItemView::CreateCell, base::Unretained(gridview_)));
    gridview_->CreateParentIfNecessary()->SetVisible(false);

    AddChildView(status_view_);
    AddChildView(gridview_->CreateParentIfNecessary());
}

void AnchorNoticeBoxView::UninitViews()
{

}

void AnchorNoticeBoxView::OnPaintBackground(gfx::Canvas* canvas)
{
    __super::OnPaintBackground(canvas);
}

gfx::Size AnchorNoticeBoxView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    return gfx::Size(770, 390);
}

void AnchorNoticeBoxView::Layout()
{
    gfx::Rect rect = GetContentsBounds();
    status_view_->SetBoundsRect(rect);
    gridview_->CreateParentIfNecessary()->SetBoundsRect(rect);
}

int AnchorNoticeBoxView::GetNumberOfGroups()
{
    return 1;
}

int AnchorNoticeBoxView::GetGroupHeaderHeight(int group_index)
{
    return 0;
}

int AnchorNoticeBoxView::GetItemCountForGroup(int group_index)
{
    return notices_.size();
}

gfx::Size AnchorNoticeBoxView::GetItemSizeForGroup(const gfx::Rect &show_bounds, int group_index)
{
    return gfx::Size(show_bounds.width(), kMinItemHeigth);
}

int AnchorNoticeBoxView::GetGroupHeaderTypeId(int group_index)
{
    return -1;
}

int AnchorNoticeBoxView::GetGroupItemTypeId(int group_index, int item_index)
{
    return NoticeItemTypeId;
}

void AnchorNoticeBoxView::UpdateData(GridItemView *item)
{

}

bool AnchorNoticeBoxView::EnableGroupItemDifferentSize(int group_index)
{
    return true;
}

gfx::Size AnchorNoticeBoxView::GetItemSizeForEnableGroupItemDifferentSize(
    const gfx::Rect &show_bounds, int group_index, int item_index)
{
    if (item_index >= 0 && item_index < static_cast<int>(notices_.size()))
    {
        if (notices_[item_index].item_size.IsEmpty())
        {

        }
        return notices_[item_index].item_size;
    }
    return gfx::Size(show_bounds.width(), kMinItemHeigth);
}

void AnchorNoticeBoxView::OnVisibleBoundsNearContentsBoundsBottom(bool horiz, ScrollDirection dir, ScrollReason reason)
{
    if (!horiz && dir == GridModel::Backwards)
    {
        
    }
}