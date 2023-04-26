#pragma once

#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_view.hpp"
#include "bililive/bililive_motion/bililive_motion.h"
#include "base/memory/weak_ptr.h"

#include <deque>

class DanmakuMp4View :
    public DanmakuView<DanmakuMp4View>,
    livehime::motion::RGBAFrameCallback
{
public:
    DanmakuMp4View();
    ~DanmakuMp4View();

    gfx::Size GetPreferredSize() override;

    void OnPaint(gfx::Canvas* canvas) override;

private:
    livehime::motion::MP4Motion* mp4_player_ = nullptr;

    void OnFrameCallback(std::unique_ptr<livehime::motion::RGBAVideoFrame> frame) override;

    std::deque<livehime::motion::RGBAVideoFrame*> frame_queue_;

    base::WeakPtrFactory<DanmakuMp4View> wpf_;

};

