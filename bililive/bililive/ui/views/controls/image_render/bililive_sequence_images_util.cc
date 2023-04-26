#include "bililive_sequence_images_util.h"

#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/bililive/utils/image_util.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/sequenced_image_package.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "SkBitmap.h"
#include "SkPixelRef.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_util.h"
#include "ui/gfx/codec/png_codec.h"


namespace
{
    class SequenceImageTimerThread : public base::Thread
    {
        typedef base::RepeatingTimer<BililiveSequenceImageWrapper> SequenceTimer;
        typedef std::pair<RefSequenceImageWrapper, std::unique_ptr<SequenceTimer>> TimerPair;
        typedef std::vector<TimerPair> TimerVector;

    public:
        SequenceImageTimerThread()
            : base::Thread("sequence_imgs_timer") {};
        virtual ~SequenceImageTimerThread() {}

        void AddGifTimer(RefSequenceImageWrapper render)
        {
            TimerVector::iterator iter = std::find_if(timers_.begin(), timers_.end(),
                [render](const TimerPair &iter)
            {
                if (iter.first == render)
                {
                    return true;
                }
                return false;
            }
            );
            if (iter != timers_.end())
            {
                return;
            }

            std::unique_ptr<SequenceTimer> timer = std::make_unique<SequenceTimer>();
            timer->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(render->gif_frame_delta_in_ms_),
                render, &BililiveSequenceImageWrapper::ChangeGifFrame);
            timers_.push_back(std::make_pair(render, std::move(timer)));
        };

        void RemoveGifTimer(RefSequenceImageWrapper render)
        {
            for (size_t i = 0; i < timers_.size(); i++)
            {
                if (timers_[i].first == render)
                {
                    std::unique_ptr<SequenceTimer> timer = std::move(timers_[i].second);
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
                std::unique_ptr<SequenceTimer> timer = std::move(timers_[i].second);
                if (timer->IsRunning())
                {
                    timer->Stop();
                }
            }
            timers_.clear();
        };

    private:
        TimerVector timers_;

        DISALLOW_COPY_AND_ASSIGN(SequenceImageTimerThread);
    };

    scoped_ptr<SequenceImageTimerThread> g_image_timer_thread;

    void StartTimerThread()
    {
        if (!g_image_timer_thread)
        {
            g_image_timer_thread.reset(new SequenceImageTimerThread());
        }
        if (!g_image_timer_thread->IsRunning())
        {
            g_image_timer_thread->StartWithOptions(base::Thread::Options(base::MessageLoop::TYPE_IO, 0));
        }
    }
}


BililiveSequenceImageWrapper::BililiveSequenceImageWrapper(BililiveSequenceImageDelegate* delegate,
    bool IsRepeating/* = true*/, int64 frame_delta_in_ms/* = 40*/)
    : delegate_(delegate)
    , gif_frame_delta_in_ms_(frame_delta_in_ms)
    , repeating_(IsRepeating)
    , current_frame_(0)
    , total_frame_(0)
    , started_(false)
    , total_delta_in_ms_(0)
{
}

BililiveSequenceImageWrapper::BililiveSequenceImageWrapper(const BililiveSequenceImageWrapper& rhs)
    : delegate_(nullptr)
    , current_frame_(0)
    , started_(false)
    , gif_frame_delta_in_ms_(rhs.gif_frame_delta_in_ms_)
    , repeating_(rhs.repeating_)
    , total_frame_(rhs.total_frame_.load())
    , image_size_(rhs.image_size_)
    , frames_(rhs.frames_)
    , total_delta_in_ms_(rhs.total_delta_in_ms_)
{
}

BililiveSequenceImageWrapper& BililiveSequenceImageWrapper::operator=(const BililiveSequenceImageWrapper& rhs)
{
    if (this != &rhs)
    {
        current_frame_ = 0;
        gif_frame_delta_in_ms_ = rhs.gif_frame_delta_in_ms_;
        total_frame_ = rhs.total_frame_.load();
        image_size_ = rhs.image_size_;
        frames_ = rhs.frames_;
        total_delta_in_ms_ = rhs.total_delta_in_ms_;
    }
    return (*this);
}

BililiveSequenceImageWrapper::~BililiveSequenceImageWrapper()
{
}

int BililiveSequenceImageWrapper::AddSequenceFromFolder(const base::FilePath &folder_path)
{
    DCHECK(!started_);// 已经开始跑了就不要中途增减画面帧了吧

    int num = 0;
    base::ThreadRestrictions::ScopedAllowIO allow;
    base::FileEnumerator enumerator(folder_path, false, base::FileEnumerator::FILES);
    for (base::FilePath path = enumerator.Next(); !path.empty(); path = enumerator.Next())
    {
        int64 file_size = 0;
        if (file_util::GetFileSize(path, &file_size))
        {
            std::unique_ptr<unsigned char[]> data(new unsigned char[file_size]);
            if (file_util::ReadFile(path, (char*)data.get(), static_cast<int>(file_size)) == file_size)
            {
                bililive::ImageType type = bililive::GetImageTypeFromBinary(data.get(), static_cast<unsigned int>(file_size));
                if (type == bililive::ImageType::IT_JPG || type == bililive::ImageType::IT_PNG)
                {
                    num++;
                    gfx::Image image(bililive::MakeSkiaImage(data.get(), static_cast<size_t>(file_size)));
                    if (!image.IsEmpty())
                    {
                        frames_.push_back(image);
                        image_size_.SetToMax(image.Size());
                        ++total_frame_;
                    }
                }
                else
                {
                    NOTREACHED() << "序列帧里面就不要给出gif图啦";
                }
            }
        }
    }

    if (num)
    {
        total_delta_in_ms_ = total_frame_ * gif_frame_delta_in_ms_;
        StartTimerThread();
    }
    return num;
}

int BililiveSequenceImageWrapper::AddSequenceFromChunk(const bililive::ImageChunkRef &chunk)
{
    DCHECK(!started_);// 已经开始跑了就不要中途增减画面帧了吧

    int num = 0;
    auto&& imgs = chunk.GetAllImages();
    for (const auto& img : imgs)
    {
        bililive::ImageType type = bililive::GetImageTypeFromBinary((const unsigned char *)img.data(), img.size());
        if (type == bililive::ImageType::IT_JPG || type == bililive::ImageType::IT_PNG)
        {
            num++;
            gfx::Image image(bililive::MakeSkiaImage((const unsigned char *)img.data(), img.size()));
            if (!image.IsEmpty())
            {
                frames_.push_back(image);
                image_size_.SetToMax(image.Size());
                ++total_frame_;
            }
        }
        else
        {
            NOTREACHED() << "序列帧里面就不要给出gif图啦";
        }
    }

    if (num)
    {
        total_delta_in_ms_ = total_frame_ * gif_frame_delta_in_ms_;
        StartTimerThread();
    }
    return num;
}

void BililiveSequenceImageWrapper::SetRepeating(bool repeating)
{
    repeating_ = repeating;
}

gfx::Image BililiveSequenceImageWrapper::image()
{
    gfx::Image img;
    if (frames_.size())
    {
        img = frames_[0];
    }
    return img;
}

void BililiveSequenceImageWrapper::CleanUp()
{
    delegate_ = nullptr;

    CleanResource();
}

void BililiveSequenceImageWrapper::PlayToFrame(unsigned int frame_index)
{
    if (frame_index >= 0 && frame_index < total_frame_)
    {
        current_frame_ = frame_index;
    }
    else
    {
        NOTREACHED() << "invalid frame range";
    }
}

void BililiveSequenceImageWrapper::PlayToMillisecond(int64 ms)
{
    if (ms >= 0 && ms < total_delta_in_ms_)
    {
        current_frame_ = ms / gif_frame_delta_in_ms_;
    }
    else
    {
        NOTREACHED() << "invalid delta range";
    }
}

void BililiveSequenceImageWrapper::PlayToSecond(int64 sec)
{
    PlayToMillisecond(sec * 1000);
}

void BililiveSequenceImageWrapper::CleanResource()
{
    Stop();
    image_size_.SetSize(0, 0);
    current_frame_ = 0;
    total_frame_ = 0;
    frames_.clear();
}

void BililiveSequenceImageWrapper::ChangeGifFrame()
{
    if (started_)
    {
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(&BililiveSequenceImageWrapper::SetImage, this, current_frame_.load()));

        current_frame_++;
        if (current_frame_ >= total_frame_)
        {
            current_frame_ = 0;

            if (!repeating_)
            {
                started_ = false;
                BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                    base::Bind(&BililiveSequenceImageWrapper::Stop, this));
            }
        }
    }
}

void BililiveSequenceImageWrapper::SetImage(unsigned int frame_index)
{
    if (frame_index >= 0 && frame_index < total_frame_)
    {
        if (delegate_)
        {
            delegate_->OnSequenceImageFrameChanged(
                frames_[frame_index].ToImageSkia(),
                frame_index, total_frame_);
        }
    }
}

void BililiveSequenceImageWrapper::Start()
{
    started_ = true;
    if (!g_image_timer_thread)
    {
        StartTimerThread();
    }
    g_image_timer_thread->message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&SequenceImageTimerThread::AddGifTimer, base::Unretained(g_image_timer_thread.get()),
            RefSequenceImageWrapper(this)));

    // 为了保证Start在图片完全加载之后，所以上层可能会在非UI线程调用Start
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&BililiveSequenceImageWrapper::OnSequenceMotionStart, this));
}

void BililiveSequenceImageWrapper::Stop()
{
    started_ = false;
    if (g_image_timer_thread && g_image_timer_thread->IsRunning())
    {
        g_image_timer_thread->message_loop_proxy()->PostTask(FROM_HERE,
            base::Bind(&SequenceImageTimerThread::RemoveGifTimer, base::Unretained(g_image_timer_thread.get()),
                RefSequenceImageWrapper(this)));
    }

    if (delegate_)
    {
        delegate_->OnSequenceMotionStop();
    }
}

void BililiveSequenceImageWrapper::Restart(int64 offset_in_ms_relative_to_head/* = 0*/)
{
    PlayToMillisecond(offset_in_ms_relative_to_head);
    if (!started_)
    {
        Start();
    }
}

void BililiveSequenceImageWrapper::SetDelegate(BililiveSequenceImageDelegate* delegate)
{
    delegate_ = delegate;
}

void BililiveSequenceImageWrapper::OnSequenceMotionStart()
{
    if (delegate_)
    {
        delegate_->OnSequenceMotionStart();
    }
}
