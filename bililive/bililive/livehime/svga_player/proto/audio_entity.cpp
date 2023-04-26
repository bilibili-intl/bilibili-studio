#include "bililive/bililive/livehime/svga_player/proto/audio_entity.h"


namespace svga {

    AudioEntity AudioEntity::decode(protobuf::ProtoReader& reader) {
        std::string audio_key;
        int start_frame = 0;
        int end_frame = 0;
        int start_time = 0;
        int total_time = 0;

        bool succeeded = reader.beginEmbedded();
        if (succeeded) {
            for (;;) {
                int tag = reader.nextTag();
                if (tag == -1) {
                    break;
                }

                switch (tag) {
                case 1: reader.getBytes(&audio_key); break;
                case 2: reader.getInt32(&start_frame); break;
                case 3: reader.getInt32(&end_frame); break;
                case 4: reader.getInt32(&start_time); break;
                case 5: reader.getInt32(&total_time); break;
                default: reader.skipValue(); break;
                }
            }

            reader.endEmbedded();
        }

        return AudioEntity(
            audio_key, start_frame, end_frame, start_time, total_time, succeeded);
    }

    AudioEntity::AudioEntity(
        const std::string& audio_key, int start_frame, int end_frame,
        int start_time, int total_time, bool available)
        : audio_key_(audio_key), start_frame_(start_frame), end_frame_(end_frame),
          start_time_(start_time), total_time_(total_time), is_available_(available) {}

}
