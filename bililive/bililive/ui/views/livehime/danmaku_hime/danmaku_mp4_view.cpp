#include "danmaku_mp4_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "ui/gfx/image/image_skia.h"

#include <thread>

DanmakuMp4View::DanmakuMp4View() :
    wpf_(this)
{
    set_background(views::Background::CreateSolidBackground(SkColorSetARGB(255, 255, 0, 0)));

    std::thread th([this]()
        {
            mp4_player_ = CreateBililiveMotion(this);
            mp4_player_->load(L"E:/360MoveData/Users/YE/Desktop/video.mp4");
        });

    th.detach();
}

DanmakuMp4View::~DanmakuMp4View()
{
}

gfx::Size DanmakuMp4View::GetPreferredSize()
{
    return GetSizeByDPIScale({ 600, 800 });
}

void DanmakuMp4View::OnPaint(gfx::Canvas* canvas)
{
    //__super::OnPaint(canvas);

    if (frame_queue_.empty())
    {
        return;
    }

    auto frame = frame_queue_.front();

    //canvas->FillRect(GetContentsBounds(), SkColorSetRGB(255, 0, 0));

    SkBitmap bitmap_ref;
    bitmap_ref.setConfig(SkBitmap::kARGB_8888_Config, frame->width(), frame->height());
    bitmap_ref.setPixels(reinterpret_cast<void*>(frame->data()));

    gfx::ImageSkia img = gfx::ImageSkia::CreateFrom1xBitmap(bitmap_ref);

    canvas->DrawImageInt(img, 0, 0, img.width(), img.height(), 0, 0, GetContentsBounds().width(), GetContentsBounds().height(), true);

    frame_queue_.pop_front();
}

void DanmakuMp4View::OnFrameCallback(std::unique_ptr<livehime::motion::RGBAVideoFrame> frame)
{
    livehime::motion::RGBAVideoFrame* frame_raw = frame.get();
    frame_queue_.emplace_back(frame_raw);
    frame.release();

    //Sleep(1000);

    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, base::Bind(&DanmakuMp4View::SchedulePaint, wpf_.GetWeakPtr()));
}
