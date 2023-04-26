#ifndef OBS_OBS_PROXY_APP_OBS_PROXY_EXPORT_H_
#define OBS_OBS_PROXY_APP_OBS_PROXY_EXPORT_H_

#if defined(COMPONENT_BUILD)

#if defined(OBS_PROXY_IMPLEMENTATION)
#define OBS_PROXY_EXPORT __declspec(dllexport)
#else
#define OBS_PROXY_EXPORT __declspec(dllimport)
#endif  // defined(OBS_PROXY_IMPLEMENTATION)

#else   // defined(COMPONENT_BUILD)

#define OBS_PROXY_EXPORT

#endif  // defined(COMPONENT_BUILD)

#endif  // OBS_OBS_PROXY_APP_OBS_PROXY_EXPORT_H_