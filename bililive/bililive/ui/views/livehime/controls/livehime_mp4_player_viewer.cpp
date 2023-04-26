#include "livehime_mp4_player_viewer.h"

#include "base/file_util.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/image_util.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/canvas_paint_win.h"
#include "ui/views/widget/widget.h"


//bool LivehimeMp4PlayerView::KV_USE_MP4_ = false;

LivehimeMp4PlayerView::LivehimeMp4PlayerView() :
    wpf_(this)
{
    motion_ = CreateBililiveMotion(this);
}

LivehimeMp4PlayerView::~LivehimeMp4PlayerView()
{
    delete motion_;
    motion_ = nullptr;
}

bool LivehimeMp4PlayerView::IsSupportMp4()
{
    return motion_->IsSupportMp4(); //&& KV_USE_MP4_;
}

bool LivehimeMp4PlayerView::SetOutputSize(uint32_t width, uint32_t height)
{
    return motion_->SetOutputSize(width, height);
}

bool LivehimeMp4PlayerView::SetMp4FilePath(const base::string16& mp4_path)
{
    return motion_->SetMp4File(mp4_path);
}

bool LivehimeMp4PlayerView::SetMp4Buffer(const void* data, uint64_t data_len)
{
    return motion_->SetMp4Buffer(data, data_len);
}

bool LivehimeMp4PlayerView::Play()
{
    wp_ = wpf_.GetWeakPtr();
    return motion_->Play();
}

bool LivehimeMp4PlayerView::IsPlaying()
{
    return motion_->IsPlaying();
}

void LivehimeMp4PlayerView::Destroy()
{
    motion_->Destroy();
}

void LivehimeMp4PlayerView::BindPlayStartCallback(base::Closure callback)
{
    play_start_callback_ = callback;
}

void LivehimeMp4PlayerView::BindPlayEndCallback(base::Closure callback)
{
    play_end_callback_ = callback;
}

gfx::Size LivehimeMp4PlayerView::GetPreferredSize()
{
    int width{ 0 }, height{ 0 };
    motion_->GetVideoSize(&width, &height);
    return GetSizeByDPIScale({ width, height });
}

void LivehimeMp4PlayerView::OnPaint(gfx::Canvas* canvas)
{
    auto rt = GetContentsBounds();
    if (rt.IsEmpty() ||
        image_.isNull())
    {
        return;
    }

    float aspect_ratio = (float)image_.width() / (float)image_.height();
    int render_width = std::min(GetLengthByDPIScale(400), rt.width());
    int render_height = render_width / aspect_ratio;
    if (render_height > rt.height())
    {
        float s = (float)rt.height() / (float)render_height;

        render_height = rt.height();
        render_width *= s;
    }

    int x = (rt.width() - render_width) / 2.0f;
    int y = rt.height() - render_height;

    if (lock_.try_lock())
    {
        canvas->DrawImageInt(image_, 0, 0, image_.width(), image_.height(), x, y, render_width, render_height, true);
        last_image_ = image_;
        lock_.unlock();
    }
    else if(!last_image_.isNull())
    {
        canvas->DrawImageInt(last_image_, 0, 0, last_image_.width(), last_image_.height(), x, y, render_width, render_height, true);
    }
}

//void LivehimeMp4PlayerView::SetUsingMp4Player(bool kv_use_mp4)
//{
//    KV_USE_MP4_ = kv_use_mp4;
//}

void LivehimeMp4PlayerView::OnMp4PlayStart(livehime::motion::MP4Motion* mp4)
{
    if (!play_start_callback_.is_null())
    {
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, play_start_callback_);
    }
}

void LivehimeMp4PlayerView::OnMp4PlayFrameCallback(livehime::motion::MP4Motion* mp4, void* data, uint32_t width, uint32_t height, uint32_t rowptich)
{
    SkBitmap bitmap_ref;
    bitmap_ref.setConfig(SkBitmap::kARGB_8888_Config, width, height, rowptich);
    bitmap_ref.setPixels(const_cast<void*>(data));
    //SkBitmap 只是引用内存，所以deep copy 遍保存副本
    SkBitmap bitmap;
    bitmap_ref.deepCopyTo(&bitmap, SkBitmap::kARGB_8888_Config);

    lock_.lock();
    image_ = gfx::ImageSkia::CreateFrom1xBitmap(bitmap);
    lock_.unlock();

    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, base::Bind(&LivehimeMp4PlayerView::SchedulePaint, wp_));
}

void LivehimeMp4PlayerView::OnMp4PlayEnd(livehime::motion::MP4Motion* mp4)
{
    if (!play_end_callback_.is_null())
    {
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, play_end_callback_);
    }
}

gfx::ImageSkia& LivehimeMp4PlayerView::CurIamge()
{
    return image_;
}

gfx::ImageSkia& LivehimeMp4PlayerView::LastImage()
{
    return last_image_;
}

std::mutex& LivehimeMp4PlayerView::ImageLock()
{
    return lock_;
}

