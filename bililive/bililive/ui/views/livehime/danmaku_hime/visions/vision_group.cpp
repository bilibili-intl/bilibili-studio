#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/vision_group.h"

#include "base/logging.h"
#include "base/stl_util.h"


namespace dmkhime {

VisionGroup::VisionGroup() {
}

VisionGroup::~VisionGroup() {
    STLDeleteElements(&visions_);
}

void VisionGroup::AddVision(Vision* v) {
    DCHECK(v != nullptr);

    visions_.push_back(v);
}

void VisionGroup::AddVision(Vision* v, size_t index) {
    DCHECK(v != nullptr);
    DCHECK(index >= 0 && index <= visions_.size());

    visions_.insert(visions_.begin() + index, v);
}

void VisionGroup::RemoveVision(Vision* v) {
    DCHECK(v != nullptr);

    for (auto it = visions_.cbegin(); it != visions_.cend();) {
        if (*it == v) {
            it = visions_.erase(it);
        } else {
            ++it;
        }
    }
}

void VisionGroup::RemoveVisions(size_t start, size_t size) {
    DCHECK(start >= 0 && size > 0 && start + size <= visions_.size());

    visions_.erase(visions_.begin() + start, visions_.begin() + start + size);
}

void VisionGroup::OffsetChildren(int dx, int dy) {
    for (auto& v : visions_) {
        v->OffsetLeftAndRight(dx);
        v->OffsetTopAndBottom(dy);
    }
}

void VisionGroup::OnDraw(Sculptor* s) {
    for (auto& v : visions_) {
        v->Draw(s);
    }

    OnDrawOver(s);
}

}