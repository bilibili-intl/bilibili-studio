#include <obs-module.h>
#include "obs-filters-config.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-bili-filters", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Bilibili filters for OBS";
}

extern struct obs_source_info beauty_filter;
extern struct obs_source_info forcemono_filter;
extern struct obs_source_info reverb_filter;
extern struct obs_source_info preview_filter;
extern struct obs_source_info mp4_motion_filter;
extern struct obs_source_info mic_record_filter;
bool obs_module_load(void)
{
    obs_register_source(&beauty_filter);
    obs_register_source(&forcemono_filter);
    obs_register_source(&reverb_filter);
    obs_register_source(&mp4_motion_filter);
	  obs_register_source(&mic_record_filter);
	  return true;
}
