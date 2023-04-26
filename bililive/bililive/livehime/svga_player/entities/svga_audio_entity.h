#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_AUDIO_ENTITY_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_AUDIO_ENTITY_H_

#include <string>

#include "bililive/bililive/livehime/svga_player/proto/audio_entity.h"


namespace svga {

    class SVGAAudioEntity {
    public:
        std::string audio_key;
        int start_frame;
        int end_frame;
        int start_time;
        int total_time;

        int sound_id = -1;
        int play_id = -1;
        std::wstring path;

        explicit SVGAAudioEntity(const AudioEntity& audio_item);
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_ENTITIES_SVGA_AUDIO_ENTITY_H_