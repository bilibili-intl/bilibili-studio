#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_DRAWER_SVGA_DRAWER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_DRAWER_SVGA_DRAWER_H_

#include "ui/gfx/canvas.h"

#include "bililive/bililive/livehime/svga_player/entities/svga_video_sprite_frame_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_video_entity.h"
#include "bililive/bililive/livehime/svga_player/utils/svga_scale_info.h"


namespace svga {

    class SVGADrawer {
    public:
        class SVGADrawerSprite {
        public:
            SVGADrawerSprite(
                const std::string& key,
                const std::shared_ptr<SVGAVideoSpriteFrameEntity>& frame)
                : img_key(key), frame_entity(frame) {}

            std::string img_key;
            std::shared_ptr<SVGAVideoSpriteFrameEntity> frame_entity;
        };

        explicit SVGADrawer(const std::shared_ptr<SVGAVideoEntity>& video_item);
        virtual ~SVGADrawer() = default;

        void requestFrameSprites(
            int frame_index, std::vector<SVGADrawerSprite>* out);

        virtual void drawFrame(
            gfx::Canvas* c, const gfx::Size& view_size, int frame_index, ScaleType scale_type);

    protected:
        SVGAScaleInfo scale_info_;
        std::shared_ptr<SVGAVideoEntity> video_item_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_DRAWER_SVGA_DRAWER_H_