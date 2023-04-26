#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_MOVIE_PARAMS_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_MOVIE_PARAMS_H_

#include <string>

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"


namespace svga {

    class MovieParams {
    public:
        static MovieParams decode(protobuf::ProtoReader& reader);

        MovieParams(
            float viewbox_width, float viewbox_height, int fps, int frames,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float viewbox_width_ = 0;
        // [2:float]
        float viewbox_height_ = 0;
        // [3:int32]
        int fps_ = 0;
        // [4:int32]
        int frames_ = 0;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_MOVIE_PARAMS_H_