#include "bililive/bililive/livehime/svga_player/proto/sprite_entity.h"


namespace svga {

    SpriteEntity SpriteEntity::decode(protobuf::ProtoReader& reader) {
        std::string img_key;
        std::vector<FrameEntity> frames;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getBytes(&img_key); break;
                case 2: frames.push_back(FrameEntity::decode(reader)); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return SpriteEntity(img_key, frames, succeeded);
    }

    SpriteEntity::SpriteEntity(
        const std::string& img_key, const std::vector<FrameEntity>& frames,
        bool available)
        : img_key_(img_key), frames_(frames), is_available_(available) {}

}