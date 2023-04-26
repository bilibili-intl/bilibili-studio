#include "bililive/bililive/livehime/svga_player/svga_drawable.h"


namespace svga {

    SVGADrawable::SVGADrawable(const std::shared_ptr<SVGAVideoEntity>& video_item)
        : video_item_(video_item),
          drawer_(video_item_, {}) {}

    SVGADrawable::SVGADrawable(
        const std::shared_ptr<SVGAVideoEntity>& video_item,
        const std::shared_ptr<SVGADynamicEntity>& dynamic_item)
        : video_item_(video_item), dynamic_item_(dynamic_item),
          drawer_(video_item_, dynamic_item)
    {}

    bool SVGADrawable::setCleared(bool cleared) {
        if (cleared_ == cleared) {
            return false;
        }

        cleared_ = cleared;
        return true;
    }

    bool SVGADrawable::setCurrentFrame(int frame) {
        if (cur_frame_ == frame) {
            return false;
        }

        cur_frame_ = frame;
        return true;
    }

    void SVGADrawable::setScaleType(ScaleType type) {
        scale_type_ = type;
    }

    int SVGADrawable::getCurrentFrame() const {
        return cur_frame_;
    }

    void SVGADrawable::draw(gfx::Canvas* c, const gfx::Size& view_size) {
        if (cleared_) {
            return;
        }
        if (c) {
            drawer_.drawFrame(c, view_size, cur_frame_, scale_type_);
        }
    }

}