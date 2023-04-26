#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_IMAGE_RENDER_BILILIVE_SEQUENCE_IMAGES_UTIL_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_IMAGE_RENDER_BILILIVE_SEQUENCE_IMAGES_UTIL_H

#include <atomic>

#include "base/timer/timer.h"
#include "base/threading/thread.h"
#include "base/files/file_path.h"

#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image.h"


namespace
{
    class SequenceImageTimerThread;
}

namespace bililive
{
    class ImageChunkRef;
}

class BililiveSequenceImageDelegate
{
public:
    virtual void OnSequenceImageFrameChanged(const gfx::ImageSkia* image, 
        size_t curent_frame, size_t frame_counts) = 0;
    virtual void OnSequenceMotionStart() {};
    virtual void OnSequenceMotionStop() {};
    //virtual void OnSequenceMotionOneRoundEnd() {};
};

class BililiveSequenceImageWrapper
    : public base::RefCountedThreadSafe<BililiveSequenceImageWrapper>
{
public:
    BililiveSequenceImageWrapper(BililiveSequenceImageDelegate* delegate, 
        bool IsRepeating = true, int64 frame_delta_in_ms = 40);
    BililiveSequenceImageWrapper(const BililiveSequenceImageWrapper& rhs);

    BililiveSequenceImageWrapper& operator=(const BililiveSequenceImageWrapper& rhs);

    int AddSequenceFromFolder(const base::FilePath &folder_path);
    int AddSequenceFromChunk(const bililive::ImageChunkRef &chunk);

    void SetRepeating(bool repeating);
    void SetDelegate(BililiveSequenceImageDelegate* delegate);
    void Restart(int64 offset_in_ms_relative_to_head = 0);
    void Start();
    void Stop();
    // 上层必须显式调用CleanUp()以便让TimerThread释放引用
    void CleanUp();

    void PlayToFrame(unsigned int frame_index);
    void PlayToMillisecond(int64 ms);
    void PlayToSecond(int64 sec);
    
    gfx::Size image_size() const { return image_size_; }
    gfx::Image image();
    size_t frame_counts() const { return frames_.size(); }
    int64 frame_delta() const { return gif_frame_delta_in_ms_; }
    int64 total_delta() const { return total_delta_in_ms_; }
    bool repeating() const { return repeating_; }

private:
    virtual ~BililiveSequenceImageWrapper();

    // work thread
    void ChangeGifFrame();

    void SetImage(unsigned int frame_index);
    void CleanResource();
    virtual void OnSequenceMotionStart();

private:
    friend class base::RefCountedThreadSafe<BililiveSequenceImageWrapper>;
    friend class SequenceImageTimerThread;

    BililiveSequenceImageDelegate* delegate_;

    int64 gif_frame_delta_in_ms_;
    bool repeating_;
    bool started_;
    std::atomic<unsigned int> current_frame_;
    std::atomic<unsigned int> total_frame_;
    std::vector<gfx::Image> frames_;
    gfx::Size image_size_;
    int64 total_delta_in_ms_;
};

typedef scoped_refptr<BililiveSequenceImageWrapper> RefSequenceImageWrapper;

#endif