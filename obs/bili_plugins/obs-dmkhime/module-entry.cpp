#include <obs-module.h>

#include "dmkhime-tex-service.h"


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-dmkhime", "en-US")

void register_dmkhime_source();


extern "C"
{
    bool obs_module_load(void)
    {
        register_dmkhime_source();
        bililive::DmkhimeTexService::Create();
        return true;
    }

    void obs_module_unload(void)
    {
        bililive::DmkhimeTexService::Destroy();
    }
};
