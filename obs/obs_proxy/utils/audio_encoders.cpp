#include "obs/obs_proxy/utils/audio_encoders.h"

#include <algorithm>

#include "base/logging.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_piece.h"

#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/utils/obs_type_scope_wrapper.h"

namespace {

using obs_proxy::BitrateMap;

struct int_property_tag {};

const char kSampleRatePropertyName[] = "samplerate";
const char kBitRatePropertyName[] = "bitrate";
const base::StringPiece kPredefinedEncoders[] {
    "ffmpeg_aac",
    "mf_aac",
    "libfdk_aac",
    "CoreAudio_AAC"
};

const base::StringPiece kTabooEncoders[] {
    "colive_aenc",
    "rtc_colive_aenc"
};

const base::StringPiece kFallbackEncoder = kPredefinedEncoders[0];
const base::StringPiece kAACCodec = "AAC";

bool g_bitrate_table_populated = false;
BitrateMap g_bitrate_table;

void HandleProperty(obs_property_t* property, const char *encoder_id, int_property_tag)
{
    const int max_count = obs_property_int_max(property);
    const int step = obs_property_int_step(property);

    for (int i = obs_property_int_min(property); i <= max_count; i += step)
    {
        g_bitrate_table[i] = encoder_id;
    }
}

void HandleSampleRate(obs_property_t* properties, const char* encoder_id)
{
    obs_proxy::scoped_obs_data data(obs_encoder_defaults(encoder_id), &obs_data_release);
    if (!data)
    {
        LOG(WARNING) << "Failed to obtain defaults data for encoder " << encoder_id;
        return;
    }

    uint32_t sample_rate = obs_proxy::GetPrefs()->GetInteger(prefs::kAudioSampleRate);
    obs_data_set_int(data.get(), kSampleRatePropertyName, sample_rate);
    obs_property_modified(properties, data.get());
}

void HandleEncoderProperties(const char* encoder_id)
{
    obs_proxy::scoped_obs_property encoder_properties(obs_get_encoder_properties(encoder_id),
                                                      &obs_properties_destroy);
    if (!encoder_properties)
    {
        LOG(WARNING) << "Failed to obtain properties for encoder " << encoder_id;
        return;
    }

    obs_property_t *sample_rate = obs_properties_get(encoder_properties.get(), kSampleRatePropertyName);
    if (sample_rate) {
        HandleSampleRate(sample_rate, encoder_id);
    }

    obs_property_t *bitrate = obs_properties_get(encoder_properties.get(), kBitRatePropertyName);
    obs_property_type type = obs_property_get_type(bitrate);
    switch (type)
    {
        case OBS_PROPERTY_INT:
            HandleProperty(bitrate, encoder_id, int_property_tag());
            break;

        default:
            NOTREACHED();
    }
}

void PopulateBitrateTable()
{
    // Fallback encoder case.
    HandleEncoderProperties(kFallbackEncoder.data());

    // Encoders from plugin/modules.
    const char* encoder_id = nullptr;
    for (size_t i = 0; obs_enum_encoder_types(i, &encoder_id); i++)
    {
        // TOOD: Use a better approach later
        if (std::any_of(std::begin(kTabooEncoders), std::end(kTabooEncoders),
            [encoder_id](const base::StringPiece& encoder) {
            return encoder == encoder_id;
        }))
        {
            continue;
        }

        if (std::any_of(std::begin(kPredefinedEncoders), std::end(kPredefinedEncoders),
                        [encoder_id](const base::StringPiece& encoder) {
                            return encoder == encoder_id;
                        }))
        {
            continue;
        }

        if (kAACCodec != obs_get_encoder_codec(encoder_id))
        {
            continue;
        }

        HandleEncoderProperties(encoder_id);
    }

    // Predefiend encoders.
    for (const auto& encoder : kPredefinedEncoders)
    {
        if (encoder != kFallbackEncoder && kAACCodec == obs_get_encoder_codec(encoder.data()))
        {
            HandleEncoderProperties(encoder.data());
        }
    }

    g_bitrate_table_populated = true;
    DCHECK(!g_bitrate_table.empty()) << "Could not enumerate any AAC encoder bitrate";
}

int FindClosestAvailableAudioBitrate(int bitrate)
{
    const int kFallbackBitrate = 192;
    const int kSentinelBitrate = 10000;
    const auto& bitrate_table = obs_proxy::GetAudioEncoderBitrateTable();
    int prev = 0;
    int next = kSentinelBitrate;

    for (const auto& mapping : bitrate_table)
    {
        if (mapping.first == bitrate)
        {
            next = mapping.first;
            break;
        }
        else if (bitrate < mapping.first && mapping.first < next)
        {
            next = mapping.first;
            break;
        }
        else if (prev < mapping.first && mapping.first < bitrate)
        {
            prev = mapping.first;
        }
    }

    if (next != kSentinelBitrate)
    {
        return next;
    }

    if (prev != 0)
    {
        return prev;
    }

    return kFallbackBitrate;
}

}   // namespace

namespace obs_proxy {

const BitrateMap& GetAudioEncoderBitrateTable()
{
    if (!g_bitrate_table_populated) {
        PopulateBitrateTable();
    }

    return g_bitrate_table;
}

int GetProperAudioBitrate()
{
    int configured_bitrate = GetPrefs()->GetInteger(prefs::kAudioBitRate);
    return FindClosestAvailableAudioBitrate(configured_bitrate);
}

std::string GetEncoderIDForBitrate(int bitrate)
{
    const auto& bitrate_table = obs_proxy::GetAudioEncoderBitrateTable();
    auto it = bitrate_table.find(bitrate);
    return it == bitrate_table.end() ? std::string() : it->second;
}

}   // namespace obs_proxy