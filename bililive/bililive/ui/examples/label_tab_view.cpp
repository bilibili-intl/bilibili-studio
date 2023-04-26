#include "label_tab_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace examples {

    void LabelTabView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
        __super::ViewHierarchyChanged(details);

        if (details.child == this) {
            if (details.is_add) {
                InitViews();
            }
        }
    }

    void LabelTabView::InitViews() {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();

        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);


        auto column = layout->AddColumnSet(0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
        BililiveLabel* label = new LivehimeTitleLabel(L"LivehimeTitleLabel");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeSmallTitleLabel(L"LivehimeSmallTitleLabel");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeContentLabel(L"LivehimeContentLabel");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeSmallContentLabel(L"LivehimeSmallContentLabel");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeTipLabel(L"LivehimeTipLabel");
        layout->AddView(label);


        column = layout->AddColumnSet(1);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForDiffGroups);
        label = new LivehimeTitleLabel(L"Innovation in China 中国智造， 慧及全球 0123456789");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeSmallTitleLabel(L"我能吞下玻璃而不伤身体。");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeContentLabel(L"色は匂へど 散りぬるを");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeSmallContentLabel(L"The quick brown fox jumps over the lazy dog.");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeTipLabel(L"ᚠᛇᚻ᛫ᛒᛦᚦ᛫ᚠᚱᚩᚠᚢᚱ᛫ᚠᛁᚱᚪ᛫ᚷᛖᚻᚹᛦᛚᚳᚢᛗ");
        layout->AddView(label);

        column = layout->AddColumnSet(2);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForDiffGroups);
        label = new LivehimeTitleLabel(L"床前明月光");
        layout->AddView(label);
        label = new LivehimeTitleLabel(L"疑是地上霜");
        layout->AddView(label);

        layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeTitleLabel(L"床前明月光");
        label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
        layout->AddView(label);
        label = new LivehimeTitleLabel(L"疑是地上霜");
        label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
        layout->AddView(label);

        layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeTitleLabel(L"床前明月光");
        layout->AddView(label);
        label = new LivehimeTitleLabel(L"疑是地上霜");
        layout->AddView(label);

        // DWrite Test
        column = layout->AddColumnSet(3);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::LEADING,
            0, views::GridLayout::FIXED, 50, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 3, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeTitleLabel(L"Innovation in China 中国智造， 慧及全球 0123456789");
        label->SetMultiLine(true);
        label->SetAllowCharacterBreak(true);
        label->SetElideBehavior(views::Label::ElideBehavior::NO_ELIDE);
        layout->AddView(label);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    }

}