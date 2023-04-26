#include <obs-module.h>

#include "reverb/reverb.h"

#define do_log(level, format, ...) \
	blog(level, "[reverb filter: '%s'] " format, \
			obs_source_get_name(rd->context), ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)

static const char* prop_room_size = "room-size";
static const float default_room_size = 0.15f;
static const char* prop_damping = "damping";
static const float default_damping = 0.9f;
static const char* prop_wet_level = "wet-level";
static const float default_wet_level = 0.3f;
static const char* prop_dry_level = "dry-level";
static const float default_dry_level = 0.7f;
static const char* prop_width = "width";
static const float default_width = 1.0f;
static const char* prop_freeze_mode = "freeze-mode";
static const float default_freeze_mode = 0.0f;
static const char* prop_enabled = "enabled";
static const bool default_enabled = false;

struct reverb_data {
	obs_source_t *context;

    int sample_rate;
    int channels;
    struct Reverb* reverb;
    bool is_enabled;

    struct ReverbParameters reverb_params;
};

static const char *reverb_name(void *unused)
{
	return obs_module_text("Reverb filter");
}

static void reverb_destroy(void *data)
{
    struct reverb_data *rd = data;
    if (rd->reverb)
        bili_reverb_destroy(rd->reverb);
	bfree(rd);
}

static void reverb_update(void *data, obs_data_t *s)
{
    struct reverb_data *rd = data;

	if (s)
	{
        rd->is_enabled = obs_data_get_bool(s, prop_enabled);
        rd->reverb_params.roomSize = (float)obs_data_get_double(s, prop_room_size);
        rd->reverb_params.damping = (float)obs_data_get_double(s, prop_damping);
        rd->reverb_params.wetLevel = (float)obs_data_get_double(s, prop_wet_level);
        rd->reverb_params.dryLevel = (float)obs_data_get_double(s, prop_dry_level);
        rd->reverb_params.width = (float)obs_data_get_double(s, prop_width);
        rd->reverb_params.freezeMode = (float)obs_data_get_double(s, prop_freeze_mode);

        bili_reverb_set_parameters(rd->reverb, &rd->reverb_params);
	}
}

static void *reverb_create(obs_data_t *settings, obs_source_t *filter)
{
    struct reverb_data *rd = bzalloc(sizeof(*rd));
    rd->context = filter;
    rd->reverb = bili_reverb_create();

    rd->sample_rate = audio_output_get_sample_rate(obs_get_audio());
    rd->channels = audio_output_get_channels(obs_get_audio());

    bili_reverb_set_sample_rate(rd->reverb, rd->sample_rate);

    rd->reverb_params.roomSize = default_room_size;
    rd->reverb_params.damping = default_damping;
    rd->reverb_params.wetLevel = default_wet_level;
    rd->reverb_params.dryLevel = default_dry_level;
    rd->reverb_params.width = default_width;
    rd->reverb_params.freezeMode = default_freeze_mode;

    reverb_update(rd, settings);
	return rd;
}

static struct obs_audio_data *reverb_filter_audio(void *data, struct obs_audio_data *audio)
{
    struct reverb_data *rd = data;

    if (rd->is_enabled)
    {
        if (rd->channels == 1 && audio->data[0])
        {
            bili_reverb_process(rd->reverb, (float*)audio->data[0], 0, audio->frames);
        }
        else if (rd->channels == 2 && audio->data[0] && audio->data[1])
        {
            bili_reverb_process(rd->reverb, (float*)audio->data[0], (float*)audio->data[1], audio->frames);
        }
    }

	return audio;
}

static void reverb_defaults(obs_data_t *s)
{
	if (s)
	{
#define SET_DEFAULT(name) obs_data_set_default_double(s, prop_##name, default_##name)
        SET_DEFAULT(room_size);
        SET_DEFAULT(damping);
        SET_DEFAULT(wet_level);
        SET_DEFAULT(dry_level);
        SET_DEFAULT(width);
        SET_DEFAULT(freeze_mode);
#undef SET_DEFAULT
        obs_data_set_default_bool(s, prop_enabled, default_enabled);
	}
}

static obs_properties_t *reverb_properties(void *data)
{
	obs_properties_t *ppts = obs_properties_create();
    obs_properties_add_bool(ppts, prop_enabled, "enabled");
    obs_properties_add_float_slider(ppts, prop_room_size, "room size", 0.0, 1.0, 0.01);
    obs_properties_add_float_slider(ppts, prop_damping, "damping", 0.0, 1.0, 0.01);
    obs_properties_add_float_slider(ppts, prop_wet_level, "wet level", 0.0, 1.0, 0.01);
    obs_properties_add_float_slider(ppts, prop_dry_level, "dry level", 0.0, 1.0, 0.01);
    obs_properties_add_float_slider(ppts, prop_width, "width", 0.0, 1.0, 0.01);
    obs_properties_add_float_slider(ppts, prop_freeze_mode, "freeze mode", 0.0, 1.0, 0.01);
	return ppts;
}

struct obs_source_info reverb_filter = {
	.id = "reverb_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = reverb_name,
	.create = reverb_create,
	.destroy = reverb_destroy,
	.update = reverb_update,
	.filter_audio = reverb_filter_audio,
	.get_defaults = reverb_defaults,
	.get_properties = reverb_properties,
};
