#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_ADAPTER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_ADAPTER_H_

#include <vector>

#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_list.h"


namespace dmkhime {

class DanmakuVision;

class DanmakuListAdapter : public ListItemRecycledListener {
public:
    DanmakuListAdapter();

    virtual DanmakuVision* OnCreateDanmakuVision(int position);
    virtual void OnBindDanmakuVision(DanmakuVision* vision, int position);
    virtual int GetItemCount() const;

    virtual void AddDanmakuVision(const DanmakuRendererPtr& dp);
    virtual void RemoveDanmakuVision(int position);
    virtual void RemoveDanmakuVisions(int start, int count);
    virtual std::vector<DanmakuRendererPtr>& GetItems();
    virtual void Clear();

    virtual bool RemoveDanmakuVisionByID(uint64_t id);

    virtual void SetRenderingParams(const DanmakuRenderingParams& params);
    virtual DanmakuRenderingParams GetRenderingParams();

protected:
    // ListItemRecycledListener
    void OnChildRecycled(DanmakuVision* vision) override;

private:
    std::vector<DanmakuRendererPtr> renderers_;
    DanmakuRenderingParams rendering_params_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_ADAPTER_H_