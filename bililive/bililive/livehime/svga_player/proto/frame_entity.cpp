#include "bililive/bililive/livehime/svga_player/proto/frame_entity.h"


namespace svga {

    FrameEntity FrameEntity::decode(protobuf::ProtoReader& reader) {
        float alpha = 0;
        Layout layout(0, 0, 0, 0);
        Transform transform(0, 0, 0, 0, 0, 0);
        std::string clip_path;
        std::vector<ShapeEntity> shapes;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&alpha); break;
                case 2: layout = Layout::decode(reader); break;
                case 3: transform = Transform::decode(reader); break;
                case 4: reader.getBytes(&clip_path); break;
                case 5: shapes.push_back(ShapeEntity::decode(reader));
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return FrameEntity(alpha, layout, transform, clip_path, shapes, succeeded);
    }

    FrameEntity::FrameEntity(
        float alpha, const Layout& layout, const Transform& transform,
        const std::string& clip_path, const std::vector<ShapeEntity>& shapes,
        bool available)
        : alpha_(alpha), layout_(layout), transform_(transform),
          clip_path_(clip_path), shapes_(shapes), is_available_(available) {}

}
