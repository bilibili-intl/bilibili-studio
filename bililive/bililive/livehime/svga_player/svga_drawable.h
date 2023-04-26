#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DRAWABLE_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DRAWABLE_H_

#include "bililive/bililive/livehime/svga_player/utils/svga_scale_info.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_video_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_dynamic_entity.h"
#include "bililive/bililive/livehime/svga_player/drawer/svga_canvas_drawer.h"


namespace svga {

    class SVGADrawable {
    public:
        explicit SVGADrawable(
            const std::shared_ptr<SVGAVideoEntity>& video_item);
        SVGADrawable(
            const std::shared_ptr<SVGAVideoEntity>& video_item,
            const std::shared_ptr<SVGADynamicEntity>& dynamic_item);

        bool setCleared(bool cleared);
        bool setCurrentFrame(int frame);
        void setScaleType(ScaleType type);

        int getCurrentFrame() const;

        void draw(gfx::Canvas* c, const gfx::Size& view_size);

    private:
        bool cleared_ = true;
        int cur_frame_ = 0;
        ScaleType scale_type_ = ScaleType::CENTER_INSIDE;

    public:
        std::shared_ptr<SVGAVideoEntity> video_item_;
        std::shared_ptr<SVGADynamicEntity> dynamic_item_;
        SVGACanvasDrawer drawer_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DRAWABLE_H_