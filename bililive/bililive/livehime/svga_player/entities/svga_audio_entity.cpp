#include "bililive/bililive/livehime/svga_player/entities/svga_audio_entity.h"


namespace svga {

    SVGAAudioEntity::SVGAAudioEntity(const AudioEntity& audio_item)
        : audio_key(audio_item.audio_key_),
          start_frame(audio_item.start_frame_),
          end_frame(audio_item.end_frame_),
          start_time(audio_item.start_time_),
          total_time(audio_item.total_time_) {}

}
