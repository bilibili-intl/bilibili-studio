#include <obs-module.h>

struct unpremultiply_data {
    obs_source_t                   *context;

    gs_effect_t                    *effect;
};

static const char *unpremultiply_filter_get_name(void *unused)
{
    return "UnpremultiplyFilter";
}

static void *unpremultiply_filter_create(obs_data_t *settings, obs_source_t *context)
{
    struct unpremultiply_data *filter = bzalloc(sizeof(*filter));
    char *effect_path = obs_module_file("unpremultiply.effect");

    filter->context = context;

    obs_enter_graphics();
    filter->effect = gs_effect_create_from_file(effect_path, NULL);
    obs_leave_graphics();

    bfree(effect_path);

    if (!filter->effect) {
        bfree(filter);
        return NULL;
    }

    return filter;
}

static void unpremultiply_filter_destroy(void *data)
{
    struct unpremultiply_data *filter = data;

    obs_enter_graphics();
    gs_effect_destroy(filter->effect);
    obs_leave_graphics();

    bfree(filter);
}

static void unpremultiply_filter_update(void *data, obs_data_t *settings)
{
}

static obs_properties_t *unpremultiply_filter_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();
    return props;
}

static void unpremultiply_filter_defaults(obs_data_t *settings)
{
}

static void unpremultiply_filter_render(void *data, gs_effect_t *effect)
{
    struct unpremultiply_data *filter = data;

    obs_source_t* parent_source = obs_filter_get_parent(filter->context);

    if (!obs_source_process_filter_begin(filter->context, GS_BGRA,
        OBS_ALLOW_DIRECT_RENDERING))
        return;

    obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

    UNUSED_PARAMETER(effect);
}

struct obs_source_info unpremultiply_filter = {
    .id = "unpremultiply_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = unpremultiply_filter_get_name,
    .create = unpremultiply_filter_create,
    .destroy = unpremultiply_filter_destroy,
    .update = unpremultiply_filter_update,
    .get_properties = unpremultiply_filter_properties,
    .get_defaults = unpremultiply_filter_defaults,
    .video_render = unpremultiply_filter_render,
};
