#include <obs-module.h>


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-texture-renderer", "en-US")

void register_texture_source();

extern "C"
{
    bool obs_module_load(void)
    {
        register_texture_source();
        return true;
    }

    void obs_module_unload(void)
    {
    }
};
