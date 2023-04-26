#include "bililive/bililive/livehime/svga_player/drawer/svga_drawer.h"


namespace svga {

    SVGADrawer::SVGADrawer(const std::shared_ptr<SVGAVideoEntity>& video_item)
        : video_item_(video_item) {
    }

    void SVGADrawer::requestFrameSprites(
        int frame_index, std::vector<SVGADrawerSprite>* out)
    {
        for (const auto& s : video_item_->sprites_) {
            if (frame_index >= 0 && frame_index < int(s.frames_.size())) {
                if (s.frames_[frame_index]->alpha_ > 0.0) {
                    out->push_back(SVGADrawerSprite(s.img_key_, s.frames_[frame_index]));
                }
            }
        }
    }

    void SVGADrawer::drawFrame(
        gfx::Canvas* c, const gfx::Size& view_size, int frame_index, ScaleType scale_type)
    {
        scale_info_.performScaleType(
            view_size.width(), view_size.height(),
            video_item_->video_size_.width(), video_item_->video_size_.height(), scale_type);
    }

}