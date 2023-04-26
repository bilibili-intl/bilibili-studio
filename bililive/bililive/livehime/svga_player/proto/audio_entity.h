#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_AUDIO_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_AUDIO_ENTITY_H_

#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"


namespace svga {

    class AudioEntity {
    public:
        static AudioEntity decode(protobuf::ProtoReader& reader);

        AudioEntity(
            const std::string& audio_key, int start_frame, int end_frame,
            int start_time, int total_time, bool available = false);

        bool isAvailable() const { return is_available_; }

        // [1:string]
        std::string audio_key_;

        // [2:int32]
        int start_frame_;

        // [3:int32]
        int end_frame_;

        // [4:int32]
        int start_time_;

        // [5:int32]
        int total_time_;

    private:
        bool is_available_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTO_AUDIO_ENTITY_H_