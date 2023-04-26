#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SPRITE_FRAME_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SPRITE_FRAME_ENTITY_H_

#include "base/values.h"

#include "ui/gfx/rect.h"

#include "bililive/bililive/livehime/svga_player/proto/frame_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_path_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_video_shape_entity.h"


namespace svga {

    class SVGAVideoSpriteFrameEntity {
    public:
        explicit SVGAVideoSpriteFrameEntity(const Value& obj);
        explicit SVGAVideoSpriteFrameEntity(const FrameEntity& obj);


        double alpha_;
        gfx::RectF layout_;
        SkMatrix transform_;
        std::shared_ptr<SVGAPathEntity> mask_path_;
        std::vector<std::shared_ptr<SVGAVideoShapeEntity>> shapes_;
    };

}

#endif   // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SPRITE_FRAME_ENTITY_H_