#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_TRANSFORM_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_TRANSFORM_H_

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"


namespace svga {

    class Transform {
    public:
        static Transform decode(protobuf::ProtoReader& reader);

        Transform(
            float a, float b, float c, float d, float tx, float ty,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float a_;
        // [2:float]
        float b_;
        // [3:float]
        float c_;
        // [4:float]
        float d_;
        // [5:float]
        float tx_;
        // [6:float]
        float ty_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_TRANSFORM_H_