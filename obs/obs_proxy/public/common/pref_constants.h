#ifndef OBS_OBS_PROXY_PUBLIC_COMMON_PREF_CONSTANTS_H_
#define OBS_OBS_PROXY_PUBLIC_COMMON_PREF_CONSTANTS_H_

#include "obs/obs_proxy/app/obs_proxy_export.h"

namespace prefs {

OBS_PROXY_EXPORT extern const int kDefaultAudioSampleRate;
OBS_PROXY_EXPORT extern const char* const kDefaultAudioChannelSetup;
OBS_PROXY_EXPORT extern const int kDefaultAudioBitRate;	// 音频码率
OBS_PROXY_EXPORT extern const bool kDefaultAudioDenoise;// 麦克风降噪默认值false
OBS_PROXY_EXPORT extern const char kAudioMonoNoMix[];// 单声道设置无混合
OBS_PROXY_EXPORT extern const char kAudioMonoLToDouble[];// 左声道到双声道
OBS_PROXY_EXPORT extern const char kAudioMonoRToDouble[];// 右声道到双声道
OBS_PROXY_EXPORT extern const char kAudioMonoMix[];// 混合
OBS_PROXY_EXPORT extern const char kAudioFilterDenoiser[];// 降噪filter名称
OBS_PROXY_EXPORT extern const char kFilterNoiseSuppressLevel[];// 降噪level
OBS_PROXY_EXPORT extern const char kAudioFilterMono[];// 单声filter名称
OBS_PROXY_EXPORT extern const char kNoAudioDevice[];         // Indicates no speaker/microphone device available
OBS_PROXY_EXPORT extern const char kDefaultAudioDevice[];    // Indicates the default speaker/microphone device
OBS_PROXY_EXPORT extern const int kDefaultVideoBitRate;	// 视频码率
OBS_PROXY_EXPORT extern const char kDefaultVideoBitRateControl[];	// 视频码率控制
OBS_PROXY_EXPORT extern const int kDefaultVideoFPSType;
OBS_PROXY_EXPORT extern const char kDefaultFPSCommon[];	// 视频帧速
OBS_PROXY_EXPORT extern const int kDefaultVideoInt;
OBS_PROXY_EXPORT extern const int kDefaultVideoNum;
OBS_PROXY_EXPORT extern const int kDefaultVideoDen;
OBS_PROXY_EXPORT extern const char* const kDefaultVideoRenderer;
OBS_PROXY_EXPORT extern const char* const kDefaultVideoColorFormat;
OBS_PROXY_EXPORT extern const char* const kDefaultVideoColorSpace;
OBS_PROXY_EXPORT extern const char* const kDefaultVideoColorRange;
OBS_PROXY_EXPORT extern const char* const kDefaultVideoScaleType;
OBS_PROXY_EXPORT extern const char* const kDefaultOutputStreamVideoEncoder;	// 视频编码器
OBS_PROXY_EXPORT extern const char* const kDefaultOutputRecordVideoEncoder;
OBS_PROXY_EXPORT extern const char* const kDefaultOutputRecordVideoQuality;
OBS_PROXY_EXPORT extern const char* const kDefaultRecordingFormat;
OBS_PROXY_EXPORT extern const int kDefaultVideoOutputWidth;
OBS_PROXY_EXPORT extern const int kDefaultVideoOutputHeight;
OBS_PROXY_EXPORT extern const bool kDefaultRecordingSync;// 同步录制默认false
OBS_PROXY_EXPORT extern const char kDefaultSceneCollectionName[];   // Must be UTF-8 compatible.
OBS_PROXY_EXPORT extern const wchar_t kFallbackOutputDir[];
OBS_PROXY_EXPORT extern const bool kDefaultOutputReconnect;
OBS_PROXY_EXPORT extern const int kDefaultOutputRetryDelay;
OBS_PROXY_EXPORT extern const int kDefaultOutputMaxRetries;
OBS_PROXY_EXPORT extern const bool kDefaultOutputUseDelay;
OBS_PROXY_EXPORT extern const int kDefaultOutputDelaySec;
OBS_PROXY_EXPORT extern const bool kDefaultOutputDelayPreserve;

OBS_PROXY_EXPORT extern const bool kDefaultCenterSnapping;
OBS_PROXY_EXPORT extern const bool kDefaultScreenSnapping;
OBS_PROXY_EXPORT extern const bool kDefaultSourceSnapping;
OBS_PROXY_EXPORT extern const bool kDefaultSnappingEnabled;
OBS_PROXY_EXPORT extern const double kDefaultSnapDistance;

OBS_PROXY_EXPORT extern const char kSourceDragModeZoom[];   // Must be UTF-8 compatible.
OBS_PROXY_EXPORT extern const char kSourceDragModeStretch[];   // Must be UTF-8 compatible.
OBS_PROXY_EXPORT extern const char kSourceDragModeCut[];   // Must be UTF-8 compatible.

extern const char kAudioSpeakMono[];
extern const char kAudioSpeakStereo[];
extern const char kVideoRendererD3D[];
extern const char kVideoColorSpace601[];
extern const char kVideoColorSpace709[];
extern const char kVideoRangeFull[];
extern const char kVideoRangePartial[];
extern const char kVideoFormatI420[];
extern const char kVideoFormatI444[];
extern const char kVideoFormatNV12[];
extern const char kVideoScaleBicubic[];
extern const char kVideoScaleLanczos[];
extern const char kRendererD3DModuleName[];
extern const char kRendererOpenGLModuleName[];
extern const char kVideoStreamX264[];
extern const char kVideoStreamX264LowCPU[];
extern const char kVideoStreamQSV[];
extern const char kVideoStreamNVENC[];
extern const char kVideoStreamNVTURINGENC[];
extern const char kVideoStreamAMD[];
extern const char kVideoQualityStream[];
extern const char kVideoQualityLossless[];
extern const char kVideoQualityHD[];

extern const int kVideoStreamQualityOptionsCount;
extern const int kVideoStreamQualityDefault;

extern const char kRecordingFormatFLV[];
extern const char kRecordingFormatMP4[];
extern const char kRecordingFormatFMP4[];

extern const char kAnalysisLiveQuality[];
extern const bool kDefaultAnalysisLiveQuality;
// Default scene collection data constants.
extern const char kFirstDefaultSceneName[];
extern const char kSecondDefaultSceneName[];
extern const char kThirdDefaultSceneName[];
extern const char kVerticalFirstDefaultSceneName[];
extern const char kVerticalSecondDefaultSceneName[];
extern const char kVerticalThirdDefaultSceneName[];
extern const wchar_t kVtuberSceneName[];
extern const char kJoinMicSceneName[];
extern const char kJoinMicSceneRemoteSourceName[];
extern const char kJoinMicSceneMaskSourceName[];
extern const char kJoinMicSceneMainSourceName[];
extern const char kJoinMicScenePreviewMasSourceName[];
extern const char kVtuberCameraSceneName[];
extern const char kMicRecordMediaSourceName[];
extern const char kJoinMicSceneMultiVCBgImageSourceName[];
extern const char* kColiveLiveRoomSceneName;
}   // namespace pref_constants

#endif  // OBS_OBS_PROXY_PUBLIC_COMMON_PREF_CONSTANTS_H_