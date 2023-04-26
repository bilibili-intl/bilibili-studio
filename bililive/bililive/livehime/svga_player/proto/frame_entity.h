#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_FRAME_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_FRAME_ENTITY_H_

#include <vector>

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"
#include "bililive/bililive/livehime/svga_player/proto/layout.h"
#include "bililive/bililive/livehime/svga_player/proto/transform.h"
#include "bililive/bililive/livehime/svga_player/proto/shape_entity.h"


namespace svga {

    class FrameEntity {
    public:
        static FrameEntity decode(protobuf::ProtoReader& reader);

        FrameEntity(
            float alpha, const Layout& layout, const Transform& transform,
            const std::string& clip_path, const std::vector<ShapeEntity>& shapes,
            bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:float]
        float alpha_;

        // [2:embedded]
        Layout layout_;

        // [3:embedded]
        Transform transform_;

        // [4:string]
        std::string clip_path_;

        // [5:embedded repeat]
        std::vector<ShapeEntity> shapes_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_FRAME_ENTITY_H_