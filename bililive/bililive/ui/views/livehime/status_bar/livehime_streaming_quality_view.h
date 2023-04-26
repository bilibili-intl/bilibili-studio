#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STREAMING_QUALITY_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_STREAMING_QUALITY_VIEW_H

#include "ui/views/view.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

enum class StatusBarNetworkStatus
{
	Network_Good,//较好
	Network_Secondary,//一般
	Network_Poor,//较差
	Network_Anomaly//异常
};

enum class StatusBarStreamingQuality
{
	Quality_Good,//较好
	Quality_Secondary,//一般
	Quality_Poor//较差
};

class StreamingDataDetailsView : public views::View
{
public:
	StreamingDataDetailsView() = default;
	~StreamingDataDetailsView() = default;
	void UpdateBitrateAndLossLabel(const base::string16& bitrate, const base::string16& loss);
	void UpdateCpuAndMemoryLabel(const base::string16& cpu, const base::string16& memory);
protected:
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
	gfx::Size GetPreferredSize() override;
	void OnPaintBackground(gfx::Canvas* canvas)override;
private:
	void InitViews();
private:
	BililiveLabel* bitrate_label_ = nullptr;
	BililiveLabel* frame_loss_label_ = nullptr;
	BililiveLabel* cpu_label_ = nullptr;
	BililiveLabel* memory_label_ = nullptr;
};

class StreamingQualityView : public views::View,
	public views::ButtonListener
{
public:
	static void ShowWindow(views::View* anchor_view);
	static void SetStreamingQuality(StatusBarStreamingQuality quality);
	static void SetBitrateAndLossLabel(const base::string16 & bitrate, const base::string16& loss);
	static void SetCpuAndMemoryLabel(const base::string16& cpu, const base::string16& memory);
	StreamingQualityView();
	~StreamingQualityView();
	void UpdateStreamingQuality(StatusBarStreamingQuality quality);
	void UpdateBitrateAndLossLabel(const base::string16& bitrate, const base::string16& loss);
	void UpdateCpuAndMemoryLabel(const base::string16& cpu, const base::string16& memory);
protected:
	void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
	gfx::Size GetPreferredSize() override;
	void OnPaintBackground(gfx::Canvas* canvas)override;
	// ButtonListener
	void ButtonPressed(views::Button* sender, const ui::Event& event) override;
private:
	void InitViews();
private:
	StatusBarStreamingQuality cur_quality_ = StatusBarStreamingQuality::Quality_Good;
	BililiveLabel* quality_label_ = nullptr;
	LivehimeImageRightButton* help_bt_ = nullptr;
	StreamingDataDetailsView* data_details_view_ = nullptr;
};

#endif