#include "bililive_volume_bar.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "skia/include/effects/SkGradientShader.h"

#include "ui/gfx/canvas.h"


const int kVolThickness = 15;

BililiveVolumeBar::BililiveVolumeBar() : 
    is_muted_(false),
    volume_state_(false)
{

}

BililiveVolumeBar::~BililiveVolumeBar() 
{

}

void BililiveVolumeBar::SetVolumeState(bool state)
{
    volume_state_ = state;
}

void BililiveVolumeBar::SetMuted(bool muted)
{
    if (muted == is_muted_)
    {
        return;
    }

    is_muted_ = muted;

    SetEnabled(!is_muted_);
    SchedulePaint();
}

void BililiveVolumeBar::VolumeLevelUpdate(const std::string& source_name,
    const std::vector<float>& magnitude,
    const std::vector<float>& peak,
    const std::vector<float>& inputPeak)
{
    if (source_name != obs_proxy::kDefaultInputAudio)
    {
        return;
    }

    for (int channelNr = 0; channelNr < MAX_AUDIO_CHANNELS; channelNr++) {
        currentPeak_[channelNr] = peak[channelNr];
    }

    SchedulePaint();
}

void BililiveVolumeBar::OnPaint(gfx::Canvas* canvas)
{
    views::View::OnPaint(canvas);

    gfx::Rect content = GetContentsBounds();

    int vol_x = content.x();
    int vol_y = content.y() + (content.height() - kVolThickness) / 2;

    canvas->FillRect(gfx::Rect(vol_x, vol_y, content.width(), kVolThickness), clrVolumeBack);

    if (volume_state_)
    {
        float peak = 0.0f;
        float max_peek = obs_proxy::GetMaxPeek(currentPeak_);
        peak = (obs_proxy::kPeekRange + max_peek) / obs_proxy::kPeekRange;

        if (peak > 0.0)
        {
            SkColor usedColor = clrVolumeNormal;
            int cy = peak * content.width();

            SkColor color[2];
            color[0] = SkColorSetRGB(199, 241, 80);
            color[1] = SkColorSetRGB(86, 235, 179);

            SkPoint pts[2];
            pts[0] = { };
            pts[1] = { static_cast<float>(content.width()), 0.f };

            SkScalar pos[2];
            pos[0] = 0.f;
            pos[1] = 1.f;


            skia::RefPtr<SkShader> s = skia::AdoptRef(SkGradientShader::CreateLinear(
                pts, color, pos, 2, SkShader::kClamp_TileMode, 0));

            SkPaint paint;
            paint.setStyle(SkPaint::Style::kFill_Style);
            paint.setShader(s.get());

            canvas->DrawRect(gfx::Rect(vol_x, vol_y, cy, kVolThickness), paint);
        }
    }
}

gfx::Size BililiveVolumeBar::GetPreferredSize()
{
    gfx::Size size = views::View::GetPreferredSize();

    return gfx::Size(size.width(), GetLengthByDPIScale(kVolThickness));
}

