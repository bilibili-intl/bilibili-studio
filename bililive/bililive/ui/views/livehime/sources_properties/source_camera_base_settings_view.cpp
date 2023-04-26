#include "source_camera_base_settings_view.h"

#include "base/strings/stringprintf.h"
#include "base/prefs/pref_service.h"
#include "ui/views/layout/box_layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/ui/views/controls/average_layout.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/linear_layout.h"
#include "bililive/bililive/livehime/sources_properties/source_camera_property_presenter_impl.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_fliter_select_color_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_filter_base_property_view.h"

#include "bililive/bililive/ui/views/livehime/pre_start_live/pre_start_live_source_preview_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_view.h"
#include "bililive/bililive/ui/views/controls/blank_view.h"

namespace {

    static bool show_old_beauty_ = false;

    class CustomLivehimeTopStripPosStripView : public LivehimeTopStripPosStripView {
    public:
        CustomLivehimeTopStripPosStripView(const base::string16 &text, gfx::ImageSkia* image_skia = nullptr)
            :LivehimeTopStripPosStripView(text, image_skia) {
        }

        void OnPaintBackground(gfx::Canvas* canvas) override {
            canvas->FillRect(GetLocalBounds(), clrWindowsContent);
        }

        gfx::Size GetPreferredSize() override
        {
            return GetSizeByDPIScale({57, 20});
        }

        void Layout() override {
            gfx::Size size = label_->GetPreferredSize();
            label_->SetBounds(0, (height() - size.height()) / 2, size.width(), size.height());
        }

    };

    //class
}

SourceCameraBaseSettingsView::SourceCameraBaseSettingsView(obs_proxy::SceneItem* scene_item) :
    presenter_(std::make_shared<SourceCameraPropertyPresenterImpl>(scene_item, this)),
    scene_item_(scene_item),
    source_filter_base_view_(nullptr)
{
    presenter_->Initialize();
}

SourceCameraBaseSettingsView::~SourceCameraBaseSettingsView()
{
}

void SourceCameraBaseSettingsView::InitView()
{
    auto& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *grid_layout = new views::GridLayout(this);

    auto camera_label = new BililiveLabel(rb.GetLocalizedString(IDS_TOOLBAR_CAMERA));
    camera_label->SetFont(ftFourteen);
    

    camera_label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    camera_label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    camera_label->SetVerticalAlignment(gfx::VerticalAlignment::ALIGN_VCENTER);

    device_combox_ = new LivehimeSrcPropCombobox();
    device_combox_->set_listener(this);

    device_set_button_ = new CustomStyleButton(this, rb.GetLocalizedString(IDS_CAMERA_BEAUTY_ADJUST_CAMERA));
    device_set_button_->SetAllStateTextColor(GetColor(LivehimeColorType::TextTitle));
    device_set_button_->SetAllStateBorderRoundRatio(GetLengthByDPIScale(4.0f));
    device_set_button_->SetAllStateBorder(true);
    //device_set_button_->SetAllStateBorderColor(SkColorSetRGB(227, 229, 231));

    device_set_button_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    device_set_button_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    device_set_button_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScale(4.0f));
    device_set_button_->SetPreferredSize(GetSizeByDPIScale({ 120, 30 }));


    

    auto resolution_label = new BililiveLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_STREAMING_RES));
    resolution_label->SetFont(ftFourteen);
    resolution_label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    resolution_label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    resolution_label->SetVerticalAlignment(gfx::VerticalAlignment::ALIGN_VCENTER);

    dpi_combox_ = new LivehimeSrcPropCombobox();
    dpi_combox_->set_listener(this);

    auto cameta_dir_label = new BililiveLabel(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_DIRECTION));
    cameta_dir_label->SetFont(ftFourteen);
    cameta_dir_label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    cameta_dir_label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    cameta_dir_label->SetVerticalAlignment(gfx::VerticalAlignment::ALIGN_VCENTER);

    vertical_flip_btn_ = new CustomStyleButton(this);

    vertical_flip_btn_->SetPreferredSize(GetSizeByDPIScale({ 100, 30 }));
    vertical_flip_btn_->SetText(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_DIR_VERFLIP));
    vertical_flip_btn_->SetFont(ftTwelve);
    // vertical_flip_btn_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_VERT_FLIP));
    // vertical_flip_btn_->SetImage(views::Button::STATE_HOVERED, *rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_VERT_FLIP_FOCUS));
    // vertical_flip_btn_->SetImage(views::Button::STATE_PRESSED, *rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_VERT_FLIP_FOCUS));
    // vertical_flip_btn_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // vertical_flip_btn_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    vertical_flip_btn_->SetAllStateTextColor(GetColor(LivehimeColorType::TextTitle));
    // vertical_flip_btn_->SetTextColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    // vertical_flip_btn_->SetTextColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));
    vertical_flip_btn_->SetAllStateBorder(true);
    vertical_flip_btn_->SetAllStateBorderLineWidth(GetLengthByDPIScaleF(0.5f));
    // vertical_flip_btn_->SetAllStateBorderColor(SkColorSetRGB(227, 229, 231));
    // vertical_flip_btn_->SetBorderColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    // vertical_flip_btn_->SetBorderColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));
    vertical_flip_btn_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));
    vertical_flip_btn_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    vertical_flip_btn_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    vertical_flip_btn_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));


    horizintal_flip_btn_ = new CustomStyleButton(this);

    horizintal_flip_btn_->SetPreferredSize(GetSizeByDPIScale({ 100, 30 }));
    horizintal_flip_btn_->SetText(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_DIR_HORFLIP));
    horizintal_flip_btn_->SetFont(ftTwelve);
    // horizintal_flip_btn_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_HORZ_FLIP));
    // horizintal_flip_btn_->SetImage(views::Button::STATE_HOVERED, *rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_HORZ_FLIP_FOCUS));
    // horizintal_flip_btn_->SetImage(views::Button::STATE_PRESSED, *rb.GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_HORZ_FLIP_FOCUS));
    // horizintal_flip_btn_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // horizintal_flip_btn_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    horizintal_flip_btn_->SetAllStateTextColor(GetColor(LivehimeColorType::TextTitle));
    // horizintal_flip_btn_->SetTextColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    // horizintal_flip_btn_->SetTextColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));
    horizintal_flip_btn_->SetAllStateBorder(true);
    horizintal_flip_btn_->SetAllStateBorderLineWidth(GetLengthByDPIScaleF(0.5f));
    // horizintal_flip_btn_->SetAllStateBorderColor(SkColorSetRGB(227, 229, 231));
    // horizintal_flip_btn_->SetBorderColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    // horizintal_flip_btn_->SetBorderColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));
    // horizintal_flip_btn_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));

    horizintal_flip_btn_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));
    horizintal_flip_btn_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    horizintal_flip_btn_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    horizintal_flip_btn_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));

    auto choice_angle_label = new BililiveLabel(rb.GetLocalizedString(IDS_CAMERA_BEAUTY_CHOICE_ANGLE));
    choice_angle_label->SetFont(ftFourteen);
    choice_angle_label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    choice_angle_label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    choice_angle_label->SetVerticalAlignment(gfx::VerticalAlignment::ALIGN_VCENTER);

    rotation_btn_ = new CustomStyleButton(this);

    rotation_btn_->SetPreferredSize(GetSizeByDPIScale({ 120, 30 }));
    rotation_btn_->SetText(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_ROTATE_RIGHT));
    rotation_btn_->SetFont(ftTwelve);
    // rotation_btn_->SetAllStateImage(*rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_CLOCKWISE));
    // rotation_btn_->SetImage(views::Button::STATE_HOVERED, *rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_CLOCKWISE_FOCUS));
    // rotation_btn_->SetImage(views::Button::STATE_PRESSED, *rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_CLOCKWISE_FOCUS));
    // rotation_btn_->SetAllStateImageAlign(CustomStyleButton::ImageAlign::kLeft);
    // rotation_btn_->SetAllStateImagePadding(GetLengthByDPIScale(4));
    rotation_btn_->SetAllStateTextColor(GetColor(LivehimeColorType::TextTitle));
    // rotation_btn_->SetTextColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    // rotation_btn_->SetTextColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));

    rotation_btn_->SetAllStateBorder(true);
    rotation_btn_->SetAllStateBorderLineWidth(GetLengthByDPIScaleF(1.f));
    // rotation_btn_->SetAllStateBorderColor(SkColorSetRGB(227, 229, 231));
    // rotation_btn_->SetBorderColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    // rotation_btn_->SetBorderColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));
    rotation_btn_->SetAllStateBorderRoundRatio(GetLengthByDPIScaleF(4.0f));

    rotation_btn_->SetAllStateBorderColor(SkColorSetRGB(64, 65, 85));
    rotation_btn_->SetAllStateBackgroundColor(SkColorSetRGB(64, 65, 85));
    rotation_btn_->SetAllStateBackgroundRoundRatio(GetLengthByDPIScaleF(4.0f));

    danmaku_mask_contaner_ = new HideAwareView();
    danmaku_mask_contaner_->SetVisible(false);
    auto mask_layout = new views::GridLayout(danmaku_mask_contaner_);
    danmaku_mask_contaner_->SetLayoutManager(mask_layout);
    auto col_set = mask_layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(12));
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);

    mask_layout->StartRowWithPadding(0.0f, 0, 0.0f, GetLengthByDPIScale(10));

    auto mask_label = new BililiveLabel(rb.GetLocalizedString(IDS_CAMERA_BEAUTY_DANMAKU_MASK));
    mask_label->SetFont(ftFourteen);
    mask_label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    mask_label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    mask_label->SetVerticalAlignment(gfx::VerticalAlignment::ALIGN_VCENTER);
    mask_layout->AddView(mask_label);

    danmaku_checkbox_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_CAMERA_BEAUTY_DANMAKU_MASK_TEXT));
    danmaku_checkbox_->set_id(DANMAKU_CHECKBOX);
    danmaku_checkbox_->set_listener(this);
    danmaku_checkbox_->SetFont(ftFourteen);
    mask_layout->AddView(danmaku_checkbox_);

    danmaku_switch_tip_ = new LivehimeHoverTipButton(
        GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
        rb.GetLocalizedString(IDS_CAMERA_BEAUTY_DANMAKU_MASK_TIP),
        views::BubbleBorder::Arrow::BOTTOM_RIGHT);
    danmaku_switch_tip_->SetPreferredSize(GetSizeByDPIScale({12, 12}));
    mask_layout->AddView(danmaku_switch_tip_);

    col_set = grid_layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.10673f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(12));
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.65176f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(35));
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0.14897f, views::GridLayout::USE_PREF, 0, 0);

    col_set = grid_layout->AddColumnSet(1);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(12));
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(8));
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);

    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(8));

    col_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    col_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(12));
    col_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);

    col_set = grid_layout->AddColumnSet(2);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);

    SetLayoutManager(grid_layout);

    grid_layout->StartRow(0, 0);
    grid_layout->AddView(camera_label);
    grid_layout->AddView(device_combox_);
    grid_layout->AddView(device_set_button_);

    grid_layout->StartRowWithPadding(0.0f, 0, 0.0f, GetLengthByDPIScale(16));
    grid_layout->AddView(resolution_label);
    grid_layout->AddView(dpi_combox_);

    grid_layout->StartRowWithPadding(0.0f, 1, 0.0f, GetLengthByDPIScale(14));
    grid_layout->AddView(cameta_dir_label);
    grid_layout->AddView(vertical_flip_btn_);
    grid_layout->AddView(horizintal_flip_btn_);
    grid_layout->AddView(choice_angle_label);
    grid_layout->AddView(rotation_btn_);

    grid_layout->StartRowWithPadding(0.0f, 2, 0.0f, GetLengthByDPIScale(0));
    grid_layout->AddView(danmaku_mask_contaner_);

    grid_layout->StartRowWithPadding(1.0f, 2, 0, GetLengthByDPIScale(16));
    source_filter_base_view_ = new SourceFilterBaseView(scene_item_);
    source_filter_base_view_->SetMaterialType((size_t)livehime::SourceMaterialType::VIDEO_CAPTURE_MATERIAL);
    grid_layout->AddView(source_filter_base_view_);

    grid_layout->StartRowWithPadding(1.0f, 2, 0, GetLengthByDPIScale(0));
    source_audio_base_view_ = new SourceAudioBaseView(scene_item_);
    grid_layout->AddView(source_audio_base_view_);

}

void SourceCameraBaseSettingsView::UninitView()
{

}

void SourceCameraBaseSettingsView::InitData()
{
    RefreshCameraList();
    RefreshResolutionList();

    device_combox_->SetSelectedData(presenter_->GetSelectedCamera());
    OnSelectedIndexChanged(device_combox_);       //ÏÔÊ¾ÉãÏñÍ·Ô´

    dpi_combox_->SetSelectedData(presenter_->GetSelectedResolution());

    Layout();

    presenter_->CameraSnapshot();

    data_loaded_ = true;
}

bool SourceCameraBaseSettingsView::SaveSetupChange()
{
    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetSelectedCamera(device_combox_->GetSelectedData<std::string>());
    presenter_->Update();

    return true;
}

void SourceCameraBaseSettingsView::PostSaveSetupChange(obs_proxy::SceneItem* scene_item)
{
    base::StringPairs data;
    std::pair<std::string, std::string> pair1("material_type", std::to_string((size_t)secret::BehaviorEventMaterialType::Camera));
    std::pair<std::string, std::string> pair6("subtract_background", std::to_string(source_filter_base_view_->GetSwitchButtonTagVal()));
    data.push_back(pair1);
    data.push_back(pair6);
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingSubmitClick, data);
}

bool SourceCameraBaseSettingsView::Cancel()
{
    source_filter_base_view_->RecoveryConfig();
    presenter_->CameraRestore();

    return true;
}

void SourceCameraBaseSettingsView::ShowDanmakuMaskCtrls(bool show)
{
    if (!show)
    {
        danmaku_checkbox_->SetChecked(false);
    }

    danmaku_mask_contaner_->SetVisible(show);
    InvalidateLayout();
    Layout();
}

gfx::Size SourceCameraBaseSettingsView::GetPreferredSize()
{
    return gfx::Size{300, 300};
}

void SourceCameraBaseSettingsView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == device_set_button_)
    {
        presenter_->LaunchConfig();
    }
    else if (sender == horizintal_flip_btn_)
    {
        EffectiveImmediately(CAMERA_FLIP_HORIZINTAL);
    }
    else if (sender == vertical_flip_btn_)
    {
        EffectiveImmediately(CAMERA_FLIP_VERTICAL);
    }
	else if (sender == rotation_btn_)
    {
		presenter_->ClockwiseRotateCamera();
		int angle = presenter_->GetSourcesItemRotateVal();
		presenter_->SetCameraRotate(angle);
		presenter_->Update();
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::CameraRotateClick, "angle:" + std::to_string(angle));
	}
    else if (sender == danmaku_checkbox_)
    {

    }
    else if (sender == auto_beauty_settings_ || sender == logo_image_)
    {
        presenter_->UpdateFilters();
    }
}

void SourceCameraBaseSettingsView::RefreshResolutionListCallback()
{
    RefreshResolutionList();
    device_combox_->SetEnabled(true);
    device_set_button_->SetEnabled(true);
}

void SourceCameraBaseSettingsView::OnBililiveComboboxPressed(BililiveComboboxEx* combobox)
{
    if (combobox == device_combox_ &&
        !device_combox_->IsDropDown())
    {
        auto index = device_combox_->selected_index();
        RefreshCameraList();
        device_combox_->SetSelectedIndex(index);
    }
}

void SourceCameraBaseSettingsView::OnSelectedIndexChanged(BililiveComboboxEx* combobox){
    if (combobox == device_combox_)
    {
        auto device_id = device_combox_->GetSelectedData<std::string>();
        if (device_id != presenter_->GetSelectedCamera())
        {
            presenter_->SetSelectedCamera(device_combox_->GetSelectedData<std::string>());
            device_combox_->SetEnabled(false);
            device_set_button_->SetEnabled(false);
            presenter_->RegisterSceneItemDeferredUpdate();
            source_audio_base_view_->InitData();
            RefreshResolutionList();
            EffectiveImmediately(CAMERA_DPI);
        }
        else
        {
            RefreshResolutionList();
        }

        if (device_id.empty())
        {
            device_set_button_->SetEnabled(false);
        }
    }
    else if (combobox == dpi_combox_)
    {
        EffectiveImmediately(CAMERA_DPI);
    }
}

void SourceCameraBaseSettingsView::OnBililiveComboboxExDropDown(BililiveComboboxEx* combobox)
{

}

void SourceCameraBaseSettingsView::RefreshCameraList()
{
    auto list = presenter_->GetCameraList(L""/*GetLocalizedString(IDS_SRCPROP_CAMERA_DEVICE_NOSELECT)*/);

    device_combox_->LoadList<std::string>(list);
}

void SourceCameraBaseSettingsView::RefreshResolutionList()
{
    auto list = presenter_->GetResolutionList(GetLocalizedString(IDS_SRCPROP_CAMERA_RESOLUTION_NOTUSED));

    dpi_combox_->LoadList<std::string>(list);
}

void SourceCameraBaseSettingsView::RefreshAudioOutputList()
{
    PropertyList<int64> list;

    list.push_back(std::make_tuple(GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIOOUTPUT_STREAM), 0LL, true));
    list.push_back(std::make_tuple(GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIOOUTPUT_DIRECTSOUND), 1LL, true));
    list.push_back(std::make_tuple(GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIOOUTPUT_WAVEOUT), 2LL, true));

    audio_output_combox_->LoadList<int64>(list);
}

void SourceCameraBaseSettingsView::EffectiveImmediately(CameraEffectiveControl effetive_control)
{
    switch (effetive_control)
    {
    case CAMERA_DPI:
        presenter_->SetSelectedResolution(dpi_combox_->GetSelectedData<std::string>());
        break;
    case CAMERA_FLIP_VERTICAL:
    {
		presenter_->VerticalFlip();
		presenter_->SetVerticalFlip(!presenter_->GetVerticalFlip());
		auto trans = presenter_->GetSceneItem()->GetTransformState();
		if (trans.scalex < 0)
		{
			presenter_->SetCameraXScale(true);
		}
		else
		{
			presenter_->SetCameraXScale(false);
		}

		if (trans.scaley < 0)
		{
			presenter_->SetCameraYScale(true);
		}
		else
		{
			presenter_->SetCameraYScale(false);
		}
		presenter_->UpdateFilters();
		return;
    }
    case CAMERA_FLIP_HORIZINTAL:
    {
		presenter_->HorizontalFlip();
		presenter_->SetHorizontalFlip(!presenter_->GetHorizontalFlip());
		auto trans = presenter_->GetSceneItem()->GetTransformState();
		if (trans.scalex < 0)
		{
			presenter_->SetCameraXScale(true);
		}
		else
		{
			presenter_->SetCameraXScale(false);
		}

		if (trans.scaley < 0)
		{
			presenter_->SetCameraYScale(true);
		}
		else
		{
			presenter_->SetCameraYScale(false);
		}
		presenter_->UpdateFilters();
		return;
    }
    default:
        break;
    }

    presenter_->Update();
}

