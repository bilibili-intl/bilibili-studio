#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-colive", "en-US")

void register_colive_source();
void register_colive_video_encoder();
void register_colive_audio_encoder();
void register_colive_output();

extern "C"
{
    bool obs_module_load(void)
    {
        register_colive_source();
        register_colive_video_encoder();
        register_colive_audio_encoder();
        register_colive_output();
        return true;
    }

    void obs_module_unload(void)
    {
    }
};
