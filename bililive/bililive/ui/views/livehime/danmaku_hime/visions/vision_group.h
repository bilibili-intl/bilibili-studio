#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_GROUP_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_GROUP_H_

#include <vector>

#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/vision.h"


namespace dmkhime {

// 可添加多个 Vision，用于布局，其本身也是 Vision
class VisionGroup : public Vision {
public:
    VisionGroup();
    ~VisionGroup();

    void AddVision(Vision* v);
    void AddVision(Vision* v, size_t index);

    void RemoveVision(Vision* v);
    void RemoveVisions(size_t start, size_t size);

    void OffsetChildren(int dx, int dy);

protected:
    void OnDraw(Sculptor* s) override;

    virtual void OnDrawOver(Sculptor* s) {}

    std::vector<Vision*> visions_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_GROUP_H_