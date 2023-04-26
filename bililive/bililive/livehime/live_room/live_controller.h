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



// 开播类型，开播成功/失败的埋点在v3.12中新增了开播类型的标识，标识的值和这里的枚举值是对应的，
// 如果要对这个枚举类型进行增删，要确认ReportLiveStreamingOutcome中的livetype字段的值不要覆盖，
// 否则会导致埋点数据分类型统计时不正确
enum class StartLiveType
{
    Normal, // 常规开播，调开播接口-拿推流地址-OBS推流
    ThirdPartyProxy, // 第三方推流转发，纯代理转发，不对流做处理

    // mask
    ThirdPartyProxy_OBS = (1 << 8),// 第三方模式下的OBS向
};

// 开播流程发起结果
enum class StartLiveFlowResult
{
    Succeed,
    InvalidParamsRetry = (1 << 0),  // 房间ID或分区ID无效，还可以重试开播
    InvalidParams = (1 << 1),    // 房间ID或分区ID无效，没必要重试开播了，要嘛直播间没开通要嘛本地网络太差始终没有成功获取到房间信息
    Failed = (1 << 2),  // 流程状态不对
    ChooseArea = (1 << 3),  // 需要先选分区
    //SetCover = (1 << 4),  // 需要先设置封面
};

// 一次开播的定义是：“startlive开启直播间 -> 推流/重推（可能多次） -> stoplive关闭直播间”,
// 推流设置改变涉及到线路切换的SERVER变更时是需要结束当前直播（关直播间）重开直播的，即结束本次开播重新发起新的开播；
// 推流设置改变仅涉及到推流参数的LOCAL变更时不需要结束当前直播，只是断流重连重推而已，这个情况在一次直播中可能发生多次，
// 比如推流设置改变、连麦开启/关闭时的引发的推/断流切换
class LivehimeLiveRoomObserver
{
public:

    // 正在发起开播流程
    virtual void OnOpenLiveRoom() {}

    // 开播成功，仅是startlive接口成功，不包含流也推成功了，
    // 一次开播只会通知一次
    virtual void OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info) {}

    // 开播失败，仅是startlive接口失败，error_msg和error_code是startlive接口的数据字段，
    // 一次开播只会通知一次
    virtual void OnOpenLiveRoomError(const std::wstring& error_msg, int error_code,bool need_face_auth,const std::string& qr) {}

    // 准备推流，
    // 一次开播可能通知多次，先于OnLiveRoomStreamingStarted触发
    virtual void OnPreLiveRoomStartStreaming(bool is_restart) {}

    // 正在推流，
    // 一次开播可能通知多次，比如错误断流尝试重连地址列表
    virtual void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) {}

    // 准备断流（主动的），
    // 一次开播可能通知多次，比如推流设置变更、连麦推流线路切换重推
    virtual void OnPreLiveRoomStopStreaming() {}

    // 流已经断了（无错误的断，主动断开的），
    // 一次开播可能通知多次，比如推流设置变更、连麦推流线路切换重推
    virtual void OnLiveRoomStreamingStopped() {}

    // 正准备用地址列表中的下一个地址来重连
    // 一次开播可能通知多次，比如错误断流时尝试重连地址列表的下一个地址
    virtual void OnLiveRoomStreamingRetryNextAddr(int index, int total) {}

    // 开播失败，推流层的推流失败，旧逻辑是推流错误和开播接口的错误都用这个命令来通知，相当于开播失败，一次开播只会产生一次
    // 新逻辑接口失败为OnOpenLiveRoomError，推流层失败为OnLiveRoomStreamingError
    virtual void OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code) {}

    // 正在发起关播流程
    virtual void OnCloseLiveRoom() {}

    // 直播间已关闭，
    // 一次开播只会通知一次
    virtual void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) {}

    // 补充实名认证通知
    virtual void OnNoticeAuthNeedSupplement(const secret::LiveStreamingService::StartLiveNotice& notice) {}

#pragma region(第三方推流转发相关)

    // 主要为了通知预设素材页面，流来了，自动进入主界面
	virtual void OnThirdPartyStreamingIncoming() {}

    // 进入第三方推流模式，仅仅是切换模式，与流和直播间的开关状态无关，流和房间的状态还是通过上面的回调来通知
    virtual void OnEnterIntoThirdPartyStreamingMode() {}

    // 退出第三方推流模式，仅仅是切换模式，与流和直播间的开关状态无关，流和房间的状态还是通过上面的回调来通知
    virtual void OnLeaveThirdPartyStreamingMode() {}

    // 第三方推流底层正在监听
    virtual void OnThirdPartyStreamingListened(const std::vector<std::string>& ip_list, int port) {}

    // 第三方模式本地流断了，一分钟等待倒计时
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
        WaitForSpeedTestResultWhileOpenLiveRoom, // 开播时等待测速结果返回
        WaitForSpeedTestResultWhileRestream, // 开播中重试推流时等待测速结果返回
        LiveRoomOpening,    // startlive接口调用等结果中
        LiveRoomOpenFailed, // 房间打开失败，接下来马上回到Ready状态
        LiveRoomOpened,     // 房间已打开
        StreamStarting,     // 开启推流层推流逻辑
        Streaming,          // 正在推流
        StreamStopping,     // 推流底层断流中（上层主动断流才会产生的状态）
        StreamStoped,       // 流已然终止
        LiveRoomClosing,    // stoplive接口调用等结果中
        LiveRoomClosed,     // 房间已关闭，接下来马上回到Ready状态
        //Count,
    };

    static LivehimeLiveRoomController* GetInstance();

    // 确认连接的地址信息，根据new_link重新获取推流地址
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

    // 以指定开播类型打开直播间，打开成功后
    virtual StartLiveFlowResult OpenLiveRoom(StartLiveType start_type, bool start_by_hotkey,
        bool ignore_cover_check = false);

    //关闭直播间前处理
    virtual void CloseLiveRoomPre();

    // 关闭直播间
    virtual void CloseLiveRoom();

    // 重新开播
    virtual void RestartLive(LiveRestartMode mode);

    // 尝试切线路，当前处于自推流的场景下才允许切路线
    virtual bool TryToChangeStreamAddr(const std::string& addr, const std::string &code, const std::string &prtc);

    // 重新拿推流地址列表
    using GetStreamAddrListHandler = std::function<
        void(const secret::LiveStreamingService::ProtocolInfos& addrs)>;
    virtual void GetStreamAddrList(GetStreamAddrListHandler handler);

#pragma region(开播状态读取函数，使用时应确定到底是只需判断是否在开播流程中还是要精确到是否处于开播流程中的某个特定状态)

    // 房间是否处于待开播状态，与底层是否在推流无关，就是业务层当前是否处于开播流程进行中，
    // 当前可能处于异步结果（开/关播接口结果、推流准备等）等待中也可能正处于推流中
    virtual bool IsLiveReady() const { return live_status_ == LiveStatus::Ready; }

    // 推流底层当前是否正在持续推流
    virtual bool IsStreaming() const { return live_status_ == LiveStatus::Streaming; }

    virtual LiveStatus live_status() const { return live_status_; }

    virtual secret::LiveStreamingService::StartLiveInfo start_live_info() const { return start_live_info_; }

    virtual base::Time start_live_time() const { return start_live_time_; }

#pragma endregion

#pragma region(第三方推流转发相关)

    // 当前是否处于第三方推流模式下
    static bool IsInTheThirdPartyStreamingMode();

    // 进入第三方推流模式
    virtual bool EnterIntoThirdPartyStreamingMode();

    // 退出第三方推流模式
    virtual void LeaveThirdPartyStreamingMode();

    // 将第三方模式的相关任务、UI等关闭、状态重置
    virtual void InvalidateThirdPartyStreamingTask();

    virtual void GetBookLiveTimeInfos();
    virtual void HideBookLiveTime();

private:
    // 第三方推流底层进入监听状态
    bool ThirdPartyStreamingStartListen();
    void OnThirdPartyStreamingStartListenFailed(const base::string16& message = {});
    // 实际退出第三方模式
    void ActualLeaveThirdPartyStreamingMode();
    void ThirdPartyStreamingCloseLiveRoomTimeout();

    // 一分钟续流倒计时回调
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
    // 需要新的推流地址信息，其实就是要重新发起开播流程
    void NeedNewStreamingAddrInfos() override;
    // 需要测速
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

    // 检测一下当前程序是否处于自推流状态，自推流状态的定义由以下场景逻辑与确定：
    // 1、处于开播推流状态；
    // 2、不处于第三方推流状态；
    // 3、不处于“连线互动”状态（连线互动包括：视频连线、视频大乱斗、视频PK），
    //    这个“不处于”包括连UI都没有开的状态，即相关功能模块并未启动；
    bool CheckIfInSelfStreamingStatus();

    void OpenLiveRoomInternal();
    void CloseLiveRoomInternal(bool stop_streaming, void* param);
    // 切实调用开/关播接口
    void StartLive();
    void StopLive(void* param);

    // 开播相关
    //void OnGetPreliveInfo(bool valid, int code, const std::string& err_msg,
    //    const secret::LiveStreamingService::PreliveInfo& prelive_info);
    void OnChoosePartition(PartitionUpdateStatus status, void* param);
    void OnStartLive(bool valid, int code, const std::string& err_msg,
        const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    void OnStartLiveError(obs_proxy::StreamingErrorCode error_code,
        const std::wstring& extra_error_msg,
        int error_code_from_server,
        const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    // 处理开播接口的Notice数据段，截止目前(v3.12)只有实名认证补充认证使用这个扩展notice数据段
    void HandleStartLiveNotice(const secret::LiveStreamingService::StartLiveNotice& notice);

    // 关播相关
    void OnStopLive(bool success, int result_code, const std::string& err_msg,
        void* param);
    void TerminateStartLiveFlow();

    static void GetLiveErrorMsg(obs_proxy::StreamingErrorCode streaming_error_code, base::string16& error_msg, int& code);
    static const int kNoneServerErrorCode = 0;

#pragma region(回调业务聚合处理)

    // 直播间已开启
    void HandleLiveRoomOpened();

    // 正在发起推流
    void HandlePreStartStreaming(bool is_restart);

    // 流已经在推了
    void HandleStreamingStarted(const bililive::StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details);

    // 推流失败
    void HandleLiveStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code);

    // 推流终止，仅在无错误场景下的流终止情况进行通知，不会与HandleLiveStreamingError串行
    void HandleLiveStreamingStoped();

    // 正在发起断流
    void HandlePreStopStreaming(bool is_restart);

    // 直播间已关闭
    void HandleLiveRoomClosed(void* param);

#pragma endregion

private:
    base::NotificationRegistrar notifation_register_;

    std::atomic<LiveStatus> live_status_ = LiveStatus::Ready;
    bool stop_live_now_ = false;

    StartLiveType start_type_ = StartLiveType::Normal;
    base::Time last_set_channel_time_; // 最近一次设置分区成功的时间戳，未开播时超过5分钟后开播需重选分区
    base::Time start_live_time_;    // 开播的起始时间，调开播接口成功的时候

    bililive::OutputController* output_controller_ = nullptr;

    // 标识推流过程是否由快捷键发起
    bool start_by_hotkey_ = false;
    // 标识分区选择UI是否为开播流程弹出
    bool choose_area_showed_by_start_live_flow_ = false;
    // 标识是否正在更新房间信息
    bool updating_room_info_ = false;

    secret::LiveStreamingService::StartLiveInfo start_live_info_;

    //是否已经关播拦截过,一场直播只拦截一次
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