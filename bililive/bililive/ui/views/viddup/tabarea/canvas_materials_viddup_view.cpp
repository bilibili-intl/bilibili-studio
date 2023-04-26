#include "bililive/bililive/ui/views/viddup/tabarea/canvas_materials_viddup_view.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_widget.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/pref_names.h"


namespace
{
    enum class ButtonType
    {
        ButtonAdd=0,
        ButtonDlg,
        ButtonCam,
        ButtonMore,
        Projection,
        ButtonVtuber,
    };

    const int kMaterialsLength = GetLengthByDPIScale(100);
    const int kMaterialsLengthMin = GetLengthByDPIScale(80);
    const int kAllMaterialsLength = GetLengthByDPIScale(307);

    class MaterialsButtonView
        : public views::View
        , public views::ButtonListener
    {
    public:
        MaterialsButtonView(ButtonType type, views::View * parent)
            : type_(type)
            , delegate_(parent)
        {
            is_install_ios_drive_ = GetBililiveProcess()->bililive_obs()->obs_view()->GetInstalledResult();
            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

            img_ = new LivehimeImageView(this);
            SetImgByType(type);

            layout->StartRow(0, 0);
            layout->AddView(img_);
        }


        void ChangeButtonType(ButtonType type)
        {
            if (type != type_)
            {
                type_ = type;
                SetImgByType(type);
                InvalidateLayout();
            }
        }

        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = __super::GetPreferredSize();
            if (type_ == ButtonType::ButtonAdd)
            {
                size.SetSize(kMaterialsLength, kMaterialsLength);
            } else
            {
                size.SetSize(kMaterialsLengthMin, kMaterialsLengthMin);
            }

            return size;
        }

    protected:
        void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            OnClicked();
        }

    private:
        void SetImgByType(ButtonType type)
        {
            switch (type)
            {
            case ButtonType::ButtonAdd:
                img_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_ADD_MATERIALS));
                break;
            case ButtonType::ButtonDlg:
                img_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_DIALOG));
                break;
            case ButtonType::ButtonCam:
                img_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_CAMERA));
                break;
            case ButtonType::ButtonMore:
                img_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_ADD_MORE));
                break;
            case ButtonType::Projection:
                img_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_ADD_PROJECTION));
                break;
            case ButtonType::ButtonVtuber:
                img_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_V3_TABAREA_ADD_VTUBER));
                break;
            default:
                break;
            }
        }

        void OnClicked()
        {
            switch (type_) {
            case ButtonType::ButtonAdd:
            case ButtonType::ButtonMore:
                TabAreaMaterialsWidget::ShowWindow();
                break;

            case ButtonType::ButtonDlg:
            {
                bililive::CreatingSourceParams params(bililive::SourceType::Window);
                bililive::ExecuteCommandWithParams(
                    GetBililiveProcess()->bililive_obs(),
                    IDC_LIVEHIME_ADD_SOURCE,
                    CommandParams<bililive::CreatingSourceParams>(&params));
                break;
            }

            case ButtonType::ButtonCam:
            {
                bililive::CreatingSourceParams params(bililive::SourceType::Camera);
                bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ADD_SOURCE,
                    CommandParams<bililive::CreatingSourceParams>(&params));
                break;
            }

            case ButtonType::Projection:
            {
                int projection_type = 0;
                if (is_install_ios_drive_) {
                    projection_type = 1;
                }
                
                bililive::ProjectionData data(projection_type);
                bililive::CreatingSourceParams params(bililive::SourceType::Receiver, &data);
                bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ADD_SOURCE,
                    CommandParams<bililive::CreatingSourceParams>(&params));
                break;
            }

            case ButtonType::ButtonVtuber:
            {
                break;
            }

            default:
                NOTREACHED();
                break;
            }
        }

    private:
        ButtonType type_;
        views::View* delegate_ = nullptr;
        LivehimeImageView* img_ = nullptr;
        bool is_install_ios_drive_ = false;
    };

    class VtuberBubbleView
        : public views::View
    {
    public:
        static void Show(views::View* anchor, bool vtuber_opened) {
            Close();

            auto prefs = GetBililiveProcess()->profile()->GetPrefs();
            bool showed = prefs->GetBoolean(prefs::kVtuberBubbleShowed);
           
        }

        static void Close() {
            if (vtuber_bubble_view_) {
                if (vtuber_bubble_view_->GetWidget()) {
                    vtuber_bubble_view_->GetWidget()->Close();
                }

                vtuber_bubble_view_ = nullptr;
            }
        }

        VtuberBubbleView() {
            auto layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            auto col = layout->AddColumnSet(0);
            col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

            layout->StartRow(0, 0);
            auto label = new LivehimeContentLabel(GetLocalizedString(IDS_VTUBER_GUIDE_BUBBLE));
            label->SetTextColor(GetColor(WindowTitleText));
            layout->AddView(label);
        }

        virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
        {
            if (details.child == this)
            {

            }
        }

        ~VtuberBubbleView() {
            vtuber_bubble_view_ = nullptr;
        }

    private:
        static VtuberBubbleView* vtuber_bubble_view_;
    };

    // static
    VtuberBubbleView* VtuberBubbleView::vtuber_bubble_view_ = nullptr;
}

CanvasMaterialsViddupView::CanvasMaterialsViddupView(views::View* parent, bool land_model)
    : area_delegate_(parent)
    , is_land_model_(land_model)
{
}

void CanvasMaterialsViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails & details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();
            LiveModelController::GetInstance()->AddObserver(this);
        }
        else
        {
            VtuberBubbleView::Close();
            LiveModelController::GetInstance()->RemoveObserver(this);
        }
    }
}

void CanvasMaterialsViddupView::InitViews()
{
}

void CanvasMaterialsViddupView::OnPaintBackground(gfx::Canvas* canvas)
{
    __super::OnPaintBackground(canvas);

    auto bounds = GetContentsBounds();

    if (is_land_model_)
    {
        int canvas_hight = (bounds.width() / 16.f) * 9;
        int py = (bounds.height() - canvas_hight) / 2;
        canvas->FillRect(gfx::Rect(0, py, bounds.width(), canvas_hight), GetColor(Theme));
    }
    else
    {
        int canvas_width = (int)(bounds.height()/16.f)*9 + GetLengthByDPIScale(4);
        canvas->FillRect(gfx::Rect((bounds.width() - std::max(canvas_width, kAllMaterialsLength))/2, 0, std::max(canvas_width, kAllMaterialsLength), bounds.height()), SkColorSetRGB(27, 27, 27));
    }
}

void CanvasMaterialsViddupView::Layout() {
}

void CanvasMaterialsViddupView::LayoutPortraitVtuber() {
    auto bounds = GetContentsBounds();

    auto add_btn_size = add_btn_->GetPreferredSize();
    auto win_btn_size = window_btn_->GetPreferredSize();
    auto proj_btn_size = proj_btn_->GetPreferredSize();
    auto cam_btn_size = cam_btn_->GetPreferredSize();
    auto vtb_btn_size = vtb_btn_->GetPreferredSize();
    auto more_btn_size = more_btn_->GetPreferredSize();

    int second_line_width = proj_btn_size.width() + cam_btn_size.width() + kPaddingColWidthForGroupCtrls;
    int third_line_width = vtb_btn_size.width() + more_btn_size.width() + kPaddingColWidthForGroupCtrls;

    int second_line_height = std::max(
        proj_btn_size.height(), cam_btn_size.height());
    int third_line_height = std::max(
        vtb_btn_size.height(), more_btn_size.height());

    int content_height = add_btn_size.height() + second_line_height + third_line_height
        + kPaddingRowHeightForGroupCtrls + kPaddingRowHeightForCtrlTips;

    add_btn_->SetBounds(
        bounds.x() + (bounds.width() - add_btn_size.width()) / 2,
        bounds.y() + (bounds.height() - content_height) / 2,
        add_btn_size.width(), add_btn_size.height());

    int x = bounds.x() + (bounds.width() - second_line_width) / 2;
    int y = add_btn_->bounds().bottom() + kPaddingRowHeightForGroupCtrls;
    proj_btn_->SetBounds(
        x, y,
        proj_btn_size.width(), proj_btn_size.height());
    x = proj_btn_->bounds().right() + kPaddingColWidthForGroupCtrls;

    cam_btn_->SetBounds(
        x, y,
        cam_btn_size.width(), cam_btn_size.height());

    x = bounds.x() + (bounds.width() - third_line_width) / 2;
    y += second_line_height + kPaddingRowHeightForCtrlTips;
    vtb_btn_->SetBounds(
        x, y,
        vtb_btn_size.width(), vtb_btn_size.height());
    x = vtb_btn_->bounds().right() + kPaddingColWidthForGroupCtrls;

    more_btn_->SetBounds(
        x, y,
        more_btn_size.width(), more_btn_size.height());
}

void CanvasMaterialsViddupView::LayoutOthers() {
    auto bounds = GetContentsBounds();

    auto add_btn_size = add_btn_->GetPreferredSize();
    auto win_btn_size = window_btn_->GetPreferredSize();
    auto proj_btn_size = proj_btn_->GetPreferredSize();
    auto cam_btn_size = cam_btn_->GetPreferredSize();
    auto vtb_btn_size = vtb_btn_->GetPreferredSize();
    auto more_btn_size = more_btn_->GetPreferredSize();

    int second_line_height = std::max(
        cam_btn_size.height(), more_btn_size.height());
    if (is_land_model_) {
        second_line_height = std::max(second_line_height, win_btn_size.height());
    } else {
        second_line_height = std::max(second_line_height, proj_btn_size.height());
    }
    if (is_vtuber_opened_) {
        second_line_height = std::max(second_line_height, vtb_btn_size.height());
    }

    int content_height = add_btn_size.height() + second_line_height + kPaddingRowHeightForGroupCtrls;

    int second_line_width = cam_btn_size.width() + more_btn_size.width()
        + kPaddingColWidthForGroupCtrls * 2;
    if (is_land_model_) {
        second_line_width += win_btn_size.width();
    } else {
        second_line_width += proj_btn_size.width();
    }

    if (is_vtuber_opened_) {
        second_line_width += vtb_btn_size.width() + kPaddingColWidthForGroupCtrls;
    }

    add_btn_->SetBounds(
        bounds.x() + (bounds.width() - add_btn_size.width()) / 2,
        bounds.y() + (bounds.height() - content_height) / 2,
        add_btn_size.width(), add_btn_size.height());

    int x;
    int y = add_btn_->bounds().bottom() + kPaddingRowHeightForGroupCtrls;
    if (is_vtuber_opened_) {
        vtb_btn_->SetBounds(
            bounds.x() + (bounds.width() - second_line_width) / 2,
            y,
            vtb_btn_size.width(), vtb_btn_size.height());
        x = vtb_btn_->bounds().right() + kPaddingColWidthForGroupCtrls;
    } else {
        x = bounds.x() + (bounds.width() - second_line_width) / 2;
    }

    if (is_land_model_) {
        window_btn_->SetBounds(
            x, y,
            win_btn_size.width(), win_btn_size.height());
        x = window_btn_->bounds().right() + kPaddingColWidthForGroupCtrls;
    } else {
        proj_btn_->SetBounds(
            x, y,
            proj_btn_size.width(), proj_btn_size.height());
        x = proj_btn_->bounds().right() + kPaddingColWidthForGroupCtrls;
    }

    cam_btn_->SetBounds(
        x, y,
        cam_btn_size.width(), cam_btn_size.height());
    x = cam_btn_->bounds().right() + kPaddingColWidthForGroupCtrls;

    more_btn_->SetBounds(
        x, y,
        more_btn_size.width(), more_btn_size.height());
}

void CanvasMaterialsViddupView::OnLiveLayoutModelChanged(bool user_invoke)
{
    is_land_model_ = LiveModelController::GetInstance()->IsLandscapeModel();
    Layout();
}