#include "livehime_mic_volume_beat_view.h"
#include "base/strings/utf_string_conversions.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"

//音量跳动view
MicVolumeBeatView::MicVolumeBeatView() :
	volume_controllers_(std::make_unique<BililiveVolumeControllersPresenterImpl>())
{
	for (int channelNr = 0; channelNr < MAX_AUDIO_CHANNELS; channelNr++) {
		currentPeak_[channelNr] = -obs_proxy::kPeekRange;
	}

	InitViews();

	if (!volume_controllers_->ControllerIsValid())
	{
		volume_controllers_->SelectController(base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio));
		if (volume_controllers_->ControllerIsValid())
		{
			volume_controllers_->RegisterVolumeLevelUpdatedHandler(
				std::bind(&MicVolumeBeatView::OnVolumeLevelUpdate, this, std::placeholders::_1, std::placeholders::_2,
					std::placeholders::_3, std::placeholders::_4));
		}
	}
}
void MicVolumeBeatView::InitViews()
{
	image_ = new views::ImageView();
	image_->set_interactive(false);
	image_->SetVerticalAlignment(views::ImageView::Alignment::CENTER);
	image_->SetHorizontalAlignment(views::ImageView::Alignment::CENTER);
	gfx::ImageSkia* img = GetImageSkiaNamed(IDR_LIVEHIME_V3_VOICE_LINK_MIC_VOLUME);
	image_->SetImage(img);//需要初始化一个图片，才能拿到size

	image_size_ = image_->GetPreferredSize();
	//float f_x = (float)image_size_.width() * (6.0f / 18.0f);
	//float f_y = (float)image_size_.height() * (12.0f / 18.0f);//这里的y是矩形底部的
	//float f_w = (float)image_size_.width() * (6.0f / 18.0f);
	//float f_h = (float)image_size_.height() * (9.0f / 18.0f);
	float f_x = (float)image_size_.width() * (4.0f / 18.0f);
	float f_y = (float)image_size_.height() * (12.0f / 18.0f);//这里的y是矩形底部的
	float f_w = (float)image_size_.width() * (10.0f / 18.0f);
	float f_h = (float)image_size_.height() * (9.0f / 18.0f);
	target_rect_.set_x((int)f_x);
	target_rect_.set_y((int)f_y);
	target_rect_.set_width((int)f_w);
	target_rect_.set_height((int)f_h);

	AddChildView(image_);
}
void MicVolumeBeatView::SetImage(const gfx::ImageSkia* image_skia)
{
	image_->SetImage(image_skia);
}
gfx::Size MicVolumeBeatView::GetPreferredSize()
{
	gfx::Size image_size = image_->GetPreferredSize();
	return image_size;
}
void MicVolumeBeatView::Layout()
{
	gfx::Size image_size = image_->GetPreferredSize();
	image_->SetBounds(0, 0, image_size.width(), image_size.height());
}
void MicVolumeBeatView::OnPaintBackground(gfx::Canvas* canvas)
{
	float max_peek = obs_proxy::GetMaxPeek(currentPeak_);
	height_ratio_ = (obs_proxy::kPeekRange + max_peek) / obs_proxy::kPeekRange;

	//画白色背景
	SkPaint paint;
	paint.setColor(SkColorSetRGB(0xFF, 0xFF, 0xFF));
	int space = GetLengthByDPIScale(2);
	int width = image_size_.width() - space * 2;
	gfx::Rect bg_rect(space, space, width, width);
	canvas->DrawRoundRect(bg_rect, width / 2, paint);

	//画音量高度
	float f_h = (float)target_rect_.height() * height_ratio_;
	paint.setColor(SkColorSetRGB(0x0E, 0xBE, 0xFF));
	gfx::Rect rect(target_rect_.x(), target_rect_.y() - (int)f_h, target_rect_.width(), (int)f_h);
	canvas->DrawRoundRect(rect, 0, paint);

	__super::OnPaintBackground(canvas);

	//bililive::FillRoundRect(canvas, target_rect_.x(),
	//	target_rect_.y() - (int)f_h,
	//	target_rect_.width(), (int)f_h,
	//	0,paint);
}

void MicVolumeBeatView::VisibilityChanged(views::View* starting_from, bool is_visible)
{
	is_visible_ = is_visible;
}

void MicVolumeBeatView::OnVolumeLevelUpdate(const std::string& source_name,
	const std::vector<float>& magnitude,
	const std::vector<float>& peak,
	const std::vector<float>& inputPeak)
{
	if (!is_visible_)
	{
		return;
	}
	for (int channelNr = 0; channelNr < MAX_AUDIO_CHANNELS; channelNr++) {
		currentPeak_[channelNr] = peak[channelNr];
	}
	SchedulePaint();
}