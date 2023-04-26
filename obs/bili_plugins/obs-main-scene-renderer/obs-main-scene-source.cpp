#include <obs-module.h>


struct msr_source {
    obs_source_t* source;

    uint32_t cx;
    uint32_t cy;
};

static void msr_source_defaults(obs_data_t* settings) {
}

static obs_properties_t* msr_source_getproperties(void* data) {
    obs_properties_t *props = obs_properties_create();
    return props;
}

static uint32_t msr_source_getwidth(void* data) {
    struct msr_source* context = (struct msr_source*)data;
    return context->cx;
}

static uint32_t msr_source_getheight(void* data) {
    struct msr_source* context = (struct msr_source*)data;
    return context->cy;
}

static void msr_source_update(void* data, obs_data_t* settings) {
}

static const char* msr_source_getname(void* unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("OBSMainSceneRendererSource");
}

static void* msr_source_create(obs_data_t* settings, obs_source_t* source) {
    struct msr_source* context
        = (struct msr_source*)bzalloc(sizeof(struct msr_source));
    context->source = source;

    gs_texture* main_scene_tex = obs_get_main_texture();
    if (main_scene_tex) {
        obs_enter_graphics();
        context->cx = gs_texture_get_width(main_scene_tex);
        context->cy = gs_texture_get_height(main_scene_tex);
        obs_leave_graphics();
    }

    msr_source_update(context, settings);
    return context;
}

static void msr_source_destroy(void* data) {
}

static void msr_source_activate(void* data) {
}

static void msr_source_deactivate(void* data) {
}

static void msr_source_show(void* data) {
}

static void msr_source_hide(void* data) {
}

static void msr_source_render(void* data, gs_effect_t* effect) {
    struct msr_source* context = (struct msr_source*)data;
    gs_texture* main_scene_tex = obs_get_main_texture();
    if (!main_scene_tex) {
        return;
    }

    context->cx = gs_texture_get_width(main_scene_tex);
    context->cy = gs_texture_get_height(main_scene_tex);

    gs_effect_set_texture(
        gs_effect_get_param_by_name(effect, "image"),
        main_scene_tex);
    gs_draw_sprite(main_scene_tex, 0, context->cx, context->cy);
}

void register_main_scene_renderer_source()
{
    struct obs_source_info source;
    memset(&source, 0, sizeof(source));

    source.id = "main_scene_renderer_source";
    source.type = OBS_SOURCE_TYPE_INPUT;
    source.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_DO_NOT_DUPLICATE;
    source.get_name = msr_source_getname;
    source.create = msr_source_create;
    source.destroy = msr_source_destroy;
    source.get_defaults = msr_source_defaults;
    source.get_properties = msr_source_getproperties;
    source.get_width = msr_source_getwidth;
    source.get_height = msr_source_getheight;
    source.activate = msr_source_activate;
    source.deactivate = msr_source_deactivate;
    source.show = msr_source_show;
    source.hide = msr_source_hide;
    source.update = msr_source_update;
    source.video_render = msr_source_render;
    obs_register_source_s(&source, sizeof(source));
}