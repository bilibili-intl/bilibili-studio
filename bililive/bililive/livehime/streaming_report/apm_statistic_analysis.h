#pragma once

#include <numeric>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/prefs/pref_change_registrar.h"
#include "base/threading/thread.h"
#include "base/timer/timer.h"

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
//#include "bililive/bililive/livehime/melee/melee_ui_presenter.h"
#include "bililive/bililive/livehime/obs/obs_status.h"
#include "bililive/bililive/livehime/streaming_report/streaming_qmcs_types.h"
//#include "bililive/bililive/livehime/video_conn/video_conn_ui_presenter.h"
//#include "bililive/bililive/livehime/voice_link/voice_link_ui_presenter.h"

#include "obs/obs-studio/libobs/obs.h"


class ApmStatisticAnalysisService : 
    LivehimeLiveRoomObserver,
    //livehime::MeleeUIObserver,
    //livehime::VideoConnUIObserver,
    //livehime::VoiceLinkUIObserver,
    base::NotificationObserver
{
    // apm埋点的静态数据，程序启动后不会变、变更频率很低或者说我们不在乎它的变更的数据
    struct ApmStaticInfo
    {
        std::string ip;     // 端上自己获取的公网IP
        std::string buvid;  // 
        const std::string proid = "1000";   // 产品编号
        const std::string chid = "bilibili";      // 渠道标识
#ifdef OS_WIN
        const std::string pid = "1001";           // 平台
#elif (OS_MACOSX)
        const std::string pid = "1002";
#endif
        std::string osver;      // 系统版本
        std::string mid;        // 登录的用户id
        std::string ver;        // 客户端版本号
        std::string net;        // 客户端使用的网络类型
        std::string oid;        // 运营商
        const std::string event_id = "live.pc.windows.apm";     // 埋点事件
        std::string process_flag;// 程序本次运行的标识

        std::string cpu_info;       // 处理器参数
        std::string memory;         // 内存参数
        std::string gpu_info;       // 显卡参数
        std::string monitor_info;   // 显示器参数
        std::string os_setting; // 系统设置

        // 变更不会很频繁的“静态”数据 ---------------------------
          
        std::string media_setting;  // 音视频参数设置        
        std::string dmk_setting;    // 弹幕姬设置
        std::string live_info;      // 直播信息
    };

    // apm埋点的动态数据，各种采样数据
    struct ApmDymicInfo
    {
        void Clear()
        {
            cpus.clear();
            mems.clear();
            gpus.clear();
            ndfs.clear();
            rdfs.clear();
            edfs.clear();
            tss.clear();
        }

        std::vector<StreamingReportSampleData> cpus;
        std::vector<StreamingReportSampleData> mems;
        std::vector<StreamingReportSampleData> gpus;
        std::vector<StreamingReportSampleData> ndfs;
        std::vector<StreamingReportSampleData> rdfs;
        std::vector<StreamingReportSampleData> edfs;
        std::vector<StreamingReportSampleData> dmk_rdrs;
        std::vector<int64> tss;

        bool gpu_valid = false;
    };

    // 要统计的功能模块
    enum class ApmMonitoringModule
    {
        Invalid,
        Stream = 1,
        Record = 1 << 1,
        VoiceLink = 1 << 2,
        VideoConn = 1 << 3,
        Melee = 1 << 4,
        VMelee = 1 << 5,
        Dmk = 1 << 6,
    };

    // 功能模块的运行信息
    struct ModuleStatusInfo
    {
        std::string status;         // 模块的运行状态
        int64 start_ts = 0;         // 模块此次开始运行的时间戳，秒
    };

public:
    static ApmStatisticAnalysisService* GetInstance();

protected:
    // LivehimeLiveRoomObserver
    // 要开播成功并已经开始推流的时候才开启记录，记录推流的地址信息
    void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) override;
    // 关播就停止记录，不在乎是什么原因引起的关播
    void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;

    // MeleeUIObserver
    //void OnMeleeStatusChanged(livehime::MeleeStatus status) override;

    //// VideoConnUIObserver
    //void OnVideoConnStreamingStart(bool is_inviting) override;
    //void OnVideoConnStreamingEnd() override;

    //// VoiceLinkUIObserver
    //void OnVlinkStatusChanged(livehime::VoiceLinkUIStatus status) override;

    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

private:
    ApmStatisticAnalysisService();
    ~ApmStatisticAnalysisService();

    void OnSamplingTimerOnWorkerThread(const StreamingReportInfo& info, bool gpu_valid, 
        bool df_valid, const streaming_info& s_info);
    void OnAppCpuSampling(const StreamingReportSampleData& sampledata);
    void OnAppMemorySampling(const StreamingReportSampleData& sampledata);
    void OnGpuSampling(const StreamingReportSampleData& sampledata);
    void OnNetworkDropFrameSampling(const StreamingReportSampleData& sampledata, const streaming_info& info);
    void OnRenderDropFrameSampling(const StreamingReportSampleData& sampledata, const streaming_info& info);
    void OnEncodeDropFrameSampling(const StreamingReportSampleData& sampledata, const streaming_info& info);

    void UpdateMediaSetting();
    void UpdateDmkSetting();
    void UpdateRoomInfo();

    void OnProfileDanmakuShowChanged();
    void OnProfileDanmakuFluencyChanged();

    void GenerateReportItem();
    void DoReport();

private:
    friend class StreamingReportService;
    friend struct std::default_delete<ApmStatisticAnalysisService>;

    PrefChangeRegistrar profile_pref_registrar_;
    base::NotificationRegistrar notifation_register_;

    ApmCtrl apm_ctrl_;
    int64 init_ts_ = 0;

    base::Lock info_lock_;
    ApmStaticInfo static_info_;
    std::map<ApmMonitoringModule, ModuleStatusInfo> module_status_;
    ApmDymicInfo apm_info_;

    std::list<std::string> report_items_;

    base::WeakPtrFactory<ApmStatisticAnalysisService> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ApmStatisticAnalysisService);
};