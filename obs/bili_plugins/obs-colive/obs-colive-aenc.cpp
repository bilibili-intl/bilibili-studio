#include <obs-module.h>
#include <util/base.h>
#include <assert.h>
#include "colive-service.h"
#include "avdata-util.h"

#define do_log(level, format, ...) \
	blog(level, "[agora audio encoder: '%s'] " format, \
			enc->type, \
			obs_encoder_get_name(enc->encoder), \
			##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG,   format, ##__VA_ARGS__)

using namespace bililive::colive;

class colive_aenc_encoder {
public:
    obs_encoder_t* encoder;
    ColiveMediaWPtr agoraService;

    size_t samplerate;
    size_t channels;
    size_t colive_bitrate;
    size_t rtmp_bitrate;
};

static const char *colive_aenc_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("colive audio encoder");
}

static void colive_aenc_destroy(void *data)
{
    if (data)
    {
        colive_aenc_encoder *enc = (colive_aenc_encoder*)data;
        delete enc;
    }
}

static void *colive_aenc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
    ColiveMediaPtr p = GetColiveServiceFactory()->AquireMedia();
    if (!p)
    {
        blog(LOG_WARNING, "%s", "[colive aenc] fail to create agora-aenc before colive service created");
        return nullptr;
    }

    if (!settings)
    {
        blog(LOG_WARNING, "%s", "[colive aenc] fail to create agora-aenc without settings");
        return nullptr;
    }

    size_t colive_bitrate = (size_t)obs_data_get_int(settings, "colive_bitrate");
    size_t rtmp_bitrate = (size_t)obs_data_get_int(settings, "rtmp_bitrate");
    if (colive_bitrate == 0 || rtmp_bitrate == 0)
    {
        blog(LOG_WARNING, "%s", "[colive aenc] not both bitrates specified.");
        return nullptr;
    }

    audio_t* audioOutput = obs_get_audio();
    if (!audioOutput)
    {
        blog(LOG_WARNING, "%s", "[colive aenc] no audio output.");
        return nullptr;
    }

    size_t samplerate = audio_output_get_sample_rate(audioOutput);
    size_t channels = audio_output_get_channels(audioOutput);

    colive_aenc_encoder *enc = new colive_aenc_encoder();
    enc->encoder = encoder;
    enc->agoraService = p;

    enc->samplerate = samplerate;
    enc->channels = channels;
    enc->colive_bitrate = colive_bitrate;
    enc->rtmp_bitrate = rtmp_bitrate;

    p->SetAudioParam(enc->channels, enc->samplerate, enc->colive_bitrate, enc->rtmp_bitrate);

    blog(LOG_INFO, "%s", "[colive aenc] agora-aenc created.");

    return enc;
}

static bool colive_aenc_encode(void *data, struct encoder_frame *frame,
struct encoder_packet *packet, bool *received_packet)
{
    colive_aenc_encoder *enc = (colive_aenc_encoder*)data;
    ColiveMediaPtr p = enc->agoraService.lock();
    if (!p)
        return false;

    std::unique_ptr<IColiveMedia::AFrame> af;
    if (!FromOBSFrame(af, frame, enc->samplerate, enc->channels))
        return false;

    af->presentationTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(ColiveClock::now().time_since_epoch()).count();
    p->PushLocalAudio(std::move(af));
    return true;
}

static void colive_aenc_defaults(obs_data_t *settings)
{
}

static obs_properties_t *colive_aenc_properties(void *unused)
{
    UNUSED_PARAMETER(unused);

    obs_properties_t *props = obs_properties_create();
    obs_properties_add_int(props, "colive_bitrate", "colive_bitrate", 32, 320, 32);
    obs_properties_add_int(props, "rtmp_bitrate", "rtmp_bitrate", 32, 320, 32);
    return props;
}

static bool colive_aenc_extra_data(void *data, uint8_t **extra_data, size_t *size)
{
    colive_aenc_encoder *enc = (colive_aenc_encoder*)data;
    return true;
}

static void colive_aenc_audio_info(void *data, struct audio_convert_info *info)
{
    colive_aenc_encoder *enc = (colive_aenc_encoder*)data;

    if (enc->channels == 1)
        info->speakers = SPEAKERS_MONO;
    else if (enc->channels == 2)
        info->speakers = SPEAKERS_STEREO;
    else
        assert(false);

    info->format = AUDIO_FORMAT_16BIT;
    info->samples_per_sec = enc->samplerate;
}

static size_t colive_aenc_frame_size(void *data)
{
    colive_aenc_encoder *enc = (colive_aenc_encoder*)data;
    return 1024;
}

void register_colive_audio_encoder()
{
    struct obs_encoder_info colive_aenc_info = {};
    colive_aenc_info.id = "colive_aenc";
    colive_aenc_info.type = OBS_ENCODER_AUDIO;
    colive_aenc_info.codec = "AAC";
    colive_aenc_info.get_name = colive_aenc_getname;
    colive_aenc_info.create = colive_aenc_create;
    colive_aenc_info.destroy = colive_aenc_destroy;
    colive_aenc_info.encode = colive_aenc_encode;
    colive_aenc_info.get_frame_size = colive_aenc_frame_size;
    colive_aenc_info.get_defaults = colive_aenc_defaults;
    colive_aenc_info.get_properties = colive_aenc_properties;
    colive_aenc_info.get_extra_data = colive_aenc_extra_data;
    colive_aenc_info.get_audio_info = colive_aenc_audio_info;

    obs_register_encoder(&colive_aenc_info);
}
