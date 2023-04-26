#include "button_tab_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/examples/examples_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace {
    const int kActionButtonId = 1000;
    const int kFunctionButtonId = 1001;
    const int kLinkButtonId = 1002;
    const int kSmallLinkButtonId = 1003;
    const int kCapsuleButtonId = 1004;
    const int kVerticalLabelButtonId = 1005;
    const int kCheckBoxId = 1006;
}

namespace examples {

    ButtonTabView::ButtonTabView()
        : using_dwrite_(gfx::Canvas::IsDWriteEnabled()),
          checkbox_(nullptr) {
    }

    void ButtonTabView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
        __super::ViewHierarchyChanged(details);

        if (details.child == this) {
            if (details.is_add) {
                InitViews();
            }
        }
    }

    void ButtonTabView::InitViews() {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();

        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        auto column = layout->AddColumnSet(0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        string16 dwrite_status;
        if (using_dwrite_) {
            dwrite_status = L"DWrite当前状态：已启用";
        } else {
            dwrite_status = L"DWrite当前状态：已禁用";
        }

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
        BililiveLabelButton* button
            = new LivehimeActionLabelButton(this, dwrite_status, true);
        button->set_id(kActionButtonId);
        layout->AddView(button);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        button = new LivehimeFunctionLabelButton(this, L"LivehimeFunctionLabelButton");
        button->set_id(kFunctionButtonId);
        layout->AddView(button);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        button = new LivehimeLinkButton(this, L"LivehimeLinkButton");
        button->set_id(kLinkButtonId);
        layout->AddView(button);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        button = new LivehimeSmallTitleLinkButton(this, L"LivehimeSmallTitleLinkButton");
        button->set_id(kSmallLinkButtonId);
        layout->AddView(button);


        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
        LivehimeCapsuleButton* capsule_button
            = new LivehimeCapsuleButton(this, L"LivehimeCapsuleButton");
        capsule_button->set_id(kCapsuleButtonId);
        layout->AddView(capsule_button);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        LivehimeVerticalLabelButton* vertical_button
            = new LivehimeVerticalLabelButton(this, L"LivehimeVerticalLabelButton");
        vertical_button->SetImage(
            views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_MELEE_SEASON_TITLE_BACKGROUND));
        vertical_button->SetImage(
            views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_MELEE_SEASON_TITLE_BACKGROUND));
        vertical_button->SetImage(
            views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_MELEE_SEASON_TITLE_BACKGROUND));
        vertical_button->set_id(kVerticalLabelButtonId);
        layout->AddView(vertical_button);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        checkbox_ = new LivehimeCheckbox(L"LivehimeCheckbox");
        checkbox_->set_id(kCheckBoxId);
        layout->AddView(checkbox_);


        column = layout->AddColumnSet(1);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::FILL,
            0, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::FILL,
            0, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForDiffGroups);
        BililiveRadioButton* radio_button = new LivehimeRadioButton(L"LivehimeRadioButton", 0);
        layout->AddView(radio_button);
        LivehimeHoverTipButton* tip_button = new LivehimeHoverTipButton(
            L"Title", L"Tips", views::BubbleBorder::Arrow::BOTTOM_RIGHT);
        layout->AddView(tip_button);

        column = layout->AddColumnSet(2);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForGroupCtrls);
        radio_button = new LivehimeColorRadioButton(0, SK_ColorRED);
        layout->AddView(radio_button);
        radio_button = new LivehimeColorRadioButton(0, SK_ColorYELLOW);
        layout->AddView(radio_button);
        radio_button = new LivehimeColorRadioButton(0, SK_ColorBLUE);
        layout->AddView(radio_button);
        radio_button = new LivehimeColorRadioButton(0, SK_ColorGREEN);
        layout->AddView(radio_button);

        layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForDiffGroups);
        LivehimeFilterRadioButton* filter_radio_button = new LivehimeFilterRadioButton(1);
        filter_radio_button->SetImageAndText(
            rb.GetImageSkiaNamed(IDR_LIVEHIME_CAMERA_FILTER_BING), L"01");
        layout->AddView(filter_radio_button);

        filter_radio_button = new LivehimeFilterRadioButton(1);
        filter_radio_button->SetImageAndText(
            rb.GetImageSkiaNamed(IDR_LIVEHIME_CAMERA_FILTER_FENG), L"02");
        layout->AddView(filter_radio_button);

        filter_radio_button = new LivehimeFilterRadioButton(1);
        filter_radio_button->SetImageAndText(
            rb.GetImageSkiaNamed(IDR_LIVEHIME_CAMERA_FILTER_LAN), L"03");
        layout->AddView(filter_radio_button);

        filter_radio_button = new LivehimeFilterRadioButton(1);
        filter_radio_button->SetImageAndText(
            rb.GetImageSkiaNamed(IDR_LIVEHIME_CAMERA_FILTER_QI), L"04");
        layout->AddView(filter_radio_button);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    }

    void ButtonTabView::ButtonPressed(views::Button* sender, const ui::Event& event) {
        switch (sender->id()) {
        case kActionButtonId:
        {
            using_dwrite_ = !using_dwrite_;
            string16 dwrite_status;
            if (using_dwrite_) {
                dwrite_status = L"DWrite当前状态：已启用";
            } else {
                dwrite_status = L"DWrite当前状态：已禁用";
            }

            static_cast<BililiveLabelButton*>(sender)->SetText(dwrite_status);
            gfx::Canvas::SetDWriteEnabled(using_dwrite_);
            InvalidateLayout();
            Layout();
            SchedulePaint();
            //GetWidget()->Close();
            break;
        }

        case kFunctionButtonId: {
            livehime::ShowMessageBox(
                GetWidget()->GetNativeView(), L"提示", L"这是一个提示框", L"Ok,Cancel");
            break;
        }

        case kLinkButtonId:
            GetWidget()->FlashFrame(true);
            break;

        case kSmallLinkButtonId:
            ExamplesFrameView::FlashModalWidget(GetWidget()->GetNativeView());
            break;

        case kCapsuleButtonId:
            break;

        case kVerticalLabelButtonId:
            break;

            // 可以使用 id 来判断，也可以使用下面的 if 来判断。
            // 其他控件也一样。
        case kCheckBoxId:
            break;

        default:
            break;
        }

        if (sender == checkbox_) {
            if (checkbox_->checked()) {
                // Do something...
            }
        }
    }
}