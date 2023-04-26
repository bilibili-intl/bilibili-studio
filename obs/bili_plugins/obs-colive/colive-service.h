#ifndef COLIVE_SERVICE_H_
#define COLIVE_SERVICE_H_

#include <memory>
#include <vector>
#include <chrono>
#include <string>

#ifdef INSIDE_COLIVE_MODULE
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif


namespace bililive
{
    //���ն�Ӧuid��Щ���ͣ�
    enum RtcMediaType {
        //��Ƶ
        MEDIA_TYPE_AUDIO = 0,
        //��Ƶ����
        MEDIA_TYPE_VIDEO = 1,
        //rtp data
        MEDIA_TYPE_DATA = 2,
        //��ƵС��
        MEDIA_TYPE_SMALL_VIDEO = 3,
        //sctp data
        MEDIA_TYPE_SCTP_DATA = 4,
        //����
        OTHER = 15
    };

    namespace colive
    {

        enum SDKColiveType
        {
            Default = 0,
            HasAudio = 1 << 0,
            HasVideo = 1 << 1,
            HasData = 1 << 2,
            HasLittleVideo = 1 << 4,
        };

        const char kMuteWatermarkUrl[] = "https://i0.hdslb.com/bfs/live/8d90f10265f8e4b8333e7914830f18de262dbbbe.png";
        struct VideoParamInfo {
            size_t colive_min_bitrate = 0;
            size_t colive_start_bitrate = 0;
            size_t colive_max_bitrate = 0;
            size_t colive_width = 0;
            size_t colive_height = 0;
            size_t colive_fps = 0;

            size_t bypass_bitrate = 0;
            size_t bypass_fps = 0;
            size_t bypass_width = 0;
            size_t bypass_height = 0;
        };

        typedef std::chrono::steady_clock ColiveClock;
        typedef std::chrono::time_point<ColiveClock> ColiveTime;
        typedef std::chrono::duration<ColiveClock> ColiveDuration;

        template<class Duration>
        inline int CvtDurationToMs(const Duration& dur)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
        }

        class IColiveController
        {
        public:
            class OnControllerEvent : public std::enable_shared_from_this<OnControllerEvent>
            {
            public:
                using Ptr = std::shared_ptr<OnControllerEvent>;
                using WPtr = std::weak_ptr<OnControllerEvent>;

                virtual ~OnControllerEvent() {}
                virtual void OnJoinChannelSuccess() {}
                virtual void onUserJoined(uint64_t uid) {}
                virtual void onUserOffline(uint64_t uid, bool quit) {}
                virtual void OnLeaveChannel() {}
                virtual void OnFirstRemoteVideoDecoded() {}
                virtual void onConnectionInterrupted() {}
                virtual void onConnectionLost() {}
                virtual void OnConnectionBanned() {}
                virtual void onStreamPublished(const char* url, int error) {}
                virtual void onStreamUnpublished(const char* url) {}
                virtual void onStreamMessageError(uint64_t uid, int code) {}
                virtual void OnError(int err) {}
                virtual void OnReportInfo(const std::string& info) {}
                virtual void OnChannelDataReceived(uint64_t uid, const char* data, size_t data_len, int seq_num, uint32_t timestamp){}
                virtual void OnSCTPChannelDataReceived(uint64_t uid, const char* data, size_t data_len, int sid, int seq_num, uint32_t timestamp){}
                virtual void OnAudioLevel(uint64_t uid, int level) {}
                virtual void OnRealtimeStats(const char* json, int json_len){}
            };

            virtual ~IColiveController() {}

            /**
             * ��ʼ������ SDK��
             * @param callback    �¼��ص�
             * @param agora_appid �������õ� App ID
             * @param access_key  �������õ� Key
             */
            virtual bool Initialize(
                OnControllerEvent::Ptr callback, const char* agora_appid,
                const char* access_key, const char* cookies) = 0;

            // uid: head = self, tail = remote users
            virtual bool JoinChannel(const uint64_t uid[],int uid_size, const char* roomId, int type,int report_type,int send_type) = 0;
            virtual bool LeaveChannel() = 0;
            //���ö��ģ����ý��գ��ĸ��û�������Щ���ͣ�recev����RtcMediaType�ṹ
            virtual void SetSubscribe(uint64_t uid, const int* recev, size_t recev_count) = 0;
            //���ö�Ӧuid���ŵ�����,��Χ0-1
            virtual bool SetRemotePlayVolume(uint64_t uid, double volume) = 0;

            //��ȡ��ǰƵ���ﵱǰ�����˵�����,�����Լ�,�����ͨ���ص�OnAudioLevel�ص�����,��Χ0-32767
            virtual void GetAudioLevel() = 0;

            //type���սṹ��bilibili::SetSendType,����ĳ������ͨ���Ƿ���
            //��������Ƶ��������Ƶͬʱ����,���÷���:set_send(SendType::SendVideo | SendType::SendAudio,true);
            //���絥�����ò�������Ƶ�ĵ��÷���:set_send(SendType::SendAudio,false);
            virtual void SetSend(int type, bool send) = 0;

            //��ȡʵʱ����������json
            virtual bool GetRealtimeStats() = 0;
        };

        using ColiveControllerPtr = std::shared_ptr<IColiveController>;
        using ColiveControllerWPtr = std::weak_ptr<IColiveController>;


        class IColiveMedia
        {
        public:
            struct VFrame
            {
                int64_t presentationTimeMs;

                int width;
                int height;

                std::vector<uint8_t> yPlanar;
                int yStride;
                std::vector<uint8_t> uPlanar;
                int uStride;
                std::vector<uint8_t> vPlanar;
                int vStride;

                ColiveTime generatedTime;
            };

            struct AFrame
            {
                int64_t presentationTimeMs;

                int samplerate;
                int channels;
                std::vector<short> data;
            };

            class OnRemoteMedia : public std::enable_shared_from_this<OnRemoteMedia>
            {
            public:
                using Ptr = std::shared_ptr<OnRemoteMedia>;
                using WPtr = std::weak_ptr<OnRemoteMedia>;

                virtual ~OnRemoteMedia() {}
                virtual void OnVideo(uint64_t uid, VFrame* vf, bool is_little_video = false) {}
                virtual void OnAudio(uint64_t uid, AFrame* af) {}
            };

            virtual ~IColiveMedia() {}

            virtual bool Start() = 0;
            virtual bool Stop() = 0;

            virtual bool SetRTMPPublish(const std::string& url) = 0;
            //virtual bool SetVideoParam(size_t width, size_t height, size_t fps, size_t colive_bitrate, size_t rtmp_bitrate) = 0;
            virtual bool SetVideoParam(const VideoParamInfo& info) = 0;
            virtual bool SetAudioParam(size_t channels, size_t samplerate, size_t colive_bitrate, size_t rtmp_bitrate) = 0;

            virtual void PushRemoteVideo(uint64_t uid, std::unique_ptr<VFrame> vFrame) = 0;
            virtual void PushRemoteAudio(uint64_t uid, std::unique_ptr<AFrame> aFrame) = 0;
            virtual void AddOnRemoteMedia(OnRemoteMedia::Ptr callback) = 0;

            virtual void PushLocalVideo(std::shared_ptr<VFrame> vFrame) = 0;
            virtual void PushLocalAudio(std::unique_ptr<AFrame> aFrame) = 0;

            virtual void SetRemoteMute(bool mute) = 0;
            virtual bool PushChannelData(const char* data, size_t len) = 0;
            //ordered:���� reliable:�ɿ� ������ζ��һ���ɿ����ɿ���һ�����򣬿ɿ�������£����ᶪ�������粻�ȶ�������¿��ܻ�����
            //����ͬһ��sid����ͬ����ordered��reliable
            virtual int PushSCTPData(uint32_t sid,const char* data, size_t len,bool ordered, bool reliable) = 0;
            virtual bool AddSCTPSid(uint32_t sid) = 0;
            virtual bool RemoveSCTPSid(uint32_t sid) = 0;
        };

        using ColiveMediaPtr = std::shared_ptr<IColiveMedia>;
        using ColiveMediaWPtr = std::weak_ptr<IColiveMedia>;


        class IColiveFactory
        {
        public:
            virtual ~IColiveFactory() = default;

            virtual ColiveControllerPtr Create(
                IColiveController::OnControllerEvent::Ptr callback, const char* agora_app_id,
                const char* access_key = nullptr, const char* cookies = nullptr) = 0;

            virtual ColiveControllerPtr AquireController() = 0;
            virtual ColiveMediaPtr AquireMedia() = 0;
        };
    };
};


extern "C"
{
    DLLIMPORT bililive::colive::IColiveFactory* GetColiveServiceFactory();
};

#endif
