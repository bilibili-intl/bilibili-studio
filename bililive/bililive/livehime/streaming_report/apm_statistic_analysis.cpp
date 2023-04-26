#include "bililive/bililive/livehime/streaming_report/apm_statistic_analysis.h"

#include <psapi.h>

#include "base/base64.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/ext/callable_callback.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/bililive/utils/setting_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/user_info.h"

#include "obs/obs-studio/libobs/util/platform.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

namespace
{
    ApmStatisticAnalysisService* g_single_instance = nullptr;

    const std::string kApmLogid = "006474";  // APM LogId, b_ods.ods_s_live_livehime_apm_l_d

    // 显示器枚举
    BOOL CALLBACK MonitorEnumCallbackProc(HMONITOR, HDC, LPRECT rect, LPARAM param)
    {
        std::vector<std::string>& monitors =
            *reinterpret_cast<std::vector<std::string>*>(param);

        monitors.emplace_back(base::StringPrintf("%d-%d-%d-%d", 
            rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top));

        return true;
    }

}


ApmStatisticAnalysisService* ApmStatisticAnalysisService::GetInstance()
{
    return g_single_instance;
}

ApmStatisticAnalysisService::ApmStatisticAnalysisService()
    : weakptr_factory_(this)
{
    g_single_instance = this;

    if (AppFunctionController::GetInstance())
    {
        // APM有效才进行数据采样和上报
        apm_ctrl_ = AppFunctionController::GetInstance()->apm_ctrl();
    }
    DCHECK(apm_ctrl_.valid);

    static_info_.ip = GetBililiveProcess()->secret_core()->network_info().ip();
    static_info_.buvid = GetBililiveProcess()->secret_core()->network_info().buvid();

    // 系统配置
    cpu_and_os_version si = obs_get_cpu_and_os_version();
    static_info_.osver = si.os_version;

    static_info_.mid = std::to_string(GetBililiveProcess()->secret_core()->account_info().mid());
    static_info_.ver = BililiveContext::Current()->GetExecutableVersionAsASCII();

    // 地址信息
    static_info_.net = "5";
    if (secret::BehaviorEventNetwork::Wifi == (secret::BehaviorEventNetwork)GetBililiveProcess()->secret_core()->network_info().network_type())
    {
        static_info_.net = "1";
    }
    // 运营商可能是中文，即使UTF8转码后在拼成埋点格式时其后也不一定能正确拼上竖线分隔符“|”，所以对其进行base64编码
    static_info_.oid = base::UTF16ToUTF8(GetBililiveProcess()->secret_core()->network_info().communications_operator());
    base::Base64Encode(static_info_.oid, &static_info_.oid);

    base::ProcessId pid = base::Process::Current().pid();
    static_info_.process_flag = base::StringPrintf("%lld_%d", (int64)base::Time::Now().ToDoubleT(), pid);

    // 处理器参数
    static_info_.cpu_info = JoinString({ 
        base::StringPrintf("model=%s", si.cpu_name),
        base::StringPrintf("speed=%d", si.speed),
        base::StringPrintf("pc=%d", os_get_physical_cores()),
        base::StringPrintf("lc=%d", os_get_logical_cores()) },
        ";");

    // 内存参数
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);
    static_info_.memory = base::StringPrintf("total=%.1f", ms.ullTotalPhys * 1.0f / 1073741824);

    // 显卡参数
    std::vector<base::string16> gpus;
    gpus_info gi = obs_get_gpus_info();
    for (int i = 0; i < gi.gpu_count; i++)
    {
        if (i == gi.cur_load_gpu_index)
        {
            static_info_.gpu_info = base::StringPrintf("model=%s", gi.gpus[i].gpu_name);
            break;
        }
    }

    // 所有显示器分辨率
    std::vector<std::string> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumCallbackProc, (LPARAM)&monitors);
    static_info_.monitor_info = base::StringPrintf("num=%d;info=%s", monitors.size(),
        JoinString(monitors, ",").c_str());

    // 系统设置
    static_info_.os_setting = base::StringPrintf("admin=%d;aero=%d", 
        si.admin ? 1 : 0, si.aero ? 1 : 0);

    // 音视频参数设置
    UpdateMediaSetting();

    // 弹幕姬设置
    UpdateDmkSetting();

    // 直播信息
    UpdateRoomInfo();


    LivehimeLiveRoomController::GetInstance()->AddObserver(this);
    //livehime::MeleeUIPresenter::GetInstance()->AddObserver(this, false);
    //livehime::VideoConnUIPresenter::GetInstance()->AddObserver(this);
    //livehime::VoiceLinkUIPresenter::GetInstance()->AddObserver(this, false);

    profile_pref_registrar_.Init(GetBililiveProcess()->profile()->GetPrefs());
   /* profile_pref_registrar_.Add(prefs::kDanmakuHimeShow,
        base::Bind(&ApmStatisticAnalysisService::OnProfileDanmakuShowChanged, weakptr_factory_.GetWeakPtr()));*/
    profile_pref_registrar_.Add(prefs::kDanmakuHimeFluency,
        base::Bind(&ApmStatisticAnalysisService::OnProfileDanmakuFluencyChanged, weakptr_factory_.GetWeakPtr()));

    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS,
        base::NotificationService::AllSources());
    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_START_RECORDING,
        base::NotificationService::AllSources());
    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_STOP_RECORDING,
        base::NotificationService::AllSources());
}

ApmStatisticAnalysisService::~ApmStatisticAnalysisService()
{
    g_single_instance = nullptr;
    LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
    //livehime::MeleeUIPresenter::GetInstance()->RemoveObserver(this);
    //livehime::VideoConnUIPresenter::GetInstance()->RemoveObserver(this);
    //livehime::VoiceLinkUIPresenter::GetInstance()->RemoveObserver(this);
}

void ApmStatisticAnalysisService::OnSamplingTimerOnWorkerThread(const StreamingReportInfo& info, bool gpu_valid,
    bool df_valid, const streaming_info& s_info)
{
    // 这里的调用是由推流质量监测线程触发的，可以直接在这里对程序当前的各个性能参数进行采用记录，
    // CPU、丢帧等参数直接由推流质量检测服务提供，程序的其他动态或静态参数由在这里取
    if (0 == init_ts_)
    {
        init_ts_ = base::Time::Now().ToDoubleT();
    }

    if (!apm_ctrl_.valid)
    {
        return;
    }

    base::AutoLock lock(info_lock_);

    apm_info_.tss.push_back(info.data[StreamingSampleType::P_CPU].ts);
    apm_info_.gpu_valid = gpu_valid;

    OnAppCpuSampling(info.data[StreamingSampleType::P_CPU]);
    OnAppMemorySampling(info.data[StreamingSampleType::P_MEM]);
    OnGpuSampling(info.data[StreamingSampleType::GPU]);
    if (df_valid)
    {
        OnNetworkDropFrameSampling(info.data[StreamingSampleType::NDF], s_info);
        OnRenderDropFrameSampling(info.data[StreamingSampleType::RDF], s_info);
        OnEncodeDropFrameSampling(info.data[StreamingSampleType::EDF], s_info);
    }

    // 采样时间段达到预设值就进行本段数据计算，同时生成上报条目
    static int64 last_report_ts = base::Time::Now().ToDoubleT();
    int64 ts = base::Time::Now().ToDoubleT();
    int64 interval = ts - last_report_ts;
    if (interval >= apm_ctrl_.apm_sample_interval)
    {
        GenerateReportItem();
        last_report_ts = ts;
    }
}

void ApmStatisticAnalysisService::OnAppCpuSampling(const StreamingReportSampleData& sampledata)
{
    StreamingReportSampleData sample(sampledata);
    sample.data = sampledata.data;
    apm_info_.cpus.push_back(sample);
}

void ApmStatisticAnalysisService::OnAppMemorySampling(const StreamingReportSampleData& sampledata)
{
    StreamingReportSampleData sample(sampledata);
    sample.data = (int64)sampledata.data / (3.0f * 1024) * 100;
    apm_info_.mems.push_back(sample);
}

void ApmStatisticAnalysisService::OnGpuSampling(const StreamingReportSampleData& sampledata)
{
    StreamingReportSampleData sample(sampledata);
    sample.data = (int64)sampledata.data;
    apm_info_.gpus.push_back(sample);
}

void ApmStatisticAnalysisService::OnNetworkDropFrameSampling(const StreamingReportSampleData& sampledata, const streaming_info& info)
{
    StreamingReportSampleData sample(sampledata);
    sample.data = (int64)sampledata.data * 100;
    apm_info_.ndfs.push_back(sample);
}

void ApmStatisticAnalysisService::OnRenderDropFrameSampling(const StreamingReportSampleData& sampledata, const streaming_info& info)
{
    StreamingReportSampleData sample(sampledata);
    sample.data = (int64)sampledata.data * 100;
    apm_info_.rdfs.push_back(sample);
}

void ApmStatisticAnalysisService::OnEncodeDropFrameSampling(const StreamingReportSampleData& sampledata, const streaming_info& info)
{
    StreamingReportSampleData sample(sampledata);
    sample.data = (int64)sampledata.data * 100;
    apm_info_.edfs.push_back(sample);
}

void ApmStatisticAnalysisService::UpdateMediaSetting()
{
    // 编码设置信息
    auto pf = GetBililiveProcess()->profile()->GetPrefs();
    std::string  fps_s = pf->GetString(prefs::kVideoFPSCommon);
    int fps = atoi(fps_s.c_str());
    int bit_rate = pf->GetInteger(prefs::kVideoBitRate);
    std::string rate_type = pf->GetString(prefs::kVideoBitRateControl);
    int width = pf->GetInteger(prefs::kVideoOutputCX);
    int height = pf->GetInteger(prefs::kVideoOutputCY);
    int encode_quality = pf->GetInteger(prefs::kOutputStreamVideoQuality);
    std::string encoder = pf->GetString(prefs::kOutputStreamVideoEncoder);
    std::string record_type = pf->GetString(prefs::kOutputRecordingFormat);
    int a_bitrate = pf->GetInteger(prefs::kAudioBitRate);

    base::AutoLock lock(info_lock_);
    static_info_.media_setting = JoinString({
        base::StringPrintf("bitrate=%d", bit_rate),
        base::StringPrintf("rate_ctrl=%s", rate_type.c_str()),
        base::StringPrintf("fps=%d", fps),
        base::StringPrintf("size=%dx%d", width, height),
        base::StringPrintf("encoder=%s", encoder.c_str()),
        base::StringPrintf("encoder_quality=%d", encode_quality),
        base::StringPrintf("record_type=%s", record_type.c_str()),
        base::StringPrintf("a_bitrate=%d", a_bitrate) },
        ";");
}

void ApmStatisticAnalysisService::UpdateDmkSetting()
{
    auto pf = GetBililiveProcess()->profile()->GetPrefs();
    int fluency = pf->GetInteger(prefs::kDanmakuHimeFluency);

    base::AutoLock lock(info_lock_);
    static_info_.dmk_setting = base::StringPrintf("fluency=%d", fluency);
}

void ApmStatisticAnalysisService::UpdateRoomInfo()
{
    int64 roomid = GetBililiveProcess()->secret_core()->user_info().room_id();
    int area_id = GetBililiveProcess()->secret_core()->anchor_info().current_parent_area();
    int sec_area_id = GetBililiveProcess()->secret_core()->anchor_info().current_area();
    std::string live_key = GetBililiveProcess()->secret_core()->anchor_info().live_key();

    base::AutoLock lock(info_lock_);
    static_info_.live_info = JoinString({
        base::StringPrintf("roomid=%lld", roomid),
        base::StringPrintf("area_id=%d", area_id),
        base::StringPrintf("sec_area_id=%d", sec_area_id),
        base::StringPrintf("live_key=%s", live_key.c_str()) },
        ";");
}

void ApmStatisticAnalysisService::OnProfileDanmakuShowChanged()
{
  /*  auto pf = GetBililiveProcess()->profile()->GetPrefs();
    bool show = pf->GetBoolean(prefs::kDanmakuHimeShow);

    base::AutoLock lock(info_lock_);
    if (show)
    {
        ModuleStatusInfo module;
        module.status = "1";
        module.start_ts = base::Time::Now().ToDoubleT();
        module_status_[ApmMonitoringModule::Dmk] = module;
    }
    else
    {
        module_status_.erase(ApmMonitoringModule::Dmk);
    }*/
}

void ApmStatisticAnalysisService::OnProfileDanmakuFluencyChanged()
{
    UpdateDmkSetting();
}

void ApmStatisticAnalysisService::OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details)
{
    UpdateRoomInfo();
    UpdateMediaSetting();

    base::AutoLock lock(info_lock_);
    ModuleStatusInfo module;
    module.status = "1";
    module.start_ts = base::Time::Now().ToDoubleT();
    module_status_[ApmMonitoringModule::Stream] = module;
}

void ApmStatisticAnalysisService::OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    base::AutoLock lock(info_lock_);
    module_status_.erase(ApmMonitoringModule::Stream);
}

//void ApmStatisticAnalysisService::OnMeleeStatusChanged(livehime::MeleeStatus status)
//{
//    if (status == livehime::MeleeStatus::MeleeCountDown)
//    {
//        ApmMonitoringModule type = livehime::MeleeUIPresenter::GetInstance()->IsMeleeColive() ? 
//            ApmMonitoringModule::VMelee : ApmMonitoringModule::Melee;
//
//        base::AutoLock lock(info_lock_);
//        ModuleStatusInfo module;
//        module.status = "1";
//        module.start_ts = base::Time::Now().ToDoubleT();
//        module_status_[type] = module;
//    }
//    else if (status == livehime::MeleeStatus::Ready)
//    {
//        base::AutoLock lock(info_lock_);
//        module_status_.erase(ApmMonitoringModule::Melee);
//        module_status_.erase(ApmMonitoringModule::VMelee);
//    }
//}

//// VideoConnUIObserver
//void ApmStatisticAnalysisService::OnVideoConnStreamingStart(bool is_inviting)
//{
//    base::AutoLock lock(info_lock_);
//    ModuleStatusInfo module;
//    module.status = "1";
//    module.start_ts = base::Time::Now().ToDoubleT();
//    module_status_[ApmMonitoringModule::VideoConn] = module;
//}
//
//void ApmStatisticAnalysisService::OnVideoConnStreamingEnd()
//{
//    base::AutoLock lock(info_lock_);
//    module_status_.erase(ApmMonitoringModule::VideoConn);
//}
//
//// VoiceLinkUIObserver
//void ApmStatisticAnalysisService::OnVlinkStatusChanged(livehime::VoiceLinkUIStatus status)
//{
//    if (status == livehime::VoiceLinkUIStatus::Linking)
//    {
//        base::AutoLock lock(info_lock_);
//        ModuleStatusInfo module;
//        module.status = "1";
//        module.start_ts = base::Time::Now().ToDoubleT();
//        module_status_[ApmMonitoringModule::VoiceLink] = module;
//    }
//    else if (status == livehime::VoiceLinkUIStatus::Ready)
//    {
//        base::AutoLock lock(info_lock_);
//        module_status_.erase(ApmMonitoringModule::VoiceLink);
//    }
//}

// NotificationObserver
void ApmStatisticAnalysisService::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS:
        UpdateRoomInfo();
        break;
    case bililive::NOTIFICATION_LIVEHIME_START_RECORDING:
        {
            base::AutoLock lock(info_lock_);
            ModuleStatusInfo module;
            module.status = "1";
            module.start_ts = base::Time::Now().ToDoubleT();
            module_status_[ApmMonitoringModule::Record] = module;
        }
        break;
    case bililive::NOTIFICATION_LIVEHIME_STOP_RECORDING:
        {
            base::AutoLock lock(info_lock_);
            module_status_.erase(ApmMonitoringModule::Record);
        }
        break;
    default:
        break;
    }
}

void ApmStatisticAnalysisService::GenerateReportItem()
{
    int64 ts = base::Time::Now().ToDoubleT();
    int64 jts = base::Time::Now().ToJsTime();

    std::vector<std::string> fields;

    // 静态数据
    {
        fields.push_back(kApmLogid + std::to_string(jts) + static_info_.ip);   // logid+ts(ms)+first_field(0 local_ip)
        fields.push_back("2");                      // 1 version
        fields.push_back(static_info_.buvid);       // 2 buvid
        fields.push_back(static_info_.proid);       // 3 proid
        fields.push_back(static_info_.chid);        // 4 chid
        fields.push_back(static_info_.pid);         // 5 pid
        fields.push_back(static_info_.osver);       // 6 osver
        fields.push_back(std::to_string(jts));      // 7 ctime
        fields.push_back(static_info_.mid);         // 8 mid
        fields.push_back(static_info_.ver);         // 9 ver
        fields.push_back(static_info_.net);         // 10 net
        fields.push_back(static_info_.oid);         // 11 oid
        fields.push_back(static_info_.event_id);    // 12 event_id
        fields.push_back(static_info_.process_flag);// 13 process_flag
        fields.push_back(std::to_string(std::max(0ll, ts - init_ts_)));// 14 last_report_interval
        fields.push_back(static_info_.cpu_info);    // 15 cpu_info
        fields.push_back(static_info_.memory);      // 16 memory
        fields.push_back(static_info_.gpu_info);    // 17 gpu_info
        fields.push_back(static_info_.monitor_info);// 18 monitor_info
        fields.push_back(static_info_.os_setting);  // 19 os_setting
        fields.push_back(static_info_.media_setting);// 20 media_setting
        fields.push_back(static_info_.dmk_setting); // 21 dmk_setting
        fields.push_back(static_info_.live_info);   // 22 live_info
    }

    // 程序功能模块状态
    {
        static std::map<ApmMonitoringModule, std::string> kModuleName{
            { ApmMonitoringModule::Stream, "stream" },
            { ApmMonitoringModule::Record, "record" },
            { ApmMonitoringModule::VoiceLink, "vlink" },
            { ApmMonitoringModule::VideoConn, "vconn" },
            { ApmMonitoringModule::Melee, "melee_c" },
            { ApmMonitoringModule::VMelee, "melee_v" },
            { ApmMonitoringModule::Dmk, "dmk" },
        };

        std::vector<std::string> module_status;
        for (auto& iter : module_status_)
        {
            ModuleStatusInfo& module = iter.second;
            std::string name = kModuleName[iter.first];
            module_status.push_back(base::StringPrintf("%s=%s,%lld", name.c_str(), 
                module.status.c_str(), std::max(0ll, ts - module.start_ts)));
        }
        fields.push_back(JoinString(module_status, ";"));// 23 module_status
    }

    // APM数据
    {
        std::vector<std::string> apm_info;

        static auto avg_fn = [](const std::vector<StreamingReportSampleData>& samples)->int
        {
            if (samples.empty())
            {
                return 0;
            }

            int64 total = 0;
            for (auto& iter : samples)
            {
                total += (int64)(iter.data);
            }

            int avg = total / samples.size();
            return avg;
        };

        int cpu = avg_fn(apm_info_.cpus);
        int mem = avg_fn(apm_info_.mems);
        int gpu = avg_fn(apm_info_.gpus);
        int ndr = avg_fn(apm_info_.ndfs);
        int rdr = avg_fn(apm_info_.rdfs);
        int edr = avg_fn(apm_info_.edfs);
        int dmk_rdrs = avg_fn(apm_info_.dmk_rdrs);
        apm_info.push_back(base::StringPrintf("cpu=%d", cpu));
        apm_info.push_back(base::StringPrintf("mem=%d", mem));
        apm_info.push_back(base::StringPrintf("gpu=%d", gpu));
        apm_info.push_back(base::StringPrintf("ndr=%d", ndr));
        apm_info.push_back(base::StringPrintf("rdr=%d", rdr));
        apm_info.push_back(base::StringPrintf("edr=%d", edr));
        apm_info.push_back(base::StringPrintf("dmk_rdr=%d", dmk_rdrs));
        apm_info.push_back(base::StringPrintf("st_begin=%lld", apm_info_.tss.empty() ? 0ll : apm_info_.tss.front()));
        apm_info.push_back(base::StringPrintf("st_end=%lld", apm_info_.tss.empty() ? 0ll : apm_info_.tss.back()));
        apm_info.push_back(base::StringPrintf("gpu_valid=%d", apm_info_.gpu_valid ? 1 : 0));

        // 生成一条apm统计数据之后把当前记录的所有apm数据清除
        apm_info_.Clear();

        fields.push_back(JoinString(apm_info, ";"));// 24 apm_info
    }

    // 扩展数据
    static std::string ext_info;
    fields.push_back(ext_info);// 25 ext_info

    // 从26开始，本意是想让埋点接口自己加这些字段，但是现在埋点接口并没加，那么我们自己加上
    fields.push_back("web");// 26 request_uri

    std::string msg = JoinString(fields, "|");

    report_items_.push_back(msg);

    // 埋点数据积压到一定数目就一次性整合发出
    if ((int)report_items_.size() >= apm_ctrl_.apm_cumulant)
    {
        DoReport();
    }
}

void ApmStatisticAnalysisService::DoReport()
{
    if (report_items_.empty())
    {
        return;
    }

    // 当前暂时继续采用/log/web接口上报（不能一次性报多条），就不拼接了
    /*std::string msg;
    for (auto& iter : report_items_)
    {
        msg.append(iter);
        msg.push_back('\003');
    }
    msg.pop_back();
    report_items_.clear();
    livehime::ApmEventReport(msg);*/

    // /log/web不拼接直接发出
    for (auto& iter : report_items_)
    {
        livehime::ApmEventReport(iter);
    }
    report_items_.clear();
}
