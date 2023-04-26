#ifndef LIVEHIME_MIC_VOLUME_BEAT_VIEW_H
#define LIVEHIME_MIC_VOLUME_BEAT_VIEW_H

#include "ui/views/view.h"
#include "ui/views/controls/image_view.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"

//麦克风音量跳动view
class MicVolumeBeatView :public views::View
{
public:
	MicVolumeBeatView();
	~MicVolumeBeatView() = default;
	gfx::Size GetPreferredSize()override;
	void SetImage(const gfx::ImageSkia* image_skia);
protected:
	void Layout()override;
	void OnPaintBackground(gfx::Canvas* canvas)override;
	void VisibilityChanged(views::View* starting_from, bool is_visible)override;
	void OnVolumeLevelUpdate(const std::string& source_name,
		const std::vector<float>& magnitude,
		const std::vector<float>& peak,
		const std::vector<float>& inputPeak);
private:
	void InitViews();
private:
	bool is_visible_ = true;
	float height_ratio_ = 0.0f;
	gfx::Size image_size_;
	gfx::Rect target_rect_;
	views::ImageView* image_ = nullptr;
	float currentPeak_[MAX_AUDIO_CHANNELS];
	std::unique_ptr<contracts::BililiveVolumeControllersContract> volume_controllers_;
};

#endif
