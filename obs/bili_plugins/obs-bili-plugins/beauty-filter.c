#include <obs-module.h>
#include <graphics/image-file.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>

struct beauty_filter_data {
	obs_source_t                   *context;

	gs_effect_t                    *effect;
	gs_eparam_t                    *param_beauty_params;
	gs_eparam_t                    *param_step_offset;
    gs_eparam_t                    *param_lookup_table;

	struct vec4                    beauty_params;
	struct vec2                    step_offset;
    gs_image_file_t                lookup_table;
};

static const char* color_filters[] =
{
    "lkt_orig.png",
    "lkt_auto_enhance.png",
    "lkt_sakura.png",
    "lkt_ice_dessert.png",
    "lkt_blue_moon.png",
    "lkt_purple_hydrangea.png",
    "lkt_kazehaya_sawako.png",
    "lkt_firefly_forest.png",
    "lkt_kino_no_iro.png"
};

static const char *beauty_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("BeautyFilter");
}

static void *beauty_filter_create(obs_data_t *settings, obs_source_t *context)
{
	struct beauty_filter_data *filter = bzalloc(sizeof(*filter));
	char *effect_path = obs_module_file("beauty_filter.effect");

	filter->context = context;

	obs_enter_graphics();
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	obs_leave_graphics();

	bfree(effect_path);

	if (!filter->effect) {
		bfree(filter);
		return NULL;
	}

	filter->param_beauty_params = gs_effect_get_param_by_name(filter->effect,
		"beauty_params");
	filter->param_step_offset = gs_effect_get_param_by_name(filter->effect,
		"step_offset");
    filter->param_lookup_table = gs_effect_get_param_by_name(filter->effect,
        "lookup_table");

	obs_source_update(context, settings);
	return filter;
}

static void beauty_filter_destroy(void *data)
{
	struct beauty_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
    gs_image_file_free(&filter->lookup_table);
	obs_leave_graphics();

	bfree(filter);
}

static void beauty_filter_update(void *data, obs_data_t *settings)
{
	struct beauty_filter_data *filter = data;

	int blurlevel = (int)obs_data_get_int(settings, "old_blur_level");
    if (blurlevel < 0 || blurlevel > 100)
        blurlevel = 0;

    int lightenlevel = (int)obs_data_get_int(settings, "old_light_level");
    if (lightenlevel < 0 || lightenlevel > 100)
        lightenlevel = 0;

    //3-0
    float blur = 3.0f - blurlevel * 0.03f;

    //0-0.1
    float yThreshold = 0.0f + blurlevel * 0.0010f;

    //0-0.08
    float uvThreshold = 0.0f + blurlevel * 0.0008f;

    //0-1
    float lightLevel = lightenlevel / 100.0f;

    int color_filter = (int)obs_data_get_int(settings, "color_filter");
    if (color_filter < 0 || color_filter >= sizeof(color_filters) / sizeof(*color_filters))
        color_filter = 0;

    obs_enter_graphics();
    gs_image_file_free(&filter->lookup_table);
    obs_leave_graphics();

    char* lookup_table_file = obs_module_file(color_filters[color_filter]);
    if (lookup_table_file)
    {
        gs_image_file_init(&filter->lookup_table, lookup_table_file);
        bfree(lookup_table_file);
    }

    obs_enter_graphics();
    gs_image_file_init_texture(&filter->lookup_table);
    obs_leave_graphics();

    //ÆÁ±ÎµôÃÀ°×Ä¥Æ¤£¬ÔÝÊ±Ö»ÁôÂË¾µ
	vec4_set(&filter->beauty_params, blur,
									 yThreshold,
									 uvThreshold,
									 lightLevel);
    //vec4_set(&filter->beauty_params, 3,
    //    0,
    //    0,
    //    0);

}

static obs_properties_t *beauty_filter_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	return props;
}

static void beauty_filter_defaults(obs_data_t *settings)
{
	obs_data_set_int(settings, "beauty_level", 0);
}

static void beauty_filter_render(void *data, gs_effect_t *effect)
{
	struct beauty_filter_data *filter = data;

	obs_source_t* parent_source = obs_filter_get_parent(filter->context);

	uint32_t base_width = obs_source_get_base_width(parent_source);
	uint32_t base_height = obs_source_get_base_height(parent_source);

    if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
        OBS_ALLOW_DIRECT_RENDERING))
        return;

	vec2_set(&filter->step_offset, 1.0f / base_width, 1.0f / base_height);

	gs_effect_set_vec4(filter->param_beauty_params, &filter->beauty_params);
	gs_effect_set_vec2(filter->param_step_offset, &filter->step_offset);
    gs_effect_set_texture(filter->param_lookup_table, filter->lookup_table.texture);

	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

	UNUSED_PARAMETER(effect);
}

struct obs_source_info beauty_filter = {
	.id = "beauty_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = beauty_filter_get_name,
	.create = beauty_filter_create,
	.destroy = beauty_filter_destroy,
	.update = beauty_filter_update,
	.get_properties = beauty_filter_properties,
	.get_defaults = beauty_filter_defaults,
	.video_render = beauty_filter_render,
};
