#include "bililive_image_render.h"

#include "base/file_util.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "SkBitmap.h"
#include "SkPixelRef.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_util.h"
#include "ui/gfx/codec/png_codec.h"

#include <xmemory>


namespace bililive
{
    gfx::Size CalculateEqualProportionSize(const gfx::Size &container, const gfx::Size &source)
    {
        gfx::Size size;
        float fsrc = source.width()*1.0f / source.height();
        int width = container.width();
        int height = std::round(width / fsrc);
        if (height > container.height())
        {
            height = container.height();
            width = std::min((int)std::round(height*fsrc), width);
        }
        size.SetSize(width, height);

        return size;
    }
}

namespace
{
    class ImageRenderTimerThread : public base::Thread
    {
        typedef base::RepeatingTimer<BililiveImageRender> GifTimer;
        typedef std::pair<RefImageRender, std::unique_ptr<GifTimer>> TimerPair;
        typedef std::vector<TimerPair> TimerVector;

    public:
        ImageRenderTimerThread()
            : base::Thread("gif_timer") {};
        virtual ~ImageRenderTimerThread() {}

        void AddGifTimer(RefImageRender render)
        {
            std::unique_ptr<GifTimer> timer = std::make_unique<GifTimer>();
            timer->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(render->gif_frame_delta_),
                render, &BililiveImageRender::ChangeGifFrame);
            timers_.push_back(std::make_pair(render, std::move(timer)));
        };

        void RemoveGifTimer(RefImageRender render)
        {
            for (size_t i = 0; i < timers_.size(); i++)
            {
                if (timers_[i].first == render)
                {
                    std::unique_ptr<GifTimer> timer = std::move(timers_[i].second);
                    if (timer->IsRunning())
                    {
                        timer->Stop();
                    }
                    timers_.erase(timers_.begin() + i);

                    break;
                }
            }
        };

    protected:
        virtual void CleanUp()
        {
            for (size_t i = 0; i < timers_.size(); i++)
            {
                std::unique_ptr<GifTimer> timer = std::move(timers_[i].second);
                if (timer->IsRunning())
                {
                    timer->Stop();
                }
            }
            timers_.clear();
        };

    private:
        TimerVector timers_;

        DISALLOW_COPY_AND_ASSIGN(ImageRenderTimerThread);
    };

    scoped_ptr<ImageRenderTimerThread> g_image_timer_thread;

    void StartTimerThread()
    {
        if (!g_image_timer_thread)
        {
            g_image_timer_thread.reset(new ImageRenderTimerThread());
        }
        if (!g_image_timer_thread->IsRunning())
        {
            g_image_timer_thread->StartWithOptions(base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
        }
    }
}


BililiveImageRender::BililiveImageRender(BililiveImageRenderDelegate* delegate)
    : delegate_(delegate)
    , data_(NULL)
    , data_len_(0)
    , gif_frame_delta_(0)
    , image_type_(bililive::ImageType::IT_UNKNOW)
    , enable_gif_(false)
{
}

BililiveImageRender::~BililiveImageRender()
{
    CleanResource();
}

bililive::ImageType BililiveImageRender::InitFromFile(const base::FilePath &file_path)
{
    if (image_type_ != bililive::ImageType::IT_UNKNOW)
    {
        return bililive::ImageType::IT_UNKNOW;
    }

    bililive::ImageType type = bililive::ImageType::IT_UNKNOW;
    base::ThreadRestrictions::ScopedAllowIO allow;
    base::FilePath path(file_path);
    int64 file_size = 0;
    if (file_util::GetFileSize(path, &file_size))
    {
        std::unique_ptr<unsigned char[]> data(new unsigned char[file_size]);
        if (file_util::ReadFile(path, (char*)data.get(), static_cast<int>(file_size)) == file_size)
        {
            type = InitFromBinary(data.get(), static_cast<unsigned int>(file_size), true);
            if (type == bililive::ImageType::IT_GIF)
            {
                // gif数据不删，libnsgif需要一直持有
                data.release();
            }
        }
    }
    return type;
}

bililive::ImageType BililiveImageRender::InitFromBinary(
    const unsigned char * const data, unsigned int len, bool attach_gif_data/* = false*/)
{
    if (image_type_ != bililive::ImageType::IT_UNKNOW)
    {
        return bililive::ImageType::IT_UNKNOW;
    }

    bool valid = false;
    bililive::ImageType type = bililive::GetImageTypeFromBinary(data, len);
    if (type == bililive::ImageType::IT_JPG ||
        type == bililive::ImageType::IT_PNG)
    {
        gfx::ImageSkia image = bililive::MakeSkiaImage(data, len);
        if (!image.isNull())
        {
            image_type_ = type;
            image_size_ = image.size();
            image_ = image;
            valid = true;
        }
    }
    else if (type == bililive::ImageType::IT_GIF)
    {
        if (attach_gif_data)
        {
            data_ = (unsigned char *)data;
            data_len_ = len;
        }
        else
        {
            data_ = new unsigned char[len];
            data_len_ = len;
            memcpy(data_, data, len);
        }

        gfx::ImageSkia image = bililive::MakeSkiaImage(data, len, &gif_);
        if (!image.isNull())
        {
            image_type_ = type;
            image_size_ = image.size();
            image_ = image;
            gif_.decoded_frame = -1;
            if (gif_.frames->frame_delay)
            {
                gif_frame_delta_ = gif_.frames->frame_delay * 10;
            }
            else
            {
                gif_frame_delta_ = 120;
            }
            valid = true;

            SetEnableGif(enable_gif_);
        }
    }

    if (valid)
    {
        if (delegate_)
        {
            delegate_->OnImageFrameChanged(&image_);
        }
    }
    else
    {
        if (delegate_)
        {
            delegate_->OnImageFrameChanged(NULL);
        }

        CleanResource();
    }
    return image_type_;
}

void BililiveImageRender::SetEnableGif(bool enable)
{
    enable_gif_ = enable;
    if (image_type_ == bililive::ImageType::IT_GIF)
    {
        if (enable)
        {
            StartTimerThread();

            Start();
        }
        else
        {
            Stop();
        }
    }
}

void BililiveImageRender::CleanUp()
{
    if (delegate_)
    {
        delegate_ = NULL;
    }

    if (image_type_ == bililive::ImageType::IT_GIF)
    {
        Stop();
    }
}

void BililiveImageRender::CleanResource()
{
    if (image_type_ == bililive::ImageType::IT_GIF)
    {
        gif_finalise(&gif_);
    }
    gif_frame_delta_ = 0;
    if (data_)
    {
        delete data_;
        data_ = NULL;
    }
    data_len_ = 0;
    image_size_.SetSize(0, 0);
    image_type_ = bililive::ImageType::IT_UNKNOW;
}

void BililiveImageRender::ChangeGifFrame()
{
    int cur_frame = gif_.decoded_frame + 1;
    cur_frame = std::max(0, cur_frame);
    cur_frame = ((unsigned int)cur_frame >= gif_.frame_count) ? 0 : cur_frame;
    if (GIF_OK == gif_decode_frame(&gif_, (unsigned int)cur_frame))
    {
        void *curdata = reinterpret_cast<void*>(gif_.frame_image);
        size_t curlen = gif_.width * gif_.height * 4;

        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(&BililiveImageRender::SetImage, this, curdata, curlen));
    }
}

gfx::ImageSkia BililiveImageRender::GetCurrentFrame(void *data, size_t count)
{
    return bililive::MakeSkiaImageFromRawPixels(
        reinterpret_cast<const unsigned char*>(data), count, gfx::Size(gif_.width, gif_.height));
}

void BililiveImageRender::SetImage(void *data, size_t count)
{
    if (delegate_)
    {
        gfx::ImageSkia image = GetCurrentFrame(data, count);
        delegate_->OnImageFrameChanged(&image);
    }
}

void BililiveImageRender::Start()
{
    if (!g_image_timer_thread)
    {
        StartTimerThread();
    }
    g_image_timer_thread->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&ImageRenderTimerThread::AddGifTimer, base::Unretained(g_image_timer_thread.get()),
            RefImageRender(this)));
}

void BililiveImageRender::Stop()
{
    if (g_image_timer_thread && g_image_timer_thread->IsRunning())
    {
        g_image_timer_thread->message_loop_proxy()->PostTask(FROM_HERE,
            base::Bind(&ImageRenderTimerThread::RemoveGifTimer, base::Unretained(g_image_timer_thread.get()),
                RefImageRender(this)));
    }
}
