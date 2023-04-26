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
    // apm���ľ�̬���ݣ����������󲻻�䡢���Ƶ�ʺܵͻ���˵���ǲ��ں����ı��������
    struct ApmStaticInfo
    {
        std::string ip;     // �����Լ���ȡ�Ĺ���IP
        std::string buvid;  // 
        const std::string proid = "1000";   // ��Ʒ���
        const std::string chid = "bilibili";      // ������ʶ
#ifdef OS_WIN
        const std::string pid = "1001";           // ƽ̨
#elif (OS_MACOSX)
        const std::string pid = "1002";
#endif
        std::string osver;      // ϵͳ�汾
        std::string mid;        // ��¼���û�id
        std::string ver;        // �ͻ��˰汾��
        std::string net;        // �ͻ���ʹ�õ���������
        std::string oid;        // ��Ӫ��
        const std::string event_id = "live.pc.windows.apm";     // ����¼�
        std::string process_flag;// ���򱾴����еı�ʶ

        std::string cpu_info;       // ����������
        std::string memory;         // �ڴ����
        std::string gpu_info;       // �Կ�����
        std::string monitor_info;   // ��ʾ������
        std::string os_setting; // ϵͳ����

        // ��������Ƶ���ġ���̬������ ---------------------------
          
        std::string media_setting;  // ����Ƶ��������        
        std::string dmk_setting;    // ��Ļ������
        std::string live_info;      // ֱ����Ϣ
    };

    // apm���Ķ�̬���ݣ����ֲ�������
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

    // Ҫͳ�ƵĹ���ģ��
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

    // ����ģ���������Ϣ
    struct ModuleStatusInfo
    {
        std::string status;         // ģ�������״̬
        int64 start_ts = 0;         // ģ��˴ο�ʼ���е�ʱ�������
    };

public:
    static ApmStatisticAnalysisService* GetInstance();

protected:
    // LivehimeLiveRoomObserver
    // Ҫ�����ɹ����Ѿ���ʼ������ʱ��ſ�����¼����¼�����ĵ�ַ��Ϣ
    void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) override;
    // �ز���ֹͣ��¼�����ں���ʲôԭ������Ĺز�
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