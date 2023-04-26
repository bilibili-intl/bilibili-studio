#ifndef OBS_OBS_PROXY_COMMON_OBS_PROXY_CONSTANTS_H_
#define OBS_OBS_PROXY_COMMON_OBS_PROXY_CONSTANTS_H_

namespace obs_proxy {

extern const char kRTMPFallbackServiceID[];
extern const char kRTMPBilibiliServiceID[];
extern const char kRTMPOutputID[];
extern const char kSRTOutputID[];
extern const char kKeyOutputTunnelName[];
extern const char kOBSServiceName[];
extern const char kOutputName[];
extern const char kVideoEncoderQSVID[];
extern const char kVideoEncoderNVENCID[];
extern const char kVideoEncoderX264ID[];
extern const char kVideoEncoderAMFID[];
extern const char kVideoEncoderNVTURINGENCID[];
extern const char kStreamingVideoEncoderName[];
extern const char kStreamingAudioEncoderName[];
extern const char kFFmpegOutputID[];
extern const char kFFmpegOutputName[];
extern const char kRecordingVideoEncoderName[];
extern const char kRecordingAudioEncoderName[];
extern const char kFFmpegMuxerID[];
extern const char kFileOutputName[];
extern const char kDesktopAudio1[];
extern const char kDesktopAudio2[];
extern const char kAuxAudio1[];
extern const char kAuxAudio2[];
extern const char kAuxAudio3[];
extern const int kBaseRecordingHDCRF;

// For signal literals.
extern const char kSignalStarting[];
extern const char kSignalStopping[];
extern const char kSignalStart[];
extern const char kSignalStop[];
extern const char kSignalTransitionVideoStop[];
extern const char kSignalItemAdd[];
extern const char kSignalItemRemove[];
extern const char kSignalItemSelect[];
extern const char kSignalItemDeselect[];
extern const char kSignalItemVisible[];
extern const char kSignalItemReorder[];
extern const char kSignalSourceActivate[];
extern const char kSignalSourceDeactivate[];
extern const char kSignalSourceRename[];
extern const char kSignalSourceShow[];
extern const char kSignalSourceLoad[];
extern const char kSignalMute[];
extern const char kSignalVolume[];
extern const char kSignalEncodedQP[];
extern const char kSignalPostSendFail[];

// For scene and scene items.
extern const char kSceneID[];
extern const char kColiveSourceID[];
extern const char kVoiceLinkSourceID[];
extern const char kDmkhimeSourceID[];
extern const char kColorSourceItemID[];
extern const char kReceiverSourceItemID[];
extern const char kBrowserSourceItemID[];
extern const char kImageItemID[];
extern const char kSliderItemID[];
extern const char kMediaSourceItemID[];
extern const char kTextItemID[];
extern const char kDisplayCaptureItemID[];
extern const char kWindowCaptureItemID[];
extern const char kGameCaptureItemID[];
extern const char kVideoCaptureDeviceItemID[];
extern const char kAudioInputCaptureItemID[];
extern const char kAudioOutputCaptureItemID[];
extern const char kRtcColiveSourceID[];
extern const char kTextureRendererSourceID[];
extern const char kVtuberInputItemID[];
//extern const char kOrnamentInputItemID[];
extern const char kVtuberRendererItemID[];
extern const char kRtcVoiceLinkSourceID[];
extern const char kThreeVtuberSourceID[];
extern const char kMainSceneRendererSourceID[];
// For filter type ids.
extern const char kFilterMaskID[];
extern const char kFilterCropID[];
extern const char kFilterGainID[];
extern const char kFilterColorID[];
extern const char kFilterScrollID[];
extern const char kFilterColorKeyID[];
extern const char kFilterSharpnessID[];
extern const char kFilterChromaKeyID[];
extern const char kFilterAsyncDelayID[];
extern const char kFilterNoiseSuppressID[];
extern const char kFilterBeautyID[];
extern const char kFilterForceMonoID[];
extern const char kFilterReverbID[];
extern const char kFilterUnpremultiplyID[];
extern const char kFilterFlipSettingID[];
extern const char kFilterVtuberSettingsID[];
//extern const char kFilterOrnamentSettingsID[];
extern const char kFilterMP4MotionID[];
extern const char kFilterSingIdentifyID[];
extern const char kFilterMicRecordID[];
extern const char kFadeTransitionID[];

// encoder quality
extern const char kVideoStreamX264QualityKey[];
extern const char kVideoStreamX264QualityPerformance[];
extern const char kVideoStreamX264QualityBalanced[];
extern const char kVideoStreamX264QualityQuality[];
extern const char kVideoStreamNVENCQualityKey[];
extern const char kVideoStreamNVENCQualityPerformance[];
extern const char kVideoStreamNVENCQualityBalanced[];
extern const char kVideoStreamNVENCQualityQuality[];
extern const char kVideoStreamQSVQualityKey[];
extern const char kVideoStreamQSVQualityPerformance[];
extern const char kVideoStreamQSVQualityBalanced[];
extern const char kVideoStreamQSVQualityQuality[];
extern const char kVideoStreamAMDQualityKey[];
extern const char kVideoStreamAMDQualityPerformance[];
extern const char kVideoStreamAMDQualityBalanced[];
extern const char kVideoStreamAMDQualityQuality[];
extern const char kVideoStreamNVTURINGENCQualityKey[];
extern const char kVideoStreamNVTURINGENCQualityPerformance[];
extern const char kVideoStreamNVTURINGENCQualityBalanced[];
extern const char kVideoStreamNVTURINGENCQualityQuality[];


// For Audio Devices Gain
extern const float kAudioGain;

// For OBS Channel
extern const int kDesktopAudio1Channel;
extern const int kDesktopAudio2Channel;
extern const int kAuxAudio1Channel;
extern const int kAuxAudio2Channel;
extern const int kAuxAudio3Channel;
extern const int kMainSceneChannel;
extern const int kAudioPlaySourceChannel;
extern const int kColiveRemoteSourceChannel;
extern const int kColiveSystemVoiceChannel;
extern const int kColiveChatRoomChannel;
}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_COMMON_OBS_PROXY_CONSTANTS_H_