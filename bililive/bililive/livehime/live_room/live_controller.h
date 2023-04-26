#ifndef BILILIVE_BILILIVE_LIVEHIME_LIVE_ROOM_CONTROLLER_H_
#define BILILIVE_BILILIVE_LIVEHIME_LIVE_ROOM_CONTROLLER_H_

#include "base/observer_list.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/livehime/live_room/live_room_contract.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/third_party_streaming/third_party_output_controller.h"
#include "bililive/secret/public/live_streaming_service.h"

#include "obs/obs_proxy/public/proxy/obs_output_service.h"



// �������ͣ������ɹ�/ʧ�ܵ������v3.12�������˿������͵ı�ʶ����ʶ��ֵ�������ö��ֵ�Ƕ�Ӧ�ģ�
// ���Ҫ�����ö�����ͽ�����ɾ��Ҫȷ��ReportLiveStreamingOutcome�е�livetype�ֶε�ֵ��Ҫ���ǣ�
// ����ᵼ��������ݷ�����ͳ��ʱ����ȷ
enum class StartLiveType
{
    Normal, // ���濪�����������ӿ�-��������ַ-OBS����
    ThirdPartyProxy, // ����������ת����������ת����������������

    // mask
    ThirdPartyProxy_OBS = (1 << 8),// ������ģʽ�µ�OBS��
};

// �������̷�����
enum class StartLiveFlowResult
{
    Succeed,
    InvalidParamsRetry = (1 << 0),  // ����ID�����ID��Ч�����������Կ���
    InvalidParams = (1 << 1),    // ����ID�����ID��Ч��û��Ҫ���Կ����ˣ�Ҫ��ֱ����û��ͨҪ�ﱾ������̫��ʼ��û�гɹ���ȡ��������Ϣ
    Failed = (1 << 2),  // ����״̬����
    ChooseArea = (1 << 3),  // ��Ҫ��ѡ����
    //SetCover = (1 << 4),  // ��Ҫ�����÷���
};

// һ�ο����Ķ����ǣ���startlive����ֱ���� -> ����/���ƣ����ܶ�Σ� -> stoplive�ر�ֱ���䡱,
// �������øı��漰����·�л���SERVER���ʱ����Ҫ������ǰֱ������ֱ���䣩�ؿ�ֱ���ģ����������ο������·����µĿ�����
// �������øı���漰������������LOCAL���ʱ����Ҫ������ǰֱ����ֻ�Ƕ����������ƶ��ѣ���������һ��ֱ���п��ܷ�����Σ�
// �����������øı䡢������/�ر�ʱ����������/�����л�
class LivehimeLiveRoomObserver
{
public:

    // ���ڷ��𿪲�����
    virtual void OnOpenLiveRoom() {}

    // �����ɹ�������startlive�ӿڳɹ�����������Ҳ�Ƴɹ��ˣ�
    // һ�ο���ֻ��֪ͨһ��
    virtual void OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info) {}

    // ����ʧ�ܣ�����startlive�ӿ�ʧ�ܣ�error_msg��error_code��startlive�ӿڵ������ֶΣ�
    // һ�ο���ֻ��֪ͨһ��
    virtual void OnOpenLiveRoomError(const std::wstring& error_msg, int error_code,bool need_face_auth,const std::string& qr) {}

    // ׼��������
    // һ�ο�������֪ͨ��Σ�����OnLiveRoomStreamingStarted����
    virtual void OnPreLiveRoomStartStreaming(bool is_restart) {}

    // ����������
    // һ�ο�������֪ͨ��Σ���������������������ַ�б�
    virtual void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) {}

    // ׼�������������ģ���
    // һ�ο�������֪ͨ��Σ������������ñ��������������·�л�����
    virtual void OnPreLiveRoomStopStreaming() {}

    // ���Ѿ����ˣ��޴���Ķϣ������Ͽ��ģ���
    // һ�ο�������֪ͨ��Σ������������ñ��������������·�л�����
    virtual void OnLiveRoomStreamingStopped() {}

    // ��׼���õ�ַ�б��е���һ����ַ������
    // һ�ο�������֪ͨ��Σ�����������ʱ����������ַ�б����һ����ַ
    virtual void OnLiveRoomStreamingRetryNextAddr(int index, int total) {}

    // ����ʧ�ܣ������������ʧ�ܣ����߼�����������Ϳ����ӿڵĴ��������������֪ͨ���൱�ڿ���ʧ�ܣ�һ�ο���ֻ�����һ��
    // ���߼��ӿ�ʧ��ΪOnOpenLiveRoomError��������ʧ��ΪOnLiveRoomStreamingError
    virtual void OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code) {}

    // ���ڷ���ز�����
    virtual void OnCloseLiveRoom() {}

    // ֱ�����ѹرգ�
    // һ�ο���ֻ��֪ͨһ��
    virtual void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) {}

    // ����ʵ����֤֪ͨ
    virtual void OnNoticeAuthNeedSupplement(const secret::LiveStreamingService::StartLiveNotice& notice) {}

#pragma region(����������ת�����)

    // ��ҪΪ��֪ͨԤ���ز�ҳ�棬�����ˣ��Զ�����������
	virtual void OnThirdPartyStreamingIncoming() {}

    // �������������ģʽ���������л�ģʽ��������ֱ����Ŀ���״̬�޹أ����ͷ����״̬����ͨ������Ļص���֪ͨ
    virtual void OnEnterIntoThirdPartyStreamingMode() {}

    // �˳�����������ģʽ���������л�ģʽ��������ֱ����Ŀ���״̬�޹أ����ͷ����״̬����ͨ������Ļص���֪ͨ
    virtual void OnLeaveThirdPartyStreamingMode() {}

    // �����������ײ����ڼ���
    virtual void OnThirdPartyStreamingListened(const std::vector<std::string>& ip_list, int port) {}

    // ������ģʽ���������ˣ�һ���ӵȴ�����ʱ
    virtual void OnThirdPartyStreamingBreakCountDown(int sec) {}

    virtual void OnBookLiveInfoChanged(int type, const base::string16& book_info) {}

    virtual void OnRoomLikeCountChanged(int64_t like_count, bool is_show) {}

#pragma endregion

protected:
    virtual ~LivehimeLiveRoomObserver() = default;
};

class LivehimeLiveRoomController :
    public base::NotificationObserver,
    public bililive::OutputControllerDelegate,
    public livehime::ThirdPartyOutputControllerDelegate
{
    enum class PartitionUpdateStatus
    {
        Updating,
        Success,
        Failure,
    };

public:
    enum class LiveStatus
    {
        Ready,
        WaitForSpeedTestResultWhileOpenLiveRoom, // ����ʱ�ȴ����ٽ������
        WaitForSpeedTestResultWhileRestream, // ��������������ʱ�ȴ����ٽ������
        LiveRoomOpening,    // startlive�ӿڵ��õȽ����
        LiveRoomOpenFailed, // �����ʧ�ܣ����������ϻص�Ready״̬
        LiveRoomOpened,     // �����Ѵ�
        StreamStarting,     // ���������������߼�
        Streaming,          // ��������
        StreamStopping,     // �����ײ�����У��ϲ����������Ż������״̬��
        StreamStoped,       // ����Ȼ��ֹ
        LiveRoomClosing,    // stoplive�ӿڵ��õȽ����
        LiveRoomClosed,     // �����ѹرգ����������ϻص�Ready״̬
        //Count,
    };

    static LivehimeLiveRoomController* GetInstance();

    // ȷ�����ӵĵ�ַ��Ϣ������new_link���»�ȡ������ַ
    using GetNewLinkAddrHandler = std::function<
        void(const std::string& server_addr, const std::string& server_key, bool new_link_mode,
            const secret::LiveStreamingService::ProtocolInfo& live_info)>;
    static void GetNewLinkAddr(const secret::LiveStreamingService::ProtocolInfo& live_info, GetNewLinkAddrHandler handler);

    LivehimeLiveRoomController();
    virtual ~LivehimeLiveRoomController();

    virtual void Initialize();
    virtual void Uninitialize();

    virtual void AddObserver(LivehimeLiveRoomObserver *observer);
    virtual void RemoveObserver(LivehimeLiveRoomObserver *observer);

    // ��ָ���������ʹ�ֱ���䣬�򿪳ɹ���
    virtual StartLiveFlowResult OpenLiveRoom(StartLiveType start_type, bool start_by_hotkey,
        bool ignore_cover_check = false);

    //�ر�ֱ����ǰ����
    virtual void CloseLiveRoomPre();

    // �ر�ֱ����
    virtual void CloseLiveRoom();

    // ���¿���
    virtual void RestartLive(LiveRestartMode mode);

    // ��������·����ǰ�����������ĳ����²�������·��
    virtual bool TryToChangeStreamAddr(const std::string& addr, const std::string &code, const std::string &prtc);

    // ������������ַ�б�
    using GetStreamAddrListHandler = std::function<
        void(const secret::LiveStreamingService::ProtocolInfos& addrs)>;
    virtual void GetStreamAddrList(GetStreamAddrListHandler handler);

#pragma region(����״̬��ȡ������ʹ��ʱӦȷ��������ֻ���ж��Ƿ��ڿ��������л���Ҫ��ȷ���Ƿ��ڿ��������е�ĳ���ض�״̬)

    // �����Ƿ��ڴ�����״̬����ײ��Ƿ��������޹أ�����ҵ��㵱ǰ�Ƿ��ڿ������̽����У�
    // ��ǰ���ܴ����첽�������/�ز��ӿڽ��������׼���ȣ��ȴ���Ҳ����������������
    virtual bool IsLiveReady() const { return live_status_ == LiveStatus::Ready; }

    // �����ײ㵱ǰ�Ƿ����ڳ�������
    virtual bool IsStreaming() const { return live_status_ == LiveStatus::Streaming; }

    virtual LiveStatus live_status() const { return live_status_; }

    virtual secret::LiveStreamingService::StartLiveInfo start_live_info() const { return start_live_info_; }

    virtual base::Time start_live_time() const { return start_live_time_; }

#pragma endregion

#pragma region(����������ת�����)

    // ��ǰ�Ƿ��ڵ���������ģʽ��
    static bool IsInTheThirdPartyStreamingMode();

    // �������������ģʽ
    virtual bool EnterIntoThirdPartyStreamingMode();

    // �˳�����������ģʽ
    virtual void LeaveThirdPartyStreamingMode();

    // ��������ģʽ���������UI�ȹرա�״̬����
    virtual void InvalidateThirdPartyStreamingTask();

    virtual void GetBookLiveTimeInfos();
    virtual void HideBookLiveTime();

private:
    // �����������ײ�������״̬
    bool ThirdPartyStreamingStartListen();
    void OnThirdPartyStreamingStartListenFailed(const base::string16& message = {});
    // ʵ���˳�������ģʽ
    void ActualLeaveThirdPartyStreamingMode();
    void ThirdPartyStreamingCloseLiveRoomTimeout();

    // һ������������ʱ�ص�
    void OnThirdPartyStreamingBreakCountDownTimer();

    void UpdateStreamVideoBitrateAtLiveArea(int parent_id);
#pragma endregion

protected:
    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    // OutputControllerDelegate
    void OnPreStartStreaming(bool is_restart) override;
    void OnStreamingStarted(const bililive::StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details) override;
    void OnRetryStreaming(int index, int total) override;
    void OnPreStopStreaming(bool is_restart) override;
    void OnStreamingStopped(OutputControllerDelegate::StreamingErrorInfos& info) override;
    // ��Ҫ�µ�������ַ��Ϣ����ʵ����Ҫ���·��𿪲�����
    void NeedNewStreamingAddrInfos() override;
    // ��Ҫ����
    bool NeedSpeedTest() override;

    // ThirdPartyOutputControllerDelegate
    void OnTpocLocalListenError() override;
    void OnTpocLocalStreamingStarted() override;
    void OnTpocLocalStreamingStopped() override;
    void OnTpocPreStartStreaming() override;
    void OnTpocStreamingStarted(const obs_proxy::StreamingErrorDetails& streaming_error_details) override;
    void OnTpocPreStopStreaming() override;
    void OnTpocStreamingStoped(livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos& info) override;
    // static callback
    static void LocalStreamingStoppedEndDialog(int code, void* data);

    bililive::OutputController* output_controller() { return output_controller_; }
    livehime::ThirdPartyOutputController* third_party_output_controller() { return third_party_output_controller_; }

private:

    void OnPostNoviceGuideFinish();
    void SetLiveStatus(LiveStatus status);

    // ���һ�µ�ǰ�����Ƿ���������״̬��������״̬�Ķ��������³����߼���ȷ����
    // 1�����ڿ�������״̬��
    // 2�������ڵ���������״̬��
    // 3�������ڡ����߻�����״̬�����߻�����������Ƶ���ߡ���Ƶ���Ҷ�����ƵPK����
    //    ����������ڡ�������UI��û�п���״̬������ع���ģ�鲢δ������
    bool CheckIfInSelfStreamingStatus();

    void OpenLiveRoomInternal();
    void CloseLiveRoomInternal(bool stop_streaming, void* param);
    // ��ʵ���ÿ�/�ز��ӿ�
    void StartLive();
    void StopLive(void* param);

    // �������
    //void OnGetPreliveInfo(bool valid, int code, const std::string& err_msg,
    //    const secret::LiveStreamingService::PreliveInfo& prelive_info);
    void OnChoosePartition(PartitionUpdateStatus status, void* param);
    void OnStartLive(bool valid, int code, const std::string& err_msg,
        const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    void OnStartLiveError(obs_proxy::StreamingErrorCode error_code,
        const std::wstring& extra_error_msg,
        int error_code_from_server,
        const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    // �������ӿڵ�Notice���ݶΣ���ֹĿǰ(v3.12)ֻ��ʵ����֤������֤ʹ�������չnotice���ݶ�
    void HandleStartLiveNotice(const secret::LiveStreamingService::StartLiveNotice& notice);

    // �ز����
    void OnStopLive(bool success, int result_code, const std::string& err_msg,
        void* param);
    void TerminateStartLiveFlow();

    static void GetLiveErrorMsg(obs_proxy::StreamingErrorCode streaming_error_code, base::string16& error_msg, int& code);
    static const int kNoneServerErrorCode = 0;

#pragma region(�ص�ҵ��ۺϴ���)

    // ֱ�����ѿ���
    void HandleLiveRoomOpened();

    // ���ڷ�������
    void HandlePreStartStreaming(bool is_restart);

    // ���Ѿ�������
    void HandleStreamingStarted(const bililive::StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details);

    // ����ʧ��
    void HandleLiveStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code);

    // ������ֹ�������޴��󳡾��µ�����ֹ�������֪ͨ��������HandleLiveStreamingError����
    void HandleLiveStreamingStoped();

    // ���ڷ������
    void HandlePreStopStreaming(bool is_restart);

    // ֱ�����ѹر�
    void HandleLiveRoomClosed(void* param);

#pragma endregion

private:
    base::NotificationRegistrar notifation_register_;

    std::atomic<LiveStatus> live_status_ = LiveStatus::Ready;
    bool stop_live_now_ = false;

    StartLiveType start_type_ = StartLiveType::Normal;
    base::Time last_set_channel_time_; // ���һ�����÷����ɹ���ʱ�����δ����ʱ����5���Ӻ󿪲�����ѡ����
    base::Time start_live_time_;    // ��������ʼʱ�䣬�������ӿڳɹ���ʱ��

    bililive::OutputController* output_controller_ = nullptr;

    // ��ʶ���������Ƿ��ɿ�ݼ�����
    bool start_by_hotkey_ = false;
    // ��ʶ����ѡ��UI�Ƿ�Ϊ�������̵���
    bool choose_area_showed_by_start_live_flow_ = false;
    // ��ʶ�Ƿ����ڸ��·�����Ϣ
    bool updating_room_info_ = false;

    secret::LiveStreamingService::StartLiveInfo start_live_info_;

    //�Ƿ��Ѿ��ز����ع�,һ��ֱ��ֻ����һ��
    bool is_stop_live_intercept_ = false;

    ObserverList<LivehimeLiveRoomObserver> observer_list_;

    livehime::ThirdPartyOutputController* third_party_output_controller_ = nullptr;
    bool leave_third_party_mode_ = false;
    base::RepeatingTimer<LivehimeLiveRoomController> tps_break_count_down_timer_;
    int tps_break_count_down_ = 0;

    //RoomCoverInfo cover_info_;
    //bool cover_is_valid_ = true;
    base::WeakPtrFactory<LivehimeLiveRoomController> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeLiveRoomController);
};

#endif