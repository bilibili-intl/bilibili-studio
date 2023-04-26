#include "obs/obs_proxy/public/common/pref_names.h"

namespace prefs {

const char kAudioSampleRate[] = "core.audio.sample_rate";
const char kAudioChannelSetup[] = "core.audio.channel_setup";                   // 音频声道设置
const char kAudioMonoChannelSetup[] = "core.audio.mono_channel_setup";          // 音频单声道设置
const char kAudioBitRate[] = "core.audio.bit_rate";                             // 音频码率
const char kAudioDenoise[] = "core.audio.denoise";                              // 音频降噪
const char kAudioMicDeviceInUse[] = "core.audio.micaux";                        // 选中的麦克风，ID
const char kAudioSpeakerDeviceInUse[] = "core.audio.speaker";                   // 选中的扬声器，ID
const char kVideoBitRate[] = "core.video.bit_rate";	                            // 视频码率
const char kVideoBitRateControl[] = "core.video.bit_rate_control";	            // 视频码率控制
const char kVideoFPSType[] = "core.video.fps_type";
const char kVideoFPSCommon[] = "core.video.fps_common";	                        // 视频帧率
const char kVideoFPSInt[] = "core.video.fps_int";
const char kVideoFPSNum[] = "core.video.fps_num";
const char kVideoFPSDen[] = "core.video.fps_den";
const char kVideoRenderer[] = "core.video.renderer";
const char kVideoOutputCX[] = "core.video.output_cx";                           // 当前用做推流分辨率
const char kVideoOutputCY[] = "core.video.output_cy";
const char kVideoColorFormat[] = "core.video.color_format";
const char kVideoColorSpace[] = "core.video.color_space";
const char kVideoColorRange[] = "core.video.color_range";
const char kVideoScaleType[] = "core.video.scale_type";

const char kOutputStreamVideoEncoder[] = "core.output.stream.video_encoder";        // 视频编码器
const char kOutputStreamVideoQuality[] = "core.output.stream.video_quality_mode";	// 当前用做推流质量
const char kOutputRecordVideoEncoder[] = "core.output.record.video_encoder";
const char kOutputRecordVideoQuality[] = "core.output.record.video_quality";
const char kOutputFolder[] = "core.output.folder";
const char kOutputRecordingFormat[] = "core.output.recording_format";
const char kOutputRecordingPath[] = "core.output.recording_path";               // 保存路径
const char kOutputRecordingSync[] = "core.output.recording_sync";               // 同步录制
const char kOutputReconnect[] = "core.output.reconnect";
const char kOutputMaxRetries[] = "core.output.max_retries";
const char kOutputRetryDelay[] = "core.output.retry_delay";
const char kOutputUseDelay[] = "core.output.delay";
const char kOutputDelaySec[] = "core.output.delay_sec";
const char kOutputDelayPreserve[] = "core.output.delay_preserve";
const char kLastSceneCollectionName[] = "core.last_scene_collection_name";
const char kCenterSnapping[] = "core.basic.center_snapping";
const char kScreenSnapping[] = "core.basic.screen_snapping";
const char kSourceSnapping[] = "core.basic.source_snapping";
const char kSnappingEnabled[] = "core.basic.snapping_enabled";
const char kSnapDistance[] = "core.basic.snap_distance";


const char kSourceDragMode[] = "source.drag_mode";

}   // namespace prefs