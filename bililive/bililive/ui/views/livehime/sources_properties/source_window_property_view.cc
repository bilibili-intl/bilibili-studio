#pragma warning(disable: 4995)

#include "bililive/bililive/ui/views/livehime/sources_properties/source_window_property_view.h"

#include <Dwmapi.h>

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_filter_base_property_view.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "base/ext/callable_callback.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "bililive/bililive/livehime/sources_properties/source_window_property_presenter.h"
#include "base/strings/utf_string_conversions.h"
#include <bililive/secret/public/event_tracking_service.h>

namespace
{
    const int kRowSpacingBig = GetLengthByDPIScale(20);
    const int kRowSpacingSmall = GetLengthByDPIScale(10);

    const int kSourceWindowPropertyWindowWidth = GetLengthByDPIScale(800);
    const int kSourceWindowPropertyComboxWidth = GetLengthByDPIScale(750);
}

namespace livehime
{

BasePropertyView* CreateWindowSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
{
    return new WindowCaptureDetailView(scene_item);
}

}   // namespace livehime

WindowCaptureDetailView::WindowCaptureDetailView(obs_proxy::SceneItem* scene_item, bool preset_setting)
    :main_view_(std::make_unique<WindowPropDetailView>(scene_item, preset_setting)), preset_setting_(preset_setting){
}

WindowCaptureDetailView::~WindowCaptureDetailView() {

}



void WindowCaptureDetailView::InitView() {
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    layout->StartRow(1.0f, 0);
    layout->AddView(main_view_->Container());
}

bool WindowCaptureDetailView::SaveSetupChange() {
    return main_view_->SaveSetupChange();
}

bool WindowCaptureDetailView::Cancel() {
   return main_view_->Cancel();
}

gfx::ImageSkia* WindowCaptureDetailView::GetSkiaIcon() {
    return main_view_->GetSkiaIcon();
}
std::wstring WindowCaptureDetailView::GetCaption() {
    return main_view_->GetCaption();
}

gfx::Size WindowCaptureDetailView::GetPreferredSize() {
    if (preset_setting_)
    {
        return gfx::Size(GetLengthByDPIScale(kSourceWindowPropertyWindowWidth), GetLengthByDPIScale(200));
    } 
    return gfx::Size(GetLengthByDPIScale(kSourceWindowPropertyWindowWidth), GetLengthByDPIScale(370));
}
bool WindowCaptureDetailView::CheckSetupValid() {
    return main_view_->CheckSetupValid();
 }

// WindowPropDetailView
WindowPropDetailView::WindowPropDetailView(obs_proxy::SceneItem* scene_item,bool preset_setting)
    : presenter_(std::make_unique<SourceWindowPropertyPresenterImp>(scene_item)),
      scene_item_(scene_item),
      window_combobox_(nullptr),
      capture_mouse_checkbox_(nullptr),
      compatible_mode_checkbox_(nullptr),
      sli_mode_checkbox_(nullptr),
      source_filter_base_view_(nullptr),
      advance_switch_(nullptr),
      advance_view_(nullptr),
      weak_ptr_factory_(this),
      preset_setting_(preset_setting){
}

WindowPropDetailView::~WindowPropDetailView()
{
}

void WindowPropDetailView::InitData()
{
    RefreshList();
    window_combobox_->SetSelectedData(presenter_->SelectedWindow());
    PropertyList<std::string> model_list = presenter_->ModelList();
    ObsModelNameToLivehime(model_list);
    model_combobox_->LoadList(model_list);
    int i = 0;
    for (auto x : model_list)
    {
        if (std::get<2>(x) == false)
        {
            model_combobox_->EnabledItem(i, false);
        }
        i++;
    }
    model_combobox_->SetSelectedIndex(presenter_->GetCaptureModel());
    capture_mouse_checkbox_->SetChecked(presenter_->IsCaptureCursor());
    compatible_mode_checkbox_->SetChecked(presenter_->IsCompatibleMode());
    sli_mode_checkbox_->SetChecked(presenter_->IsSliMode());
	BOOL enable_aero = false;
	DwmIsCompositionEnabled(&enable_aero);
    aero_mode_checkbox_->SetChecked(enable_aero);
    presenter_->Snapshot();

    SaveSetupChange();
}

bool WindowPropDetailView::CheckSetupValid()
{
    if (window_combobox_->GetSelectedData<std::string>().empty())
    {
        label_warning_->SetVisible(true);
        window_combobox_->SetBorderColor(SkColorSetRGB(0xff, 0x66, 0x66), clrCtrlBorderHov, clrDroplistBorder);
        return false;
    }
    return true;
}

bool WindowPropDetailView::SaveSetupChange()
{
    presenter_->SelectedWindow(window_combobox_->GetSelectedData<std::string>());
    presenter_->SetCaptureModel(model_combobox_->selected_index());
    presenter_->IsCaptureCursor(capture_mouse_checkbox_->checked());
    presenter_->IsCompatibleMode(compatible_mode_checkbox_->checked());
    presenter_->IsSliMode(sli_mode_checkbox_->checked());
    presenter_->Update();
    return true;
}

void WindowPropDetailView::PostSaveSetupChange(obs_proxy::SceneItem* scene_item)
{
    ReportAdvanceSettingInfoAction();
}

bool WindowPropDetailView::Cancel()
{
    presenter_->Restore();
    source_filter_base_view_->RecoveryConfig();
    return true;
}

gfx::ImageSkia* WindowPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_PRINTSCREEN_TITLE);
}

std::wstring WindowPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTION);
}

void WindowPropDetailView::RefreshList()
{
    window_combobox_->LoadList(presenter_->WindowList());
    if (!window_combobox_->GetSelectedData<std::string>().empty())
    {
		label_warning_->SetVisible(false);
		window_combobox_->SetBorderColor(clrCtrlBorderNor, clrCtrlBorderHov, clrDroplistBorder);
    }
}

void WindowPropDetailView::ObsModelNameToLivehime(PropertyList<std::string>& list)
{
    if (list.size() > 3)
        return;

    auto& res = ui::ResourceBundle::GetSharedInstance();
    std::wstring replace[3]{ res.GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTURE_MODEL_AUTO),
    res.GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTURE_MODEL_BITBLT),
    res.GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTURE_MODEL_WINDOWS)};

    for (auto &x : list)
    {
        if (std::get<0>(x) == L"WindowCapture.Method.Auto")
        {
            std::get<0>(x) = replace[0];
        }
        else if (std::get<0>(x) == L"WindowCapture.Method.BitBlt")
        {
            std::get<0>(x) = replace[1];
        }
        else if (std::get<0>(x) == L"WindowCapture.Method.WindowsGraphicsCapture")
        {
            std::get<0>(x) = replace[2];
        }
    }
}

void WindowPropDetailView::UpdateSwitchButtonStatus(bool turn_on) {
    int dst_tag = turn_on ? 1 : 0;
    advance_switch_->SetAllStateImage(*GetImageSkiaNamed(
        turn_on ? IDR_LIVEMAIN_TABAREA_SCENE_DOWN : IDR_LIVEMAIN_TABAREA_SCENE_UP));
    advance_switch_->set_tag(dst_tag);
}

void WindowPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    //if (sender == refresh_button_) {
    //    RefreshList();
    //    window_combobox_->SetSelectedData(presenter_->SelectedWindow());
    //}
    //else 
    if (sender == aero_mode_checkbox_) {
        BOOL enable_aero = aero_mode_checkbox_->checked();
        if (!enable_aero) {
            DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
        }
        else {
            DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
        }
        BOOL new_aero = true;
        DwmIsCompositionEnabled(&new_aero);
		if (new_aero != enable_aero) {
			auto& res = ui::ResourceBundle::GetSharedInstance();
			livehime::ShowMessageBox(sender->GetWidget()->GetNativeView(),
				res.GetLocalizedString(IDS_SRCPROP_WINDOW_AERO_ERROR_TITLE),
				res.GetLocalizedString(IDS_SRCPROP_WINDOW_AERO_ERROR_TEXT),
				res.GetLocalizedString(IDS_SURE));
            aero_mode_checkbox_->SetChecked(!enable_aero);
		}
        aero_enable_ = enable_aero;
        ReportAction(secret::LivehimePolarisBehaviorEvent::SourceSettingAreoClick, aero_enable_);
    }
    else if (sender == advance_switch_) {
        advance_switch_->set_tag(!advance_switch_->tag());
        UpdateSwitchButtonStatus(advance_switch_->tag());
        AdjustFilterVisible();
        //埋点
        ReportAction(secret::LivehimePolarisBehaviorEvent::SourceSettingAdvanceClick,
                     advance_switch_->tag());
    }
    else if (sender == compatible_mode_checkbox_) {
        ReportAction(secret::LivehimePolarisBehaviorEvent::SourceSettingWndAreaClick,
                     compatible_mode_checkbox_->checked());
    }
    else if (sender == sli_mode_checkbox_) {
        ReportAction(secret::LivehimePolarisBehaviorEvent::SourceSettingMultiGraphicClick,
                     sli_mode_checkbox_->checked());
    }
}

void WindowPropDetailView::AdjustFilterVisible() {
    gfx::Rect rect = GetWidget()->GetWindowBoundsInScreen();
    window_init_height_ = rect.height();

    if (advance_switch_->tag()) {
        advance_view_->SetVisible(true);
        //GetWidget()->SetSize(gfx::Size(this->width(), window_init_height_ + advance_view_height_));
    }
    else {
        advance_view_->SetVisible(false);
        //GetWidget()->SetSize(gfx::Size(this->width(), window_init_height_ - advance_view_height_));
    }
}

void WindowPropDetailView::ReportAction(secret::LivehimePolarisBehaviorEvent event,
                                        bool switch_on) {
    base::StringPairs data;
    std::pair<std::string, std::string> pair1("material_type", std::to_string((size_t)livehime::SourceMaterialType::WINDOW_MATERIAL));
    std::pair<std::string, std::string> pair2("switch", std::to_string(switch_on));
    data.push_back(pair1);
    data.push_back(pair2);

    livehime::PolarisEventReport(event, data);
}

void WindowPropDetailView::ReportCaptureMethodAction(secret::LivehimePolarisBehaviorEvent event,
                                const std::string& val) {
    base::StringPairs data;
    std::pair<std::string, std::string> pair1("material_type", std::to_string((size_t)livehime::SourceMaterialType::WINDOW_MATERIAL));
    std::pair<std::string, std::string> pair2("capture_method", val);
    data.push_back(pair1);
    data.push_back(pair2);

    livehime::PolarisEventReport(event, data);
}

void WindowPropDetailView::ReportAdvanceSettingInfoAction() {
    base::StringPairs data;
    std::pair<std::string, std::string> pair1("material_type", std::to_string((size_t)livehime::SourceMaterialType::WINDOW_MATERIAL));
    std::pair<std::string, std::string> pair2("capture_method",
        base::WideToUTF8(model_combobox_->GetItemText(model_combobox_->selected_index())));
    std::pair<std::string, std::string> pair3("window_type", std::to_string(compatible_mode_checkbox_->checked()));
    std::pair<std::string, std::string> pair4("graphics_type", std::to_string(sli_mode_checkbox_->checked()));
    std::pair<std::string, std::string> pair5("aero_type", std::to_string(aero_enable_));
    std::pair<std::string, std::string> pair6("subtract_background", std::to_string(source_filter_base_view_->GetSwitchButtonTagVal()));
    data.push_back(pair1);
    data.push_back(pair2);
    data.push_back(pair3);
    data.push_back(pair4);
    data.push_back(pair5);
    data.push_back(pair6);
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingSubmitClick, data);
}

void WindowPropDetailView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    if (combobox == model_combobox_) {
        presenter_->SetCaptureModel(model_combobox_->selected_index());

        ReportCaptureMethodAction(secret::LivehimePolarisBehaviorEvent::SourceSettingCaptureMethod,
            base::WideToUTF8(model_combobox_->GetItemText(model_combobox_->selected_index())));
    }
    else if (combobox == window_combobox_) {
        presenter_->SelectedWindow(window_combobox_->GetSelectedData<std::string>());
		label_warning_->SetVisible(false);
        window_combobox_->SetBorderColor(clrCtrlBorderNor, clrCtrlBorderHov, clrDroplistBorder);
    }
    presenter_->Update();
}

void WindowPropDetailView::OnBililiveComboboxPressed(BililiveComboboxEx* combobox)
{
	//改为点击时刷新
	RefreshList();
	window_combobox_->SetSelectedData(presenter_->SelectedWindow());
}

void WindowPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    int column_set_index = 0;
	
	auto column_set = layout->AddColumnSet(column_set_index);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);

    //选择窗口标题
	auto label = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Title, res.GetLocalizedString(IDS_SRCPROP_WINDOW_SELECT_WINDOW));
	label_warning_ = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Warning, res.GetLocalizedString(IDS_SRCPROP_WINDOW_SELECT_WINDOW_WARNING));
	label_warning_->SetVisible(false);
	layout->StartRowWithPadding(0, column_set_index, 0, GetLengthByDPIScale(24));
	layout->AddView(label);
    layout->AddView(label_warning_);

    //选择框
    column_set_index++;
	column_set = layout->AddColumnSet(column_set_index);
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);

	window_combobox_ = new LivehimeSrcPropCombobox();
    window_combobox_->SetFixedWidth(kSourceWindowPropertyComboxWidth);
    window_combobox_->set_listener(this);
    layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
    layout->AddView(window_combobox_);

    //显示鼠标
    capture_mouse_checkbox_ = new LivehimeCheckbox(UIViewsStyle::Style_4_10, res.GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTURE_MOUSE));
    layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
    layout->AddView(capture_mouse_checkbox_);

    //高级设置
	auto label_advance_setting = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Title, res.GetLocalizedString(IDS_SRCPROP_WINDOW_ADVANCE_SETTING));
	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingBig);
	layout->AddView(label_advance_setting);
	//advance_switch_ = new BililiveLabelButton(this, L"高级设置"/*res.GetLocalizedString(IDS_SRCPROP_COMMON_ADVANCED_SETTING)*/);
	//advance_switch_->SetSize(gfx::Size(GetLengthByDPIScale(30), GetLengthByDPIScale(30)));
	//advance_switch_->set_tag(0);
	//advance_switch_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
	//advance_switch_->SetStyle(BililiveLabelButton::STYLE_DEFAULT);
	//advance_switch_->SetAllStateTextColor(clrLabelTitle);
	//advance_switch_->SetFont(LivehimeTitleLabel::GetFont());
	//advance_switch_->SetAllStateImage(*GetImageSkiaNamed(IDR_LIVEMAIN_TABAREA_SCENE_UP));

	//layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
	//layout->StartRow(0, WindowPropDetailColumnSet::ADVANCE_CHECKBOX);
	//layout->AddView(advance_switch_);

	advance_view_ = new BililiveHideAwareView();
	{
		views::GridLayout* advance_layout = new views::GridLayout(advance_view_);
		advance_view_->SetLayoutManager(advance_layout);
        int set_index = 0;

        //Checkbox
		column_set = advance_layout->AddColumnSet(set_index);
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
		column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        //窗口区域模式
		compatible_mode_checkbox_ = new LivehimeCheckbox(UIViewsStyle::Style_4_10, res.GetLocalizedString(IDS_SRCPROP_WINDOW_COMPATIBLE_MODE));
		compatible_mode_checkbox_->set_listener(this);
		label = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Notes, res.GetLocalizedString(IDS_SRCPROP_WINDOW_ADVANCE_SETTING_WARNING));
        advance_layout->StartRowWithPadding(0, set_index, 0, 0);
        advance_layout->AddView(compatible_mode_checkbox_);
        advance_layout->AddView(label);
        //多显卡兼容
        set_index++;
		column_set = advance_layout->AddColumnSet(set_index);
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
		column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

		sli_mode_checkbox_ = new LivehimeCheckbox(UIViewsStyle::Style_4_10, res.GetLocalizedString(IDS_SRCPROP_WINDOW_SLI_MODE));
		sli_mode_checkbox_->set_listener(this);
		label = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Notes, res.GetLocalizedString(IDS_SRCPROP_WINDOW_SLI_MODE_WARNING));
		advance_layout->StartRowWithPadding(0, set_index, 0, kRowSpacingSmall);
		advance_layout->AddView(sli_mode_checkbox_);
		advance_layout->AddView(label);
        //切换aero模式
		set_index++;
		column_set = advance_layout->AddColumnSet(set_index);
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
		column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        aero_mode_checkbox_ = new LivehimeCheckbox(UIViewsStyle::Style_4_10, res.GetLocalizedString(IDS_SRCPROP_WINDOW_AERO_SWITCH_TITLE));
        aero_mode_checkbox_->set_listener(this);
		label = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Notes, res.GetLocalizedString(IDS_SRCPROP_WINDOW_AERO_SWITCH_WARNING));
		advance_layout->StartRowWithPadding(0, set_index, 0, kRowSpacingSmall);
		advance_layout->AddView(aero_mode_checkbox_);
		advance_layout->AddView(label);
        //捕获方式
		set_index++;
		column_set = advance_layout->AddColumnSet(set_index);
		column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

		label = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Content, res.GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTURE_MODEL_TITLE));
        advance_layout->StartRowWithPadding(0, set_index, 0, kRowSpacingBig);
        advance_layout->AddView(label);

        set_index++;
		column_set = advance_layout->AddColumnSet(set_index);
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);

		model_combobox_ = new LivehimeSrcPropCombobox();
        model_combobox_->SetFixedWidth(kSourceWindowPropertyComboxWidth);
		model_combobox_->set_listener(this);

		advance_layout->StartRowWithPadding(0, set_index, 0, kRowSpacingSmall);
		advance_layout->AddView(model_combobox_);
	}

	layout->StartRowWithPadding(0, column_set_index, 0, kRowSpacingSmall);
	layout->AddView(advance_view_);

    //扣除背景色
	column_set_index++;
	column_set = layout->AddColumnSet(column_set_index);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(20));

    source_filter_base_view_ = new SourceFilterBaseView(scene_item_);
    source_filter_base_view_->SetMaterialType((size_t)livehime::SourceMaterialType::WINDOW_MATERIAL);
    source_filter_base_view_->SetWindowProDetailView(this);

	layout->StartRowWithPadding(0, column_set_index, 0, kPaddingRowHeightForGroupCtrls);
	layout->AddView(source_filter_base_view_);

	if (preset_setting_)
	{
        label_advance_setting->SetVisible(false);
		advance_view_->SetVisible(false);
        source_filter_base_view_->SetVisible(false);
	}
}
