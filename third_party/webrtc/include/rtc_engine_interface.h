#ifndef __RTC_ENGINE_INTERFACE__
#define __RTC_ENGINE_INTERFACE__
#include "rtc_av_frame.h"
namespace bilibili
{
    enum class MediaType { ANY, AUDIO, VIDEO, DATA, LITTLE_VIDEO,SCTP_DATA };

    enum class SendType
    {
        SendNone = 0,
        SendAudio = 1 << 0,
        SendVideo = 1 << 1,
        SendLittleVideo = 1 << 2,
    };

    struct RtcStat
    {
        int64_t round_trip_time = 0;
        double packet_loss_rate = 0;
        int64_t delay = 0;
        int64_t first_frame_cost = 0;
        int64_t freeze_count = 0;
        int64_t freeze_duration = 0;

        //network info
        std::string audio_tcp_local_addr;
        std::string audio_tcp_remote_addr;
        std::string audio_udp_local_addr;
        std::string audio_udp_remote_addr;
        std::string video_tcp_local_addr;
        std::string video_tcp_remote_addr;
        std::string video_udp_local_addr;
        std::string video_udp_remote_addr;
        std::string data_tcp_local_addr;
        std::string data_tcp_remote_addr;
        std::string data_udp_local_addr;
        std::string data_udp_remote_addr;

        //video sender info
        int vs_send_frame_width = 0;
        int vs_send_frame_height = 0;
        int vs_nominal_bitrate = 0;
        int vs_preferred_bitrate = 0;
        int64_t vs_bytes_sent = 0;
        int64_t vs_packets_sent = 0;
        int64_t vs_packets_lost = 0;
        int64_t vs_rtt_ms = 0;
        int64_t vs_packets_cached = 0;
        int64_t vs_firs_rcvd = 0;
        int64_t vs_plis_rcvd = 0;
        int64_t vs_nacks_rcvd = 0;
        int64_t vs_framerate_input = 0;
        int64_t vs_framerate_sent = 0;
        int64_t vs_avg_encode_ms = 0;
        int64_t vs_encode_usage_percent = 0;
        uint64_t vs_frames_encoded = 0;
        bool vs_has_entered_low_resolution = false;
        uint32_t vs_frames_dropped_by_capturer = 0;
        uint32_t vs_frames_dropped_by_encoder_queue = 0;
        uint32_t vs_frames_dropped_by_rate_limiter = 0;
        uint32_t vs_frames_dropped_by_encoder = 0;

        //video recever info
        int64_t vr_bytes_rcvd = 0;
        int vr_packets_rcvd = 0;
        int vr_packets_lost = 0;
        int vr_packets_concealed = 0;
        int vr_firs_sent = 0;
        int vr_plis_sent = 0;
        int vr_nacks_sent = 0;
        int vr_frame_width = 0;
        int vr_frame_height = 0;
        int vr_framerate_rcvd = 0;
        int vr_framerate_decoded = 0;
        int vr_framerate_output = 0;
        uint32_t vr_frames_received = 0;
        uint32_t vr_frames_decoded = 0;
        uint32_t vr_frames_rendered = 0;
        // Current frame decode latency.
        int vr_decode_ms = 0;
        // Maximum observed frame decode latency.
        int vr_max_decode_ms = 0;
        // Jitter (network-related) latency.
        int vr_jitter_buffer_ms = 0;
        // Requested minimum playout latency.
        int vr_min_playout_delay_ms = 0;
        // Requested latency to account for rendering delay.
        int vr_render_delay_ms = 0;
        // Target overall delay: network+decode+render, accounting for
        // min_playout_delay_ms.
        int vr_target_delay_ms = 0;
        // Current overall delay, possibly ramping towards target_delay_ms.
        int vr_current_delay_ms = 0;

        //小流
        //video sender info
        int little_vs_send_frame_width = 0;
        int little_vs_send_frame_height = 0;
        int little_vs_nominal_bitrate = 0;
        int little_vs_preferred_bitrate = 0;
        int64_t little_vs_bytes_sent = 0;
        int64_t little_vs_packets_sent = 0;
        int64_t little_vs_packets_lost = 0;
        int64_t little_vs_rtt_ms = 0;
        int64_t little_vs_packets_cached = 0;
        int64_t little_vs_firs_rcvd = 0;
        int64_t little_vs_plis_rcvd = 0;
        int64_t little_vs_nacks_rcvd = 0;
        int64_t little_vs_framerate_input = 0;
        int64_t little_vs_framerate_sent = 0;
        int64_t little_vs_avg_encode_ms = 0;
        int64_t little_vs_encode_usage_percent = 0;
        uint64_t little_vs_frames_encoded = 0;
        bool little_vs_has_entered_low_resolution = false;
        uint32_t little_vs_frames_dropped_by_capturer = 0;
        uint32_t little_vs_frames_dropped_by_encoder_queue = 0;
        uint32_t little_vs_frames_dropped_by_rate_limiter = 0;
        uint32_t little_vs_frames_dropped_by_encoder = 0;

        //video recever info
        int64_t little_vr_bytes_rcvd = 0;
        int little_vr_packets_rcvd = 0;
        int little_vr_packets_lost = 0;
        int little_vr_packets_concealed = 0;
        int little_vr_firs_sent = 0;
        int little_vr_plis_sent = 0;
        int little_vr_nacks_sent = 0;
        int little_vr_frame_width = 0;
        int little_vr_frame_height = 0;
        int little_vr_framerate_rcvd = 0;
        int little_vr_framerate_decoded = 0;
        int little_vr_framerate_output = 0;
        uint32_t little_vr_frames_received = 0;
        uint32_t little_vr_frames_decoded = 0;
        uint32_t little_vr_frames_rendered = 0;
        // Current frame decode latency.
        int little_vr_decode_ms = 0;
        // Maximum observed frame decode latency.
        int little_vr_max_decode_ms = 0;
        // Jitter (network-related) latency.
        int little_vr_jitter_buffer_ms = 0;
        // Requested minimum playout latency.
        int little_vr_min_playout_delay_ms = 0;
        // Requested latency to account for rendering delay.
        int little_vr_render_delay_ms = 0;
        // Target overall delay: network+decode+render, accounting for
        // min_playout_delay_ms.
        int little_vr_target_delay_ms = 0;
        // Current overall delay, possibly ramping towards target_delay_ms.
        int little_vr_current_delay_ms = 0;

        //audio sender info
        int64_t as_bytes_sent = 0;
        int as_packets_sent = 0;
        int as_packets_lost = 0;
        int64_t as_rtt_ms = 0;

        //audio receiver info
        int64_t ar_bytes_rcvd = 0;
        int ar_packets_rcvd = 0;
        int ar_packets_lost = 0;
        int ar_jitter_ms = 0;
        int ar_jitter_buffer_ms = 0;
        int ar_jitter_buffer_preferred_ms = 0;
        int ar_delay_estimate_ms = 0;
        uint64_t ar_total_samples_received = 0;
        double ar_total_output_duration = 0.0;
        uint64_t ar_concealed_samples = 0;
        uint64_t ar_concealment_events = 0;

        //data sender info
        int64_t ds_bytes_sent = 0;
        int ds_packets_sent = 0;
        int ds_packets_lost = 0;
        int64_t ds_rtt_ms = 0;

        //data receiver info
        int64_t dr_bytes_rcvd = 0;
        int dr_packets_rcvd = 0;
        int dr_packets_lost = 0;

    };


    struct RtcCallConfig {
      int min_bitrate = 0;
      int start_bitrate = 0;
      int max_bitrate = 0;
      int audio_bitrate = 0;
    };


    struct RtcVideoSenderInfo
    {
        uint32_t ssrc = 0;
        int64_t vs_packets_sent = 0;
        int64_t vs_packets_lost = 0;
        uint32_t vs_frames_dropped_by_capturer = 0;
        uint32_t vs_frames_dropped_by_encoder_queue = 0;
        uint32_t vs_frames_dropped_by_rate_limiter = 0;
        uint32_t vs_frames_dropped_by_encoder = 0;
    };

    struct RtcAudioSenderInfo
    {
        uint32_t ssrc = 0;
        int64_t as_packets_sent = 0;
        int64_t as_packets_lost = 0;
    };

    struct RtcVideoReceiverInfo
    {
        uint32_t ssrc = 0;
        int64_t freeze_duration = 0;
        int64_t freeze_count = 0;
    };

    struct RtcAudioReceiverInfo
    {
        uint32_t ssrc = 0;
        int64_t ar_total_output_duration = 0;
        int64_t ar_concealed_samples = 0;
        int64_t ar_total_samples_received = 0;
        int64_t freeze_duration = 0;
    };

    struct RtcRealtimeSendStat
    {
        uint64_t uid = 0;
        std::vector<RtcVideoSenderInfo> video;
        std::vector<RtcAudioSenderInfo> audio;
    };

    struct RtcRealtimeReceiverStat
    {
        uint64_t uid = 0;
        std::vector<RtcVideoReceiverInfo> video;
        std::vector<RtcAudioReceiverInfo> audio;
    };

    struct RtcRealtimeStat
    {
        std::vector<RtcRealtimeSendStat> senders;
        std::vector<RtcRealtimeReceiverStat> receivers;
    };

    class RtcEngine
    {
 public:
      virtual int initialize() = 0;
      virtual void clean_up() = 0;
      virtual int get_version() = 0;

      virtual int join_channel(uint64_t channel_id,
                               uint32_t call_id,
                               uint32_t video_send_ssrc,
                               uint32_t audio_send_ssrc,
                               uint32_t data_send_ssrc,
                               uint32_t little_video_send_ssrc,
                               uint64_t send_uid,
                               const char* host,
                               uint16_t port,
                               uint32_t token,
                               uint8_t* extra_header_data,
                               uint32_t extra_header_size,
                               //send_type参照结构体bilibili::SetSendType,设置哪些媒体类型需要加入频道就发送
                               //比如让视频大流合音频同时发送,send_type为:SetSendType::SendVideo | SetSendType::SendAudio
                               //比如默认什么都不发(后续调set_send再发送),send_type为SendType::SendNone
                               int send_type) = 0;
      virtual void leave_channel() = 0;
      virtual void set_rtc_transport_parameter(const char* host,
                                       uint16_t port,
                                       uint32_t token,
                                       const uint8_t* extra_header_data,
                                       uint32_t extra_header_size) = 0;
      virtual void set_http_dns(const char* data, size_t len) = 0;
      virtual void set_subscribe(const char* serialize_map,uint32_t serialize_size) = 0;
      virtual void add_recv_ssrc(uint32_t ssrc, uint64_t uid, MediaType type) = 0;
      virtual void remove_recv_ssrc(uint32_t ssrc, MediaType type) = 0;
      virtual int send_data(const char* data, size_t len) = 0;
      //ordered:有序 reliable:可靠 有序意味着一定可靠，可靠不一定有序
      virtual int sctp_send_data(uint32_t sid,const char* data, size_t len,bool ordered,bool reliable) = 0;
      virtual int send_video(int32_t width,
                     int32_t height,
                     uint64_t timestamp,
                     const uint8_t* data_y,
                     uint32_t stride_y,
                     const uint8_t* data_u,
                     uint32_t stride_u,
                     const uint8_t* data_v,
                     uint32_t stride_v) = 0;
      virtual int send_audio(const uint8_t* audio_data,
                     int32_t bits_per_sample,
                     int32_t sample_rate,
                     int32_t number_of_channels,
                     int32_t number_of_frames) = 0;
      virtual bool get_status(RtcStat* stat) = 0;
      virtual bool get_realtime_status(RtcRealtimeStat* stat) = 0;
      virtual int set_config(const RtcCallConfig* call_config) = 0;
      virtual void add_send_sid(uint32_t sid) = 0;
      virtual void remove_send_sid(uint32_t sid) = 0;

      //设置对应ssrc播放的音量,范围0-1
      virtual bool set_remote_play_volume(uint32_t ssrc, double volume) = 0;

      //获取当前频道里当前所有人的音量,包括自己,最后结果通过回掉on_audio_level回掉回来,范围0-32767
      virtual void get_audio_level() = 0;

      //type参照结构体bilibili::SetSendType,设置某个传输通道是否发送
      //比如让视频大流合音频同时发生,调用方法:set_send(SetSendType::SendVideo | SetSendType::SendAudio,true);
      //比如单个设置不发送音频的调用方法:set_send(SetSendType::SendAudio,false);
      virtual void set_send(int type, bool send) = 0;

      virtual ~RtcEngine(){};
    };

    RtcEngine* CreateRtcEngine(RtcAVFrameNotify* av_frame_callback);
    }
#endif