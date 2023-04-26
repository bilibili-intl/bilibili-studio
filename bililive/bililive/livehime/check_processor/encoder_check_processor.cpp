#include "bililive/bililive/livehime/check_processor/encoder_check_processor.h"

#include "obs/obs-studio/libobs/obs.h"
#include <Windows.h>

namespace EncoderCheckProcessor
{
    bool EncoderIsSupported(const std::string& encoder)
    {
        bool ret = false;
        __try
        {
            obs_output_t* record_output = nullptr;
            obs_encoder_t* video_encoder = nullptr;
            obs_encoder_t* audio_encoder = nullptr;
            video_encoder = obs_video_encoder_create(encoder.c_str(), "h264_streaming", nullptr, nullptr);
            if (!video_encoder)
            {
                return ret;
            }

            audio_encoder = obs_audio_encoder_create("ffmpeg_aac", "aac_streaming", nullptr, 0, nullptr);
            if (!audio_encoder)
            {
                return ret;
            }

            record_output = obs_output_create("ffmpeg_muxer", "file_output", nullptr, nullptr);
            if (!record_output)
            {
                return ret;
            }

            obs_data_t* video_settings = obs_data_create();
            obs_data_t* audio_settings = obs_data_create();
            obs_output_set_video_encoder(record_output, video_encoder);
            obs_output_set_audio_encoder(record_output, audio_encoder, 0);

            auto video_enc = obs_output_get_video_encoder(record_output);
            if (video_enc)
            {
                if (!obs_encoder_active(video_enc))
                {
                    obs_encoder_set_video(video_enc, obs_get_video());
                }
            }

            auto audio_enc = obs_output_get_audio_encoder(record_output, 0);

            if (!obs_encoder_active(audio_enc))
            {
                obs_encoder_set_audio(audio_enc, obs_get_audio());
            }

            if (obs_output_initialize_encoders(record_output, 0))
            {
                ret = true;
            }

            obs_output_release(record_output);
            obs_encoder_release(video_encoder);
            obs_encoder_release(audio_encoder);
            obs_data_release(video_settings);
            obs_data_release(audio_settings);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            blog(LOG_ERROR, "check encoder crash:%s",encoder.c_str());
            return false;
        }
        return ret;
    }
}   // namespace

