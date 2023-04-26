#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_LAYOUT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_LAYOUT_H_

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"


namespace svga {

    class Layout {
    public:
        static Layout decode(protobuf::ProtoReader& reader);

        Layout(float x, float y, float width, float height, bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float x_;

        // [2:float]
        float y_;

        // [3:float]
        float width_;

        // [4:float]
        float height_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_LAYOUT_H_