#include "misc_tab_view.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_image_view.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace examples {

    enum {
        IDB_SVGA_PREV_FILE = 98,
        IDB_SVGA_PREV_FRAME = 99,
        IDB_SVGA_PAUSE_RESUME = 100,
        IDB_SVGA_STOP = 101,
        IDB_SVGA_NEXT_FRAME = 102,
        IDB_SVGA_NEXT_FILE = 103,
    };

    MiscTabView::MiscTabView()
        : slider_(nullptr),
          combobox_(nullptr),
          edit_view_(nullptr) {
    }

    void MiscTabView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
        __super::ViewHierarchyChanged(details);

        if (details.child == this) {
            if (details.is_add) {
                InitViews();
                InitViewsData();
            }
        }
    }

    void MiscTabView::InitViews() {
        ResourceBundle& rb = ResourceBundle::GetSharedInstance();

        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        auto column = layout->AddColumnSet(0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        column = layout->AddColumnSet(2);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::FIXED, 0, 0);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::FIXED, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
        auto img_view = new LivehimeImageView();
        img_view->SetImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_AVATAR_FRAME_GOVERNOR));
        img_view->SetPreferredSize(gfx::Size(40, 80));
        img_view->SetScaleType(LivehimeImageView::ST_FILL);
        //img_view->SetImageVertAlign(LivehimeImageView::LEADING);
        //img_view->SetImageBoundSize(true, gfx::Size(30, 30));
        //img_view->SetImagePadding(gfx::Insets(60, 0, 0, 50));
        img_view->set_border(views::Border::CreateSolidBorder(1, SK_ColorBLACK));
        layout->AddView(img_view);

        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_COUNTDOWN);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_READYGO);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_FIRE_LTR);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_FIRE_RTL);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_BOOM_LEFT);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_BOOM_RIGHT);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_VICTORY);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_FAILURE);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_EVEN_22);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_EVEN_33);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_LIGHTNING);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_CRY_TV);
        svga_ids_.push_back(IDR_LIVEMAIN_SVGA_MELEE_CUTE_TV);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
        svga_player_ = new LivehimeSVGAImageView();
        svga_player_->setDelegate(this);
        svga_player_->setSVGAResourceId(IDR_LIVEMAIN_SVGA_MELEE_COUNTDOWN);
        svga_player_->set_border(views::Border::CreateSolidBorder(1, SK_ColorBLACK));
        svga_player_->setPreferredSize(GetSizeByDPIScale(gfx::Size(100, 200)));
        svga_player_->setScaleType(svga::ScaleType::FIT_CENTER);
        svga_player_->setLoopCount(-1);
        layout->AddView(svga_player_);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
        svga_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        layout->AddView(svga_slider_);

        layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForCtrlTips);
        size_label_ = new LivehimeContentLabel(L"Loading");
        size_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        layout->AddView(size_label_);
        frame_label_ = new LivehimeContentLabel(L"--/--");
        frame_label_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
        layout->AddView(frame_label_);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
        {
            auto c_view = new View();
            auto c_layout = new views::GridLayout(c_view);
            c_view->SetLayoutManager(c_layout);

            auto c_col = c_layout->AddColumnSet(0);
            c_col->AddPaddingColumn(1, 0);
            c_col->AddColumn(
                views::GridLayout::LEADING, views::GridLayout::CENTER,
                0, views::GridLayout::USE_PREF, 0, 0);
            c_col->AddPaddingColumn(0, kPaddingRowHeightForGroupTips);
            c_col->AddColumn(
                views::GridLayout::LEADING, views::GridLayout::CENTER,
                0, views::GridLayout::USE_PREF, 0, 0);
            c_col->AddPaddingColumn(0, kPaddingRowHeightForGroupTips);
            c_col->AddColumn(
                views::GridLayout::LEADING, views::GridLayout::CENTER,
                0, views::GridLayout::USE_PREF, 0, 0);
            c_col->AddPaddingColumn(0, kPaddingRowHeightForGroupTips);
            c_col->AddColumn(
                views::GridLayout::LEADING, views::GridLayout::CENTER,
                0, views::GridLayout::USE_PREF, 0, 0);
            c_col->AddPaddingColumn(0, kPaddingRowHeightForGroupTips);
            c_col->AddColumn(
                views::GridLayout::LEADING, views::GridLayout::CENTER,
                0, views::GridLayout::USE_PREF, 0, 0);
            c_col->AddPaddingColumn(0, kPaddingRowHeightForGroupTips);
            c_col->AddColumn(
                views::GridLayout::LEADING, views::GridLayout::CENTER,
                0, views::GridLayout::USE_PREF, 0, 0);
            c_col->AddPaddingColumn(1, 0);

            c_layout->StartRow(0, 0);

            auto img_button = new BililiveImageButton(this);
            img_button->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_PREV_FILE));
            img_button->set_id(IDB_SVGA_PREV_FILE);
            c_layout->AddView(img_button);

            img_button = new BililiveImageButton(this);
            img_button->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_PREV_FRAME));
            img_button->set_id(IDB_SVGA_PREV_FRAME);
            c_layout->AddView(img_button);

            pause_btn_ = new BililiveImageButton(this);
            pause_btn_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_PAUSE));
            pause_btn_->set_id(IDB_SVGA_PAUSE_RESUME);
            c_layout->AddView(pause_btn_);

            img_button = new BililiveImageButton(this);
            img_button->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_STOP));
            img_button->set_id(IDB_SVGA_STOP);
            c_layout->AddView(img_button);

            img_button = new BililiveImageButton(this);
            img_button->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_NEXT_FRAME));
            img_button->set_id(IDB_SVGA_NEXT_FRAME);
            c_layout->AddView(img_button);

            img_button = new BililiveImageButton(this);
            img_button->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_NEXT_FILE));
            img_button->set_id(IDB_SVGA_NEXT_FILE);
            c_layout->AddView(img_button);

            layout->AddView(c_view);
        }

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
        combobox_ = new LivehimeCombobox(false);
        combobox_->set_listener(this);
        layout->AddView(combobox_);

        column = layout->AddColumnSet(1);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);
        column->AddColumn(
            views::GridLayout::FILL, views::GridLayout::FILL,
            1, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        column->AddColumn(
            views::GridLayout::LEADING, views::GridLayout::FILL,
            0, views::GridLayout::USE_PREF, 0, 0);
        column->AddPaddingColumn(0, kPaddingColWidthEnds);

        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        layout->AddView(slider_);

        LivehimeContentLabel* label = new LivehimeContentLabel(L"0");
        label->SetPreferredSize(gfx::Size(LivehimeContentLabel::GetFont().GetStringWidth(L"000"), 1));
        slider_->SetAssociationLabel(label);
        layout->AddView(label);

        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        edit_view_ = new LivehimeNativeEditView();
        edit_view_->set_placeholder_text(L"placeholder text");
        layout->AddView(edit_view_);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    }

    void MiscTabView::InitViewsData() {
        int index = combobox_->AddItem(L"Item 01", string16(L"你选中了第1项"));
        combobox_->AddItem(L"Item 02", string16(L"你选中了第2项"));
        combobox_->AddItem(L"Item 03", string16(L"你选中了第3项"));
        combobox_->AddItem(L"Item 04", string16(L"你选中了第4项"));
        combobox_->SetSelectedIndex(index);
    }

    void MiscTabView::ButtonPressed(views::Button* sender, const ui::Event& event) {
        static int svga_idx = 0;
        static double percent = 5.5;

        switch (sender->id()) {
        case IDB_SVGA_PAUSE_RESUME:
            if (svga_player_->isLoadingSucceeded()) {
                bool is_anim = svga_player_->isAnimating();
                if (is_anim) {
                    svga_player_->pauseAnimation();
                } else {
                    svga_player_->startAnimation();
                }
            }
            break;

        case IDB_SVGA_STOP:
            if (svga_player_->isLoadingSucceeded()) {
                svga_player_->stopAnimation();
            }
            break;

        case IDB_SVGA_PREV_FRAME:
            if (svga_player_->isLoadingSucceeded()) {
                int cur = svga_player_->getCurFrame();
                svga_player_->stepToFrame(cur - 1, false);

                /*if (percent >= 10) {
                    percent -= 10;
                }
                double per = double(percent) / 100;
                svga_player_->stepToPercentage(per, false);*/
            }
            break;

        case IDB_SVGA_NEXT_FRAME:
            if (svga_player_->isLoadingSucceeded()) {
                int cur = svga_player_->getCurFrame();
                svga_player_->stepToFrame(cur + 1, false);

                /*if (percent <= 90) {
                    percent += 10;
                }
                double per = double(percent) / 100;
                svga_player_->stepToPercentage(per, false);*/
            }
            break;

        case IDB_SVGA_PREV_FILE:
            if (svga_idx > 0) {
                --svga_idx;

                int svga_id = svga_ids_[svga_idx];
                svga_player_->setSVGAResourceId(svga_id);
                size_label_->SetText(L"Loading");
            }
            break;

        case IDB_SVGA_NEXT_FILE:
            if (svga_idx < int(svga_ids_.size()) - 1) {
                ++svga_idx;

                int svga_id = svga_ids_[svga_idx];
                svga_player_->setSVGAResourceId(svga_id);
                size_label_->SetText(L"Loading");
            }
            break;

        default:
            break;
        }
    }

    void MiscTabView::SliderValueChanged(
        views::Slider* sender,
        float value, float old_value, views::SliderChangeReason reason)
    {
        if (sender == slider_) {
            auto label = slider_->GetAssociationLabel();
            if (label) {
                label->SetText(base::IntToString16(value * 100));
            }
        } else if (sender == svga_slider_) {
            if (reason == views::SliderChangeReason::VALUE_CHANGED_BY_USER) {
                auto val = svga_slider_->value();
                svga_player_->stepToPercentage(val, false);
            }
        }
    }

    void MiscTabView::OnSelectedIndexChanged(BililiveComboboxEx* combobox) {
        if (combobox == combobox_) {
            auto index = combobox_->selected_index();
            if (index >= 0 && index < combobox_->GetItemCount()) {
                edit_view_->SetText(combobox_->GetItemData<string16>(index));
            }
        }
    }

    void MiscTabView::onSVGAParseComplete(LivehimeSVGAImageView* v, bool succeeded) {
        DLOG(INFO) << "*** SVGA *** onSVGAParseComplete: " << succeeded;
        if (succeeded) {
            auto size = svga_player_->getVideoSize();
            auto str = base::StringPrintf(L"%d x %d", size.width(), size.height());
            size_label_->SetText(str);
        } else {
            size_label_->SetText(L"Failed");
        }
    }

    void MiscTabView::onSVGAStart(LivehimeSVGAImageView* v, bool resumed) {
        DLOG(INFO) << "*** SVGA *** onSVGAStart: " << resumed;
        pause_btn_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_PAUSE));
    }

    void MiscTabView::onSVGAPause(LivehimeSVGAImageView* v) {
        DLOG(INFO) << "*** SVGA *** onSVGAPause";
        pause_btn_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_PLAY));
    }

    void MiscTabView::onSVGAFinished(LivehimeSVGAImageView* v) {
        DLOG(INFO) << "*** SVGA *** onSVGAFinished";
        pause_btn_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_EXAMPLE_SVGA_PLAY));
    }

    void MiscTabView::onSVGARepeat(LivehimeSVGAImageView* v, int repeat_count) {
        DLOG(INFO) << "*** SVGA *** onSVGARepeat: " << repeat_count;
    }

    void MiscTabView::onSVGAStep(
        LivehimeSVGAImageView* v,
        bool anim_triggered, int frame, int total_frame, double percentage)
    {
        //DLOG(INFO) << "*** SVGA *** onSVGAStart";
        auto str = base::StringPrintf(L"%d/%d", frame + 1, svga_player_->getFrameCount());
        frame_label_->SetText(str);

        svga_slider_->SetValue(percentage);
    }

}