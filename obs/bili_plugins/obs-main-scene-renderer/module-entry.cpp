#include <obs-module.h>


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-main-scene-renderer", "en-US")

void register_main_scene_renderer_source();

extern "C"
{
    bool obs_module_load(void)
    {
        register_main_scene_renderer_source();
        return true;
    }

    void obs_module_unload(void)
    {
    }
};
