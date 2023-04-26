#include "bililive/bililive/livehime/svga_player/proto/transform.h"


namespace svga {

    Transform Transform::decode(protobuf::ProtoReader& reader) {
        float a = 0;
        float b = 0;
        float c = 0;
        float d = 0;
        float tx = 0;
        float ty = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&a); break;
                case 2: reader.getFloat(&b); break;
                case 3: reader.getFloat(&c); break;
                case 4: reader.getFloat(&d); break;
                case 5: reader.getFloat(&tx); break;
                case 6: reader.getFloat(&ty); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return Transform(a, b, c, d, tx, ty, succeeded);
    }

    Transform::Transform(
        float a, float b, float c, float d, float tx, float ty,
        bool available)
        : a_(a), b_(b), c_(c), d_(d), tx_(tx), ty_(ty), is_available_(available) {
    }

}
