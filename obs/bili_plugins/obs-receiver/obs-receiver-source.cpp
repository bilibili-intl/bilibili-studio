#include <obs-module.h>
#include <util/base.h>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <fstream>
#include <vector>

#define NOMINMAX
#include <winsock2.h>
#include <WS2tcpip.h>

#include "device_selector.h"
#include "ios_bridge.h"

extern "C" {
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-colive", "en-US")

void register_receiver_source();

extern "C"
{
    bool obs_module_load(void)
    {
        register_receiver_source();
        return true;
    }

    void obs_module_unload(void)
    {
    }
}

namespace
{
    enum receiver_type
    {
        RECV_T_WIRELESS,
        RECV_T_WIRED_IOS,
    };

    enum receiver_status
    {
        RECEIVER_START = 0,
        RECEIVER_RUNNING,
        RECEIVER_LISTEN,
        RECEIVER_RESET,
    };

    enum ios_event_type
    {
        IOS_EV_SRV_STATUS,
        IOS_EV_DEV_COUNT,
        IOS_EV_CONN_STATUS,
        IOS_EV_RECV_STATUS,
    };

    enum ios_streaming_status
    {
        IOS_SS_FAILED,
        IOS_SS_STARTED,
        IOS_SS_STOPPED,
        IOS_SS_INTR,
    };

    enum ios_exit_code
    {
        IOS_EC_NORMAL = 0,
        IOS_EC_ERROR,
        IOS_EC_END,
        IOS_EC_CUR,
    };

    static const char kReceiverStatus[] = "screen_cast_receiver_status";
    static const char kRtmpIp[] = "rtmp://0.0.0.0:";
    static const char kRtmpName[] = "/livehime";
    static const char kRtmpPort[] = "rtmp_port";
    static const char kRtmpStatus[] = "rtmp_status";

    static const char kIOSReceiverStatus[] = "ios_screen_cast_receiver_status";
    static const char kIOSEventType[] = "ios_ev_type";
    static const char kIOSIntValue[] = "ios_int_val";
    static const int  kIOSPort = 22336;

    static const char kRecvType[] = "recv_type";
    static const char kEncoderInfo[] = "encoder_info";
    static const char kIsLandscapeModel[] = "is_landscape_model";
    static const int  kDelaySecond = 5;
    static const int  kAVinputOpenDelaySecond = 10;
    static const int  kCacheFrame = 3;
    static inline enum video_format convert_pixel_format(int f)
    {
        switch (f) {
        case AV_PIX_FMT_NONE:    return VIDEO_FORMAT_NONE;
        case AV_PIX_FMT_YUV420P: return VIDEO_FORMAT_I420;
        case AV_PIX_FMT_YUVJ420P: return VIDEO_FORMAT_I420;
        case AV_PIX_FMT_NV12:    return VIDEO_FORMAT_NV12;
        case AV_PIX_FMT_YUYV422: return VIDEO_FORMAT_YUY2;
        case AV_PIX_FMT_UYVY422: return VIDEO_FORMAT_UYVY;
        case AV_PIX_FMT_RGBA:    return VIDEO_FORMAT_RGBA;
        case AV_PIX_FMT_BGRA:    return VIDEO_FORMAT_BGRA;
        case AV_PIX_FMT_BGR0:    return VIDEO_FORMAT_BGRX;
        default:;
        }

        return VIDEO_FORMAT_NONE;
    }

    static inline enum audio_format convert_sample_format(int f)
    {
        switch (f) {
        case AV_SAMPLE_FMT_U8:   return AUDIO_FORMAT_U8BIT;
        case AV_SAMPLE_FMT_S16:  return AUDIO_FORMAT_16BIT;
        case AV_SAMPLE_FMT_S32:  return AUDIO_FORMAT_32BIT;
        case AV_SAMPLE_FMT_FLT:  return AUDIO_FORMAT_FLOAT;
        case AV_SAMPLE_FMT_U8P:  return AUDIO_FORMAT_U8BIT_PLANAR;
        case AV_SAMPLE_FMT_S16P: return AUDIO_FORMAT_16BIT_PLANAR;
        case AV_SAMPLE_FMT_S32P: return AUDIO_FORMAT_32BIT_PLANAR;
        case AV_SAMPLE_FMT_FLTP: return AUDIO_FORMAT_FLOAT_PLANAR;
        default:;
        }

        return AUDIO_FORMAT_UNKNOWN;
    }

    static inline enum video_colorspace convert_color_space(enum AVColorSpace s)
    {
        return s == AVCOL_SPC_BT709 ? VIDEO_CS_709 : VIDEO_CS_DEFAULT;
    }

    static inline enum video_range_type convert_color_range(enum AVColorRange r)
    {
        return r == AVCOL_RANGE_JPEG ? VIDEO_RANGE_FULL : VIDEO_RANGE_DEFAULT;
    }

    static int open_codec_context(int *stream_idx,
        AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
    {
        int ret, stream_index;
        AVStream *st;
        AVCodec *dec = NULL;
        AVDictionary *opts = NULL;

        ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
        if (ret < 0)
        {
            return ret;
        }
        else
        {
            stream_index = ret;
            st = fmt_ctx->streams[stream_index];

            dec = avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec)
            {
                return AVERROR(EINVAL);
            }

            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx) {
                return AVERROR(ENOMEM);
            }

            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
            {
                return ret;
            }

            av_dict_set(&opts, "refcounted_frames", "0",0);
            if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
            {
                return ret;
            }
            *stream_idx = stream_index;
        }

        return 0;
    }

    static void VideoFrameToObsFrame(const AVFrame& avframe, obs_source_frame& frame,
        const AVFormatContext& fmt_ctx,int stream_idx)
    {
        enum video_colorspace new_space;
        enum video_range_type new_range;

        bool flip = avframe.linesize[0] < 0 && avframe.linesize[1] == 0;

        for (size_t i = 0; i < MAX_AV_PLANES; i++)
        {
            frame.data[i] = avframe.data[i];
            frame.linesize[i] = abs(avframe.linesize[i]);
        }

        if (flip)
        {
            frame.data[0] -= frame.linesize[0] * (avframe.height - 1);
        }

        new_space = convert_color_space(avframe.colorspace);
        new_range = convert_color_range(avframe.color_range);
        AVStream* stream = fmt_ctx.streams[stream_idx];

        frame.full_range = new_range == VIDEO_RANGE_FULL;
        frame.timestamp = uint64_t((avframe.pts * double(stream->time_base.num) / stream->time_base.den) * 1000000000L);
        frame.flip = flip;

        video_format_get_parameters(new_space, new_range,
            frame.color_matrix,
            frame.color_range_min,
            frame.color_range_max);
    }

    static void AudioFrameToObsFrame(const AVFrame& avframe, obs_source_audio& frame,
        const AVFormatContext& fmt_ctx, int stream_idx)
    {
        for (size_t i = 0; i < MAX_AV_PLANES; i++)
        {
            frame.data[i] = avframe.data[i];
        }

        AVStream* stream = fmt_ctx.streams[stream_idx];

        frame.samples_per_sec = avframe.sample_rate;
        frame.speakers = (enum speaker_layout)avframe.channels;
        frame.format = convert_sample_format(avframe.format);
        frame.frames = avframe.nb_samples;
        frame.timestamp = uint64_t((avframe.pts * double(stream->time_base.num) / stream->time_base.den) * 1000000000L);

    }
}

class AVFramePtr {
    AVFrame* frame_;

    void Dispose()
    {
        if (frame_)
        {
            av_frame_free(&frame_);
        }
    }

    void CopyFrom(AVFrame* frame)
    {
        Dispose();
        if (frame)
            frame_ = av_frame_clone(frame);
    }

public:
    operator AVFrame* () const {
        return frame_;
    }

    AVFramePtr(AVFrame* frame) {
        frame_ = 0;
        CopyFrom(frame);
    }

    AVFramePtr(const AVFramePtr& frame) {
        frame_ = 0;
        CopyFrom(frame.frame_);
    }

    void Attach(AVFrame* frame)
    {
        Dispose();
        frame_ = frame;
    }

    AVFramePtr& operator = (const AVFramePtr& frame) {
        if (this != &frame) {
            CopyFrom(frame.frame_);
        }
        return *this;
    }

    AVFrame* operator -> () const
    {
        return frame_;
    }

    AVFramePtr() {
        frame_ = 0;
    }

    ~AVFramePtr() {
        Dispose();
    }
};

class ReceiverSourceImpl :public std::enable_shared_from_this<ReceiverSourceImpl>
{
public:
    using Ptr = std::shared_ptr<ReceiverSourceImpl>;
    ReceiverSourceImpl();
    ~ReceiverSourceImpl();

    void InitReceiver();
    bool CreateSocket();
    bool ConnectIOSDevice(bool use_cur);
    void SetRecvType(receiver_type type);

    int ReceiveFromURL();
    int ReceiveFromIOS();

    void ReceiverProcess();
    void ReceiverStop();
    void ReceiverClear();

    void SendRtmpRunning();
    void SendRtmpListening();
    void SendRtmpStatus();
    void SendRtmpReset();

    void SendIOSServiceStatus(bool normal);
    void SendIOSDeviceCount(int count);
    void SendIOSConnection(bool succeeded);
    void SendIOSRecvStatus(int status);

    static void InitThread(std::shared_ptr<ReceiverSourceImpl> lparam);

    obs_source_t *source_;

    SOCKET sock_;

    obs_data_t *settings_;

    receiver_type recv_type_;

    std::atomic_bool is_receiving_;

    std::atomic_bool is_create_tips_image_frame_;

private:
    void OnIOSStreamingFailed();
    void OnIOSStreamingInterrupted();

    void SendPacketToObs(const AVPacket& packet,
        bool& has_first_pts, int64_t& first_pts,
        int& preloaded_video_frames,
        bool& is_preloading_video);

private:
    int video_stream_idx_;
    int audio_stream_idx_;
    AVCodecContext* video_dec_ctx_;
    AVCodecContext* audio_dec_ctx_;
    AVFormatContext* ifmt_ctx_;
    std::string file_name_;
    std::thread receiver_thread_;
    std::atomic_bool is_aborting_;
    std::atomic_bool is_stop_;
    time_t last_time_;
    time_t last_open_input_time_;

    std::string name_;
    std::ofstream file_;
    ios::IOSBridge ios_bridge_;
    ios::DeviceSelector dev_sel_;
    std::string selected_dev_;
    int ios_exit_code_ = 0;
    bool exiting_ = false;
};

ReceiverSourceImpl::ReceiverSourceImpl()
    : video_stream_idx_(-1)
    , audio_stream_idx_(-1)
    , video_dec_ctx_(nullptr)
    , audio_dec_ctx_(nullptr)
    , ifmt_ctx_(nullptr)
    , source_(nullptr)
    , sock_(SOCKET_ERROR)
    , settings_(nullptr)
    , last_time_(0)
    , last_open_input_time_(0)
    , recv_type_(RECV_T_WIRELESS)
{
    is_aborting_ = false;
    is_stop_ = false;
    is_receiving_ = false;
}

ReceiverSourceImpl::~ReceiverSourceImpl()
{
    ReceiverClear();
    if (sock_ != SOCKET_ERROR)
    {
        closesocket(sock_);
    }
}

void ReceiverSourceImpl::InitThread(std::shared_ptr<ReceiverSourceImpl> ptr)
{
    while (ptr->is_stop_ == false)
    {
        ptr->ReceiverClear();
        ptr->ReceiverProcess();
    }
}

void ReceiverSourceImpl::InitReceiver()
{
    name_ = obs_source_get_name(source_);
    recv_type_ = (receiver_type)obs_data_get_int(settings_, kRecvType);

    if (recv_type_ == RECV_T_WIRELESS) {
        CreateSocket();
    }

    receiver_thread_ = std::thread(InitThread, shared_from_this());
}

bool ReceiverSourceImpl::CreateSocket() {
    blog(LOG_INFO, "[receiver source] receiver_source_create");
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        blog(LOG_ERROR, "[receiver source] receiver_source_create failed：sock == INVALID_SOCKET");
        return false;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);
    sin.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        closesocket(sock);
        blog(LOG_ERROR, "[receiver source] receiver_source_create failed：bind socket error");
        return false;
    }

    sockaddr addr;
    int len = sizeof(addr);
    int ret = getsockname(sock, &addr, &len);

    if (addr.sa_family != AF_INET)
    {
        closesocket(sock);
        blog(LOG_ERROR, "[receiver source] receiver_source_create failed：addr.sa_family != AF_INET");
        return false;
    }

    sockaddr_in add_in;
    memcpy(&add_in, &addr, len);
    int port = ntohs(add_in.sin_port);

    std::string rtmp = kRtmpIp;
    rtmp += std::to_string(port);
    rtmp += kRtmpName;

    obs_data_set_int(settings_, kRtmpPort, port);

    sock_ = sock;
    file_name_ = rtmp;

    blog(LOG_INFO, "[receiver source] receiver_source_create port:%s", std::to_string(port).c_str());
    return true;
}

bool ReceiverSourceImpl::ConnectIOSDevice(bool use_cur) {
    int ret;
    std::string dev;
    if (use_cur) {
        dev = selected_dev_;
    } else {
        std::vector<std::string> devs;
        ret = ios::IOSBridge::GetDevices(&devs);

        if (ret == ios::IOSBridge::RC_E_NO_SERVICE) {
            SendIOSServiceStatus(false);
            return false;
        }
        SendIOSServiceStatus(true);

        if (ret == ios::IOSBridge::RC_E_NO_DEVICE) {
            SendIOSDeviceCount(0);
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
            return false;
        }
        SendIOSDeviceCount((int)devs.size());

        dev = dev_sel_.Select(devs);
        if (dev.empty()) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
            return false;
        }
    }

    ret = ios_bridge_.Connect(kIOSPort, dev.c_str());
    if (ret != ios::IOSBridge::RC_OK) {
        SendIOSConnection(false);
        dev_sel_.RemoveFromGRL(dev);
        return false;
    }

    if (!ios_bridge_.Subscribe()) {
        SendIOSConnection(false);
        dev_sel_.RemoveFromGRL(dev);
        return false;
    }
    SendIOSConnection(true);
    selected_dev_ = dev;
    return true;
}

void ReceiverSourceImpl::SetRecvType(receiver_type type) {
    if (type != recv_type_) {
        ReceiverStop();
        ReceiverClear();
        if (sock_ != SOCKET_ERROR) {
            closesocket(sock_);
            sock_ = SOCKET_ERROR;
        }

        recv_type_ = type;
        is_stop_ = false;
        is_receiving_ = false;

        InitReceiver();
        SendRtmpReset();
    }
}

int ReceiverSourceImpl::ReceiveFromURL() {
    AVDictionary* options_dict = nullptr;
    av_dict_set(&options_dict, "listen", AV_STRINGIFY(1), 0);
    ifmt_ctx_ = avformat_alloc_context();
    ifmt_ctx_->interrupt_callback.opaque = this;
    ifmt_ctx_->interrupt_callback.callback = [](void* pthis) {
        auto This = static_cast<ReceiverSourceImpl*>(pthis);
        if (This->last_time_ > 0 && This->is_aborting_ == false &&
            time(nullptr) - This->last_time_ > kDelaySecond)
        {
            blog(LOG_ERROR, "[receiver source] timeout");
            This->is_aborting_ = true;
        }
        if (This->last_open_input_time_ > 0 && This->is_aborting_ == false &&
            time(nullptr) - This->last_open_input_time_ > kAVinputOpenDelaySecond)
        {
            This->is_aborting_ = true;
        }
        return This->is_aborting_ ? 1 : 0;
    };
    last_open_input_time_ = time(nullptr);
    int ret = avformat_open_input(&ifmt_ctx_, file_name_.c_str(), NULL, &options_dict);
    last_open_input_time_ = 0;
    av_dict_free(&options_dict);
    return ret;
}

int ReceiverSourceImpl::ReceiveFromIOS() {
    if (ios_exit_code_ != IOS_EC_END) {
        bool use_cur = ios_exit_code_ == IOS_EC_CUR &&
            !selected_dev_.empty();

        if (!ConnectIOSDevice(use_cur)) {
            ios_exit_code_ = 0;
            selected_dev_.clear();
            return AVERROR_EXIT;
        }
    }

    /*file_.close();
    file_.open("D:\\test.flv", std::ios::binary);*/

    ifmt_ctx_ = avformat_alloc_context();
    ifmt_ctx_->interrupt_callback.opaque = this;
    ifmt_ctx_->interrupt_callback.callback = [](void* pthis) {
        auto This = static_cast<ReceiverSourceImpl*>(pthis);
        if (This->last_time_ > 0 && This->is_aborting_ == false &&
            time(nullptr) - This->last_time_ > kDelaySecond)
        {
            blog(LOG_ERROR, "[receiver source] timeout");
            This->is_aborting_ = true;
        }
        if (This->last_open_input_time_ > 0 && This->is_aborting_ == false &&
            time(nullptr) - This->last_open_input_time_ > kAVinputOpenDelaySecond)
        {
            This->is_aborting_ = true;
        }
        return This->is_aborting_ ? 1 : 0;
    };

    last_open_input_time_ = time(nullptr);

    ifmt_ctx_->pb = avio_alloc_context(
        nullptr, 0, 0, this,
        [](void* opaque, uint8_t* buf, int buf_size) -> int
        {
            if (buf_size == 0) {
                return 0;
            }

            auto This = static_cast<ReceiverSourceImpl*>(opaque);
            if (This->exiting_) {
                return AVERROR_EOF;
            }

            int ret = This->ios_bridge_.Recv(buf, buf_size);
            if (ret > 0) {
                //This->file_.write((char*)buf, ret);
            } else {
                switch (ret) {
                case ios::IOSBridge::RC_E_RECV: This->ios_exit_code_ = IOS_EC_ERROR; break;
                case ios::IOSBridge::RC_S_END:  This->ios_exit_code_ = IOS_EC_END; This->exiting_ = true; break;
                default: This->ios_exit_code_ = IOS_EC_ERROR; break;
                }

                ret = AVERROR_EOF;
            }
            return ret;
        },
        nullptr, nullptr);

    int ret = avformat_open_input(&ifmt_ctx_, NULL, NULL, NULL);
    last_open_input_time_ = 0;
    if (ret != 0) {
        OnIOSStreamingFailed();
    }
    return ret;
}

void ReceiverSourceImpl::ReceiverProcess()
{
    int ret;

    switch (recv_type_) {
    case RECV_T_WIRELESS:
        ret = ReceiveFromURL();
        break;

    case RECV_T_WIRED_IOS:
        ret = ReceiveFromIOS();
        break;

    default:
        ret = AVERROR_EXIT;
        break;
    }

    if (ret != 0)
    {
        //调ReceiverStop()退出的返回值为AVERROR_EXIT 视为正常退出
        if (ret != AVERROR_EXIT)
        {
            char buf[MAX_PATH]{0};
            av_strerror(ret, buf, MAX_PATH);
            blog(LOG_ERROR, "[receiver source] avformat_open_input:failed url:%s  reason:%s", file_name_.c_str(), buf);
            Sleep(100);
        }
        return;
    }

    if (avformat_find_stream_info(ifmt_ctx_, NULL) < 0)
    {
        OnIOSStreamingFailed();
        blog(LOG_ERROR, "[receiver source] avformat_find_stream_info:failed");
        return;
    }

    std::string encoder_info;
    AVDictionaryEntry *av_dictionary_entry = NULL;
    av_dictionary_entry = av_dict_get(ifmt_ctx_->metadata, "Encoder", av_dictionary_entry, 0);
    if (av_dictionary_entry)
    {
        encoder_info = av_dictionary_entry->value;
    }
    else
    {
        encoder_info = "";
    }
    if (settings_)
    {
        obs_data_set_string(settings_, kEncoderInfo, encoder_info.c_str());
    }

    SendRtmpRunning();

    //preload status variables
    bool is_preloading_video = true;
    int preloaded_video_frames = 0;

    is_receiving_ = true;

    int video_ret = open_codec_context(&video_stream_idx_, &video_dec_ctx_, ifmt_ctx_, AVMEDIA_TYPE_VIDEO);
    int audio_ret = open_codec_context(&audio_stream_idx_, &audio_dec_ctx_, ifmt_ctx_, AVMEDIA_TYPE_AUDIO);
    if (video_ret != 0 && audio_ret != 0)
    {
        OnIOSStreamingFailed();
        blog(LOG_ERROR, "[receiver source] open_codec_context:failed");
        return;
    }

    SendIOSRecvStatus(IOS_SS_STARTED);

    bool has_first_pts = false;
    int64_t first_pts = 0;

    int video_packet_count = 0;
    time_t last_info_time = time(nullptr);

    while (true)
    {
        AVPacket pkt;
        av_init_packet(&pkt);

        if (av_read_frame(ifmt_ctx_, &pkt) < 0)
        {
            av_packet_unref(&pkt);
            break;
        }
        last_time_ = time(nullptr);

        if ((last_time_ - last_info_time) >= 10)
        {
            blog(LOG_INFO, "[receiver source]  fps = %.2f", video_packet_count / 10.0f);
            video_packet_count = 0;
            last_info_time = last_time_;
        }
        else if (pkt.stream_index == video_stream_idx_)
        {
            ++video_packet_count;
        }
        SendPacketToObs(pkt, has_first_pts, first_pts, preloaded_video_frames, is_preloading_video);
        av_packet_unref(&pkt);
    }

    exiting_ = false;
    is_receiving_ = false;

    SendRtmpListening();
    OnIOSStreamingInterrupted();
}

void ReceiverSourceImpl::OnIOSStreamingFailed() {
    if (recv_type_ == RECV_T_WIRED_IOS) {
        ios_bridge_.Close();
        dev_sel_.RemoveFromGRL(selected_dev_);
        selected_dev_.clear();
        SendIOSRecvStatus(IOS_SS_FAILED);
    }
}

void ReceiverSourceImpl::OnIOSStreamingInterrupted() {
    if (recv_type_ == RECV_T_WIRED_IOS) {
        /**
         * 退出时，看看是不是 IOS 主动要求我们退出的。如果是，仍然维持连接，随后仅初始化 ffmpeg；
         * 如果不是，则断开连接，但下次循环仍然使用这个设备重试一次。
         */
        if (ios_exit_code_ != IOS_EC_END) {
            ios_bridge_.Unsubscribe();
            ios_bridge_.Close();
            ios_exit_code_ = IOS_EC_CUR;
            SendIOSRecvStatus(IOS_SS_INTR);
        } else {
            SendIOSRecvStatus(IOS_SS_STOPPED);
        }
    }
}

static void adjustTimeStamp(bool& hasFirstPts, int64_t& firstPts, uint64_t& pts)
{
    if (hasFirstPts == false)
    {
        hasFirstPts = true;
        firstPts = pts;
    }
    pts -= firstPts;
    if (pts > std::numeric_limits<uint64_t>::max() / 2)
        pts = 0;
}

void ReceiverSourceImpl::SendPacketToObs(const AVPacket& packet,
    bool& has_first_pts, int64_t& first_pts,
    int& preloaded_video_frames,
    bool& is_preloading_video)
{
    if (video_stream_idx_ == packet.stream_index)
    {
        if (avcodec_send_packet(video_dec_ctx_, &packet) == 0)
        {
            while (true)
            {
                AVFramePtr frame;
                frame.Attach(av_frame_alloc());
                int recvFrameRet = avcodec_receive_frame(video_dec_ctx_, frame);
                if (recvFrameRet == AVERROR(EAGAIN))
                    break;
                if (recvFrameRet < 0 && recvFrameRet != AVERROR(EAGAIN))
                    break;

                obs_source_frame obs_frame = {};
                obs_frame.format = convert_pixel_format(frame->format);
                obs_frame.width = frame->width;
                obs_frame.height = frame->height;
                VideoFrameToObsFrame(*frame, obs_frame, *ifmt_ctx_, video_stream_idx_);
                adjustTimeStamp(has_first_pts, first_pts, obs_frame.timestamp);

                if (is_preloading_video == true)
                {
                    obs_source_preload_video(source_, &obs_frame);
                    ++preloaded_video_frames;
                    if (preloaded_video_frames >= kCacheFrame)
                    {
                        is_preloading_video = false;
                        obs_source_show_preloaded_video(source_);
                    }
                }
                else
                    obs_source_output_video(source_, &obs_frame);
            }

        }
    }
    else if (audio_stream_idx_ == packet.stream_index)
    {
        if (avcodec_send_packet(audio_dec_ctx_, &packet) == 0)
        {
            while (true)
            {
                AVFramePtr frame;
                frame.Attach(av_frame_alloc());
                int recvFrameRet = avcodec_receive_frame(audio_dec_ctx_, frame);
                if (recvFrameRet == AVERROR(EAGAIN))
                    break;
                if (recvFrameRet < 0 && recvFrameRet != AVERROR(EAGAIN))
                    break;

                obs_source_audio obs_audio{};
                AudioFrameToObsFrame(*frame, obs_audio, *ifmt_ctx_, audio_stream_idx_);
                adjustTimeStamp(has_first_pts, first_pts, obs_audio.timestamp);
                obs_source_output_audio(source_, &obs_audio);
            }
        }
    }
}

void ReceiverSourceImpl::ReceiverClear()
{
    if (ifmt_ctx_)
    {
        avformat_close_input(&ifmt_ctx_);
        ifmt_ctx_ = nullptr;
    }

    if (video_dec_ctx_)
    {
        avcodec_free_context(&video_dec_ctx_);
        video_dec_ctx_ = nullptr;
    }

    if (audio_dec_ctx_)
    {
        avcodec_free_context(&audio_dec_ctx_);
        audio_dec_ctx_ = nullptr;
    }

    last_time_ = 0;
    last_open_input_time_ = 0;
    video_stream_idx_ = -1;
    audio_stream_idx_ = -1;
    is_aborting_ = false;
}

void ReceiverSourceImpl::ReceiverStop()
{
    blog(LOG_INFO, "[receiver source] ReceiverStop()");

    dev_sel_.ClearLRL();
    dev_sel_.RemoveFromGRL(selected_dev_);
    selected_dev_.clear();
    is_stop_ = true;
    is_aborting_ = true;

    ios_bridge_.Disconnect();

    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }

    ios_bridge_.Close();
}

void ReceiverSourceImpl::SendRtmpRunning() {
    if (recv_type_ != RECV_T_WIRELESS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kRtmpStatus, RECEIVER_RUNNING);
        signal_handler_signal(signal, kReceiverStatus, &calldata);
        calldata_free(&calldata);

        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kRtmpStatus, RECEIVER_START);
        signal_handler_signal(signal, kReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendRtmpListening() {
    if (recv_type_ != RECV_T_WIRELESS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kRtmpStatus, RECEIVER_LISTEN);
        signal_handler_signal(signal, kReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendRtmpStatus() {
    if (recv_type_ != RECV_T_WIRELESS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        if (is_receiving_ == true)
        {
            calldata_set_int(&calldata, kRtmpStatus, RECEIVER_RUNNING);
        } else
        {
            calldata_set_int(&calldata, kRtmpStatus, RECEIVER_LISTEN);
        }
        signal_handler_signal(signal, kReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendRtmpReset() {
    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kRtmpStatus, RECEIVER_RESET);
        signal_handler_signal(signal, kReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendIOSServiceStatus(bool normal) {
    if (recv_type_ != RECV_T_WIRED_IOS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kIOSEventType, IOS_EV_SRV_STATUS);
        calldata_set_int(&calldata, kIOSIntValue, normal ? 1 : 0);
        signal_handler_signal(signal, kIOSReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendIOSDeviceCount(int count) {
    if (recv_type_ != RECV_T_WIRED_IOS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kIOSEventType, IOS_EV_DEV_COUNT);
        calldata_set_int(&calldata, kIOSIntValue, count);
        signal_handler_signal(signal, kIOSReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendIOSConnection(bool succeeded) {
    if (recv_type_ != RECV_T_WIRED_IOS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kIOSEventType, IOS_EV_CONN_STATUS);
        calldata_set_int(&calldata, kIOSIntValue, succeeded ? 1 : 0);
        signal_handler_signal(signal, kIOSReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}

void ReceiverSourceImpl::SendIOSRecvStatus(int status) {
    if (recv_type_ != RECV_T_WIRED_IOS) {
        return;
    }

    signal_handler_t* signal = obs_source_get_signal_handler(source_);
    if (signal)
    {
        calldata_t calldata;
        calldata_init(&calldata);
        calldata_set_ptr(&calldata, "source", source_);
        calldata_set_int(&calldata, kIOSEventType, IOS_EV_RECV_STATUS);
        calldata_set_int(&calldata, kIOSIntValue, status);
        signal_handler_signal(signal, kIOSReceiverStatus, &calldata);
        calldata_free(&calldata);
    }
}


class ReceiverSource
{
public:
    ReceiverSourceImpl::Ptr impl;
};

//未收到数据时配图
static bool CreateTipsImageFrame(
    obs_source_t* source, obs_data_t* settings, receiver_type type)
{
    if (!source || !settings)
    {
        return false;
    }
    if (!obs_data_has_user_value(settings, kIsLandscapeModel))
    {
        return false;
    }

    const char* file_name[][2]{
        {"projection_suggestion_h.png", "projection_suggestion_v.png"},
        {"projection_suggestion_ios_h.png", "projection_suggestion_ios_v.png"},
    };

    bool is_landscape_model = obs_data_get_bool(settings, kIsLandscapeModel);
    obs_source_frame* tips_image_frame = nullptr;

    char* image_file = obs_module_file(file_name[type == RECV_T_WIRELESS ? 0 : 1][is_landscape_model ? 0 : 1]);
    if (image_file)
    {
        uint32_t image_width, image_height;
        gs_color_format image_format;
        uint8_t* image_data = gs_create_texture_file_data(
            image_file, &image_format, &image_width, &image_height);
        if (image_data)
        {
            video_format image_frame_format;
            switch (image_format)
            {
            case GS_RGBA:
                image_frame_format = VIDEO_FORMAT_RGBA;
                break;
            case GS_BGRX:
                image_frame_format = VIDEO_FORMAT_BGRX;
                break;
            case GS_BGRA:
                image_frame_format = VIDEO_FORMAT_BGRA;
                break;
            default:
                image_frame_format = VIDEO_FORMAT_NONE;
                blog(LOG_ERROR, "[receiver-source] Unsupport color format for fail_image");
                break;
            }

            if (image_frame_format != VIDEO_FORMAT_NONE)
            {
                obs_source_frame* frame = obs_source_frame_create(
                    image_frame_format, image_width, image_height);
                if (frame)
                {
                    memcpy(frame->data[0], image_data, image_width * image_height * 4);
                    tips_image_frame = frame;
                }
            }
            bfree(image_data);
        }
    }

    //因为有缓存，所以只设置一次图片数据就够了。
    if (source && tips_image_frame)
    {
        obs_source_output_video(source, tips_image_frame);
        obs_source_frame_destroy(tips_image_frame);
        return true;
    }
    return false;
}

static const char *receiver_source_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("ReceiverSource");
}

static void *receiver_source_create(obs_data_t *settings, obs_source_t *source)
{
    ReceiverSource *p = new ReceiverSource();
    p->impl = std::make_shared<ReceiverSourceImpl>();
    p->impl->source_ = source;
    p->impl->settings_ = settings;
    p->impl->InitReceiver();
    p->impl->is_create_tips_image_frame_ = false;
    if (settings)
    {
        p->impl->is_create_tips_image_frame_ =
            CreateTipsImageFrame(p->impl->source_, settings, p->impl->recv_type_);
    }
    return p;
}

static void receiver_source_destroy(void *data)
{
    if (data)
    {
        ReceiverSource* p = static_cast<ReceiverSource*>(data);
        p->impl->ReceiverStop();
        delete p;
    }
}

static void receiver_source_defaults(obs_data_t *settings)
{
    obs_data_set_default_int(settings, kRtmpPort, -1);
    obs_data_set_default_string(settings, kEncoderInfo, "");
    obs_data_set_default_bool(settings, kIsLandscapeModel, false);
    obs_data_set_default_int(settings, kRecvType, RECV_T_WIRELESS);
}

static obs_properties_t *receiver_source_getproperties(void *data)
{
    obs_properties_t *props = obs_properties_create();
    return props;
}

static void receiver_source_activate(void *data)
{
    //切换场景会清掉TipsImage数据，这里再处理一下
    if (data)
    {
        ReceiverSource* p = static_cast<ReceiverSource*>(data);
        if (!p->impl->is_receiving_)
        {
            p->impl->is_create_tips_image_frame_ =
                CreateTipsImageFrame(p->impl->source_, p->impl->settings_, p->impl->recv_type_);
        }
    }
}

static void receiver_source_deactivate(void *data)
{
}

static void receiver_source_update(void *data, obs_data_t *settings)
{
    if (data)
    {
        ReceiverSource* p = static_cast<ReceiverSource*>(data);

        receiver_type type = (receiver_type)obs_data_get_int(settings, kRecvType);
        p->impl->SetRecvType(type);
        p->impl->is_create_tips_image_frame_ = false;

        if (!p->impl->is_create_tips_image_frame_ && settings)
        {
            p->impl->is_create_tips_image_frame_ =
                CreateTipsImageFrame(p->impl->source_, settings, p->impl->recv_type_);
        }

        p->impl->SendRtmpStatus();
    }
}


void register_receiver_source()
{
    struct obs_source_info source = { 0 };
    source.id = "receiver_source";
    source.type = OBS_SOURCE_TYPE_INPUT;
    source.output_flags =
        OBS_SOURCE_ASYNC_VIDEO |
        OBS_SOURCE_AUDIO |
        OBS_SOURCE_DO_NOT_DUPLICATE;
    source.get_name = receiver_source_getname;
    source.create = receiver_source_create;
    source.destroy = receiver_source_destroy;
    source.get_defaults = receiver_source_defaults;
    source.get_properties = receiver_source_getproperties;
    source.activate = receiver_source_activate;
    source.deactivate = receiver_source_deactivate;
    source.update = receiver_source_update;
    obs_register_source_s(&source, sizeof(source));
}