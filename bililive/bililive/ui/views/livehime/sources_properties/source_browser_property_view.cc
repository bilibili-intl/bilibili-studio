#include "bililive/bililive/ui/views/livehime/sources_properties/source_browser_property_view.h"

#include "bililive/bililive/livehime/sources_properties/source_browser_property_presenter_impl.h"

#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_filter_base_property_view.h"
#include <bililive/secret/public/event_tracking_service.h>

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"




namespace
{
    int StrToIntWithDefault(const std::wstring& str, int default_val)
    {
        int val = 0;
        if (base::StringToInt(str, &val))
        {
            return val;
        }
        else
        {
            return default_val;
        }
    }
}   // namespace


namespace livehime
{

    BasePropertyView* CreateBrowserSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
    {
        return new BrowserCaptureDetailView(scene_item);
    }

}   // namespace livehime

BrowserCaptureDetailView::BrowserCaptureDetailView(obs_proxy::SceneItem* scene_item)
    :scene_item_(scene_item),
    main_view_(std::make_unique<BrowserPropDetailView>(scene_item)){

}

BrowserCaptureDetailView::~BrowserCaptureDetailView() {

}

void BrowserCaptureDetailView::InitView() {
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1.0f, views::GridLayout::USE_PREF, 0, 0);

    layout->StartRow(1.0f, 0);
    layout->AddView(main_view_->Container());
    //layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

bool BrowserCaptureDetailView::SaveSetupChange() {
    main_view_->SaveSetupChange();
    return true;
}

void BrowserCaptureDetailView::PostSaveSetupChange(obs_proxy::SceneItem* scene_item)
{
    main_view_->PostSaveSetupChange(scene_item);
}

bool BrowserCaptureDetailView::Cancel() {
    main_view_->Cancel();
    return true;
}

gfx::ImageSkia* BrowserCaptureDetailView::GetSkiaIcon() {
    return main_view_->GetSkiaIcon();
}

std::wstring BrowserCaptureDetailView::GetCaption() {
    return main_view_->GetCaption();
}

gfx::Size BrowserCaptureDetailView::GetPreferredSize() {
    return gfx::Size(GetLengthByDPIScale(450), GetLengthByDPIScale(450));
}

bool BrowserCaptureDetailView::CheckSetupValid() {
    return true;
}

BrowserPropDetailView::BrowserPropDetailView(obs_proxy::SceneItem* scene_item)
    : data_loaded_(false)
    , name_edit_(nullptr)
    , url_edit_(nullptr)
    , width_edit_(nullptr)
    , height_edit_(nullptr)
    , css_edit_(nullptr)
    , refresh_button_(nullptr)
    , presenter_(std::make_unique<SourceBrowserPropertyPresenterImpl>(scene_item))
    , scene_item_(scene_item)
    , source_filter_base_view_(nullptr)
{
}

BrowserPropDetailView::~BrowserPropDetailView()
{
}

void BrowserPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    //column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(75), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(1);
    //column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    BililiveLabel* label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_COMMON_SOURCENAME));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    name_edit_ = new LivehimeNativeEditView();
    name_edit_->LimitText(64);
    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(name_edit_);

    volume_control_view_ = new SouceVolumeControlView(presenter_->GetSceneItem(), SourceVolumeCtrlType::Browser);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 1);
    layout->AddView(volume_control_view_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_BROWSER_URL));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    url_edit_ = new LivehimeNativeEditView();
    url_edit_->LimitText(1024);
    url_edit_->SetController(this);

    BrowserItemHelper helper(scene_item_);
    if (helper.GetIsPlugin()) {
        url_edit_->SetEnabled(false);
        plugin_id_ = helper.GetPluginId();

        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::PluginSetDlgShow, base::StringPrintf("plugin_id:%lld", plugin_id_));
    }

    //url_edit_->SetMinHeight(kUrlEditMinHeight);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(url_edit_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_BROWSER_WIDTH));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    width_edit_ = new LivehimeNativeEditView();
    width_edit_->LimitText(8);
    width_edit_->SetNumberOnly(true);
    width_edit_->SetController(this);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(width_edit_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_BROWSER_HEIGHT));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    height_edit_ = new LivehimeNativeEditView();
    height_edit_->LimitText(8);
    height_edit_->SetNumberOnly(true);
    height_edit_->SetController(this);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(height_edit_);

    label = new LivehimeTitleLabel(res.GetLocalizedString(IDS_SRCPROP_BROWSER_CSS));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    css_edit_ = new LivehimeNativeEditView();
    css_edit_->LimitText(20480);
    css_edit_->SetMultiLine(true, false, true);
    css_edit_->SetMinHeight(kUrlEditMinHeight);
    css_edit_->SetController(this);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(css_edit_);

    refresh_button_ = new LivehimeFunctionLabelButton(this, res.GetLocalizedString(IDS_SRCPROP_BROWSER_REFRESH));
    views::View* refresh_view = new views::View();
    {
        refresh_view->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal, 0, 0, 0));
        refresh_view->AddChildView(refresh_button_);
    }

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->SkipColumns(1);
    layout->AddView(refresh_view);

    //refresh_check_ = new LivehimeCheckbox(res.GetLocalizedString(IDS_SRCPROP_BROWSER_ACTIVE_REFRESH));
    //layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    //layout->StartRow(0, 0);
    //layout->SkipColumns(1);
    //layout->AddView(refresh_check_);

    //源背景过滤
    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));

    source_filter_base_view_ = new SourceFilterBaseView(scene_item_, plugin_id_);
    source_filter_base_view_->SetMaterialType((size_t)livehime::SourceMaterialType::BROWSER_MATERIAL);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 2);
    layout->AddView(source_filter_base_view_);
}

void BrowserPropDetailView::InitData()
{
    name_edit_->SetText(presenter_->GetName());
    url_edit_->SetText(base::UTF8ToUTF16(presenter_->GetUrl()));
    width_edit_->SetText(base::IntToString16(presenter_->GetWidth()));
    height_edit_->SetText(base::IntToString16(presenter_->GetHeight()));
    css_edit_->SetText(base::UTF8ToUTF16(presenter_->GetCSS()));
    //refresh_check_->SetChecked(presenter_->GetRestartWhenActive());
    presenter_->SetRestartWhenActive(true);  //默认激活刷新
    presenter_->Snapshot();
    data_loaded_ = true;
}

bool BrowserPropDetailView::CheckSetupValid()
{
    return true;
}

bool BrowserPropDetailView::SaveSetupChange()
{
    if (plugin_id_ > 0) {
        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::PluginSetSureClick, base::StringPrintf("plugin_id:%lld", plugin_id_));
    }

    if (!data_loaded_)
    {
        return false;
    }

    presenter_->SetUrl(base::UTF16ToUTF8(url_edit_->GetText()));
    presenter_->SetWidth(StrToIntWithDefault(width_edit_->GetText(), presenter_->GetWidth()));
    presenter_->SetHeight(StrToIntWithDefault(height_edit_->GetText(), presenter_->GetHeight()));
    presenter_->SetCSS(base::UTF16ToUTF8(css_edit_->GetText()));
    //presenter_->SetRestartWhenActive(refresh_check_->checked());
    presenter_->Update();
    if (presenter_->SetName(name_edit_->GetText()) == false)
    {
        livehime::ShowMessageBox(livehime::UniversalMsgboxType::CannotRenameSceneItem);
        return false;
    }
    return true;
}

void BrowserPropDetailView::PostSaveSetupChange(obs_proxy::SceneItem* scene_item)
{
    base::StringPairs data;
    std::pair<std::string, std::string> pair1("material_type", std::to_string((size_t)secret::BehaviorEventMaterialType::Browser));
    std::pair<std::string, std::string> pair6("subtract_background", std::to_string(source_filter_base_view_->GetSwitchButtonTagVal()));
    data.push_back(pair1);
    data.push_back(pair6);
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingSubmitClick, data);
}

bool BrowserPropDetailView::Cancel()
{
    presenter_->Restore();
    source_filter_base_view_->RecoveryConfig();
    return true;
}

gfx::ImageSkia* BrowserPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_BROWSER_TITLE);
}

std::wstring BrowserPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_BROWSER_CAPTION);
}

void BrowserPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    EffectiveControl effective_control = BROWSER_INVALID;
    if (sender == refresh_button_)
    {
        effective_control = BROWSER_REFRESH;

        if (plugin_id_ > 0) {
            livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::BrowserRefreshClick, base::StringPrintf("plugin_id:%d", plugin_id_));
        }
    }
    //else if (sender == refresh_check_)
    //{
    //    effective_control = BROWSER_REFRESH_CHECK;
    //}
    EffectiveImmediately(effective_control);
}

//gfx::Size BrowserPropDetailView::GetPreferredSize()
//{
//    static gfx::Size max_size(GetLengthByDPIScale(450), GetLengthByDPIScale(450));
//    gfx::Size size = __super::GetPreferredSize();
//    size.SetToMax(max_size);
//    return size;
//}

void BrowserPropDetailView::ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents)
{
    if (!new_contents.empty() && (sender == width_edit_ || sender == height_edit_))
    {
        if (std::stoi(new_contents) > 1920)
        {
            sender->SetText(L"1920");
        }

        if (new_contents.length() > 1 && new_contents.at(0) == '0')
        {
            int val = std::stoi(new_contents);
            auto val_str = std::to_wstring(val);
            sender->SetText(val_str);
        }
    }
}


void BrowserPropDetailView::EffectiveImmediately(EffectiveControl effective_control)
{
    switch (effective_control)
    {
    case BROWSER_URL:
        presenter_->SetUrl(base::UTF16ToUTF8(url_edit_->GetText()));
        break;
    case BROWSER_WIDTH:
        presenter_->SetWidth(StrToIntWithDefault(width_edit_->GetText(), presenter_->GetWidth()));
        break;
    case BROWSER_HEIGHT:
        presenter_->SetHeight(StrToIntWithDefault(height_edit_->GetText(), presenter_->GetHeight()));
        break;
    case BROWSER_CSS:
        presenter_->SetCSS(base::UTF16ToUTF8(css_edit_->GetText()));
        break;
    case BROWSER_REFRESH:
        presenter_->Refresh();
        break;
    case BROWSER_REFRESH_CHECK:
        //presenter_->SetRestartWhenActive(refresh_check_->checked());
        break;
    default:
        return;
    }
    presenter_->Update();
}

bool BrowserPropDetailView::PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_KILLFOCUS)
    {
        EffectiveControl effective_control = BROWSER_INVALID;
        if (sender == width_edit_)
        {
            effective_control = BROWSER_WIDTH;
        }
        else if (sender == height_edit_)
        {
            effective_control = BROWSER_HEIGHT;
        }
        else if (sender == url_edit_)
        {
            effective_control = BROWSER_URL;
        }
        else if (sender == css_edit_)
        {
            effective_control = BROWSER_CSS;
        }

        EffectiveImmediately(effective_control);
    }
    return true;
}