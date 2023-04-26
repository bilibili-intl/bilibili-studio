#ifndef OBS_OBS_PROXY_PUBLIC_COMMON_PREF_NAMES_H_
#define OBS_OBS_PROXY_PUBLIC_COMMON_PREF_NAMES_H_

#include "obs/obs_proxy/app/obs_proxy_export.h"

namespace prefs {

OBS_PROXY_EXPORT extern const char kAudioSampleRate[];
OBS_PROXY_EXPORT extern const char kAudioChannelSetup[];
OBS_PROXY_EXPORT extern const char kAudioMonoChannelSetup[];    // 单声道设置
OBS_PROXY_EXPORT extern const char kAudioBitRate[];
OBS_PROXY_EXPORT extern const char kAudioDenoise[];             // 麦克风降噪
OBS_PROXY_EXPORT extern const char kAudioMicDeviceInUse[];        // 当前使用的麦克风设备，记录其ID
OBS_PROXY_EXPORT extern const char kAudioSpeakerDeviceInUse[];    // 当前使用的扬声器设备，记录其ID

OBS_PROXY_EXPORT extern const char kVideoBitRate[];
OBS_PROXY_EXPORT extern const char kVideoBitRateControl[];
OBS_PROXY_EXPORT extern const char kVideoFPSType[];
OBS_PROXY_EXPORT extern const char kVideoFPSCommon[];
OBS_PROXY_EXPORT extern const char kVideoFPSInt[];
OBS_PROXY_EXPORT extern const char kVideoFPSNum[];
OBS_PROXY_EXPORT extern const char kVideoFPSDen[];
OBS_PROXY_EXPORT extern const char kVideoRenderer[];
OBS_PROXY_EXPORT extern const char kVideoOutputCX[];
OBS_PROXY_EXPORT extern const char kVideoOutputCY[];
OBS_PROXY_EXPORT extern const char kVideoColorFormat[];
OBS_PROXY_EXPORT extern const char kVideoColorSpace[];
OBS_PROXY_EXPORT extern const char kVideoColorRange[];
OBS_PROXY_EXPORT extern const char kVideoScaleType[];

OBS_PROXY_EXPORT extern const char kOutputStreamVideoEncoder[];
OBS_PROXY_EXPORT extern const char kOutputStreamVideoQuality[]; // 视频质量
OBS_PROXY_EXPORT extern const char kOutputRecordVideoEncoder[];
OBS_PROXY_EXPORT extern const char kOutputRecordVideoQuality[];
OBS_PROXY_EXPORT extern const char kOutputFolder[];
OBS_PROXY_EXPORT extern const char kOutputRecordingFormat[];
OBS_PROXY_EXPORT extern const char kOutputRecordingPath[];      // 录像保存路径
OBS_PROXY_EXPORT extern const char kOutputRecordingSync[];      // 同步录制
OBS_PROXY_EXPORT extern const char kOutputReconnect[];
OBS_PROXY_EXPORT extern const char kOutputMaxRetries[];
OBS_PROXY_EXPORT extern const char kOutputRetryDelay[];
OBS_PROXY_EXPORT extern const char kOutputUseDelay[];
OBS_PROXY_EXPORT extern const char kOutputDelaySec[];
OBS_PROXY_EXPORT extern const char kOutputDelayPreserve[];

OBS_PROXY_EXPORT extern const char kLastSceneCollectionName[];

OBS_PROXY_EXPORT extern const char kCenterSnapping[];
OBS_PROXY_EXPORT extern const char kScreenSnapping[];
OBS_PROXY_EXPORT extern const char kSourceSnapping[];
OBS_PROXY_EXPORT extern const char kSnappingEnabled[];
OBS_PROXY_EXPORT extern const char kSnapDistance[];

OBS_PROXY_EXPORT extern const char kSourceDragMode[];

}   // namespace prefs

#endif  // OBS_OBS_PROXY_PUBLIC_COMMON_PREF_NAMES_H_