#ifndef __RTC_AV_FRAME_H__
#define __RTC_AV_FRAME_H__

#include <cstdint>

namespace bilibili
{
    enum NativeEvent
    {
        JOIN_CHANNEL_COMPLETED = 1,
    };

    class RtcAVFrameNotify
    {
    public:
        virtual void on_video_frame(uint64_t uid, uint32_t width, uint32_t height, uint64_t timestamp,
            const uint8_t* data_y, uint32_t stride_y,
            const uint8_t* data_u, uint32_t stride_u,
            const uint8_t* data_v, uint32_t stride_v, bool is_little_video = false) = 0;

        virtual void on_audio_frame(uint64_t uid,const void *audio_data,
            int bits_per_sample,
            int sample_rate,
            size_t number_of_channels,
            size_t number_of_frames) = 0;

        virtual void on_data(uint64_t uid, const char* data, size_t data_len,int seq_num, uint32_t timestamp) = 0;

        virtual void on_sctp_data(uint64_t uid, const char* data, size_t data_len,int sid, int seq_num, uint32_t timestamp) = 0;

        //事件回调,一些底层异步的接口事件回调
        //native_event参照bilibili::NativeEvent
        //code为0成功
        virtual void on_native_event(int native_event, int code, const char* message, int message_size) = 0;

        //audio level回调,范围0-32767
        virtual void on_audio_level(uint32_t ssrc, int level) = 0;

    };
}
#endif
