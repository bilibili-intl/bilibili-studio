#include "obs/obs_proxy/public/common/pref_constants.h"

namespace prefs {

const int kDefaultAudioSampleRate = 48000;
const int kDefaultAudioBitRate = 256;
const bool kDefaultAudioDenoise = false;
const char kAudioMonoNoMix[] = "forcemono_notouch";
const char kAudioMonoLToDouble[] = "forcemono_leftonly";
const char kAudioMonoRToDouble[] = "forcemono_rightonly";
const char kAudioMonoMix[] = "forcemono_mix";
const char kAudioFilterDenoiser[] = "audio_denoiser";
const char kFilterNoiseSuppressLevel[] = "suppress_level";
const char kAudioFilterMono[] = "forcemono_filter";
const char kNoAudioDevice[] = "";
const char kDefaultAudioDevice[] = "default";
const int kDefaultVideoBitRate = 2000;
const char kDefaultVideoBitRateControl[] = "CBR";
const char* const kDefaultAudioChannelSetup = kAudioSpeakStereo;
const int kDefaultVideoFPSType = 0;
const char kDefaultFPSCommon[] = "30";
const int kDefaultVideoInt = 30;
const int kDefaultVideoNum = 30;
const int kDefaultVideoDen = 1;
const char* const kDefaultVideoRenderer = kVideoRendererD3D;
const char* const kDefaultVideoColorFormat = kVideoFormatNV12;
const char* const kDefaultVideoColorSpace = kVideoColorSpace709;
const char* const kDefaultVideoColorRange = kVideoRangePartial;
const char* const kDefaultVideoScaleType = kVideoScaleBicubic;
const char* const kDefaultOutputStreamVideoEncoder = kVideoStreamX264;
const char* const kDefaultOutputRecordVideoEncoder = kVideoStreamX264;
const char* const kDefaultOutputRecordVideoQuality = kVideoQualityStream;
const char* const kDefaultRecordingFormat = kRecordingFormatFMP4;
const bool kDefaultRecordingSync = false;
const char kDefaultSceneCollectionName[] = "untitled.json";
const wchar_t kFallbackOutputDir[] = L".";
const bool kDefaultOutputReconnect = true;
const int kDefaultOutputRetryDelay = 3;     // in seconds
const int kDefaultOutputMaxRetries = 10;
const bool kDefaultOutputUseDelay = false;
const int kDefaultOutputDelaySec = 20;
const bool kDefaultOutputDelayPreserve = true;

const char kAudioSpeakMono[] = "mono";
const char kAudioSpeakStereo[] = "stereo";
const char kVideoRendererD3D[] = "direct3d 11";
const char kRendererD3DModuleName[] = "libobs-d3d11.dll";
const char kRendererOpenGLModuleName[] = "libobs-opengl.dll";
const char kVideoColorSpace601[] = "601";
const char kVideoColorSpace709[] = "709";
const char kVideoRangeFull[] = "full";
const char kVideoRangePartial[] = "partial";
const char kVideoFormatI420[] = "i420";
const char kVideoFormatI444[] = "i444";
const char kVideoFormatNV12[] = "nv12";
const char kVideoScaleBicubic[] = "bicubic";
const char kVideoScaleLanczos[] = "lanczos";
const char kVideoStreamX264[] = "x264";
const char kVideoStreamX264LowCPU[] = "x264_lowcpu";
const char kVideoStreamQSV[] = "qsv";
const char kVideoStreamNVENC[] = "nvenc";
const char kVideoStreamAMD[] = "amd";
const char kVideoStreamNVTURINGENC[] = "nv_turing_enc";
const char kVideoQualityStream[] = "stream";
const char kVideoQualityLossless[] = "lossless";
const char kVideoQualityHD[] = "hd";

const int kDefaultVideoOutputWidth = 1280;
const int kDefaultVideoOutputHeight = 720;

const int kVideoStreamQualityDefault = 2;
const int kVideoStreamQualityOptionsCount = 3;

const char kRecordingFormatFLV[] = "flv";
const char kRecordingFormatMP4[] = "mp4";
const char kRecordingFormatFMP4[] = "fmp4";

const bool kDefaultCenterSnapping = false;
const bool kDefaultScreenSnapping = true;
const bool kDefaultSourceSnapping = true;
const bool kDefaultSnappingEnabled = true;
const double kDefaultSnapDistance = 10.0;

const char kLiveReplaySaveMode[] = "live_replay_save_mode_new";
const bool kDefaultLiveReplayMode = false;

const char kAnalysisLiveQuality[] = "analysis_live_quality";
const bool kDefaultAnalysisLiveQuality = false;

const char kSourceDragModeZoom[] = "zoom";
const char kSourceDragModeStretch[] = "stretch";
const char kSourceDragModeCut[] = "cut";

// Default scene collection data constants.
const char kFirstDefaultSceneName[] = "Scene 1";
const char kSecondDefaultSceneName[] = "Scene 2";
const char kThirdDefaultSceneName[] = "Scene 3";
// obs-proxy层加载场景文件的时候，会按照场景名称进行排序，所以新增的场景名称要按ASCII排序
const char kVerticalFirstDefaultSceneName[] = "Vertical Scene 1";
const char kVerticalSecondDefaultSceneName[] = "Vertical Scene 2";
const char kVerticalThirdDefaultSceneName[] = "Vertical Scene 3";
const wchar_t kVtuberSceneName[] = L"虚拟窗口";
const char kJoinMicSceneName[] = "Scene JoinMic";
const char kJoinMicSceneRemoteSourceName[] = "remote join mic";
const char kJoinMicSceneMaskSourceName[] = "mask source join mic";
const char kJoinMicSceneMainSourceName[] = "colive_main_source";
const char kJoinMicScenePreviewMasSourceName[] = "mask preview source join mic";
const char kMicRecordMediaSourceName[] = "echo_test_media_source";
const char kJoinMicSceneMultiVCBgImageSourceName[] = "multi vc bg image source join mic";
const char* kColiveLiveRoomSceneName = "colive_live_room_scene";


}   // namespace pref_constants