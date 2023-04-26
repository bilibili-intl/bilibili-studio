#include "obs/obs_proxy/common/obs_proxy_constants.h"

namespace obs_proxy {

// Note: all constants that have ID-suffix are mandated to keep consistent with
// which defined in obs-core module. DO NOT change their content.

const char kAudioInputCaptureItemID[] = "wasapi_input_capture";
const char kAudioOutputCaptureItemID[] = "wasapi_output_capture";
const char kAuxAudio1[] = "Mic/Aux";
const char kAuxAudio2[] = "Mic/Aux 2";
const char kAuxAudio3[] = "Mic/Aux 3";
const char kColorSourceItemID[] = "color_source";
const char kReceiverSourceItemID[] = "receiver_source";
const char kBrowserSourceItemID[] = "browser_source";
const char kDesktopAudio1[] = "Desktop Audio";
const char kDesktopAudio2[] = "Desktop Audio 2";
const char kDisplayCaptureItemID[] = "monitor_capture";
const char kFFmpegMuxerID[] = "ffmpeg_muxer";
const char kFFmpegOutputID[] = "ffmpeg_output";
const char kFFmpegOutputName[] = "default_ffmpeg_output";
const char kFadeTransitionID[] = "fade_transition";
const char kFileOutputName[] = "file_output";
const char kFilterMaskID[] = "mask_filter";
const char kFilterCropID[] = "crop_filter";
const char kFilterGainID[] = "gain_filter";
const char kFilterColorID[] = "color_filter";
const char kFilterScrollID[] = "scroll_filter";
const char kFilterColorKeyID[] = "color_key_filter";
const char kFilterSharpnessID[] = "sharpness_filter";
const char kFilterChromaKeyID[] = "chroma_key_filter";
const char kFilterAsyncDelayID[] = "async_delay_filter";
const char kFilterNoiseSuppressID[] = "noise_suppress_filter";
const char kFilterBeautyID[] = "beauty_filter";
const char kFilterForceMonoID[] = "forcemono_filter";
const char kFilterReverbID[] = "reverb_filter";
const char kFilterUnpremultiplyID[] = "unpremultiply_filter";
const char kFilterFlipSettingID[] = "flip_setting_filter";
const char kFilterVtuberSettingsID[] = "vtuber_settings_filter";
//const char kFilterOrnamentSettingsID[] = "ornament_settings_filter";
const char kFilterMP4MotionID[] = "mp4_motion_filter";
const char kFilterSingIdentifyID[] = "sing_identify_filter";
const char kFilterMicRecordID[] = "mic_record_filter";
const char kGameCaptureItemID[] = "game_capture";
const char kImageItemID[] = "image_source";
const char kSliderItemID[] = "slideshow";
const char kMediaSourceItemID[] = "ffmpeg_source";
const char kOBSServiceName[] = "default_service";
const char kOutputName[] = "default_stream";
const char kRTMPBilibiliServiceID[] = "rtmp_custom";
const char kRTMPFallbackServiceID[] = "rtmp_common";
const char kRTMPOutputID[] = "rtmp_output";
const char kSRTOutputID[] = "bvc_srt_output";
const char kKeyOutputTunnelName[] = "tunnel_name";
const char kRecordingAudioEncoderName[] = "aac_recording";
const char kRecordingVideoEncoderName[] = "h264_recording";
const char kSceneID[] = "scene";
const char kColiveSourceID[] = "colive_source";
const char kVoiceLinkSourceID[] = "voice_link_source";
const char kDmkhimeSourceID[] = "dmkhime_source";
const char kRtcColiveSourceID[] = "rtc_colive_source";
const char kTextureRendererSourceID[] = "texture_renderer_source";
const char kSignalItemAdd[] = "item_add";
const char kSignalItemRemove[] = "item_remove";
const char kSignalItemSelect[] = "item_select";
const char kSignalItemDeselect[] = "item_deselect";
const char kSignalItemVisible[] = "item_visible";
const char kSignalItemReorder[] = "reorder";
const char kSignalSourceActivate[] = "source_activate";
const char kSignalSourceDeactivate[] = "source_deactivate";
const char kSignalSourceRename[] = "source_rename";
const char kSignalSourceShow[] = "source_show";
const char kSignalSourceLoad[] = "source_load";
const char kSignalStart[] = "start";
const char kSignalStarting[] = "starting";
const char kSignalStop[] = "stop";
const char kSignalStopping[] = "stopping";
const char kSignalTransitionVideoStop[] = "transition_video_stop";
const char kSignalMute[] = "mute";
const char kSignalVolume[] = "volume";
const char kSignalEncodedQP[] = "encoded_qp";
const char kSignalPostSendFail[] = "post_send_fail";
const char kStreamingAudioEncoderName[] = "aac_streaming";
const char kStreamingVideoEncoderName[] = "h264_streaming";
const char kTextItemID[] = "text_gdiplus";
const char kVideoCaptureDeviceItemID[] = "dshow_input";
const char kVtuberInputItemID[] = "vtuber_input";
//const char kOrnamentInputItemID[] = "ornament_input";
const char kThreeVtuberSourceID[] = "three_vtuber_input";
const char kMainSceneRendererSourceID[] = "main_scene_renderer_source";
const char kVtuberRendererItemID[] = "vtuber_renderer_source";
const char kRtcVoiceLinkSourceID[] = "rtc_voice_link_source";
const char kVideoEncoderNVENCID[] = "ffmpeg_nvenc";
const char kVideoEncoderQSVID[] = "obs_qsv11";
const char kVideoEncoderX264ID[] = "obs_x264";
const char kVideoEncoderAMFID[] = "amd_amf_h264";
const char kVideoEncoderNVTURINGENCID[] = "jim_nvenc";
const char kWindowCaptureItemID[] = "window_capture";

const char kVideoStreamX264QualityKey[] = "preset";
const char kVideoStreamX264QualityPerformance[] = "ultrafast";
const char kVideoStreamX264QualityBalanced[] = "veryfast";
const char kVideoStreamX264QualityQuality[] = "fast";

const char kVideoStreamNVENCQualityKey[] = "preset";
const char kVideoStreamNVENCQualityPerformance[] = "hp";
const char kVideoStreamNVENCQualityBalanced[] = "default";
const char kVideoStreamNVENCQualityQuality[] = "hq";

const char kVideoStreamQSVQualityKey[] = "target_usage";
const char kVideoStreamQSVQualityPerformance[] = "speed";
const char kVideoStreamQSVQualityBalanced[] = "balanced";
const char kVideoStreamQSVQualityQuality[] = "quality";

const char kVideoStreamAMDQualityKey[] = "preset";
const char kVideoStreamAMDQualityPerformance[] = "speed";
const char kVideoStreamAMDQualityBalanced[] = "balanced";
const char kVideoStreamAMDQualityQuality[] = "quality";

const char kVideoStreamNVTURINGENCQualityKey[] = "preset";
const char kVideoStreamNVTURINGENCQualityPerformance[] = "hp";
const char kVideoStreamNVTURINGENCQualityBalanced[] = "default";
const char kVideoStreamNVTURINGENCQualityQuality[] = "hq";

const int kBaseRecordingHDCRF = 16;

const float kAudioGain = 10.0F;

const int kDesktopAudio1Channel = 1;
const int kDesktopAudio2Channel = 2;
const int kAuxAudio1Channel = 3;
const int kAuxAudio2Channel = 4;
const int kAuxAudio3Channel = 5;
const int kMainSceneChannel = 0;
const int kAudioPlaySourceChannel = 62;
const int kColiveRemoteSourceChannel = 63;
const int kColiveSystemVoiceChannel = 61;
const int kColiveChatRoomChannel = 60;
}   // namespace obs_proxy