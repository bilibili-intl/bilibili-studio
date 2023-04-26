#include "bililive/bililive/ui/views/livehime/sources_properties/source_monitor_property_view.h"
#include "base/strings/string_number_conversions.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

namespace livehime
{

BasePropertyView* CreateMonitorSourcePropertyDetailView(obs_proxy::SceneItem* scene_item)
{
    return new MonitorPropDetailView(scene_item);
}

}   // namespace livehime

// MonitorPropDetailView
MonitorPropDetailView::MonitorPropDetailView(obs_proxy::SceneItem* scene_item)
    : item_(scene_item),
      monitor_combobox_(nullptr),
      capture_mouse_checkbox_(nullptr)
{
}

MonitorPropDetailView::~MonitorPropDetailView()
{
}

void MonitorPropDetailView::InitData()
{
    RefreshList();

    monitor_combobox_->SetSelectedData(item_.SelectedMonitor());
    capture_mouse_checkbox_->SetChecked(item_.IsCaptureCursor());
}

bool MonitorPropDetailView::CheckSetupValid()
{
    return true;
}

bool MonitorPropDetailView::SaveSetupChange()
{
    item_.SelectedMonitor(monitor_combobox_->GetSelectedData<int64>());
    item_.IsCaptureCursor(capture_mouse_checkbox_->checked());
    //item_.IsSliMode(sli_mode_checkbox_->checked());
    item_.Update();
    return true;
}

bool MonitorPropDetailView::Cancel()
{
    return true;
}

gfx::ImageSkia* MonitorPropDetailView::GetSkiaIcon()
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SOURCE_PROPERTY_PRINTSCREEN_TITLE);
}

std::wstring MonitorPropDetailView::GetCaption()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    return res.GetLocalizedString(IDS_SRCPROP_MONITOR_CAPTION);
}

void MonitorPropDetailView::RefreshList()
{
    auto list = item_.MonitorList();
    int index = 1;
    for (auto& x : list)
	{
        std::wstring key1(L"Monitor ");
        std::wstring key2(L"Display ");
        int offset = -1;
        if (std::get<0>(x).find_first_of(key1))
        {
            offset = key1.size();      
        }else if (std::get<0>(x).find_first_of(key2))
        {
            offset = key2.size();
        }
		if (offset > 0)
		{
            std::wstring caption = GetLocalizedString(IDS_SRCPROP_MONITOR_CAPTION);
            std::wstring name = caption + base::IntToString16(index);// std::get<0>(x).substr(offset);
            std::get<0>(x) = name;
            index++;
		}
    }
    monitor_combobox_->LoadList(list);
}

void MonitorPropDetailView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    //if (sender == refresh_button_)
    //{
    //    RefreshList();
    //}
}

void MonitorPropDetailView::InitView()
{
    auto& res = ui::ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    int column_set_index = 0;

    //选择屏幕标题
	auto column_set = layout->AddColumnSet(column_set_index);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::LEADING, 0, views::GridLayout::USE_PREF, 0, 0);
	auto label = new LivehimeStyleLabel(LivehimeStyleLabel::LabelStyle::Style_Title, GetLocalizedString(IDS_SRCPROP_MONITOR_SELETE));
	layout->StartRowWithPadding(0, column_set_index, 0, kPaddingRowHeightEnds);
	layout->AddView(label);

    //选择框
    column_set_index++;
    column_set = layout->AddColumnSet(column_set_index);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::FIXED, GetLengthByDPIScale(400), 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

	auto container_view = new views::View();
	container_view->set_background(views::Background::CreateSolidBackground(GetColor(WindowClient)));
    container_view->SetLayoutManager(new views::FillLayout());
    monitor_combobox_ = new LivehimeSrcPropCombobox();
    monitor_combobox_->set_listener(this);
    container_view->AddChildView(monitor_combobox_);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, column_set_index);
    layout->AddView(container_view);

    //显示鼠标
    column_set_index++;
    column_set = layout->AddColumnSet(column_set_index);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

    capture_mouse_checkbox_ = new LivehimeCheckbox(UIViewsStyle::Style_4_10, res.GetLocalizedString(IDS_SRCPROP_MONITOR_CAPTURE_MOUSE));
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, column_set_index);
    layout->AddView(capture_mouse_checkbox_);
}

void MonitorPropDetailView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{

}

void MonitorPropDetailView::OnBililiveComboboxPressed(BililiveComboboxEx* combobox)
{
    RefreshList();
}