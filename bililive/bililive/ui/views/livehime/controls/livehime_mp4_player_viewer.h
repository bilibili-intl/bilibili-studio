#pragma once

#include "base/memory/weak_ptr.h"
#include "bililive/bililive_motion/bililive_motion.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/view.h"

#include <deque>
#include <mutex>

class LivehimeMp4PlayerView :
    public views::View,
    public livehime::motion::Mp4PlayerCallback
{
    friend class LivehimeMediaPlayerView;

public:
    LivehimeMp4PlayerView();
    ~LivehimeMp4PlayerView();

    bool IsSupportMp4();
    bool SetOutputSize(uint32_t width, uint32_t height);
    bool SetMp4FilePath(const base::string16 &mp4_path);
    bool SetMp4Buffer(const void* data, uint64_t data_len);
    bool Play();
    bool IsPlaying();
    void Destroy();

    void BindPlayStartCallback(base::Closure callback);
    void BindPlayEndCallback(base::Closure callback);

    //static void SetUsingMp4Player(bool kv_use_mp4);
    gfx::Size GetPreferredSize() override;

protected:
    void OnPaint(gfx::Canvas* canvas) override;
    void OnMp4PlayStart(livehime::motion::MP4Motion* mp4) override;
    void OnMp4PlayFrameCallback(livehime::motion::MP4Motion* mp4, void* data, uint32_t width, uint32_t height, uint32_t rowptich) override;
    void OnMp4PlayEnd(livehime::motion::MP4Motion* mp4) override;

    gfx::ImageSkia& CurIamge();
    gfx::ImageSkia& LastImage();
    std::mutex& ImageLock();

private:
    //static bool                                     KV_USE_MP4_;

    livehime::motion::MP4Motion*                    motion_ = nullptr;
    std::mutex                                      lock_;
    gfx::ImageSkia                                  image_;
    gfx::ImageSkia                                  last_image_;
    SkBitmap                                        bitmap_;
    base::Closure                                   play_start_callback_;
    base::Closure                                   play_end_callback_;
    base::WeakPtr<LivehimeMp4PlayerView>            wp_;
    base::WeakPtrFactory<LivehimeMp4PlayerView>     wpf_;
};
