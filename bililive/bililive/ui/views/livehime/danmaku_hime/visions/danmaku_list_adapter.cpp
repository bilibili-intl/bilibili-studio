#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_list_adapter.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_vision.h"


namespace dmkhime {

DanmakuListAdapter::DanmakuListAdapter() {
}

DanmakuVision* DanmakuListAdapter::OnCreateDanmakuVision(int position) {
    return new DanmakuVision();
}

void DanmakuListAdapter::OnBindDanmakuVision(DanmakuVision* vision, int position) {
    auto renderer = renderers_[position];
    vision->SetRenderingParams(rendering_params_);
    vision->SetRenderer(renderer);
}

int DanmakuListAdapter::GetItemCount() const {
    return static_cast<int>(renderers_.size());
}

void DanmakuListAdapter::AddDanmakuVision(const DanmakuRendererPtr& dp) {
    renderers_.push_back(dp);
}

void DanmakuListAdapter::RemoveDanmakuVision(int position) {
    if (position < static_cast<int>(renderers_.size())) {
        renderers_.erase(renderers_.begin() + position);
    }
}

void DanmakuListAdapter::RemoveDanmakuVisions(int start, int count) {
    if (count <= 0 || renderers_.empty()) {
        return;
    }

    start = std::max(start, 0);
    start = std::min<int>(start, renderers_.size() - 1);
    int end = std::min<int>(start + count, renderers_.size());

    renderers_.erase(renderers_.begin() + start, renderers_.begin() + end);
}

std::vector<DanmakuRendererPtr>& DanmakuListAdapter::GetItems() {
    return renderers_;
}

void DanmakuListAdapter::Clear() {
    renderers_.clear();
}

bool DanmakuListAdapter::RemoveDanmakuVisionByID(uint64_t id)
{
    auto it = std::find_if(renderers_.begin(), renderers_.end(), 
        [id](const DanmakuRendererPtr& ptr)
        {
            return ptr->GetData()->GetId() == id;
        });

    if (it != renderers_.end()) 
    {
        renderers_.erase(it);
        return true;
    }
    else 
    {
        return false;
    }
}

void DanmakuListAdapter::SetRenderingParams(const DanmakuRenderingParams& params) {
    rendering_params_ = params;
}

DanmakuRenderingParams DanmakuListAdapter::GetRenderingParams()
{
    return rendering_params_;
}

void DanmakuListAdapter::OnChildRecycled(DanmakuVision* vision) {
    DCHECK(vision);
    int pos = vision->GetPosition();
    DCHECK(pos >= 0 && pos < int(renderers_.size()));
    renderers_[pos]->Invalid();
}

}