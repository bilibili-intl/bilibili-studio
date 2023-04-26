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
            obs_proxy::StreamingErrorCode streaming_errno = obs_proxy::StreamingErrorCode::Success;  // 断流错误码
            obs_proxy::StreamingErrorDetails streaming_error_details;// 断流时使用的推流信息

            bool running_by_obs = false;
        };

        // 监听失败
        virtual void OnTpocLocalListenError() = 0;

        // 第三方的流到了，可以开播了
        virtual void OnTpocLocalStreamingStarted() = 0;

        // 第三方流断了，上层要决定是否关播（如果已经在播了的话）
        virtual void OnTpocLocalStreamingStopped() = 0;

        // 正在发起推流流程
        virtual void OnTpocPreStartStreaming() = 0;

        // 推流成功，正在推流
        virtual void OnTpocStreamingStarted(const obs_proxy::StreamingErrorDetails& streaming_error_details) = 0;

        // 正在发起断流流程
        virtual void OnTpocPreStopStreaming() = 0;

        // 流已然断绝了，多种情况都用的这个回调，对于上层而言就是推流层不再工作了：
        // 1、CDN连接失败了；
        // 2、CDN推流失败了；
        // 3、主动断流；
        // 4、开启推流失败（线程就没开始跑）；
        virtual void OnTpocStreamingStoped(ThirdPartyOutputControllerDelegate::StreamingErrorInfos& info) = 0;

        // 底层完全终止工作了
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

        // 推流层是否已然在工作了
        bool IsRunning() const;
        bool running_by_obs() const { return running_by_obs_; }

        //第三方推流插入SEI（答题专用）
        void SetSeiInfo(const uint8_t* data, int len);

		//获取第三方推流的编码器信息 utf8
        std::string GetEncoderInfo();

        //第三方推流通用SEI接口
        void AddSEIInfo(const uint8_t* data, int size);

    protected:
        // LivehimeOBSPluginIPCODelegate
        void OnMessageReceived(const IPC::Message& message) override;
        void OnChannelConnected() override;
        void OnChannelError() override;

        // ProcessorCallback，invoke on worker thread
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

        // 生成端口，进入待接流监听状态
        bool StartListen(std::vector<std::string>* ip_list, int* port);

        // 给定地址列表进行推流
        bool StartStreaming(const secret::LiveStreamingService::StartLiveInfo& start_live_info);
        void Shutdown();

        // 流已然终止，且不再自动重试推流了，上层应该关播了
        void StreamingAnnihilated(obs_proxy::StreamingErrorCode streaming_errno,
            const obs_proxy::StreamingErrorDetails& streaming_error_details);

        // 根据当前重试偏移及其newlink决定实际的推流地址
        void DetermineStreamingAddr(const secret::LiveStreamingService::ProtocolInfo& info);
        void OnDetermineStreamingAddr(
            const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
            const secret::LiveStreamingService::ProtocolInfo& live_info);

        // 根据当前的推流地址信息生成供上层使用的推流信息
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

        // 开播接口返回的全部开播信息
        secret::LiveStreamingService::StartLiveInfo start_live_info_;
        // 发起实际推流流程时使用的地址信息
        StreamingAddresses stream_address_;
        //直播答题是否需要停止拉sei信息
        bool get_sei_need_stop_ = false;
        bool get_sei_is_running_ = false;

        std::shared_ptr<streaming_processor::IStreamingProcessor> processor_;

        base::WeakPtrFactory<ThirdPartyOutputController> weakptr_factory_;

        DISALLOW_COPY_AND_ASSIGN(ThirdPartyOutputController);
    };
}

#endif