#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_SPRITE_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_SPRITE_ENTITY_H_

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"
#include "bililive/bililive/livehime/svga_player/proto/frame_entity.h"


namespace svga {

    class SpriteEntity {
    public:
        static SpriteEntity decode(protobuf::ProtoReader& reader);

        SpriteEntity(
            const std::string& img_key, const std::vector<FrameEntity>& frames,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:string]
        std::string img_key_;

        // [2:embedded repeat]
        std::vector<FrameEntity> frames_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_SPRITE_ENTITY_H_