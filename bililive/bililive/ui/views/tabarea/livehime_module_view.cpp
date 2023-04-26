#include "bililive/bililive/ui/views/tabarea/livehime_module_view.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "ui/views/layout/grid_layout.h"


namespace {
    const int kLiveToolWidth = GetLengthByDPIScale(255);
    const int kLiveToolHigth = GetLengthByDPIScale(360);

    class ModuleAreaView :
        public BililiveViewWithFloatingScrollbar
    {
    public:
        ModuleAreaView()
        {
            InitViews();
        };
        ~ModuleAreaView() {};

        void InitViews()
        {
            set_background(views::Background::CreateSolidBackground(SkColorSetRGB(0xF9, 0xF9, 0xF9)));
            views::GridLayout* layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet* column_set = layout->AddColumnSet(0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
        }

    protected:
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = __super::GetPreferredSize();
            return size;
        }

    };
}


LivehimeModuleView::LivehimeModuleView()
    : weakptr_factory_(this)
{
}

LivehimeModuleView::~LivehimeModuleView()
{}

void LivehimeModuleView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
        }

    }
}

void LivehimeModuleView::InitViews()
{
    set_background(views::Background::CreateSolidBackground(SkColorSetRGB(0xF9, 0xF9, 0xF9)));
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingRowHeightForGroupCtrls);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingRowHeightForGroupCtrls);

    auto activity_label = new LivehimeTitleLabel(GetLocalizedString(IDS_TOOL_TITLE_NAME));
    layout->AddPaddingRow(0, GetLengthByDPIScale(10));
    layout->StartRow(0, 1);
    layout->AddView(activity_label);

    module_area_view_ = new ModuleAreaView();
    layout->StartRow(1.0f, 0);
    layout->AddView(module_area_view_->Container());
}

gfx::Size LivehimeModuleView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.SetSize(kLiveToolWidth, kLiveToolHigth);
    return size;
}

int LivehimeModuleView::GetMinimumHeight()
{
    static int min_cy = 0;
    //if (0 == min_cy)
    //{
    //    auto pref_size = livehime::GetEntranceViewSize(LivehimeModuleEntranceView::ViewType::MainView);
    //    min_cy = pref_size.height()*2;
    //    min_cy += GetLengthByDPIScale(40);
    //}
    return min_cy;
}