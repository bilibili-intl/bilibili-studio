#include "livehime_streaming_quality_view.h"
#include <shellapi.h>
#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/controls/bililive_round_bubble.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

StreamingQualityView* g_quality_view = nullptr;
void StreamingQualityView::ShowWindow(views::View* anchor_view)
{
	if (g_quality_view == nullptr)
	{
		g_quality_view = new StreamingQualityView();
		BililiveRoundBubbleView::ShowWindow(anchor_view, g_quality_view, BililiveRoundBubbleView::RoundBubbleType::Type_RectangleWhite);
		
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::StreamingQualityDetailPanelShow, "");
	}
}

void StreamingQualityView::SetStreamingQuality(StatusBarStreamingQuality quality)
{
	if (g_quality_view)
	{
		g_quality_view->UpdateStreamingQuality(quality);
	}
}

void StreamingQualityView::SetBitrateAndLossLabel(const base::string16& bitrate, const base::string16& loss)
{
	if (g_quality_view)
	{
		g_quality_view->UpdateBitrateAndLossLabel(bitrate, loss);
	}
}

void StreamingQualityView::SetCpuAndMemoryLabel(const base::string16& cpu, const base::string16& memory)
{
	if (g_quality_view)
	{
		g_quality_view->UpdateCpuAndMemoryLabel(cpu, memory);
	}
}

void StreamingDataDetailsView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
	if (details.child == this)
	{
		if (details.is_add)
		{
			InitViews();
		}
	}
}

gfx::Size StreamingDataDetailsView::GetPreferredSize()
{
	gfx::Size size(__super::GetPreferredSize().width()/*GetLengthByDPIScale(310)*/, GetLengthByDPIScale(33));
	return size;
}

void StreamingDataDetailsView::OnPaintBackground(gfx::Canvas* canvas)
{
	__super::OnPaintBackground(canvas);
	const gfx::Rect& rect = GetContentsBounds();
	int radius = GetLengthByDPIScale(4);
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(SkColorSetRGB(0xF5, 0xF6, 0xF7));
	paint.setStyle(SkPaint::kFill_Style);
	canvas->DrawRoundRect(GetContentsBounds(), radius, paint);
}

void StreamingDataDetailsView::InitViews()
{
	//数据指标详情view
	views::View *data_details_view_ = this;
	views::GridLayout* details_view_layout = new views::GridLayout(data_details_view_);
	data_details_view_->SetLayoutManager(details_view_layout);
	auto column_set = details_view_layout->AddColumnSet(0);
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(2));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(2));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(2));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

	details_view_layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));
	details_view_layout->StartRow(0, 0);

	ResourceBundle& rb = ResourceBundle::GetSharedInstance();
	base::string16 label_str = rb.GetLocalizedString(IDS_TOOLBAR_BITRATE);
	label_str += L"0kbps";
	bitrate_label_ = new BililiveLabel(label_str);
	bitrate_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(90), GetLengthByDPIScale(12)));
	bitrate_label_->SetFont(ftTwelve);
	bitrate_label_->SetTextColor(GetColor(LabelTitle));
	details_view_layout->AddView(bitrate_label_);

	label_str = rb.GetLocalizedString(IDS_TOOLBAR_FRAME_LOSS);
	label_str += L"0.0%";
	frame_loss_label_ = new BililiveLabel(label_str);
	frame_loss_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(74), GetLengthByDPIScale(12)));
	frame_loss_label_->SetFont(ftTwelve);
	frame_loss_label_->SetTextColor(GetColor(LabelTitle));
	details_view_layout->AddView(frame_loss_label_);

	label_str = rb.GetLocalizedString(IDS_TOOLBAR_CPU);
	label_str += L"0%";
	cpu_label_ = new BililiveLabel(label_str);
	cpu_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(74), GetLengthByDPIScale(12)));
	cpu_label_->SetFont(ftTwelve);
	cpu_label_->SetTextColor(GetColor(LabelTitle));
	details_view_layout->AddView(cpu_label_);

	label_str = rb.GetLocalizedString(IDS_TOOLBAR_MEMORY);
	label_str += L"0%";
	memory_label_ = new BililiveLabel(label_str);
	memory_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(74), GetLengthByDPIScale(12)));
	memory_label_->SetFont(ftTwelve);
	memory_label_->SetTextColor(GetColor(LabelTitle));
	details_view_layout->AddView(memory_label_);
	details_view_layout->AddPaddingRow(1.0f, GetLengthByDPIScale(1));
}

void StreamingDataDetailsView::UpdateBitrateAndLossLabel(const base::string16& bitrate, const base::string16& loss)
{
	bitrate_label_->SetText(bitrate);
	frame_loss_label_->SetText(loss);
}

void StreamingDataDetailsView::UpdateCpuAndMemoryLabel(const base::string16& cpu, const base::string16& memory)
{
	cpu_label_->SetText(cpu);
	memory_label_->SetText(memory);
}


StreamingQualityView::StreamingQualityView()
{

}

StreamingQualityView::~StreamingQualityView()
{
	g_quality_view = nullptr;
}

void StreamingQualityView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
	if (details.child == this)
	{
		if (details.is_add)
		{
			InitViews();
		}
	}
}

gfx::Size StreamingQualityView::GetPreferredSize()
{
	gfx::Size size(GetLengthByDPIScale(333), GetLengthByDPIScale(82));
	return size;
}

void StreamingQualityView::OnPaintBackground(gfx::Canvas* canvas)
{
	__super::OnPaintBackground(canvas);
	//const gfx::Rect& rect = GetContentsBounds();
	//int radius = GetLengthByDPIScale(4);
	//SkPaint paint;
	//paint.setAntiAlias(true);
	//paint.setColor(SkColorSetRGB(0xDB, 0xF5, 0xFF));
	//paint.setStyle(SkPaint::kFill_Style);
	//canvas->DrawRoundRect(GetContentsBounds(), radius, paint);
}

void StreamingQualityView::InitViews()
{
	views::GridLayout* layout = new views::GridLayout(this);
	SetLayoutManager(layout);

	views::ColumnSet* column_set = layout->AddColumnSet(0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF,0, 0);
	column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(10));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF,0, 0);
	column_set->AddPaddingColumn(0, GetLengthByDPIScale(10));

	column_set = layout->AddColumnSet(1);
	column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(10));
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
	column_set->AddPaddingColumn(1.0f, GetLengthByDPIScale(10));

	quality_label_ = new BililiveLabel(L"直播质量较好。");
	quality_label_->SetFont(ftTwelve);
	quality_label_->SetTextColor(SkColorSetRGB(0x8A, 0x98, 0xA3));
	quality_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
	quality_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(300), GetLengthByDPIScale(14)));

	help_bt_ = new LivehimeImageRightButton(this, L"查看帮助");
	help_bt_->SetFont(ftTwelve);
	help_bt_->SetColor(views::Button::STATE_NORMAL, SkColorSetRGB(0x0E, 0xBE, 0xFF));
	help_bt_->SetColor(views::Button::STATE_HOVERED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
	help_bt_->SetCursor(::LoadCursor(nullptr, IDC_HAND));
	gfx::ImageSkia* image_normal = GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_ARROW_RIGHT_HOVER);
	gfx::ImageSkia* image_hover = GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_ARROW_RIGHT_HOVER);
	help_bt_->SetImage(views::Button::STATE_NORMAL, image_normal);
	help_bt_->SetImage(views::Button::STATE_HOVERED, image_hover);

	layout->AddPaddingRow(0, GetLengthByDPIScale(12));
	layout->StartRow(0, 0);
	layout->AddView(quality_label_);
	layout->AddView(help_bt_);

	data_details_view_ = new StreamingDataDetailsView();
	layout->AddPaddingRow(0, GetLengthByDPIScale(12));
	layout->StartRow(0, 1);
	layout->AddView(data_details_view_);
}

void StreamingQualityView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
	if (sender == help_bt_)
	{
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::StreamingQualityDetailPanelHelpClick, "");
	}
}

void StreamingQualityView::UpdateStreamingQuality(StatusBarStreamingQuality quality)
{
	if (cur_quality_ == quality)
	{
		return;
	}
	cur_quality_ = quality;
	if (cur_quality_ == StatusBarStreamingQuality::Quality_Good)
	{
		quality_label_->SetText(L"直播质量较好。");
	} 
	else if (cur_quality_ == StatusBarStreamingQuality::Quality_Secondary)
	{
		quality_label_->SetText(L"直播质量中等，可能会存在卡顿或延迟。");
	}
	else if (cur_quality_ == StatusBarStreamingQuality::Quality_Poor)
	{
		quality_label_->SetText(L"直播质量较差，可能会存在卡顿或延迟。");
	}
}

void StreamingQualityView::UpdateBitrateAndLossLabel(const base::string16& bitrate, const base::string16& loss)
{
	if (data_details_view_)
	{
		data_details_view_->UpdateBitrateAndLossLabel(bitrate, loss);
	}
}

void StreamingQualityView::UpdateCpuAndMemoryLabel(const base::string16& cpu, const base::string16& memory)
{
	if (data_details_view_)
	{
		data_details_view_->UpdateCpuAndMemoryLabel(cpu, memory);
	}
}