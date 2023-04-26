#include "bililive/bililive/livehime/svga_player/proto/movie_params.h"


namespace svga {

    MovieParams MovieParams::decode(protobuf::ProtoReader& reader) {
        float width = 0;
        float height = 0;
        int fps = 0;
        int frames = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getFloat(&width); break;
                case 2: reader.getFloat(&height); break;
                case 3: reader.getInt32(&fps); break;
                case 4: reader.getInt32(&frames); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return MovieParams(width, height, fps, frames, succeeded);
    }

    MovieParams::MovieParams(
        float viewbox_width, float viewbox_height, int fps, int frames,
        bool available)
        : viewbox_width_(viewbox_width),
          viewbox_height_(viewbox_height),
          fps_(fps),
          frames_(frames),
          is_available_(available)
    {}

}