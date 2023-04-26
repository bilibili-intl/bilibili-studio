#include <obs-module.h>

struct mp4_motion_data {
    obs_source_t    *context;

    gs_effect_t     *effect;
    gs_eparam_t     *param_video_size;
    gs_eparam_t     *param_real_frame;
    gs_eparam_t     *param_alpha_frame;

    struct vec2     video_size;
    struct vec4     real_frame;
    struct vec4     alpha_frame;
};

static const char *mp4_motion_filter_get_name(void *unused)
{
    return "MP4MotionFilter";
}

static void *mp4_motion_filter_create(obs_data_t *settings, obs_source_t *context)
{
    struct mp4_motion_data*filter = bzalloc(sizeof(*filter));
    char *effect_path = obs_module_file("mp4_motion_filter.effect");

    filter->context = context;

    char* error_msg;

    obs_enter_graphics();
    filter->effect = gs_effect_create_from_file(effect_path, &error_msg);
    obs_leave_graphics();

    bfree(effect_path);

    if (!filter->effect) {
        bfree(filter);
        return NULL;
    }

    filter->param_video_size = gs_effect_get_param_by_name(filter->effect,
        "video_size");
    filter->param_real_frame = gs_effect_get_param_by_name(filter->effect,
        "real_frame");
    filter->param_alpha_frame = gs_effect_get_param_by_name(filter->effect,
        "alpha_frame");

    obs_source_update(context, settings);
    return filter;
}

static void mp4_motion_filter_destroy(void *data)
{
    struct mp4_motion_data *filter = data;

    obs_enter_graphics();
    gs_effect_destroy(filter->effect);
    obs_leave_graphics();

    bfree(filter);
}

static void mp4_motion_filter_update(void *data, obs_data_t *settings)
{
    struct mp4_motion_data* filter = data;

    float width = (float)obs_data_get_int(settings, "video_width");
    float height = (float)obs_data_get_int(settings, "video_height");

    float rf_x = (float)obs_data_get_int(settings, "real_frame_x");
    float rf_y = (float)obs_data_get_int(settings, "real_frame_y");
    float rf_width = (float)obs_data_get_int(settings, "real_frame_width");
    float rf_height = (float)obs_data_get_int(settings, "real_frame_height");

    float af_x = (float)obs_data_get_int(settings, "alpha_frame_x");
    float af_y = (float)obs_data_get_int(settings, "alpha_frame_y");
    float af_width = (float)obs_data_get_int(settings, "alpha_frame_width");
    float af_height = (float)obs_data_get_int(settings, "alpha_frame_height");

    vec2_set(&filter->video_size, width, height);
    vec4_set(&filter->real_frame, rf_x, rf_y, rf_width, rf_height);
    vec4_set(&filter->alpha_frame, af_x, af_y, af_width, af_height);
}

static obs_properties_t *mp4_motion_filter_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();
    return props;
}

static void mp4_motion_filter_defaults(obs_data_t *settings)
{
}

static void mp4_motion_filter_render(void *data, gs_effect_t *effect)
{
    struct mp4_motion_data *filter = data;

    obs_source_t* parent_source = obs_filter_get_parent(filter->context);

    if (!obs_source_process_filter_begin(
        filter->context, GS_BGRA, OBS_ALLOW_DIRECT_RENDERING))
        return;

    gs_effect_set_vec2(filter->param_video_size, &filter->video_size);
    gs_effect_set_vec4(filter->param_real_frame, &filter->real_frame);
    gs_effect_set_vec4(filter->param_alpha_frame, &filter->alpha_frame);

    obs_source_process_filter_end(
        filter->context, filter->effect,
        filter->real_frame.z, filter->real_frame.w);

    UNUSED_PARAMETER(effect);
}

static uint32_t mp4_motion_filter_get_width(void* data)
{
    struct mp4_motion_data* filter = data;
    return (uint32_t)filter->real_frame.z;
}

static uint32_t mp4_motion_filter_get_height(void* data)
{
    struct mp4_motion_data* filter = data;
    return (uint32_t)filter->real_frame.w;
}

struct obs_source_info mp4_motion_filter = {
    .id = "mp4_motion_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = mp4_motion_filter_get_name,
    .create = mp4_motion_filter_create,
    .destroy = mp4_motion_filter_destroy,
    .update = mp4_motion_filter_update,
    .get_properties = mp4_motion_filter_properties,
    .get_defaults = mp4_motion_filter_defaults,
    .video_render = mp4_motion_filter_render,
    .get_width = mp4_motion_filter_get_width,
    .get_height = mp4_motion_filter_get_height
};
