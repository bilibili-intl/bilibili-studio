#include "bililive/bililive/livehime/svga_player/proto/layout.h"


namespace svga {

    Layout Layout::decode(protobuf::ProtoReader& reader) {
        float x = 0;
        float y = 0;
        float width = 0;
        float height = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&x); break;
                case 2: reader.getFloat(&y); break;
                case 3: reader.getFloat(&width); break;
                case 4: reader.getFloat(&height); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return Layout(x, y, width, height, succeeded);
    }

    Layout::Layout(float x, float y, float width, float height, bool available)
        : x_(x), y_(y), width_(width), height_(height), is_available_(available) {}

}
