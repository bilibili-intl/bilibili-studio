#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_MOVIE_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_MOVIE_ENTITY_H_

#include <map>

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"
#include "bililive/bililive/livehime/svga_player/proto/movie_params.h"
#include "bililive/bililive/livehime/svga_player/proto/sprite_entity.h"
#include "bililive/bililive/livehime/svga_player/proto/audio_entity.h"


namespace svga {

    class MovieEntity {
    public:
        static MovieEntity decode(protobuf::ProtoReader& reader);

        MovieEntity(
            const std::string& version, const MovieParams& params,
            const std::map<std::string, std::string>& images,
            const std::vector<SpriteEntity>& sprites, const std::vector<AudioEntity>& audios,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:string]
        std::string version_;

        // [2:embedded]
        MovieParams params_;

        // [3:string:bytes]
        std::map<std::string, std::string> images_;

        // [4:embedded repeat]
        std::vector<SpriteEntity> sprites_;

        // [5:embedded repeat]
        std::vector<AudioEntity> audios_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_MOVIE_ENTITY_H_