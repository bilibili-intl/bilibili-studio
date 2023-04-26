#include <IAgoraRtcEngine.h>
#include <IAgoraMediaEngine.h>
#include <memory>
#include <vector>
#include <deque>
#include <future>
#include <mutex>
#include <string>
#include <chrono>
#include <unordered_map>
#include <process.h>

#include "colive-service.h"
#include "avdata-util.h"

#include <obs-module.h>

//#define DEBUG_COST_MEASURE

#ifdef DEBUG_COST_MEASURE
struct MeasureCost
{
    std::chrono::system_clock::time_point beginTime_;
    const char* fmt_;

    MeasureCost(const char* fmt)
    {
        beginTime_ = std::chrono::system_clock::now();
        fmt_ = fmt;
    }

    ~MeasureCost()
    {
        std::chrono::duration<double> dur = std::chrono::system_clock::now() - beginTime_;
        blog(LOG_INFO, fmt_, static_cast<int>(dur.count() * 1000));
    }
};
#else
struct MeasureCost
{
    MeasureCost(const char* fmt)
    {
    }

    ~MeasureCost()
    {
    }
};
#endif

using namespace bililive::colive;

namespace
{
    static const int LOCAL_DATA_BUFFER_FRAME_DURATION_MS = 5000;
    static const int LOCAL_DATA_AVSYNC_THRESHOLD_MS = 100;
    static const int LOCAL_AUDIO_DATA_PACKETS_CLEAR_THRESHOLD = 3;
    static const int DEBUG_FPS_INTERVAL_MS = 10000;
    static const int RETRY_PUBLISH_COUNT = 1;


    using VideoFrame = agora::media::IVideoFrameObserver::VideoFrame;
    using AudioFrame = agora::media::IAudioFrameObserver::AudioFrame;

    static bool StrIsNum(std::string& str)
    {
        if (str.empty())
            return false;

        for (size_t i = 0; i < str.length(); i++)
        {
            if (!isdigit(str[i]))
            {
                return false;
            }
        }

        return true;
    }

    class AgoraService;

    class AgoraVideoSource : public agora::media::IVideoFrameObserver
    {
        AgoraService* agoraService_;

    public:
        AgoraVideoSource(AgoraService* ptr);
        ~AgoraVideoSource();
        bool onCaptureVideoFrame(VideoFrame& videoFrame) override;
        bool onRenderVideoFrame(unsigned int uid, VideoFrame& videoFrame);
        void DebugOutputAverageDelayAndClear();

        void ClearStats();

    private:
        std::mutex captureFrameStatMutex_;
        uint64_t captureFramesDelaySum_;
        int captureFrameCount_;

        ColiveTime lastCaptureFPSCheckPoint_;
        int captureFrameCountSinceCheckPoint_;

        std::mutex renderFrameStatMutex_;
        std::unordered_map<unsigned int, ColiveTime> lastRenderFPSCheckPoint_;
        std::unordered_map<unsigned int, int> renderFrameCountSinceCheckPoint_;
    };


    class AgoraAudioSource : public agora::media::IAudioFrameObserver
    {
        AgoraService* agoraService_;
        bool isFirstLocalAudioCallback_;

    public:
        AgoraAudioSource(AgoraService* ptr);
        bool onRecordAudioFrame(AudioFrame& audioFrame) override;
        bool onPlaybackAudioFrame(AudioFrame& audioFrame) override;
        bool onMixedAudioFrame(AudioFrame& audioFrame) override;
        bool onPlaybackAudioFrameBeforeMixing(unsigned int uid, AudioFrame& audioFrame) override;
    };

    class AgoraService;

    class AgoraRtcCallback : public agora::rtc::IRtcEngineEventHandler
    {
    public:
        AgoraRtcCallback()
            : retry_count_(0)
            , need_retry_(false)
            , agoraRtcEngine_(nullptr)
        {
            retry_error_code_.insert(std::make_pair(1, false));
            retry_error_code_.insert(std::make_pair(10, false));
            retry_error_code_.insert(std::make_pair(154, false));
            retry_error_code_.insert(std::make_pair(155, false));
        }

        void SetCallback(IColiveController::OnControllerEvent::Ptr callback, agora::rtc::IRtcEngine* rtc_engine)
        {
            callback_ = callback;
            agoraRtcEngine_ = rtc_engine;
        }

        void SetRetry(bool retry)
        {
            need_retry_ = retry;
            retry_count_ = 0;
        }

        std::pair<bool, uint64_t> UidToMid(agora::rtc::uid_t uid)
        {
            agora::rtc::UserInfo info;
            int ret = agoraRtcEngine_->getUserInfoByUid(uid, &info);
            if (ret == 0)
            {
                std::string str_uid = info.userAccount;

                if (StrIsNum(str_uid))
                {
                    uint64_t mid = _atoi64(str_uid.c_str());
                    return std::pair<bool, uint64_t>(true, mid);
                }
            }

            return std::pair<bool, uint64_t>(false, 0);
        }

        void onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed) override
        {
            auto ptr = callback_.lock();
            if (ptr)
            {
                auto end_time = clock();
                std::string report = "cdn:72;status:join_channel;result:1;delay:";
                report += std::to_string(end_time - join_channel_start_time_);
                report += ";channel_id:";
                report += channel;
                report += ";self_id:";
                report += std::to_string(uid);
                ptr->OnReportInfo(report);
                ptr->OnJoinChannelSuccess();
            }
        }

        void onUserJoined(agora::rtc::uid_t uid, int elapsed) override {}

        void onUserInfoUpdated(agora::rtc::uid_t uid, const agora::rtc::UserInfo& info) override {
            auto ptr = callback_.lock();
            if (ptr)
            {
                std::pair<bool, uint64_t> ret = UidToMid(uid);
                if (!ret.first)
                {
                    blog(LOG_ERROR, "%s", "[agora service] onUserJoined: fail to get UserInfo");
                    onError(-1, nullptr);
                } else
                {
                    ptr->onUserJoined(ret.second);
                }
            }
        }

        void onUserOffline(agora::rtc::uid_t uid, agora::rtc::USER_OFFLINE_REASON_TYPE reason) override
        {
            auto ptr = callback_.lock();
            if (ptr)
            {
                std::pair<bool, uint64_t> ret = UidToMid(uid);
                if (!ret.first)
                {
                    blog(LOG_ERROR, "%s", "[agora service] onUserOffline: fail to get UserInfo");
                    onError(-1, nullptr);
                }
                else
                {
                    ptr->onUserOffline(ret.second, reason == agora::rtc::USER_OFFLINE_REASON_TYPE::USER_OFFLINE_QUIT);
                }
            }
        }

        void onLeaveChannel(const agora::rtc::RtcStats& stat) override
        {
            auto ptr = callback_.lock();
            if (ptr)
            {
                if (!channel_id_.empty() && !self_id_.empty() && !remote_id_.empty())
                {
                    std::string report = "cdn:72;status:rtc_stat;channel_id:";
                    report += channel_id_;
                    report += ";self_id:";
                    report += self_id_;
                    report += ";remote_id:";
                    report += remote_id_;
                    report += ";send_packet_loss:";
                    report += std::to_string(stat.txPacketLossRate);
                    report += ";receive_packet_loss:";
                    report += std::to_string(stat.rxPacketLossRate);
                    report += ";delay:";
                    report += std::to_string(stat.lastmileDelay);
                    report += ";send_bytes:";
                    report += std::to_string(stat.txBytes);
                    report += ";receive_bytes:";
                    report += std::to_string(stat.rxBytes);
                    report += ";send_bitrate:";
                    report += std::to_string(stat.txKBitRate);
                    report += ";receive_bitrate:";
                    report += std::to_string(stat.rxKBitRate);
                    report += ";audio_send_bytes:";
                    report += std::to_string(stat.txAudioBytes);
                    report += ";audio_receive_bytes:";
                    report += std::to_string(stat.rxAudioBytes);
                    report += ";video_send_bytes:";
                    report += std::to_string(stat.txVideoBytes);
                    report += ";video_receive_bytes:";
                    report += std::to_string(stat.rxVideoBytes);
                    report += ";audio_send_bitrate:";
                    report += std::to_string(stat.txAudioKBitRate);
                    report += ";audio_receive_bitrate:";
                    report += std::to_string(stat.rxAudioKBitRate);
                    report += ";video_send_bitrate:";
                    report += std::to_string(stat.txVideoKBitRate);
                    report += ";video_receive_bitrate:";
                    report += std::to_string(stat.rxVideoKBitRate);
                    auto leave_time = clock();
                    int64_t in_channel_time = (int64_t)((leave_time - join_channel_start_time_)/1000);
                    if (in_channel_time < 0)
                    {
                        in_channel_time = 0;
                    }
                    report += ";in_channel_time:";
                    report += std::to_string(in_channel_time);
                    report += ";report_type:";
                    report += std::to_string(report_type_);
                    ptr->OnReportInfo(report);
                }
                ptr->OnLeaveChannel();
            }
            channel_id_ = "";
            self_id_ = "";
            remote_id_ = "";
        }

        void onConnectionInterrupted() override
        {
            auto ptr = callback_.lock();
            if (ptr)
                ptr->onConnectionInterrupted();
        }

        void onConnectionLost() override
        {
            auto ptr = callback_.lock();
            if (ptr)
            {
                std::string report = "cdn:72;status:colive_sdk_error;reason:2";
                ptr->OnReportInfo(report);
                ptr->onConnectionLost();
            }
        }

        void onConnectionBanned() override
        {
            auto ptr = callback_.lock();
            if (ptr)
                ptr->OnConnectionBanned();
        }

        void onStreamPublished(const char* url, int error) override
        {
            if (need_retry_) {
                if (retry_count_ < RETRY_PUBLISH_COUNT) {
                    if (retry_error_code_.find(error) != retry_error_code_.end()) {
                        if (error == 154 || error == 1 || error == 10) {
                            agoraRtcEngine_->removePublishStreamUrl(url);
                            Sleep(1000);
                        }
                        agoraRtcEngine_->addPublishStreamUrl(url, true);
                        retry_count_++;
                        return;
                    }
                }
            }

            retry_count_ = 0;
            auto ptr = callback_.lock();
            if (ptr)
            {
                if (error == 0)
                {
                    auto end_time = clock();
                    std::string report = "cdn:72;status:set_publish;result:1;delay:";
                    report += std::to_string(end_time - set_publish_start_time_);
                    ptr->OnReportInfo(report);
                }
                else
                {
                    std::string report = "cdn:72;status:set_publish;result:0;reason:4;error:";
                    report += std::to_string(error);
                    ptr->OnReportInfo(report);
                }

                ptr->onStreamPublished(url, error);
            }
        }

        void onStreamUnpublished(const char* url) override
        {
            auto ptr = callback_.lock();
            if (ptr)
                ptr->onStreamUnpublished(url);
        }

        void onStreamMessageError(
            agora::rtc::uid_t uid, int streamId, int code, int missed, int cached) override
        {
            auto ptr = callback_.lock();
            if (ptr)
            {
                std::pair<bool, uint64_t> ret = UidToMid(uid);
                if (!ret.first)
                {
                    blog(LOG_ERROR, "%s", "[agora service] onStreamMessageError: fail to get UserInfo");
                    onError(-1, nullptr);
                }
                else
                {
                    ptr->onStreamMessageError(ret.second, code);
                }
            }
        }

        void onError(int err, const char* msg) override
        {
            auto ptr = callback_.lock();
            if (ptr)
            {
                std::string report = "cdn:72;status:colive_sdk_error;reason:1;error_code:";
                report += std::to_string(err);
                ptr->OnReportInfo(report);
                ptr->OnError(err);
            }
        }

        void onFirstRemoteVideoDecoded(agora::rtc::uid_t uid, int width, int height, int elapsed) override
        {
            auto ptr = callback_.lock();
            if (ptr)
                ptr->OnFirstRemoteVideoDecoded();
        }

        void ReportInfo(const std::string& info)
        {
            auto ptr = callback_.lock();
            if (ptr)
                ptr->OnReportInfo(info);
        }

        void ReportSetJoinChannelStartTime(clock_t time)
        {
            join_channel_start_time_ = time;
        }

        void ReportSetPublishStartTime(clock_t time)
        {
            set_publish_start_time_ = time;
        }

        void ReportSetLeaveChannelInfo(const std::string& channel_id, const std::string& self_id, const std::string& remote_id)
        {
            channel_id_ = channel_id;
            self_id_ = self_id;
            remote_id_ = remote_id;
        }

        void ReportSetType(int type)
        {
            report_type_ = type;
        }
    private:
        bool need_retry_;
        int retry_count_;
        std::unordered_map<unsigned int, bool> retry_error_code_;
        IColiveController::OnControllerEvent::WPtr callback_;
        agora::rtc::IRtcEngine* agoraRtcEngine_;
        clock_t join_channel_start_time_ = 0;
        clock_t set_publish_start_time_ = 0;
        std::string channel_id_;
        std::string self_id_;
        std::string remote_id_;
        int report_type_ = 0;
    };


    class AgoraService : public IColiveController, public IColiveMedia, public std::enable_shared_from_this<AgoraService>
    {
        static std::mutex sInstanceMutex_;
        static std::weak_ptr<AgoraService> sInstance_;

    public:
        static std::shared_ptr<AgoraService> Create(
            IColiveController::OnControllerEvent::Ptr callback,
            const char* app_id, const char* access_key, const char* cookies);
        static std::shared_ptr<AgoraService> Aquire();

    public:
        AgoraService();
        ~AgoraService();

        bool Initialize(
            OnControllerEvent::Ptr callback, const char* agora_appid,
            const char* access_key, const char* cookies) override;
        bool JoinChannel(
            const uint64_t uid[], int uid_size, const char* roomId, int type, int report_type, int send_type) override;
        bool LeaveChannel() override;
        void SetSubscribe(uint64_t uid, const int* recev, size_t recev_count) override
        {
        }

        bool SetRemotePlayVolume(uint64_t uid, double volume) override
        {
            return false;
        }

        void GetAudioLevel() override
        {
        }

        void SetSend(int type, bool send) override
        {
        }

        bool GetRealtimeStats() override
        {
            return false;
        }

        bool Start() override;
        bool Stop() override;

        bool SetRTMPPublish(const std::string& url) override;
        bool SetVideoParam(const bililive::colive::VideoParamInfo& info) override;
        bool SetAudioParam(size_t channels, size_t samplerate, size_t colive_bitrate, size_t rtmp_bitrate) override;

        void PushRemoteVideo(uint64_t uid, std::unique_ptr<VFrame> vFrame) override;
        void PushRemoteAudio(uint64_t uid, std::unique_ptr<AFrame> aFrame) override;

        void PushLocalVideo(std::shared_ptr<VFrame> vFrame) override;
        void PushLocalAudio(std::unique_ptr<AFrame> aFrame) override;
        void SetRemoteMute(bool mute) override;
        bool PushChannelData(const char* data, size_t len) override
        {
            return false;
        }
        int PushSCTPData(uint32_t sid,const char* data, size_t len, bool ordered, bool reliable) override
        {
            return -1;
        }

        bool AddSCTPSid(uint32_t sid) override
        {
            return false;
        }
        bool RemoveSCTPSid(uint32_t sid) override
        {
            return false;
        }
        std::shared_ptr<VFrame> PullLocalVideo();
        std::unique_ptr<AFrame> PullLocalAudio(size_t maxSamples);

        void ClearOutdatedLocalMediaData();

        void AddOnRemoteMedia(OnRemoteMedia::Ptr callback) override;


        std::pair<bool, uint64_t> UidToMid(agora::rtc::uid_t uid);

        std::pair<bool, uint64_t> MidToUid(int64_t mid);

        bool IsAudioOnly() const;

    private:
        bool BeginRTMPPublish(const std::string& url);
        bool EndRTMPPublish();
    private:
        agora::rtc::IRtcEngine* agoraRtcEngine_;

        AgoraRtcCallback agoraEventHandler_;

        std::mutex onRemoteMediaHandlerMutex_;
        std::vector<IColiveMedia::OnRemoteMedia::WPtr> onRemoteMediaHandler_;

    private:
        AgoraVideoSource agoraVideoSource_;
        AgoraAudioSource agoraAudioSource_;

        std::mutex vBufMutex_;
        std::mutex aBufMutex_;
        std::deque<std::shared_ptr<VFrame>> vBuf_;
        std::deque<std::unique_ptr<AFrame>> aBuf_;
        int64_t lastAudioTimestampMs_;

    private:
        size_t width_;
        size_t height_;
        size_t fps_;
        size_t colive_vbitrate_;

        size_t channels_;
        size_t samplerate_;
        size_t colive_abitrate_;

    private:
        int64_t firstLocalVideoPacketTSMs_;

        std::unordered_map<uint64_t, int64_t> firstRemoteVideoPacketTSMs_;
        std::unordered_map<uint64_t, bool> firstRemoteAudioReceived_;

        std::mutex remoteAudioBufMutex_;
        std::unordered_map<uint64_t, std::list<std::unique_ptr<AFrame> > > remoteAudioBuf_;

        std::vector<uint64_t> uids_;
        std::string roomId_;

    private:
        std::string rtmpUrl_;
        std::string lastPublishUrl_;
        size_t rtmp_vbitrate_;
        size_t rtmp_abitrate_;
        bool audio_only_;
        int report_type_ = 0;
    };

    std::mutex AgoraService::sInstanceMutex_;
    std::weak_ptr<AgoraService> AgoraService::sInstance_;


    static class AgoraServiceFactory : public IColiveFactory
    {
    public:
        ColiveControllerPtr Create(
            IColiveController::OnControllerEvent::Ptr callback,
            const char* app_id, const char* access_key/* = nullptr */, const char* cookies/* = nullptr*/) override
        {
            return AgoraService::Create(callback, app_id, access_key, cookies);
        }

        ColiveControllerPtr AquireController() override
        {
            return AgoraService::Aquire();
        }

        ColiveMediaPtr AquireMedia() override
        {
            return AgoraService::Aquire();
        }
    } s_agoraServiceFactory;


    std::shared_ptr<AgoraService> AgoraService::Create(
        IColiveController::OnControllerEvent::Ptr callback,
        const char* app_id, const char* access_key, const char* cookies)
    {
        std::lock_guard<std::mutex> lg(sInstanceMutex_);
        std::shared_ptr<AgoraService> result(new AgoraService());
        if (!result->Initialize(callback, app_id, access_key, cookies)) {
            return {};
        }
        sInstance_ = result;
        return result;
    }


    std::shared_ptr<AgoraService> AgoraService::Aquire()
    {
        std::lock_guard<std::mutex> lg(sInstanceMutex_);
        std::shared_ptr<AgoraService> result(sInstance_.lock());
        return result;
    }


    AgoraVideoSource::AgoraVideoSource(AgoraService* ptr)
        : agoraService_(ptr)
    {
        ClearStats();
    }

    AgoraVideoSource::~AgoraVideoSource()
    {
    }

    void AgoraVideoSource::DebugOutputAverageDelayAndClear()
    {
        std::lock_guard<std::mutex> lg(captureFrameStatMutex_);
        if (captureFrameCount_ > 0)
        {
            blog(LOG_INFO, "[agora service] video delay from obs to agora: average %d ms", (int)(captureFramesDelaySum_ / captureFrameCount_));
            captureFramesDelaySum_ = 0;
            captureFrameCount_ = 0;
        }
    }

    void AgoraVideoSource::ClearStats()
    {
        std::lock_guard<std::mutex> lg1(captureFrameStatMutex_);
        std::lock_guard<std::mutex> lg2(renderFrameStatMutex_);

        captureFramesDelaySum_ = 0;
        captureFrameCount_ = 0;
        lastCaptureFPSCheckPoint_ = ColiveTime();
        captureFrameCountSinceCheckPoint_ = 0;
        lastRenderFPSCheckPoint_.clear();
        renderFrameCountSinceCheckPoint_.clear();
    }

    bool AgoraVideoSource::onCaptureVideoFrame(VideoFrame& videoFrame)
    {
        MeasureCost mc("[agora service] capture video frame cost: %d");

        std::shared_ptr<IColiveMedia::VFrame> frame(agoraService_->PullLocalVideo());
        if (!frame)
        {
            //blog(LOG_ERROR, "%s", "[agora service] onCaptureVideoFrame: return false.");
            return false;
        }

        {
            std::lock_guard<std::mutex> lg(captureFrameStatMutex_);

            // delay debug info
            int delayMs = CvtDurationToMs(ColiveClock::now() - frame->generatedTime);
            captureFramesDelaySum_ += delayMs;
            ++captureFrameCount_;
            //blog(LOG_INFO, "[agora service] video frame delay: %d ms", delayMs);

            // fps debug info
            if (lastCaptureFPSCheckPoint_ == ColiveTime())
            {
                lastCaptureFPSCheckPoint_ = ColiveClock::now();
            }
            else
            {
                int elapsedTimeMs = CvtDurationToMs(ColiveClock::now() - lastCaptureFPSCheckPoint_);
                if (elapsedTimeMs > DEBUG_FPS_INTERVAL_MS)
                {
                    blog(LOG_INFO, "[agora service] capture framerate: %g FPS", static_cast<double>(captureFrameCountSinceCheckPoint_ * 1000.0 / elapsedTimeMs));

                    lastCaptureFPSCheckPoint_ = ColiveClock::now();
                    captureFrameCountSinceCheckPoint_ = 0;
                }
            }
        }

        bool retval = ToAgoraVideoFrame(videoFrame, frame.get());
        if (retval)
            ++captureFrameCountSinceCheckPoint_;

        return retval;
    }


    bool AgoraVideoSource::onRenderVideoFrame(unsigned int uid, VideoFrame& videoFrame)
    {
        uint64_t trans_id = uid;
        std::pair<bool, uint64_t> ret = agoraService_->UidToMid(uid);
        if (!ret.first)
        {
            return false;
        }
        trans_id = ret.second;

        std::unique_ptr<IColiveMedia::VFrame> frame;
        if (FromAgoraVideoFrame(frame, videoFrame))
        {
            {
                std::lock_guard<std::mutex> lg(renderFrameStatMutex_);
                if (lastRenderFPSCheckPoint_[trans_id] == ColiveTime())
                {
                    lastRenderFPSCheckPoint_[trans_id] = ColiveClock::now();
                    renderFrameCountSinceCheckPoint_[trans_id] = 0;
                }
                else
                {
                    int elapsedTimeMs = CvtDurationToMs(ColiveClock::now() - lastRenderFPSCheckPoint_[trans_id]);
                    if (elapsedTimeMs > DEBUG_FPS_INTERVAL_MS)
                    {
                        blog(LOG_INFO, "[agora service] render to obs framerate for uid=%d: %g FPS", trans_id, static_cast<double>(renderFrameCountSinceCheckPoint_[trans_id] * 1000.0 / elapsedTimeMs));

                        lastRenderFPSCheckPoint_[trans_id] = ColiveClock::now();
                        renderFrameCountSinceCheckPoint_[trans_id] = 0;
                    }
                }
            }
            ++renderFrameCountSinceCheckPoint_[trans_id];

            frame->presentationTimeMs = videoFrame.renderTimeMs;
            agoraService_->PushRemoteVideo(trans_id, std::move(frame));
        }
        return true;
    }


    AgoraAudioSource::AgoraAudioSource(AgoraService* ptr)
        : agoraService_(ptr)
        , isFirstLocalAudioCallback_(true)
    {
    }


    bool AgoraAudioSource::onRecordAudioFrame(AudioFrame& audioFrame)
    {
        MeasureCost mc("[agora service] capture audio frame cost : %d");

        int avail_samples = audioFrame.samples;

        audioFrame.bytesPerSample = 2;
        audioFrame.type = agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16;

        if (isFirstLocalAudioCallback_)
        {
            isFirstLocalAudioCallback_ = false;
            agoraService_->ClearOutdatedLocalMediaData();
        }

        std::unique_ptr<IColiveMedia::AFrame> buf(agoraService_->PullLocalAudio(avail_samples));
        if (!buf || buf->data.empty())
        {
            //blog(LOG_ERROR, "%s", "[agora service] onRecordAudioFrame: return false.");
            return false;
        }

        return ToAgoraAudioFrame(audioFrame, buf.get());
    }


    bool AgoraAudioSource::onPlaybackAudioFrame(AudioFrame& audioFrame)
    {
        return true;
    }


    bool AgoraAudioSource::onMixedAudioFrame(AudioFrame& audioFrame)
    {
        return true;
    }


    bool AgoraAudioSource::onPlaybackAudioFrameBeforeMixing(unsigned int uid, AudioFrame& audioFrame)
    {
        std::pair<bool, uint64_t> ret = agoraService_->UidToMid(uid);
        if (!ret.first)
        {
            return false;
        }
        std::unique_ptr<IColiveMedia::AFrame> af;
        if (!FromAgoraAudioFrame(af, audioFrame))
            return false;
        agoraService_->PushRemoteAudio(ret.second, std::move(af));

        return true;
    }


    AgoraService::AgoraService()
        : firstLocalVideoPacketTSMs_(-1)
        , width_{}
        , height_{}
        , fps_{}
        , colive_vbitrate_{}
        , rtmp_vbitrate_{}
        , samplerate_{}
        , channels_{}
        , colive_abitrate_{}
        , rtmp_abitrate_{}
        , agoraVideoSource_(this)
        , agoraAudioSource_(this)
        , lastAudioTimestampMs_(-1)
        , audio_only_(false)
    {
        blog(LOG_INFO, "%s", "[agora service] agora service created.");

        agoraRtcEngine_ = createAgoraRtcEngine();
    }


    AgoraService::~AgoraService()
    {
        if (agoraRtcEngine_)
        {
            Stop();

            agora::media::IMediaEngine* mediaeng = 0;
            int ret = agoraRtcEngine_->queryInterface(agora::AGORA_IID_MEDIA_ENGINE, (void**)&mediaeng);
            if (mediaeng)
            {
                mediaeng->registerVideoFrameObserver(0);
                mediaeng->registerAudioFrameObserver(0);
                mediaeng->release();
            }

            // call release in standalone thread
            HANDLE hReleaseThread = (HANDLE) _beginthreadex(0, 0,
                [](void* p)->unsigned int {
                    auto rtcEng = (agora::rtc::IRtcEngine*)p;
                    rtcEng->release(true);
                    return 0;
                }, agoraRtcEngine_,
                0, 0);
            WaitForSingleObject(hReleaseThread, INFINITE);
            CloseHandle(hReleaseThread);
        }

        blog(LOG_INFO, "%s", "[agora service] agora service destroyed.");
    }


    bool AgoraService::Initialize(
        OnControllerEvent::Ptr callback, const char* agora_appid, const char* access_key, const char* cookies)
    {
        if (!agoraRtcEngine_)
            return false;

        int ret = 0;
        agoraEventHandler_.SetCallback(callback, agoraRtcEngine_);

        agora::rtc::RtcEngineContext ctx;
        ctx.appId = agora_appid;
        ctx.eventHandler = &agoraEventHandler_;

        auto start_time = clock();
        ret = agoraRtcEngine_->initialize(ctx);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] Initialize: fail to initialize agora rtc engine.");
            return false;
        }

        agora::rtc::AParameter apm(*agoraRtcEngine_);
        ret |= apm->setParameters("{\"che.video.local.camera_index\":1024}");
        ret |= apm->setParameters("{\"che.video.local.render\":false}");
        ret |= apm->setParameters("{\"che.video.keep_prerotation\":false}");
        ret |= apm->setParameters("{\"che.audio.external_device\":true}");
        ret |= apm->setParameters("{\"che.audio.external_render\":true}");

        //ret |= apm->setParameters("{\"che.audio.enable.aec\":false}");
        //ret |= apm->setParameters("{\"che.audio.enable.ns\":false}");
        //ret |= apm->setParameters("{\"che.audio.agcOn\":false}");

        //ret |= apm->setParameters("{\"rtc.signal_debug\": {\"lbss\":\"lbss\", \"host\":\"125.88.159.176\"}}");

        apm.release();
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] Initialize: fail to set private params.");
            std::string report = "cdn:72;status:init;result:0;reason:1";
            agoraEventHandler_.ReportInfo(report);
            return false;
        }

        ret = agoraRtcEngine_->setChannelProfile(agora::rtc::CHANNEL_PROFILE_LIVE_BROADCASTING);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] Initialize: fail to set channel profile.");
            std::string report = "cdn:72;status:init;result:0;reason:2";
            agoraEventHandler_.ReportInfo(report);
            return false;
        }

        ret = agoraRtcEngine_->setClientRole(agora::rtc::CLIENT_ROLE_BROADCASTER);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] Initialize: fail to set client role.");
            std::string report = "cdn:72;status:init;result:0;reason:3";
            agoraEventHandler_.ReportInfo(report);
            return false;
        }

        auto end_time = clock();
        std::string report = "cdn:72;status:init;result:1;delay:";
        report += std::to_string(end_time - start_time);
        agoraEventHandler_.ReportInfo(report);

        return true;
    }


    bool AgoraService::JoinChannel(const uint64_t uid[], int uid_size, const char* roomId, int type, int report_type, int send_type)
    {
        assert(uid_size == 2);

        auto audio_only = (!(type & SDKColiveType::HasVideo));
        agoraEventHandler_.SetRetry(type & SDKColiveType::HasVideo);

        std::lock_guard<std::mutex> lg1(vBufMutex_);
        std::lock_guard<std::mutex> lg2(aBufMutex_);
        std::lock_guard<std::mutex> lg3(remoteAudioBufMutex_);

        firstRemoteAudioReceived_.clear();
        firstRemoteVideoPacketTSMs_.clear();
        remoteAudioBuf_.clear();
        lastAudioTimestampMs_ = -1;
        firstLocalVideoPacketTSMs_ = -1;
        vBuf_.clear();
        aBuf_.clear();

        uids_.clear();
        for (int i = 0; i < uid_size; i++)
        {
            uids_.push_back(uid[i]);
        }

        roomId_ = roomId;

        report_type_ = report_type;
        agoraEventHandler_.ReportSetType(report_type_);
        auto start_time = clock();
        agoraEventHandler_.ReportSetJoinChannelStartTime(start_time);
        std::string report = "cdn:72;status:join_channel_start;channel_id:";
        report += roomId_;
        if (uids_.size() == 2)
        {
            report += ";self_id:";
            report += std::to_string(uids_[0]);
            report += ";remote_id:";
            report += std::to_string(uids_[1]);
        }
        report += ";type:";
        report += std::to_string(type);
        report += ";report_type:";
        report += std::to_string(report_type);
        agoraEventHandler_.ReportInfo(report);

        //enable av
        int ret = agoraRtcEngine_->enableAudio();

        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] JoinChannel: fail to enable audio");
            std::string report = "cdn:72;status:join_channel;result:0;reason:1;channel_id:";
            report += roomId_;
            if (uids_.size() == 2)
            {
                report += ";self_id:";
                report += std::to_string(uids_[0]);
                report += ";remote_id:";
                report += std::to_string(uids_[1]);
            }
            agoraEventHandler_.ReportInfo(report);
            return false;
        }

        if (!audio_only)
        {
            ret = agoraRtcEngine_->enableVideo();
            if (ret != 0)
            {
                blog(LOG_ERROR, "%s", "[agora service] JoinChannel: fail to enable video");
                std::string report = "cdn:72;status:join_channel;result:0;reason:2;channel_id:";
                report += roomId_;
                if (uids_.size() == 2)
                {
                    report += ";self_id:";
                    report += std::to_string(uids_[0]);
                    report += ";remote_id:";
                    report += std::to_string(uids_[1]);
                }
                agoraEventHandler_.ReportInfo(report);
                return false;
            }
        }
        else
        {
            ret = agoraRtcEngine_->disableVideo();
            if (ret != 0)
            {
                blog(LOG_ERROR, "%s", "[agora service] JoinChannel: fail to disable video");
                std::string report = "cdn:72;status:join_channel;result:0;reason:3;channel_id:";
                report += roomId_;
                if (uids_.size() == 2)
                {
                    report += ";self_id:";
                    report += std::to_string(uids_[0]);
                    report += ";remote_id:";
                    report += std::to_string(uids_[1]);
                }
                return false;
            }
        }

        //======== join room ==========
        ret = agoraRtcEngine_->joinChannelWithUserAccount(
            nullptr, roomId_.c_str(), std::to_string(uids_[0]).c_str());
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] joinChannelWithUserAccount: fail to join channel");
            std::string report = "cdn:72;status:join_channel;result:0;reason:4;channel_id:";
            report += roomId_;
            if (uids_.size() == 2)
            {
                report += ";self_id:";
                report += std::to_string(uids_[0]);
                report += ";remote_id:";
                report += std::to_string(uids_[1]);
            }
            return false;
        }


        audio_only_ = audio_only;

        return true;
    }


    bool AgoraService::LeaveChannel()
    {
        agoraEventHandler_.ReportSetLeaveChannelInfo(roomId_, std::to_string(uids_[0]), std::to_string(uids_[1]));
        return agoraRtcEngine_->leaveChannel() == 0;
    }



    bool AgoraService::SetVideoParam(const bililive::colive::VideoParamInfo& info)
    {
        int ret = 0;

#if 1
        agora::rtc::VideoEncoderConfiguration congif(info.colive_width, info.colive_height, agora::rtc::FRAME_RATE::FRAME_RATE_FPS_30,
            info.colive_start_bitrate, agora::rtc::ORIENTATION_MODE::ORIENTATION_MODE_FIXED_PORTRAIT);
        ret = agoraRtcEngine_->setVideoEncoderConfiguration(congif);
#else
        if (height == 720 && fps == 30)
            ret = agoraRtcEngine_->setVideoProfile(agora::rtc::VIDEO_PROFILE_LANDSCAPE_720P_3, false);
        else if (height == 1080 && fps == 30)
            ret = agoraRtcEngine_->setVideoProfile(agora::rtc::VIDEO_PROFILE_LANDSCAPE_1080P_3, false);
        else if (height == 1080 && fps == 60)
            ret = agoraRtcEngine_->setVideoProfile(agora::rtc::VIDEO_PROFILE_LANDSCAPE_1080P_5, false);
        else
            ret = -1;
#endif
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] SetVideoParam: setVideoEncoderConfiguration failed.");
            return false;
        }

        agora::media::IMediaEngine* mediaeng;

        ret = agoraRtcEngine_->queryInterface(agora::AGORA_IID_MEDIA_ENGINE, (void**)&mediaeng);
        if (ret != 0 || !mediaeng)
        {
            assert(false);
            return false;
        }

        do {
            ret = mediaeng->registerVideoFrameObserver(&agoraVideoSource_);
            if (ret != 0)
            {
                break;
            }
        } while (0);
        mediaeng->release();

        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] SetVideoParam: register observer failed.");
            return false;
        }

        width_ = info.colive_width;
        height_ = info.colive_height;
        fps_ = info.colive_fps;
        colive_vbitrate_ = info.colive_start_bitrate;
        rtmp_vbitrate_ = info.bypass_bitrate;

        blog(LOG_INFO, "[agora service] video param changed: res: %d x %d fps %d bitrate: %d / %d.", width_, height_, fps_, colive_vbitrate_, rtmp_vbitrate_);

        return true;
    }


    bool AgoraService::SetAudioParam(size_t channels, size_t samplerate, size_t colive_bitrate, size_t rtmp_bitrate)
    {
        int ret = 0;

        agora::rtc::RtcEngineParameters rep(*agoraRtcEngine_);
        ret = rep.setRecordingAudioFrameParameters(samplerate, channels, agora::rtc::RAW_AUDIO_FRAME_OP_MODE_READ_WRITE, 2048);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] SetAudioParam: fail to set recording audio frame param.");
            return false;
        }

        ret = rep.setPlaybackAudioFrameParameters(samplerate, channels, agora::rtc::RAW_AUDIO_FRAME_OP_MODE_READ_ONLY, 2048);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] SetAudioParam: fail to set playback audio frame param.");
            return false;
        }

        if (colive_bitrate < 96)
        {
            colive_bitrate = 56;
            agoraRtcEngine_->setAudioProfile(agora::rtc::AUDIO_PROFILE_MUSIC_STANDARD_STEREO, agora::rtc::AUDIO_SCENARIO_SHOWROOM);
        }
        else
        {
            colive_bitrate = 192;
            agoraRtcEngine_->setAudioProfile(agora::rtc::AUDIO_PROFILE_MUSIC_HIGH_QUALITY_STEREO, agora::rtc::AUDIO_SCENARIO_GAME_STREAMING);
        }

        agora::media::IMediaEngine* mediaeng = nullptr;

        ret = agoraRtcEngine_->queryInterface(agora::AGORA_IID_MEDIA_ENGINE, (void**)&mediaeng);
        if (ret != 0 || !mediaeng)
        {
            assert(false);
            return false;
        }

        do {
            ret = mediaeng->registerAudioFrameObserver(&agoraAudioSource_);
            if (ret != 0)
            {
                break;
            }
        } while (0);
        mediaeng->release();

        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] SetAudioParam: register observer failed.");
            return false;
        }

        samplerate_ = samplerate;
        channels_ = channels;
        colive_abitrate_ = colive_bitrate;
        rtmp_abitrate_ = rtmp_bitrate;

        blog(LOG_INFO, "[agora service] audio param changed: ch: %d samplerate: %d bitrate: %d / %d.", channels_, samplerate_, colive_abitrate_, rtmp_abitrate_);

        return true;
    }

    bool AgoraService::SetRTMPPublish(const std::string& url)
    {
        int ret = 0;

        //======== setup rtmp target ==========
        std::vector<agora::rtc::TranscodingUser> region(2);
        uint64_t uid0 = uids_[0];
        uint64_t uid1 = uids_[1];

        std::pair<bool, uint64_t> ret0 = MidToUid(uids_[0]);
        if (ret0.first)
        {
            uid0 = ret0.second;
        }

        std::pair<bool, uint64_t> ret1 = MidToUid(uids_[1]);
        if (ret1.first)
        {
            uid1 = ret1.second;
        }

        region[0].x = 0;
        region[0].y = 0;
        region[0].width = width_;
        region[0].height = height_;
        region[0].uid = uid0;

        region[1].x = width_;
        region[1].y = 0;
        region[1].width = width_;
        region[1].height = height_;
        region[1].uid = uid1;

        agora::rtc::LiveTranscoding conf;
        //conf.backgroundImage = 0;
        conf.transcodingUsers = region.data();
        conf.userCount = region.size();
        conf.audioBitrate = rtmp_abitrate_;
        conf.audioChannels = channels_;
        conf.lowLatency = true;

        auto start_time = clock();
        agoraEventHandler_.ReportSetPublishStartTime(start_time);
        std::string report = "cdn:72;status:set_publish_start;channel_id:";
        report += roomId_;
        if (uids_.size() == 2)
        {
            report += ";self_id:";
            report += std::to_string(uids_[0]);
            report += ";remote_id:";
            report += std::to_string(uids_[1]);
        }
        agoraEventHandler_.ReportInfo(report);

        if (samplerate_ == 48000)
            conf.audioSampleRate = agora::rtc::AUDIO_SAMPLE_RATE_48000;
        else if (samplerate_ == 44100)
            conf.audioSampleRate = agora::rtc::AUDIO_SAMPLE_RATE_44100;
        else if (samplerate_ == 32000)
            conf.audioSampleRate = agora::rtc::AUDIO_SAMPLE_RATE_32000;
        else
        {
            blog(LOG_WARNING, "%s", "[agora service] rtmppublish: unsupported samplerate.");

            std::string report = "cdn:72;status:set_publish;result:0;reason:1;channel_id:";
            report += roomId_;
            if (uids_.size() == 2)
            {
                report += ";self_id:";
                report += std::to_string(uids_[0]);
                report += ";remote_id:";
                report += std::to_string(uids_[1]);
            }
            agoraEventHandler_.ReportInfo(report);

            assert(false);
            return false;
        }

        conf.width = width_ * 2;
        conf.height = height_;
        conf.videoBitrate = rtmp_vbitrate_;
        conf.videoCodecProfile = agora::rtc::VIDEO_CODEC_PROFILE_HIGH;
        conf.videoFramerate = fps_;
        conf.videoGop = fps_; //1s

        EndRTMPPublish();

        ret = agoraRtcEngine_->setLiveTranscoding(conf);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] SetRTMPPublish: fail to set live transcoding configuration.");
            std::string report = "cdn:72;status:set_publish;result:0;reason:2;channel_id:";
            report += roomId_;
            if (uids_.size() == 2)
            {
                report += ";self_id:";
                report += std::to_string(uids_[0]);
                report += ";remote_id:";
                report += std::to_string(uids_[1]);
            }
            agoraEventHandler_.ReportInfo(report);
            return false;
        }

        if (!BeginRTMPPublish(url))
        {
            std::string report = "cdn:72;status:set_publish;result:0;reason:3;channel_id:";
            report += roomId_;
            if (uids_.size() == 2)
            {
                report += ";self_id:";
                report += std::to_string(uids_[0]);
                report += ";remote_id:";
                report += std::to_string(uids_[1]);
            }
            agoraEventHandler_.ReportInfo(report);
            return false;
        }
        return true;
    }

    bool AgoraService::Start()
    {
        agoraVideoSource_.ClearStats();
        return true;
    }

    bool AgoraService::Stop()
    {
        EndRTMPPublish();

        //agoraRtcEngine_->disableVideo();
        //agoraRtcEngine_->disableAudio();
        agoraVideoSource_.DebugOutputAverageDelayAndClear();

        return true;
    }

    bool AgoraService::BeginRTMPPublish(const std::string& url)
    {
        int ret = agoraRtcEngine_->addPublishStreamUrl(url.c_str(), true);
        if (ret != 0)
        {
            blog(LOG_ERROR, "%s", "[agora service] BeginRTMPPublish: fail to add publish stream url.");
            return false;
        }
        lastPublishUrl_ = url;

        blog(LOG_INFO, "[agora service] begin rtmp publish to %s", url.c_str());

        return ret == 0;
    }

    bool AgoraService::EndRTMPPublish()
    {
        int ret = 0;
        if (agoraRtcEngine_ && !lastPublishUrl_.empty())
        {
            blog(LOG_INFO, "%s", "[agora service] end rtmp publish.");
            ret = agoraRtcEngine_->removePublishStreamUrl(lastPublishUrl_.c_str());

            if (ret != 0)
            {
                blog(LOG_ERROR, "%s", "[agora service] EndRTMPPublish: fail to remove publish stream url.");
            }
        }

        if (ret == 0)
        {
            lastPublishUrl_.clear();
            return true;
        }

        return false;
    }


    void AgoraService::PushLocalVideo(std::shared_ptr<VFrame> vFrame)
    {
        vFrame->generatedTime = ColiveClock::now();

        std::lock_guard<std::mutex> lg(vBufMutex_);

        //缓冲区内数据持续时间的长度会限制，已经到长度了就不入队
        if (vBuf_.size() >= 2)
        {
            if (vBuf_.back()->presentationTimeMs - vBuf_.front()->presentationTimeMs > LOCAL_DATA_BUFFER_FRAME_DURATION_MS)
            {
                return;
            }
        }

        vBuf_.emplace_back(std::move(vFrame));

        //start with buffer of half "avsync" duration
        if (firstLocalVideoPacketTSMs_ == -1)
        {
            if (vBuf_.size() >= 2)
            {
                if (vBuf_.back()->presentationTimeMs - vBuf_.front()->presentationTimeMs > LOCAL_DATA_AVSYNC_THRESHOLD_MS / 2)
                    firstLocalVideoPacketTSMs_ = vBuf_.front()->presentationTimeMs;
            }
        }
    }


    void AgoraService::PushLocalAudio(std::unique_ptr<AFrame> aFrame)
    {
        std::lock_guard<std::mutex> lg(aBufMutex_);

        //缓冲区内数据持续时间的长度会限制，已经到长度了就不入队
        if (aBuf_.size() >= 2)
        {
            if (aBuf_.back()->presentationTimeMs - aBuf_.front()->presentationTimeMs > LOCAL_DATA_BUFFER_FRAME_DURATION_MS)
            {
                return;
            }
        }

        aBuf_.emplace_back(std::move(aFrame));
    }


    void AgoraService::SetRemoteMute(bool mute)
    {
        std::vector<agora::rtc::TranscodingUser> region(2);
        uint64_t uid0 = uids_[0];
        uint64_t uid1 = uids_[1];

        std::pair<bool, uint64_t> ret0 = MidToUid(uids_[0]);
        if (ret0.first)
        {
            uid0 = ret0.second;
        }

        std::pair<bool, uint64_t> ret1 = MidToUid(uids_[1]);
        if (ret1.first)
        {
            uid1 = ret1.second;
        }

        region[0].x = 0;
        region[0].y = 0;
        region[0].width = width_;
        region[0].height = height_;
        region[0].uid = uid0;

        region[1].x = width_;
        region[1].y = 0;
        region[1].width = width_;
        region[1].height = height_;
        region[1].uid = uid1;

        agora::rtc::LiveTranscoding conf;
        //conf.backgroundImage = 0;
        conf.transcodingUsers = region.data();
        conf.userCount = region.size();
        conf.audioBitrate = rtmp_abitrate_;
        conf.audioChannels = channels_;
        conf.lowLatency = true;

        if (samplerate_ == 48000)
            conf.audioSampleRate = agora::rtc::AUDIO_SAMPLE_RATE_48000;
        else if (samplerate_ == 44100)
            conf.audioSampleRate = agora::rtc::AUDIO_SAMPLE_RATE_44100;
        else if (samplerate_ == 32000)
            conf.audioSampleRate = agora::rtc::AUDIO_SAMPLE_RATE_32000;
        else
        {
            return;
        }

        conf.width = width_ * 2;
        conf.height = height_;
        conf.videoBitrate = rtmp_vbitrate_;
        conf.videoCodecProfile = agora::rtc::VIDEO_CODEC_PROFILE_HIGH;
        conf.videoFramerate = fps_;
        conf.videoGop = fps_; //1s

        agora::rtc::RtcImage watermark;
        if (!mute)
        {
            if(conf.userCount >1)
            conf.transcodingUsers[1].audioChannel = 0;
            conf.watermark = nullptr;
        }
        else
        {
            if(conf.userCount>1)
            conf.transcodingUsers[1].audioChannel = 6;

            //用宽1280为标准,中间偏右40,40位置 40*40的图片大小
            float scal = conf.width / 1280.0f;
            watermark.width = 40*scal;
            watermark.height = 40*scal;
            watermark.url = bililive::colive::kMuteWatermarkUrl;
            watermark.x = conf.width / 2 + 40*scal;
            watermark.y = 40*scal;
            conf.watermark = &watermark;
        }
        agoraRtcEngine_->setLiveTranscoding(conf);
    }

    std::shared_ptr<IColiveMedia::VFrame> AgoraService::PullLocalVideo()
    {
        std::lock_guard<std::mutex> vlg(vBufMutex_);
        std::lock_guard<std::mutex> alg(aBufMutex_);

        if (firstLocalVideoPacketTSMs_ == -1)
            return nullptr;

        //drop frames if video too slow
        if (lastAudioTimestampMs_ == -1)
            return nullptr;

        //do av sync
        //如果画面落后，丢掉一些画面帧
        if (vBuf_.size() > 1 && lastAudioTimestampMs_ - vBuf_.front()->presentationTimeMs < LOCAL_DATA_AVSYNC_THRESHOLD_MS)
        {
            while (vBuf_.size() > 1 && lastAudioTimestampMs_ - vBuf_.front()->presentationTimeMs > 0)
                vBuf_.pop_front();
        }

        if (!vBuf_.empty())
        {
            std::shared_ptr<VFrame> result;

            //如果画面超前，则画面等声音
            if (vBuf_.front()->presentationTimeMs - lastAudioTimestampMs_ < LOCAL_DATA_AVSYNC_THRESHOLD_MS)
            {
                result = std::move(vBuf_.front());
                vBuf_.pop_front();
            }
            else
            {
                result.reset(new VFrame(*vBuf_.front()));
            }

#if 0
            FILE* fp = fopen("C:\\Users\\user\\Desktop\\11.dat", "ab");
            if (fp)
            {
                int w = result->width;
                int h = result->height;
                for (int i = 0; i < h; ++i)
                    fwrite(result->yPlanar.data() + i * result->yStride, 1, w, fp);
                for (int i = 0; i < h / 2; ++i)
                    fwrite(result->uPlanar.data() + i * result->uStride, 1, w / 2, fp);
                for (int i = 0; i < h / 2; ++i)
                    fwrite(result->vPlanar.data() + i * result->vStride, 1, w / 2, fp);
                fclose(fp);
            }

            fp = fopen("C:\\Users\\user\\Desktop\\11.tc", "a");
            if (fp)
            {
                fprintf(fp, "%d\n", result->presentationTimeMs);
fclose(fp);
            }
#endif

            return result;
        }
        else
        {
            return nullptr;
        }
    }


    std::unique_ptr<IColiveMedia::AFrame> AgoraService::PullLocalAudio(size_t maxSamples)
    {
        std::lock_guard<std::mutex> lg(aBufMutex_);

        if (!audio_only_)
        {
            //process leading samples
            if (firstLocalVideoPacketTSMs_ == -1)
                return nullptr;

            while (!aBuf_.empty() && aBuf_.front()->presentationTimeMs <= firstLocalVideoPacketTSMs_)
                aBuf_.pop_front();
        }

        if (aBuf_.empty())
            return nullptr;

        auto& front = aBuf_.front();
        if (front->channels <= 0)
        {
            blog(LOG_ERROR, "[agora service] incorrent channel count");
            assert(false);
            return nullptr;
        }

        if (front->data.size() / front->channels > maxSamples)
        {
            blog(LOG_INFO, "[agora service] pull local audio: insufficient maxSample parameter.");
            return nullptr;
        }

        std::unique_ptr<AFrame> result(std::move(aBuf_.front()));
        aBuf_.pop_front();

        lastAudioTimestampMs_ = result->presentationTimeMs;

        return result;
    }


    void AgoraService::ClearOutdatedLocalMediaData()
    {
        std::lock_guard<std::mutex> alg(aBufMutex_);
        while (aBuf_.size() >= LOCAL_AUDIO_DATA_PACKETS_CLEAR_THRESHOLD)
            aBuf_.pop_front();

        std::lock_guard<std::mutex> vlg(vBufMutex_);
        while (!vBuf_.empty() && !aBuf_.empty()
            && vBuf_.front()->presentationTimeMs < aBuf_.front()->presentationTimeMs)
        {
            vBuf_.pop_front();
        }
    }

    std::pair<bool, uint64_t> AgoraService::UidToMid(agora::rtc::uid_t uid)
    {
        agora::rtc::UserInfo info;
        int ret = agoraRtcEngine_->getUserInfoByUid(uid, &info);
        if (ret == 0)
        {
            std::string str_uid = info.userAccount;
            if (StrIsNum(str_uid))
            {
                uint64_t mid = _atoi64(str_uid.c_str());
                return std::pair<bool, uint64_t>(true, mid);
            }
        }

        return std::pair<bool, uint64_t>(false, 0);
    }

    std::pair<bool, uint64_t> AgoraService::MidToUid(int64_t mid)
    {
        agora::rtc::UserInfo info;
        int ret = agoraRtcEngine_->getUserInfoByUserAccount(
            std::to_string(mid).c_str(), &info);
        if (ret == 0)
        {
            auto uid = info.uid;
            return std::pair<bool, uint64_t>(true, uid);
        }

        return std::pair<bool, uint64_t>(false, 0);
    }

    void AgoraService::PushRemoteVideo(uint64_t uid, std::unique_ptr<VFrame> vFrame)
    {
        std::lock_guard<std::mutex> lg(onRemoteMediaHandlerMutex_);

        if (firstRemoteVideoPacketTSMs_.find(uid) == firstRemoteVideoPacketTSMs_.end())
        {
            blog(LOG_INFO, "[agora service] first remote video of uid %u received", uid);
            firstRemoteVideoPacketTSMs_[uid] = vFrame->presentationTimeMs;
        }

        vFrame->presentationTimeMs -= firstRemoteVideoPacketTSMs_[uid];

        auto x = onRemoteMediaHandler_.begin();
        while (x != onRemoteMediaHandler_.end())
        {
            OnRemoteMedia::Ptr handler;
            {
                handler = x->lock();
            }
            if (handler)
            {
                handler->OnVideo(uid, vFrame.get());
                ++x;
            }
            else
            {
                x = onRemoteMediaHandler_.erase(x);
            }
        }
    }


    void AgoraService::PushRemoteAudio(uint64_t uid, std::unique_ptr<AFrame> aFrame)
    {
        std::lock_guard<std::mutex> lg(onRemoteMediaHandlerMutex_);
        std::lock_guard<std::mutex> lg2(remoteAudioBufMutex_);

        remoteAudioBuf_[uid].emplace_back(std::move(aFrame));

        if (!audio_only_)
        {
            //第一帧视频来之前，不输出音频（时间戳没法计算）
            if (firstRemoteVideoPacketTSMs_.find(uid) == firstRemoteVideoPacketTSMs_.end())
                return;
        }

        auto& remoteBuf = remoteAudioBuf_[uid];

        while (!remoteBuf.empty())
        {
            do
            {
                auto& front = remoteBuf.front();
                if (firstRemoteAudioReceived_[uid] == false)
                {
                    blog(LOG_INFO, "[agora service] first remote audio of uid %u received", uid);
                    firstRemoteAudioReceived_[uid] = true;
                }

                if (!audio_only_)
                {
                    if (front->presentationTimeMs < firstRemoteVideoPacketTSMs_[uid])
                    {
                        //丢弃这个时间过期的数据
                        break;
                    }

                    front->presentationTimeMs -= firstRemoteVideoPacketTSMs_[uid];
                }

#if 0
                FILE* fp = fopen("C:\\Users\\user\\Desktop\\11.dat", "ab");
                if (fp)
                {
                    fwrite(front->data.data(), 1, front->data.size() * 2, fp);
                    fclose(fp);
                }
#endif

                //开始回调
                auto x = onRemoteMediaHandler_.begin();
                while (x != onRemoteMediaHandler_.end())
                {
                    OnRemoteMedia::Ptr handler;
                    {
                        handler = x->lock();
                    }
                    if (handler)
                    {
                        handler->OnAudio(uid, front.get());
                        ++x;
                    }
                    else
                    {
                        x = onRemoteMediaHandler_.erase(x);
                    }
                }
            } while (false);

            remoteBuf.pop_front();
        }
    }


    void AgoraService::AddOnRemoteMedia(OnRemoteMedia::Ptr callback)
    {
        std::lock_guard<std::mutex> lg(onRemoteMediaHandlerMutex_);
        onRemoteMediaHandler_.emplace_back(callback);
    }

    bool AgoraService::IsAudioOnly() const
    {
        return audio_only_;
    }

};


extern "C"
{
    IColiveFactory* GetColiveServiceFactory()
    {
        return &s_agoraServiceFactory;
    }
}
