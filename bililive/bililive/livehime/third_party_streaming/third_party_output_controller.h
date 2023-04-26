#ifndef BILILIVE_BILILIVE_LIVEHIME_THIRD_PARTY_STREAMING_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_THIRD_PARTY_STREAMING_PRESENTER_H_

#include <atomic>

#include "base/observer_list.h"
#include "base/memory/singleton.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_piece.h"

#include "bililive/bililive/livehime/third_party_streaming/third_party_streaming_datatypes.h"
#include "bililive/bililive/livehime/third_party_streaming/obs_livehime_plugin/livehime_obs_plugin_ipc.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/secret/public/live_streaming_service.h"

#include "obs/bili_plugins/streaming-processor/streaming-processor-interface.h"
#include "obs/obs_proxy/public/proxy/obs_output_service.h"

class LivehimeLiveRoomController;
class ViddupLiveRoomController;

namespace livehime
{
    class StreamSpeedTester;

    class ThirdPartyOutputControllerDelegate
    {
    public:
        struct StreamingErrorInfos
        {
            obs_proxy::StreamingErrorCode streaming_errno = obs_proxy::StreamingErrorCode::Success;  // ����������
            obs_proxy::StreamingErrorDetails streaming_error_details;// ����ʱʹ�õ�������Ϣ

            bool running_by_obs = false;
        };

        // ����ʧ��
        virtual void OnTpocLocalListenError() = 0;

        // �������������ˣ����Կ�����
        virtual void OnTpocLocalStreamingStarted() = 0;

        // �����������ˣ��ϲ�Ҫ�����Ƿ�ز�������Ѿ��ڲ��˵Ļ���
        virtual void OnTpocLocalStreamingStopped() = 0;

        // ���ڷ�����������
        virtual void OnTpocPreStartStreaming() = 0;

        // �����ɹ�����������
        virtual void OnTpocStreamingStarted(const obs_proxy::StreamingErrorDetails& streaming_error_details) = 0;

        // ���ڷ����������
        virtual void OnTpocPreStopStreaming() = 0;

        // ����Ȼ�Ͼ��ˣ�����������õ�����ص��������ϲ���Ծ��������㲻�ٹ����ˣ�
        // 1��CDN����ʧ���ˣ�
        // 2��CDN����ʧ���ˣ�
        // 3������������
        // 4����������ʧ�ܣ��߳̾�û��ʼ�ܣ���
        virtual void OnTpocStreamingStoped(ThirdPartyOutputControllerDelegate::StreamingErrorInfos& info) = 0;

        // �ײ���ȫ��ֹ������
        //virtual void OnTpocShutdown() = 0;

    protected:
        virtual ~ThirdPartyOutputControllerDelegate() = default;
    };

    class ThirdPartyOutputController :
        LivehimeOBSPluginIPCODelegate,
        public streaming_processor::ProcessorCallback
    {
        /*enum class StreamingStatus
        {
            Ready,
            OpeningLiveRoom,
            IgnitingStream,
            IgnitingStreamAfterColive,
            Streaming,
            ExtinguishingStream,
            ClosingLiveRoom
        };*/

        struct StreamingAddresses
        {
            std::string cdn_address;
            std::string cdn_key;
            std::string server_address;
            std::string server_key;
            std::string new_link;
            std::string protocol;
            std::string live_key;

            bool new_link_mode = false;
        };
        //using StreamingAddresses = secret::LiveStreamingService::ProtocolInfo;

    public:
        static ThirdPartyOutputController* GetInstance();

        //StreamingStatus status() const { return status_; }

        // �������Ƿ���Ȼ�ڹ�����
        bool IsRunning() const;
        bool running_by_obs() const { return running_by_obs_; }

        //��������������SEI������ר�ã�
        void SetSeiInfo(const uint8_t* data, int len);

		//��ȡ�����������ı�������Ϣ utf8
        std::string GetEncoderInfo();

        //����������ͨ��SEI�ӿ�
        void AddSEIInfo(const uint8_t* data, int size);

    protected:
        // LivehimeOBSPluginIPCODelegate
        void OnMessageReceived(const IPC::Message& message) override;
        void OnChannelConnected() override;
        void OnChannelError() override;

        // ProcessorCallback��invoke on worker thread
        void ProcessorStatus(streaming_processor::ProcessStatus status) override;

        static void InformTpListening();
        static void InformTpListenError();
        static void InformTpStreamIncoming();
        static void InformTpStreamingStopped();
        static void InformStartStreamingFailed();
        static void InformCDNConnecting();
        static void InformCDNConnectError();
        static void InformCDNStreaming();
        static void InformPreShutdown();
        static void InformCDNStreamingStopped();
        static void InformShutdown();
    private:
        ThirdPartyOutputController();
        ~ThirdPartyOutputController();

        void Initialize();
        void Uninitialize();
        //void SetStatus(StreamingStatus status);

        void set_delegate(ThirdPartyOutputControllerDelegate* delegate) { delegate_ = delegate; }

        // ���ɶ˿ڣ��������������״̬
        bool StartListen(std::vector<std::string>* ip_list, int* port);

        // ������ַ�б��������
        bool StartStreaming(const secret::LiveStreamingService::StartLiveInfo& start_live_info);
        void Shutdown();

        // ����Ȼ��ֹ���Ҳ����Զ����������ˣ��ϲ�Ӧ�ùز���
        void StreamingAnnihilated(obs_proxy::StreamingErrorCode streaming_errno,
            const obs_proxy::StreamingErrorDetails& streaming_error_details);

        // ���ݵ�ǰ����ƫ�Ƽ���newlink����ʵ�ʵ�������ַ
        void DetermineStreamingAddr(const secret::LiveStreamingService::ProtocolInfo& info);
        void OnDetermineStreamingAddr(
            const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
            const secret::LiveStreamingService::ProtocolInfo& live_info);

        // ���ݵ�ǰ��������ַ��Ϣ���ɹ��ϲ�ʹ�õ�������Ϣ
        obs_proxy::StreamingErrorDetails GenerateStreamingDetails();

        const ThirdPartyStreamingAddrInfos* local_addr_infos() const { return local_addr_infos_.get(); }

    private:
        friend class LivehimeLiveRoomController;
        friend class ViddupLiveRoomController;
        friend struct DefaultSingletonTraits<ThirdPartyOutputController>;

        ThirdPartyOutputControllerDelegate* delegate_ = nullptr;
        //StreamingStatus status_ = StreamingStatus::Ready;

        scoped_refptr<LivehimeOBSPluginIPCService> ipc_;
        std::unique_ptr<ThirdPartyStreamingAddrInfos> local_addr_infos_;
        bool running_by_obs_ = false;

        // �����ӿڷ��ص�ȫ��������Ϣ
        secret::LiveStreamingService::StartLiveInfo start_live_info_;
        // ����ʵ����������ʱʹ�õĵ�ַ��Ϣ
        StreamingAddresses stream_address_;
        //ֱ�������Ƿ���Ҫֹͣ��sei��Ϣ
        bool get_sei_need_stop_ = false;
        bool get_sei_is_running_ = false;

        std::shared_ptr<streaming_processor::IStreamingProcessor> processor_;

        base::WeakPtrFactory<ThirdPartyOutputController> weakptr_factory_;

        DISALLOW_COPY_AND_ASSIGN(ThirdPartyOutputController);
    };
}

#endif