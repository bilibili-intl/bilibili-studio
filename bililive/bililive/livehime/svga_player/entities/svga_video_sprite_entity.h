#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SPRITE_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SPRITE_ENTITY_H_

#include "base/values.h"

#include "bililive/bililive/livehime/svga_player/proto/sprite_entity.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_video_sprite_frame_entity.h"


namespace svga {

    class SVGAVideoSpriteEntity {
    public:
        explicit SVGAVideoSpriteEntity(const Value& obj);
        explicit SVGAVideoSpriteEntity(const SpriteEntity& obj);

        std::string img_key_;
        std::vector<std::shared_ptr<SVGAVideoSpriteFrameEntity>> frames_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_VIDEO_SPRITE_ENTITY_H_