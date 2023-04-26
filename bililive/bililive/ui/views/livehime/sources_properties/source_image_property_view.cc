#include "bililive/bililive/ui/views/livehime/sources_properties/source_image_property_view.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_image_property_presenter_impl.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/controls/blank_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/utils/bililive_filetype_exts.h"

#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/base/resource/resource_bundle.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"


namespace{

using namespace bililive;

enum ImagePropDetailColumnSet
{
    NAMECOLUMNSET = 0,
    FILEPATHCOLUMNSET
};

};  // namespace

namespace livehime
{

BasePropertyView* CreateImageSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
{
    return new ImagePropDetailView(scene_item);
}

}   // namespace livehime

// ImagePropDetailView
ImagePropDetailView::ImagePropDetailView(obs_proxy::SceneItem* scene_item)
    : data_loaded_(false),
      //name_edit_(nullptr),
      file_path_(nullptr),
      browse_button_(nullptr),
      transparent_slider_(nullptr),
      transparent_value_label_(nullptr),
      rotate_clockwise_button_(nullptr),
      rotate_anticlockwise_button_(nullptr),
      flip_vertical_button_(nullptr),
      flip_horizintal_button_(nullptr),
      presenter_(std::make_unique<SourceImagePropertyPresenterImpl>(scene_item))
{
}

ImagePropDetailView::~ImagePropDetailView()
{
}

void ImagePropDetailView::InitData()
{
    file_path_->SetText(presenter_->GetFilePath());
    transparent_slider_->SetValue(presenter_->GetTransparent());

    presenter_->Snapshot();

    data_loaded_ = true;

    if (presenter_->GetFilePath().empty()) 
    {
        bk_preview_->SelectCheckBox(0);
    }
}

bool ImagePropDetailView::CheckSetupValid()
{
    return true;
}

bool ImagePropDetailView::SaveSetupChange()
{
    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetFilePath(file_path_->GetText());
    presenter_->Update();

    return true;
}

bool ImagePropDetailView::Cancel()
{
    presenter_->Restore();

    return true;
}

gfx::ImageSkia* ImagePropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_PICTURE_TITLE);
}

std::wstring ImagePropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_IMAGE_CAPTION);
}

void ImagePropDetailView::SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    if (data_loaded_ && reason == views::SliderChangeReason::VALUE_CHANGED_BY_API)
    {
        ;
    }
    else if (sender == transparent_slider_)
    {
        transparent_value_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(value)));
        EffectiveImmediately(IMAGE_TRANSPARENT);
    }
}

void ImagePropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    EffectiveControl effective = IMAGE_INVALID;
    if (sender == browse_button_)
    {
        auto& res = ui::ResourceBundle::GetSharedInstance();

        std::unique_ptr<ISelectFileDialog> dlg{ CreateSelectFileDialog(sender->GetWidget()) };
        dlg->ClearFilter();
        dlg->AddFilter(res.GetLocalizedString(IDS_FILEDIALOG_IMAGE_FILTER), GetImageFileExts());
        dlg->SetDefaultPath(file_path_->GetText());
        bool selected = dlg->DoModel();
        if (selected)
        {
            file_path_->SetText(dlg->GetSelectedFileName());
        }
    }
    else if (sender == rotate_clockwise_button_ || sender == rotate_anticlockwise_button_)
    {
        effective =
            sender == rotate_clockwise_button_ ? IMAGE_ROTATE_CLOCKWISE : IMAGE_ROTATE_ANTICLOCKWISE;
    }
    else if (sender == flip_vertical_button_ || sender == flip_horizintal_button_)
    {
        effective =
            sender == flip_vertical_button_ ? IMAGE_FLIP_VERTICAL : IMAGE_FLIP_HORIZINTAL;
    }

    EffectiveImmediately(effective);
}

void ImagePropDetailView::OnBkImageSelected(const BkImageInfo& bk_info)
{
    auto file_path = bk_info.file_path.AsUTF16Unsafe();
    file_path_->SetText(file_path);
    presenter_->SetFilePath(file_path);
}

void ImagePropDetailView::OnBkImageInitialized()
{
}

void ImagePropDetailView::EffectiveImmediately(EffectiveControl effective_control)
{
    switch (effective_control)
    {
    case IMAGE_INVALID:
        return;
    case IMAGE_TRANSPARENT:
        presenter_->SetTransparent(transparent_slider_->value());
        break;
    case IMAGE_ROTATE_CLOCKWISE:
        presenter_->RotateRight();
        return;
    case IMAGE_ROTATE_ANTICLOCKWISE:
        presenter_->RotateLeft();
        return;
    case IMAGE_FLIP_VERTICAL:
        presenter_->VerticalFlip();
        return;
    case IMAGE_FLIP_HORIZINTAL:
        presenter_->HorizontalFlip();
        return;
    }

    presenter_->UpdateFilters();
}

void ImagePropDetailView::InitView()
{
    auto& rb = ui::ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    this->SetLayoutManager(layout);
    layout->SetInsets(GetLengthByDPIScale(30), GetLengthByDPIScale(30), 0, GetLengthByDPIScale(30));

    auto col_set = layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    col_set = layout->AddColumnSet(1);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.77f, views::GridLayout::FIXED, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(12));
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.2f, views::GridLayout::FIXED, 0, 0);

    col_set = layout->AddColumnSet(2);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(6));
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(20));
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);

    col_set = layout->AddColumnSet(3);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.2125f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.7875f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);

    col_set = layout->AddColumnSet(4);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);



    auto label_color = GetColor(LabelTitle);
    auto label1 = new BililiveLabel(GetLocalizedString(IDS_SRCPROP_IMAGE_ADD_LOCAL));
    label1->SetTextColor(label_color);
    label1->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label1->SetFont(ftFourteenBold);

    auto label2 = new BililiveLabel(GetLocalizedString(IDS_SRCPROP_IMAGE_ADD_DEFAULT_BACKGROUND));
    label2->SetTextColor(label_color);
    label2->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label2->SetFont(ftFourteenBold);

    auto label3 = new BililiveLabel(GetLocalizedString(IDS_SRCPROP_IMAGE_ADVANCE_SETTING));
    label3->SetTextColor(label_color);
    label3->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label3->SetFont(ftFourteenBold);

    auto label4 = new BililiveLabel(GetLocalizedString(IDS_SRCPROP_IMAGE_DIRECTION));
    label4->SetTextColor(label_color);
    label4->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label4->SetFont(ftFourteen);

    auto label5 = new BililiveLabel(GetLocalizedString(IDS_SRCPROP_IMAGE_RORATE_DEGREE));
    label5->SetTextColor(label_color);
    label5->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label5->SetFont(ftFourteen);

    auto label6 = new BililiveLabel(GetLocalizedString(IDS_SRCPROP_IMAGE_OPACITY));
    label6->SetTextColor(label_color);
    label6->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label6->SetFont(ftFourteen);

    file_path_ = new LivehimeNativeEditView();
    browse_button_ = new LivehimeFunctionLabelButton(this, rb.GetLocalizedString(IDS_SRCPROP_IMAGE_BROWSE));

    transparent_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    transparent_value_label_ = new LivehimeContentLabel(L"0");
    transparent_value_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    transparent_value_label_->SetPreferredSize(GetSizeByDPIScale({46, 18}));

    flip_horizintal_button_ = new CustomStyleButton(this);

    // flip_horizintal_button_->SetPreferredSize(GetSizeByDPIScale({ 78, 20 }));
    flip_horizintal_button_->SetText(GetLocalizedString(IDS_SRCPROP_IMAGE_FLIP_HORIZINTAL));
    flip_horizintal_button_->SetFont(ftTwelve);
    // flip_horizintal_button_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_HORZ_FLIP));
    // flip_horizintal_button_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // flip_horizintal_button_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    flip_horizintal_button_->SetAllStateTextColor(SkColorSetA(label_color, 204));
    flip_horizintal_button_->SetAllStateBorder(true);
    flip_horizintal_button_->SetAllStateBorderLineWidth(1.f);
    // flip_horizintal_button_->SetAllStateBorderColor(SkColorSetA(label_color, 204));
    flip_horizintal_button_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));


    flip_horizintal_button_->SetPreferredSize(GetSizeByDPIScale({ 180, 30 }));
    flip_horizintal_button_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    flip_horizintal_button_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    flip_horizintal_button_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));

    flip_vertical_button_ = new CustomStyleButton(this);

    flip_vertical_button_->SetPreferredSize(GetSizeByDPIScale({ 78, 20 }));
    flip_vertical_button_->SetText(GetLocalizedString(IDS_SRCPROP_IMAGE_FLIP_VERTICAL));
    flip_vertical_button_->SetFont(ftTwelve);
    // flip_vertical_button_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_VERT_FLIP));
    // flip_vertical_button_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // flip_vertical_button_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    flip_vertical_button_->SetAllStateTextColor(SkColorSetA(label_color, 204));
    flip_vertical_button_->SetAllStateBorder(true);
    flip_vertical_button_->SetAllStateBorderLineWidth(1.f);
    // flip_vertical_button_->SetAllStateBorderColor(SkColorSetA(label_color, 204));
    flip_vertical_button_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));

    flip_vertical_button_->SetPreferredSize(GetSizeByDPIScale({ 180, 30 }));
    flip_vertical_button_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    flip_vertical_button_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    flip_vertical_button_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));

    rotate_clockwise_button_ = new CustomStyleButton(this);

    rotate_clockwise_button_->SetPreferredSize(GetSizeByDPIScale({ 78, 20 }));
    rotate_clockwise_button_->SetText(GetLocalizedString(IDS_SRCPROP_IMAGE_ROTATE_LEFT));
    rotate_clockwise_button_->SetFont(ftTwelve);
    // rotate_clockwise_button_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_CLOCKWISE));
    // rotate_clockwise_button_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // rotate_clockwise_button_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    rotate_clockwise_button_->SetAllStateTextColor(SkColorSetA(label_color, 204));
    rotate_clockwise_button_->SetAllStateBorder(true);
    rotate_clockwise_button_->SetAllStateBorderLineWidth(1.f);
    // rotate_clockwise_button_->SetAllStateBorderColor(SkColorSetA(label_color, 204));
    rotate_clockwise_button_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));

    rotate_clockwise_button_->SetPreferredSize(GetSizeByDPIScale({ 180, 30 }));
    rotate_clockwise_button_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    rotate_clockwise_button_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    rotate_clockwise_button_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));


    rotate_anticlockwise_button_ = new CustomStyleButton(this);

    rotate_anticlockwise_button_->SetPreferredSize(GetSizeByDPIScale({ 78, 20 }));
    rotate_anticlockwise_button_->SetText(GetLocalizedString(IDS_SRCPROP_IMAGE_ROTATE_RIGHT));
    rotate_anticlockwise_button_->SetFont(ftTwelve);
    // rotate_anticlockwise_button_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_ANTICLOCKWISE));
    // rotate_anticlockwise_button_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // rotate_anticlockwise_button_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    rotate_anticlockwise_button_->SetAllStateTextColor(SkColorSetA(label_color, 204));
    rotate_anticlockwise_button_->SetAllStateBorder(true);
    rotate_anticlockwise_button_->SetAllStateBorderLineWidth(1.f);
    // rotate_anticlockwise_button_->SetAllStateBorderColor(SkColorSetA(label_color, 204));
    rotate_anticlockwise_button_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));

    rotate_anticlockwise_button_->SetPreferredSize(GetSizeByDPIScale({ 180, 30 }));
    rotate_anticlockwise_button_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    rotate_anticlockwise_button_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    rotate_anticlockwise_button_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));

    bk_preview_ = new PresetMaterialBackgroundPreviewView(this, LiveModelController::GetInstance()->IsLandscapeModel(), false);
    auto bk_preview_container = new BlankView();
    bk_preview_container->SetPreferredSize(GetSizeByDPIScale({400, 206}));
    bk_preview_container->SetLayoutManager(new views::FillLayout());
    bk_preview_container->AddChildView(bk_preview_->Container());

    layout->StartRow(0.0f, 0);
    layout->AddView(label1);

    layout->StartRowWithPadding(0.0f, 1, 0.0f, GetLengthByDPIScale(4));
    layout->AddView(file_path_);
    layout->AddView(browse_button_);

    layout->StartRowWithPadding(0.0f, 0, 0.0f, GetLengthByDPIScale(30));
    layout->AddView(label2);

    layout->StartRowWithPadding(0.0f, 4, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(bk_preview_container);

    layout->StartRowWithPadding(0.0f, 0, 0.0f, GetLengthByDPIScale(30));
    layout->AddView(label3);

    layout->StartRowWithPadding(0.0f, 2, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(label4);
    layout->AddView(flip_horizintal_button_);
    layout->AddView(flip_vertical_button_);

    layout->StartRowWithPadding(0.0f, 2, 0.0f, GetLengthByDPIScale(16));
    layout->AddView(label5);
    layout->AddView(rotate_clockwise_button_);
    layout->AddView(rotate_anticlockwise_button_);

    layout->StartRowWithPadding(0.0f, 3, 0.0f, GetLengthByDPIScale(16));
    layout->AddView(label6);
    layout->AddView(transparent_slider_);
    layout->AddView(transparent_value_label_);
}
 