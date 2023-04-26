#include "bililive/bililive/livehime/streaming_report/streaming_report_service.h"
#include <cinttypes>
#include <psapi.h>

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>

#include "base/file_util.h"
#include "base/guid.h"
#include "base/path_service.h"
#include "base/ext/callable_callback.h"
#include "base/json/json_writer.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_split.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/livehime/streaming_report/apm_statistic_analysis.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/bililive/utils/setting_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/protobuf/streaming_details.pb.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/user_info.h"

#include "obs.h"
#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "math.h"

namespace
{
    const int kSampleRate = 1; // times/seconds
    StreamingReportService* g_single_instance = nullptr;
    const double KMinMemoryWarn = 1200.0f;
    const double kADDWarnMemory = 400.0f;
#ifdef _WIN64
    const char kNvAPIDLL[] = "nvapi64.dll";
#else
    const char kNvAPIDLL[] = "nvapi.dll";
#endif
    int GetNumberOfProcessors()
    {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors;
    }

    int GetAppThreads()
    {
        int threads = 0;
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(pe32);
		HANDLE process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (process_snap != INVALID_HANDLE_VALUE)
		{
			DWORD pro_id = GetCurrentProcessId();
			BOOL process_first = Process32First(process_snap, &pe32);
			while (process_first)
			{
				if (pe32.th32ProcessID == pro_id)
				{
                    threads = pe32.cntThreads;
                    break;
				}
				process_first = Process32Next(process_snap, &pe32);
			}
			CloseHandle(process_snap);
		}
        return threads;
    }

    double GetLivehimeCpuRate()
    {
        HANDLE process = ::GetCurrentProcess();
        static DWORD tick_count_old = 0;
        static LARGE_INTEGER process_time_old = { 0 };
        static DWORD processor_core_num = 0;
        if (!processor_core_num)
        {
            SYSTEM_INFO sysInfo = { 0 };
            GetSystemInfo(&sysInfo);
            processor_core_num = sysInfo.dwNumberOfProcessors;
        }
        double proc_cpu_percent = 0;
        BOOL ret = FALSE;

        FILETIME create_time, exit_time, kernel_time, user_time;
        LARGE_INTEGER lg_kernel_time;
        LARGE_INTEGER lg_user_time;
        LARGE_INTEGER lg_cur_time;

        ret = GetProcessTimes(process, &create_time, &exit_time, &kernel_time, &user_time);
        if (ret)
        {
            lg_kernel_time.HighPart = kernel_time.dwHighDateTime;
            lg_kernel_time.LowPart = kernel_time.dwLowDateTime;
            lg_user_time.HighPart = user_time.dwHighDateTime;
            lg_user_time.LowPart = user_time.dwLowDateTime;
            lg_cur_time.QuadPart = (lg_kernel_time.QuadPart + lg_user_time.QuadPart);
            if (process_time_old.QuadPart)
            {
                DWORD elepsed_time = ::GetTickCount() - tick_count_old;
                if (elepsed_time > 0 && processor_core_num > 0)
                {
                    proc_cpu_percent = (double)(((double)((lg_cur_time.QuadPart - process_time_old.QuadPart) * 100)) / elepsed_time) / 10000;
                    proc_cpu_percent = proc_cpu_percent / processor_core_num;
                }
            }
            process_time_old = lg_cur_time;
            tick_count_old = ::GetTickCount();
        }

        return proc_cpu_percent;
    }

    class CpuMonitor
    {
    public:
        CpuMonitor() = default;
        ~CpuMonitor() = default;

        void Init()
        {
            if (cpu_info_is_first_)
            {
                GetSystemTimes(&pre_idle_time_, &pre_kernel_time_, &pre_user_time_);

                cpu_info_is_first_ = false;
            }
        }

        double GetUsage()
        {
            DCHECK(!cpu_info_is_first_);

            double cpurate;
            int64 idle, kernel, user;
            FILETIME idleTime;      //空闲时间
            FILETIME kernelTime;  //内核态时间
            FILETIME userTime;      //用户态时间

            //计算CPU使用率，需要收集两份样本
            /*if(cpu_info_is_first_)
            {
                GetSystemTimes(&pre_idle_time_, &pre_kernel_time_, &pre_user_time_);

                cpu_info_is_first_ = false;
                return 0;
            }*/

            GetSystemTimes(&idleTime, &kernelTime, &userTime);

            idle = DiffFileTime(pre_idle_time_, idleTime);
            kernel = DiffFileTime(pre_kernel_time_, kernelTime);
            user = DiffFileTime(pre_user_time_, userTime);

            uint64 total = kernel + user;
            if (kernel + user == 0)
                cpurate = 0.0;
            else
                cpurate = abs((total - idle) * 100.0f / total);//（总的时间-空闲时间）/总的时间=占用cpu的时间就是使用率

            pre_idle_time_ = idleTime;
            pre_kernel_time_ = kernelTime;
            pre_user_time_ = userTime;

            return cpurate;
        }

        void Uninit()
        {
            cpu_info_is_first_ = true;
        }

    private:
        int64 DiffFileTime(FILETIME time1, FILETIME time2)
        {
            int64 a = (int64)time1.dwHighDateTime << 32 | time1.dwLowDateTime;
            int64 b = (int64)time2.dwHighDateTime << 32 | time2.dwLowDateTime;
            return (b - a);
        }

    private:
        bool cpu_info_is_first_ = true;
        FILETIME pre_idle_time_ = { 0 };
        FILETIME pre_kernel_time_ = { 0 };
        FILETIME pre_user_time_ = { 0 };
    };

    class GpuMonitor
    {
       #define NVAPI_MAX_PHYSICAL_GPUS   64
       #define NVAPI_MAX_USAGES_PER_GPU  34
        // function pointer types
        typedef int *(*NvAPI_QueryInterface_t)(unsigned int offset);
        typedef int(*NvAPI_Initialize_t)();
        typedef int(*NvAPI_EnumPhysicalGPUs_t)(int **handles, int *count);
        typedef int(*NvAPI_GPU_GetUsages_t)(int *handle, unsigned int *usages);

    public:
        GpuMonitor() = default;
        ~GpuMonitor() = default;

        bool Init()
        {
            __try
            {
                HMODULE hmod = LoadLibraryA(kNvAPIDLL);
                if (!hmod)
                {
                    return false;
                }

                // nvapi_QueryInterface is a function used to retrieve other internal functions in nvapi.dll
                NvAPI_QueryInterface_ = (NvAPI_QueryInterface_t)GetProcAddress(hmod, "nvapi_QueryInterface");

                if (!NvAPI_QueryInterface_)
                    return false;

                // some useful internal functions that aren't exported by nvapi.dll
                NvAPI_Initialize_ = (NvAPI_Initialize_t)(*NvAPI_QueryInterface_)(0x0150E828);
                NvAPI_EnumPhysicalGPUs_ = (NvAPI_EnumPhysicalGPUs_t)(*NvAPI_QueryInterface_)(0xE5AC921F);
                NvAPI_GPU_GetUsages_ = (NvAPI_GPU_GetUsages_t)(*NvAPI_QueryInterface_)(0x189A1FDF);

                if (NvAPI_Initialize_ == NULL || NvAPI_EnumPhysicalGPUs_ == NULL ||
                    NvAPI_EnumPhysicalGPUs_ == NULL || NvAPI_GPU_GetUsages_ == NULL)
                {
                    return false;
                }

                int nResult = NvAPI_Initialize_();
                if (0 == nResult)
                {
                    return true;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }

            return  false;
        }

        int GetUsage()
        {
            int          gpuCount = 0;
            int         *gpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = { NULL };
            unsigned int gpuUsages[NVAPI_MAX_USAGES_PER_GPU] = { 0 };

            int usage = 0;
            // gpuUsages[0] must be this value, otherwise NvAPI_GPU_GetUsages won't work
            gpuUsages[0] = (NVAPI_MAX_USAGES_PER_GPU * 4) | 0x10000;

            int nResult = (*NvAPI_EnumPhysicalGPUs_)(gpuHandles, &gpuCount);
            if(0 == nResult)
            {
                nResult = (*NvAPI_GPU_GetUsages_)(gpuHandles[0], gpuUsages);
                if(0 == nResult)
                    usage = gpuUsages[3];
            }

            return usage;
        }
    private:
        // nvapi.dll internal function pointers
        NvAPI_QueryInterface_t      NvAPI_QueryInterface_ = NULL;
        NvAPI_Initialize_t          NvAPI_Initialize_ = NULL;
        NvAPI_EnumPhysicalGPUs_t    NvAPI_EnumPhysicalGPUs_ = NULL;
        NvAPI_GPU_GetUsages_t       NvAPI_GPU_GetUsages_ = NULL;
    };

}


StreamingReportService* StreamingReportService::GetInstance()
{
    return g_single_instance;
}

int StreamingReportService::GetSampleRate()
{
    return kSampleRate;
}

StreamingReportService::StreamingReportService()
    : worker_thread_("StreamingMonitorThread")
{
    g_single_instance = this;
}

StreamingReportService::~StreamingReportService()
{
    g_single_instance = nullptr;
}

void StreamingReportService::Initialize()
{
    cpu_monitor_ = std::make_unique<CpuMonitor>();
    cpu_monitor_->Init();

    LivehimeLiveRoomController::GetInstance()->AddObserver(this);
    AppFunctionController::GetInstance()->AddObserver(this);

    // 由工作线程来主动采样
    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_IO;
    worker_thread_.StartWithOptions(options);
    worker_thread_.message_loop()->PostTask(FROM_HERE,
        base::Bind(&StreamingReportService::StartOnWorkerThread, this));
}

void StreamingReportService::StartOnWorkerThread()
{
    sampling_timer_.reset(new base::RepeatingTimer<StreamingReportService>());
    sampling_timer_->Start(FROM_HERE, base::TimeDelta::FromSeconds(kSampleRate), this,
        &StreamingReportService::OnSamplingTimerOnWorkerThread);
}

void StreamingReportService::OnSamplingTimerOnWorkerThread()
{
    base::Time now = base::Time::Now();
    double ts = now.ToDoubleT();

    StreamingReportInfo info(ts);

    int remote_detect_duration = AppFunctionController::GetInstance()->streaming_qmcs_ctrl().remote_detect_duration;
    bool stream_valid = false;
    bool df_valid = false;
    streaming_info s_info;
    {
        std::unique_lock<std::mutex> lg(output_mutex_);
        if (stream_output_)
        {
            s_info = obs_output_get_streaming_info(stream_output_);
            df_valid = true;
        }
    }

    {
        base::AutoLock lock(pb_locker_);
        stream_valid = (bool)pb_writer_;

        OnCpuSampling(info.data[StreamingSampleType::CPU]);
        OnGpuSampling(info.data[StreamingSampleType::GPU]);

        if (is_analysis_live_quality_ || live_quality_report_info_.is_report)
        {
            cpu_queue_.PushData(info.data[StreamingSampleType::CPU].data);
            gpu_queue_.PushData(info.data[StreamingSampleType::GPU].data);
        }

        OnMemorySampling(info.data[StreamingSampleType::MEM]);
        OnLivehimeCpuSampling(info.data[StreamingSampleType::P_CPU]);
        OnLivehimeMemorySampling(info.data[StreamingSampleType::P_MEM]);
        if (df_valid)
        {
            OnNetworkDropFrameSampling(info.data[StreamingSampleType::NDF], s_info);

            OnRenderDropFrameSampling(info.data[StreamingSampleType::RDF], s_info);

            OnEncodeDropFrameSampling(info.data[StreamingSampleType::EDF], s_info);

            if (is_analysis_live_quality_ || live_quality_report_info_.is_report)
            {
                net_queue_.PushData(info.data[StreamingSampleType::NDF].data);
                rdr_queue_.PushData(info.data[StreamingSampleType::RDF].data);
                enc_queue_.PushData(info.data[StreamingSampleType::EDF].data);
                //分析推流质量
                AnalysisLiveQuality();
            }
            //需要发送的总frame数
            info.data[StreamingSampleType::NET_NEED_TOTAL_FRAMES].raw_value[0] = std::max<int64_t>(0,(s_info.total_frame - total_net_need_send_));
            total_net_need_send_ = s_info.total_frame;
            //srt相关信息
            //info.data[StreamingSampleType::SRT_SEND_PKTS].raw_value[0] = s_info.srt_sent_pkts;
            //info.data[StreamingSampleType::SRT_LOST_PKTS].raw_value[0] = s_info.srt_lost_pkts;
            //info.data[StreamingSampleType::SRT_DROP_PKTS].raw_value[0] = s_info.srt_drop_pkts;
            //info.data[StreamingSampleType::SRT_RETTRANS_PKTS].raw_value[0] = s_info.srt_retrans_pkts;
            //info.data[StreamingSampleType::SRT_SEND_RATE_KBPS].data = s_info.srt_send_rate_mbps*1024;
            //info.data[StreamingSampleType::SRT_RTT_MS].data = s_info.srt_rtt_time;

            if (fabs(info.data[StreamingSampleType::SRT_SEND_RATE_KBPS].data <= 1e-15) && fabs(srt_send_rate_mbps_last_) > 1e-6) {
                live_quality_report_info_.send_rate_zero_count++;
            }
            srt_send_rate_mbps_last_ = info.data[StreamingSampleType::SRT_SEND_RATE_KBPS].data;
        }


        // 将实时采样数据传给apm统计
        if (apm_statistic_)
        {
            apm_statistic_->OnSamplingTimerOnWorkerThread(info, IsGpuValid(), df_valid, s_info);
        }

        // 缓存最近150秒的采样数据，以便于UI展示时能立刻看到最近一段时间的情况
        last_cache_samples_.push_back(info);
        if ((int)last_cache_samples_.size() > remote_detect_duration)
        {
            last_cache_samples_.pop_front();
        }
    }

    // 往UI线程抛一下任务，通知一下性能参数的采样更新
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&StreamingReportService::InformSampling, this,
            info));

    if (stream_valid)
    {
        int delta = (now - start_record_time_).InSeconds();

        // 每n秒请求一次当前流质量
        static int detect_delta = 0;
        if ((remote_detect_duration > 0) &&
            (delta >= remote_detect_duration) &&
            (detect_delta != delta / remote_detect_duration))
        {
            detect_delta = delta / remote_detect_duration;

            RequestStreamQualityFromServer();
        }

        // 每n秒主动上报一次本地最近n秒的质量数据
        int remote_report_duration = AppFunctionController::GetInstance()->streaming_qmcs_ctrl().remote_report_duration;
        static int report_delta = 0;
        if ((remote_report_duration > 0) &&
            (delta >= remote_report_duration) &&
            (report_delta != delta / remote_report_duration))
        {
            report_delta = delta / remote_report_duration;

            ReportLocalStreamQualityToServer();
        }
    }
    //开播性能质量体验建设上报
    ReportLiveQualityInfo(info);

    //APP时长上报,每隔apptime_kv_control_上报
    if (AppFunctionController::GetInstance()->get_apptime_kv_control().valid) {
        apptime_count_++;
        if (apptime_count_ > 0 && apptime_count_ == report_time_) {
            StreamingReportService::ReportLiveAppUseTime(apptime_count_);
            apptime_count_ = 0;
        }
    }
}

void StreamingReportService::StopOnWorkerThread()
{
    if (sampling_timer_)
    {
        sampling_timer_.reset();
    }
}

void StreamingReportService::Uninitialize()
{
    if (worker_thread_.IsRunning())
    {
        worker_thread_.message_loop()->PostTask(FROM_HERE,
            base::Bind(&StreamingReportService::StopOnWorkerThread, this));
    }
    worker_thread_.Stop();
    StopRecord();
    apm_statistic_.reset();
    pb_writer_.reset();
    cpu_monitor_.reset();
    gpu_monitor_.reset();
    LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
    AppFunctionController::GetInstance()->RemoveObserver(this);
}

void StreamingReportService::StartRecord()
{
    // 开关没开就不记录了
    if (AppFunctionController::GetInstance() &&
        !AppFunctionController::GetInstance()->streaming_qmcs_ctrl().switch_on)
    {
        return;
    }

    base::AutoLock lock(pb_locker_);
    if (!pb_writer_)
    {
        pb_writer_.reset(new streaming_performance::PerformanceReport());

        quality_ext_info_.reset(new base::DictionaryValue());
        quality_report_common_fields_.clear();

        start_record_time_ = base::Time::Now();

        pb_writer_->set_logver(1);
        pb_writer_->set_sampling(kSampleRate);

        quality_ext_info_->SetInteger64("start_ts", (int64_t)start_record_time_.ToDoubleT());
        quality_ext_info_->SetInteger("sample_interval", AppFunctionController::GetInstance()->streaming_qmcs_ctrl().remote_report_duration);

        // 系统配置
        cpu_and_os_version si = obs_get_cpu_and_os_version();
        pb_writer_->mutable_app_info()->mutable_sys_info()->set_system_ver(si.os_version);
        pb_writer_->mutable_app_info()->mutable_sys_info()->set_cpu(si.cpu_name);

        std::string os_ver(si.os_version);
        quality_ext_info_->SetString("system_ver", os_ver);
        quality_ext_info_->SetString("cpu", si.cpu_name);
        ReplaceChars(os_ver, ";", ",", &os_ver);
        quality_report_common_fields_.push_back("os_ver:" + os_ver);
        quality_report_common_fields_.push_back("os_cpu:" + std::string(si.cpu_name));

        // gpu
        std::vector<std::string> gpus;
        gpus_info gi = obs_get_gpus_info();
        for (int i = 0; i < gi.gpu_count; i++)
        {
            std::string *gpu = pb_writer_->mutable_app_info()->mutable_sys_info()->add_graphic();
            *gpu = base::StringPrintf("%s (driver ver: %s)", gi.gpus[i].gpu_name, gi.gpus[i].driver_version);

            gpus.push_back(*gpu);

            if (i == gi.cur_load_gpu_index)
            {
                pb_writer_->mutable_app_info()->mutable_sys_info()->set_load_graphic(gi.gpus[i].gpu_name);
            }
        }

        quality_ext_info_->SetString("graphic", JoinString(gpus, ';'));
        quality_report_common_fields_.push_back("os_gpu:" + JoinString(gpus, ','));

        // 内存
        std::string mem;
        MEMORYSTATUSEX memory;
        memory.dwLength = sizeof(memory);
        if (GlobalMemoryStatusEx(&memory))
        {
            float gb = memory.ullTotalPhys * 1.0f / 1024 / 1024 / 1024;
            mem = base::StringPrintf("%.1fGB", gb);
            pb_writer_->mutable_app_info()->mutable_sys_info()->set_memory(mem);
        }

        quality_ext_info_->SetString("memory", mem);
        quality_report_common_fields_.push_back("os_mem:" + mem);

        // 地址信息
        streaming_performance::RuntimeNetWork nw = streaming_performance::RuntimeNetWork::ETHERNET;
        if (secret::BehaviorEventNetwork::Wifi == (secret::BehaviorEventNetwork)GetBililiveProcess()->secret_core()->network_info().network_type())
        {
            nw = streaming_performance::RuntimeNetWork::WIFI;
        }
        std::string co = base::UTF16ToUTF8(GetBililiveProcess()->secret_core()->network_info().communications_operator());
        std::string region = base::UTF16ToUTF8(GetBililiveProcess()->secret_core()->network_info().region());
        pb_writer_->mutable_app_info()->mutable_sys_info()->set_network(nw);
        pb_writer_->mutable_app_info()->mutable_sys_info()->set_co(co);
        pb_writer_->mutable_app_info()->mutable_sys_info()->set_ip(GetBililiveProcess()->secret_core()->network_info().ip());
        pb_writer_->mutable_app_info()->mutable_sys_info()->set_region(region);

        quality_ext_info_->SetString("network", std::to_string((int)nw));
        quality_report_common_fields_.push_back("network:" + std::to_string((int)nw));
        quality_report_common_fields_.push_back("co:" + co);
        quality_report_common_fields_.push_back("region:" + region);

        // 程序/用户信息
        pb_writer_->mutable_app_info()->set_app_id(1);
        pb_writer_->mutable_app_info()->set_platform(1);
        pb_writer_->mutable_app_info()->set_buvid(GetBililiveProcess()->secret_core()->network_info().buvid());
        pb_writer_->mutable_app_info()->set_version(BililiveContext::Current()->GetExecutableVersionAsASCII());
        pb_writer_->mutable_app_info()->set_build_no(BililiveContext::Current()->GetExecutableBuildNumber());
        pb_writer_->mutable_app_info()->set_account(GetBililiveProcess()->secret_core()->account_info().account());
        pb_writer_->mutable_app_info()->set_uid(GetBililiveProcess()->secret_core()->account_info().mid());
        pb_writer_->mutable_app_info()->set_roomid(GetBililiveProcess()->secret_core()->user_info().room_id());

        quality_report_common_fields_.push_back("room_id:" + std::to_string(GetBililiveProcess()->secret_core()->user_info().room_id()));
        quality_report_common_fields_.push_back("build_id:" + std::to_string(BililiveContext::Current()->GetExecutableBuildNumber()));

        // 开播信息
        pb_writer_->mutable_runtime_info()->set_start_time(start_record_time_.ToDoubleT());
        pb_writer_->mutable_runtime_info()->set_live_status(1);
        pb_writer_->mutable_runtime_info()->set_live_key(GetBililiveProcess()->secret_core()->anchor_info().live_key());
        pb_writer_->mutable_runtime_info()->set_live_type(LivehimeLiveRoomController::GetInstance()->IsInTheThirdPartyStreamingMode() ? 1 : 0);

        quality_report_common_fields_.push_back("start_live_ts:" + std::to_string((int64_t)start_record_time_.ToDoubleT()));  // 本场直播的开始时间
        quality_report_common_fields_.push_back("livekey:" + GetBililiveProcess()->secret_core()->anchor_info().live_key());  // 本场livekey
        quality_report_common_fields_.push_back("live_type:" + std::to_string(LivehimeLiveRoomController::GetInstance()->IsInTheThirdPartyStreamingMode() ? 2 : 0));
    }

    // 一次开播可能多次改设置，把每一次开始记录的编码参数都记下来，方便数据分析时区分参数设置导致的质量波动
    if (pb_writer_)
    {
        // 编码设置信息
        auto pf = GetBililiveProcess()->profile()->GetPrefs();
        auto settings = pb_writer_->mutable_runtime_info()->add_video_settings();
        std::string  fps_s = pf->GetString(prefs::kVideoFPSCommon);
        int fps = atoi(fps_s.c_str());
        int bit_rate = pf->GetInteger(prefs::kVideoBitRate);
        std::string rate_type = pf->GetString(prefs::kVideoBitRateControl);
        int width = pf->GetInteger(prefs::kVideoOutputCX);
        int height = pf->GetInteger(prefs::kVideoOutputCY);
        int encode_quality = pf->GetInteger(prefs::kOutputStreamVideoQuality);
        std::string encoder = pf->GetString(prefs::kOutputStreamVideoEncoder);
        std::string resolution = base::StringPrintf("%dx%d", width, height);

        settings->set_ts(base::Time::Now().ToDoubleT());
        settings->set_bit_rate(bit_rate);
        settings->set_rate_type(rate_type);
        settings->set_fps(fps_s);
        settings->set_resolution(resolution);
        settings->set_encoder(
            base::UTF16ToUTF8(livehime::GetVideoEncoderFriendlyName(encoder)));
        settings->set_quality(
            base::UTF16ToUTF8(livehime::GetVideoQualityFriendlyName(encode_quality)));

        if (quality_ext_info_)
        {
            quality_ext_info_->SetString("rate_type", rate_type);
            quality_ext_info_->SetString("resolution", resolution);
            quality_ext_info_->SetInteger("bit_rate", bit_rate);
            quality_ext_info_->SetInteger("fps", fps);
            quality_ext_info_->SetString("codec", encoder);
        }

        quality_report_common_fields_.push_back("v_rate_type:" + rate_type);
        quality_report_common_fields_.push_back("v_rate:" + std::to_string(bit_rate));
        quality_report_common_fields_.push_back("v_fps:" + std::to_string(fps));
        quality_report_common_fields_.push_back("v_size:" + resolution);
        quality_report_common_fields_.push_back("v_codec:" + encoder);

        //更新编码器信息
        encode_info_.fps = fps;
        encode_info_.bitrate = bit_rate;
        encode_info_.rate_type = rate_type;
        encode_info_.width = width;
        encode_info_.height = height;
        encode_info_.encoder_quality = encode_quality;
        encode_info_.encoder_name = encoder;
        memset(is_warned_, false, sizeof(is_warned_));

        //摄像头源信息和音频设备信息
        std::string camera_info = "";
        std::string audio_input_device_info = "";
        std::string audio_output_device_info = "";

        //默认音频设备
        auto audio_input_source = obs_get_source_by_name(obs_proxy::kAuxAudio1);
        if (audio_input_source)
        {
            obs_data_t* source_setting = obs_source_get_settings(audio_input_source);
            if (source_setting)
            {
                audio_input_device_info += obs_data_get_string(source_setting, "audio_device_name");
                audio_input_device_info += ",";

                obs_data_release(source_setting);
            }
            obs_source_release(audio_input_source);
        }

        auto audio_output_source = obs_get_source_by_name(obs_proxy::kDesktopAudio1);
        if (audio_output_source)
        {
            obs_data_t* source_setting = obs_source_get_settings(audio_output_source);
            if (source_setting)
            {
                audio_output_device_info += obs_data_get_string(source_setting, "audio_device_name");
                audio_output_device_info += ",";

                obs_data_release(source_setting);
            }
            obs_source_release(audio_output_source);
        }

        auto obs_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
        if (obs_proxy)
        {
            obs_proxy::Scene* scene = obs_proxy->GetCurrentSceneCollection()->current_scene();
            if (scene)
            {
                for (auto item : scene->GetItems())
                {
                    //摄像头设备
                    if (item->type() == obs_proxy::SceneItemType::VideoCaptureDevice)
                    {
                        auto camera_source = obs_get_source_by_name(item->name().c_str());
                        if (camera_source)
                        {
                            obs_data_t* source_setting = obs_source_get_settings(camera_source);
                            if (source_setting)
                            {
                                std::string camera_name = obs_data_get_string(source_setting, "video_device_id");
                                size_t index = camera_name.find(":");
                                if (index != std::string::npos)
                                {
                                    camera_name = camera_name.substr(0, index);
                                }
                                camera_info += camera_name;
                                camera_info += ",";
                                obs_data_release(source_setting);
                            }

                            obs_source_release(camera_source);
                        }
                    }
                    //添加的音频输入设备
                    else if (item->type() == obs_proxy::SceneItemType::AudioInputCapture)
                    {
                        auto audio_input_source = obs_get_source_by_name(item->name().c_str());
                        if (audio_input_source)
                        {
                            obs_data_t* source_setting = obs_source_get_settings(audio_input_source);
                            if (source_setting)
                            {
                                audio_input_device_info += obs_data_get_string(source_setting, "audio_device_name");
                                audio_input_device_info += ",";

                                obs_data_release(source_setting);
                            }

                            obs_source_release(audio_input_source);
                        }
                    }
                    //添加的音频输出设备
                    else if (item->type() == obs_proxy::SceneItemType::AudioOutputCapture)
                    {
                        auto audio_output_source = obs_get_source_by_name(item->name().c_str());
                        if (audio_output_source)
                        {
                            obs_data_t* source_setting = obs_source_get_settings(audio_output_source);
                            if (source_setting)
                            {
                                audio_output_device_info += obs_data_get_string(source_setting, "audio_device_name");
                                audio_output_device_info += ",";

                                obs_data_release(source_setting);
                            }

                            obs_source_release(audio_output_source);
                        }
                    }
                }
            }
        }
        quality_report_common_fields_.push_back("camera_info:" + camera_info);
        quality_report_common_fields_.push_back("audio_device_input_info:" + audio_input_device_info);
        quality_report_common_fields_.push_back("audio_device_output_info:" + audio_output_device_info);
    }

    PrefService* prefs = GetBililiveProcess()->profile()->GetPrefs();
    if (prefs)
    {
        is_analysis_live_quality_ = prefs->GetBoolean(prefs::kAnalysisLiveQuality);
    }
}

void StreamingReportService::StopRecord()
{
    // 看看当前统计的丢帧率等指标是不是超限了，超限了就上报，没超不用报，
    // 但保存到本地，方便过后用户反馈时我们能拿到推流报告
    {
        base::AutoLock lock(pb_locker_);
        if (pb_writer_)
        {
            pb_writer_->mutable_runtime_info()->set_live_status(0);
            pb_writer_->mutable_runtime_info()->set_end_time(base::Time::Now().ToDoubleT());
        }

        quality_ext_info_.reset();
    }

    SaveStreamingReport(start_record_time_, true);
}

bool StreamingReportService::IsRecording()
{
    base::AutoLock lock(pb_locker_);
    return !!pb_writer_;
}

base::FilePath StreamingReportService::SaveStreamingReport(const base::Time& start_time, bool full, std::string* pbstr/* = nullptr*/)
{
    std::string str;
    {
        base::AutoLock lock(pb_locker_);
        if (!pb_writer_)
        {
            return {};
        }

        str = pb_writer_->SerializeAsString();
        pb_writer_.reset();
    }

    base::FilePath path;
    if (PathService::Get(bililive::DIR_USER_DATA, &path))
    {
        path = path.Append(std::to_wstring(GetBililiveProcess()->secret_core()->account_info().mid()));
        path = path.Append(bililive::kLiveReportDirname);

        base::ThreadRestrictions::ScopedAllowIO allow;
        file_util::CreateDirectory(path);

        base::string16 name = bililive::TimeFormatFriendlyDate(start_time);
        name.append(full ? L"-full" : L"-part");
        name.append(L".dat");

        path = path.Append(name);

        if (-1 != file_util::WriteFile(path, str.data(), str.length()))
        {
            LOG(INFO) << "live report file saved, path=" << path.AsUTF8Unsafe();

            if (pbstr)
            {
                *pbstr = std::move(str);
            }

            return path;
        }
        else
        {
            PLOG(WARNING) << "live report file write failed, path=" << path.AsUTF8Unsafe();
        }
    }

    return {};
}

void StreamingReportService::SetOutput(obs_output_t* stream_output)
{
    std::unique_lock<std::mutex> lg(output_mutex_);
    if (stream_output)
    {
        stream_output_ = obs_output_get_ref(stream_output);
        total_encode_loss_ = 0;
        total_render_loss_ = 0;
        total_net_loss_ = 0;
        total_net_need_send_ = 0;
        net_queue_.Clear();
        enc_queue_.Clear();
        rdr_queue_.Clear();
        cpu_queue_.Clear();
        gpu_queue_.Clear();
    }
}

void StreamingReportService::ReleaseOutput()
{
    std::unique_lock<std::mutex> lg(output_mutex_);
    if (stream_output_)
    {
        obs_output_release(stream_output_);
        stream_output_ = nullptr;
    }
}

void StreamingReportService::OnCpuSampling(StreamingReportSampleData& sampledata)
{
    if (cpu_monitor_)
    {
        double cpu_rate = cpu_monitor_->GetUsage();

        if (pb_writer_)
        {
            auto sample = pb_writer_->mutable_statistics()->mutable_cpu()->mutable_data()->add_samples();
            sample->set_timestamp(sampledata.ts);
            sample->set_data(cpu_rate);
        }

        sampledata.data = cpu_rate;
    }
}

void StreamingReportService::OnGpuSampling(StreamingReportSampleData& sampledata)
{
    std::unique_lock<std::mutex> lg(gpu_mutex_);
    if (gpu_monitor_)
    {
        double max_usage = 0;
        max_usage = gpu_monitor_->GetUsage();

        if (pb_writer_)
        {
            auto sample = pb_writer_->mutable_statistics()->mutable_gpu()->mutable_data()->add_samples();
            sample->set_timestamp(sampledata.ts);
            sample->set_data(max_usage);
        }

        sampledata.data = max_usage;
    }
}

void StreamingReportService::OnMemorySampling(StreamingReportSampleData& sampledata)
{
    MEMORYSTATUSEX memory;
    memory.dwLength = sizeof(memory);
    if (GlobalMemoryStatusEx(&memory))
    {
        if (pb_writer_)
        {
            auto sample = pb_writer_->mutable_statistics()->mutable_mem()->mutable_data()->add_samples();
            sample->set_timestamp(sampledata.ts);
            sample->set_data(memory.dwMemoryLoad);
        }

        sampledata.data = memory.dwMemoryLoad;
    }
}

void StreamingReportService::OnLivehimeCpuSampling(StreamingReportSampleData& sampledata)
{
    double cpu_rate = GetLivehimeCpuRate();
    if (pb_writer_)
    {
        auto sample = pb_writer_->mutable_statistics()->mutable_p_cpu()->mutable_data()->add_samples();
        sample->set_timestamp(sampledata.ts);
        sample->set_data(cpu_rate);
    }

    sampledata.data = cpu_rate;
}

void StreamingReportService::OnLivehimeMemorySampling(StreamingReportSampleData& sampledata)
{
    double cur_mem = 0;
    HANDLE handle = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(handle, &pmc, sizeof(pmc)))
    {
        cur_mem = pmc.WorkingSetSize/1024/1024;
    }
    if (pb_writer_)
    {
        auto sample = pb_writer_->mutable_statistics()->mutable_p_mem()->mutable_data()->add_samples();
        sample->set_timestamp(sampledata.ts);
        sample->set_data(cur_mem);
    }

    sampledata.data = cur_mem;

#ifdef  _WIN64
    return; //x64不需要内存提醒
#endif //  _WIN64

    //x86不需要内存提醒
    if (live_quality_report_info_.is_living) {
        bool is_max_occpyu = false;
        const double eps = 1e-6;

        if (warn_memory_ - 1.0f < eps) {
            return;
        }

        if (warn_memory_ - KMinMemoryWarn < eps) {
            warn_memory_ = KMinMemoryWarn;
        }

        if (cur_mem - warn_memory_ > eps) {
            if (cur_mem - (warn_memory_ + kADDWarnMemory) > eps) {
                is_max_occpyu = true;
            }
        }
        else {
            return;
        }

        bool is_nv_encode = true;
        if (encode_info_.encoder_name != "") {
            if (encode_info_.encoder_name == "x264") {
                is_nv_encode = false;
            }
        }
        else {
            return;
        }

        StreamingWarning stream_type = is_nv_encode ? StreamingWarning::WARNING_NV_MEMORY_OCCUPY :     StreamingWarning::WARNING_MEMORY_OCCUPY;

        if (stream_type == StreamingWarning::WARNING_NV_MEMORY_OCCUPY && is_max_occpyu) {
            stream_type = StreamingWarning::WARNING_NV_MEMORY_OCCUPY_MAX;
        }
        else if (stream_type == StreamingWarning::WARNING_MEMORY_OCCUPY && is_max_occpyu) {
            stream_type = StreamingWarning::WARNING_MEMORY_OCCUPY_MAX;
        }

        if (!is_warned_[stream_type]) {
            LOG(INFO) << "streaming warning:memory max " << cur_mem;

            is_warned_[stream_type] = true;

             BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                    base::Bind(&StreamingReportService::Warning, this,
                        stream_type));
        }
    }
}

void StreamingReportService::OnNetworkDropFrameSampling(StreamingReportSampleData& sampledata, const streaming_info& info)
{
    int net_loss = 0;
    float loss_rate = 0.0f;
    //网络丢帧
    if (info.total_frame && info.dropped_frame)
    {
        if (info.dropped_frame > total_net_loss_)
        {
            net_loss = info.dropped_frame - total_net_loss_;
        }
    }
    total_net_loss_ = info.dropped_frame;

    int net_bytes_loss = std::max<int>(0, info.dropped_frame_bytes - total_net_bytes_loss_);
    total_net_bytes_loss_ = info.dropped_frame_bytes;

    //计算丢帧率
    if (encode_info_.fps > 0)
    {
        loss_rate = net_loss / (float)(encode_info_.fps * kSampleRate);
        loss_rate = std::min(loss_rate, 1.0f);
    }

    if (pb_writer_)
    {
        auto sample = pb_writer_->mutable_statistics()->mutable_ndf()->mutable_data()->add_samples();
        sample->set_timestamp(sampledata.ts);
        sample->set_data(loss_rate);
    }

    sampledata.data = loss_rate;
    sampledata.raw_value[0] = net_loss;
    sampledata.raw_value[1] = net_bytes_loss;
}

void StreamingReportService::OnRenderDropFrameSampling(StreamingReportSampleData& sampledata, const streaming_info& info)
{
    int render_loss = 0;
    float loss_rate = 0.0f;
    //渲染丢帧
    if (info.drawn && info.lagged)
    {
        if (info.lagged > total_render_loss_)
        {
            render_loss = info.lagged - total_render_loss_;
        }
    }
    total_render_loss_ = info.lagged;
    //计算丢帧率
    if (encode_info_.fps > 0)
    {
        loss_rate = render_loss / (float)(encode_info_.fps * kSampleRate);
        loss_rate = std::min(loss_rate, 1.0f);
    }

    if (pb_writer_)
    {
        auto sample = pb_writer_->mutable_statistics()->mutable_rdf()->mutable_data()->add_samples();
        sample->set_timestamp(sampledata.ts);
        sample->set_data(loss_rate);
    }

    sampledata.data = loss_rate;
    sampledata.raw_value[0] = render_loss;
}

void StreamingReportService::OnEncodeDropFrameSampling(StreamingReportSampleData& sampledata, const streaming_info& info)
{
    int encode_loss = 0;
    float loss_rate = 0.0f;
    //编码丢帧
    if (info.skipped)
    {
        if (info.skipped > total_encode_loss_)
        {
            encode_loss = info.skipped - total_encode_loss_;
        }
    }

    //计算丢帧率
    if (encode_info_.fps > 0)
    {
        loss_rate = encode_loss / (float)(encode_info_.fps * kSampleRate);
        loss_rate = std::min(loss_rate, 1.0f);
    }

    total_encode_loss_ = info.skipped;
    if (pb_writer_)
    {
        auto sample = pb_writer_->mutable_statistics()->mutable_edf()->mutable_data()->add_samples();
        sample->set_timestamp(sampledata.ts);
        sample->set_data(loss_rate);
    }

    sampledata.data = loss_rate;
    sampledata.raw_value[0] = encode_loss;
}

void StreamingReportService::InformSampling(const StreamingReportInfo& info)
{
    FOR_EACH_OBSERVER(StreamingReportObserver, observer_list_,
        OnStreamingReportSampling(info));
}

void StreamingReportService::Warning(StreamingWarning warning)
{
    if (warning >= (sizeof(is_warned_) / sizeof(bool)))
        return;
    FOR_EACH_OBSERVER(StreamingReportObserver, observer_list_,
        OnStreamingWarning(warning));
    is_warned_[warning] = true;
}

void StreamingReportService::FillQualityReportCommonFields(std::vector<std::string>& vec_outcome)
{
    vec_outcome.assign(quality_report_common_fields_.begin(), quality_report_common_fields_.end());

    vec_outcome.push_back("start_stream_ts:" + std::to_string((int64_t)start_stream_time_.ToDoubleT()));  // 本场推流的开始时间
    vec_outcome.push_back("streamkey:" + stream_guid_);  // 本次推流标识
    vec_outcome.push_back("host_name:" + startlive_details_.host_name);  // 当前推流地址
    vec_outcome.push_back("addr:" + startlive_details_.addr);  // 当前推流地址
    vec_outcome.push_back(base::StringPrintf("already_changed:%d", startlive_details_.quality_abnormal_streaming ? 1 : 0));  // 当前是否是已然切过地址的推流
}

void StreamingReportService::RequestStreamQualityFromServer()
{
    // 底层（当前只支持OutputController）切实正在推流才去请求流质量
    if (!bililive::OutputController::GetInstance()->IsStreaming())
    {
        return;
    }
}


void StreamingReportService::ReportLocalStreamQualityToServer()
{
    // 底层（当前只支持OutputController）切实正在推流才上报本地流质量
    if (!bililive::OutputController::GetInstance()->IsStreaming())
    {
        return;
    }

    base::AutoLock lock(pb_locker_);
    if (quality_ext_info_)
    {
        quality_ext_info_->SetInteger64("now_ts", (int64_t)base::Time::Now().ToDoubleT());

        // 把几种数据都进行一下最大值和平均值计算，只取最近n秒的
        int remote_report_duration = AppFunctionController::GetInstance()->streaming_qmcs_ctrl().remote_report_duration;
        std::list<StreamingReportInfo> sris;
        std::list<StreamingReportInfo>::reverse_iterator iter = last_cache_samples_.rbegin();
        std::advance(iter, std::min(remote_report_duration, (int)last_cache_samples_.size()));
        sris.assign(iter.base(), last_cache_samples_.end());

        // 最大值、均值、求和公共函数
        auto sum_value_fn = [&](StreamingSampleType type, int raw_value_index = 0) -> int64_t {
            int64_t sum_value = std::accumulate(sris.begin(), sris.end(), 0ll,
                [=](int64_t dv, const StreamingReportInfo& info) -> int64_t {
                return dv + info.data[type].raw_value[raw_value_index];
            });

            return sum_value;
        };
        auto max_value_fn = [&](StreamingSampleType type) -> int {

            auto max_item = std::max_element(sris.begin(), sris.end(), [=](const StreamingReportInfo& lft, const StreamingReportInfo& rht)->bool {
                return lft.data[type].data < rht.data[type].data;
            });
            int max_value = max_item->data[type].data;

            return max_value;
        };
        auto avg_value_fn = [&](StreamingSampleType type) -> int {

            int64_t accumulate_count = std::accumulate(sris.begin(), sris.end(), 0ll,
                [=](int64_t dv, const StreamingReportInfo& info) -> int64_t {
                return dv + info.data[type].data;
            });
            int avg_value = 0;
            if (!sris.empty())
            {
                avg_value = accumulate_count / sris.size();
            }

            return avg_value;
        };

        auto avg_rate_value_fn = [&](StreamingSampleType type) -> int {

            int64_t accumulate_count = std::accumulate(sris.begin(), sris.end(), 0ll,
                [=](int64_t dv, const StreamingReportInfo& info) -> int64_t {
                    return dv + info.data[type].data*100;
                });
            int avg_value = 0;
            if (!sris.empty())
            {
                avg_value = accumulate_count / sris.size();
            }
            if (avg_value > 100)
                avg_value = 100;

            return avg_value;
        };

        // 网络丢帧总数、字节总数
        int64_t ndf_dfc = sum_value_fn(NDF);
        int64_t ndf_dbc = sum_value_fn(NDF, 1);
        quality_ext_info_->SetInteger64("ndf.drop_frames", ndf_dfc);
        quality_ext_info_->SetInteger64("ndf.drop_bytes", ndf_dbc);

        // 渲染丢帧总数
        int64_t rdf_dfc = sum_value_fn(RDF);
        quality_ext_info_->SetInteger64("rdf.drop_frames", rdf_dfc);

        // 编码丢帧总数
        int64_t edf_dfc = sum_value_fn(EDF);
        quality_ext_info_->SetInteger64("edf.drop_frames", edf_dfc);

        // 系统GPU最大使用率、平均使用率
        int max_gpu = max_value_fn(GPU);
        int avg_gpu = avg_value_fn(GPU);
        quality_ext_info_->SetInteger("sys_gpu_usage.max_value", max_gpu);
        quality_ext_info_->SetInteger("sys_gpu_usage.avg_value", avg_gpu);

        // 系统CPU最大使用率、平均使用率
        int max_cpu = max_value_fn(CPU);
        int avg_cpu = avg_value_fn(CPU);
        quality_ext_info_->SetInteger("sys_cpu_usage.max_value", max_cpu);
        quality_ext_info_->SetInteger("sys_cpu_usage.avg_value", avg_cpu);

        // 系统内存最大使用率、平均使用率
        int max_mem = max_value_fn(MEM);
        int avg_mem = avg_value_fn(MEM);
        quality_ext_info_->SetInteger("sys_memory_usage.max_value", max_mem);
        quality_ext_info_->SetInteger("sys_memory_usage.avg_value", avg_mem);

        // 程序CPU最大使用率、平均使用率
        int max_pcpu = max_value_fn(P_CPU);
        int avg_pcpu = avg_value_fn(P_CPU);
        quality_ext_info_->SetInteger("app_cpu_usage.max_value", max_pcpu);
        quality_ext_info_->SetInteger("app_cpu_usage.avg_value", avg_pcpu);

        // 程序内存最大使用量、平均使用量，MB
        int max_pmem = max_value_fn(P_MEM);
        int avg_pmem = avg_value_fn(P_MEM);
        quality_ext_info_->SetInteger("app_memory_usage.max_value", max_pmem);
        quality_ext_info_->SetInteger("app_memory_usage.avg_value", avg_pmem);

        std::string ext_info;
        base::JSONWriter::Write(quality_ext_info_.get(), &ext_info);

        // 本地质量埋点
        std::vector<std::string> vec_outcome;
        FillQualityReportCommonFields(vec_outcome);

        vec_outcome.push_back(base::StringPrintf("max_cpu:%d", max_cpu));
        vec_outcome.push_back(base::StringPrintf("avg_cpu:%d", avg_cpu));
        vec_outcome.push_back(base::StringPrintf("max_mem:%d", max_mem));
        vec_outcome.push_back(base::StringPrintf("avg_mem:%d", avg_mem));
        vec_outcome.push_back(base::StringPrintf("max_gpu:%d", max_gpu));
        vec_outcome.push_back(base::StringPrintf("avg_gpu:%d", avg_gpu));
        vec_outcome.push_back(base::StringPrintf("max_pcpu:%d", max_pcpu));
        vec_outcome.push_back(base::StringPrintf("avg_pcpu:%d", avg_pcpu));
        vec_outcome.push_back(base::StringPrintf("max_pmem:%d", max_pmem));
        vec_outcome.push_back(base::StringPrintf("avg_pmem:%d", avg_pmem));

        vec_outcome.push_back(base::StringPrintf("ndf_dfc:%d", ndf_dfc));
        vec_outcome.push_back(base::StringPrintf("edf_dfc:%d", edf_dfc));
        vec_outcome.push_back(base::StringPrintf("rdf_dfc:%d", rdf_dfc));

        if (!startlive_details_.addr.empty())
        {
            std::string addr = startlive_details_.addr;
            if (addr.find("rtmpsrt:") != std::string::npos)
            {
                //srt推流
                vec_outcome.push_back(base::StringPrintf("streaming_type:1"));
            }
            else if (addr.find("rtmp:") != std::string::npos)
            {
                //rtmp推流
                vec_outcome.push_back(base::StringPrintf("streaming_type:2"));
            }
        }
        //网络需要发送的总frames
        int64_t net_need_total_frames = sum_value_fn(NET_NEED_TOTAL_FRAMES);
        vec_outcome.push_back(base::StringPrintf("net_need_total_frames:%" PRId64, net_need_total_frames));
        //SRT发送总包数
        int64_t srt_pkts = sum_value_fn(SRT_SEND_PKTS);
        vec_outcome.push_back(base::StringPrintf("srt_send_pkts:%" PRId64, srt_pkts));
        //SRT丢包总数
        int64_t srt_lost_pkts = sum_value_fn(SRT_LOST_PKTS);
        vec_outcome.push_back(base::StringPrintf("srt_lost_pkts:%" PRId64, srt_lost_pkts));
        //SRT丢弃包数
        int64_t srt_drop_pkts = sum_value_fn(SRT_DROP_PKTS);
        vec_outcome.push_back(base::StringPrintf("srt_drop_pkts:%" PRId64, srt_drop_pkts));
        //SRT重传包数
        int64_t srt_rettans_pkts = sum_value_fn(SRT_RETTRANS_PKTS);
        vec_outcome.push_back(base::StringPrintf("srt_rettans_pkts:%" PRId64, srt_rettans_pkts));
        //SRT RTT时间MS
        int64_t srt_rtt_time = avg_value_fn(SRT_RTT_MS);
        vec_outcome.push_back(base::StringPrintf("srt_rtt_time:%" PRId64, srt_rtt_time));
        //SRT 发送码率
        int64_t srt_send_rate = avg_value_fn(SRT_SEND_RATE_KBPS);
        vec_outcome.push_back(base::StringPrintf("srt_send_rate:%" PRId64, srt_send_rate));

        //网络丢帧率
        int ndf_dfc_rate = avg_rate_value_fn(NDF);
        vec_outcome.push_back(base::StringPrintf("ndf_dfc_rate:%d", ndf_dfc_rate));
        //编码丢帧率
        int edf_dfc_rate = avg_rate_value_fn(EDF);
        vec_outcome.push_back(base::StringPrintf("edf_dfc_rate:%d", edf_dfc_rate));
        //渲染丢帧率
        int rdf_dfc_rate = avg_rate_value_fn(RDF);
        vec_outcome.push_back(base::StringPrintf("rdf_dfc_rate:%d", rdf_dfc_rate));
        //显卡丢失次数
        vec_outcome.push_back(base::StringPrintf("device_loss_count:%d", obs_get_device_loss_count()));

        std::string outcome = JoinString(vec_outcome, ";");
        livehime::TechnologyEventReport(secret::LivehimeBehaviorEvent::LivehimeLocalStreamQuality, outcome);
    }
    else
    {
        NOTREACHED();
    }
}

// LivehimeLiveRoomObserver
void StreamingReportService::OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details)
{
    startlive_details_ = details;

    // 不管是重开播还是重推流，都会触发到这里，这里不在乎这个细节，只要流重新开始了，就对本次推流进行标识，
    // 本场直播标识能用服务端的livekey，但是细致到每次推流就只能由本地自己进行标识。
    // 这个标识能方便埋点数据分析时能针对推流场次这个粒度进行统计
    stream_guid_ = base::GenerateGUID();
    start_stream_time_ = base::Time::Now();

    StartRecord();

    base::AutoLock lock(pb_locker_);
    if (pb_writer_)
    {
        pb_writer_->mutable_runtime_info()->set_cdn_host(details.host_name);
        pb_writer_->mutable_runtime_info()->set_addr(details.addr);
        pb_writer_->mutable_runtime_info()->set_code(details.code);
    }

    //开播性能质量体验建设上报
    {
        //直播中更改分辨率也会触发OnLiveRoomStreamingStarted，这里更新resolution_height
		int resolution = 1080;
		auto pf = GetBililiveProcess()->profile()->GetPrefs();
		if (pf)
		{
			resolution = pf->GetInteger(prefs::kVideoOutputCY);//需要放在主线程中调用
			if (resolution >= 1080)
			{
				resolution = 1080;
			}
			else if (resolution >= 720)
			{
				resolution = 720;
			}
			else
			{
				resolution = 540;
			}
		}
		live_quality_report_info_.resolution_height = resolution;

		if (!live_quality_report_info_.is_living)//OnLiveRoomStreamingStarted在断流重连后也会调用，这里is_living判定是不是同一场直播
		{
			live_quality_report_info_.is_living = true;
			live_quality_report_info_.index_report = 0;
			live_quality_report_info_.stream_guid = stream_guid_;
			live_quality_report_info_.time_count = live_quality_report_info_.time_interval_second;//开始推流时上报一次
		}
    }
}

void StreamingReportService::OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    // 如果仅仅只是重试推流，并没有关播的不要停止记录，
    // 确保我们的文件记录的是一次完整的开播流程（一次开播可能多次断流/推流）的质量
    if (!is_restreaming)
    {
        StopRecord();

        live_quality_report_info_.is_living = false;
        ReportLiveQualityInfo(last_cache_samples_.back(), true);
    }
}

// AppFunctionCtrlObserver
void StreamingReportService::OnUpdateAppCtrlInfo()
{
    // 开关打开才尝试初始化GPU相关监控模块
    bool switch_on = true;
    if (AppFunctionController::GetInstance())
    {
        switch_on = AppFunctionController::GetInstance()->streaming_qmcs_ctrl().switch_on;
    }

    {
        std::unique_lock<std::mutex> lg(gpu_mutex_);
        if (switch_on)
        {
            gpu_monitor_ = std::make_unique<GpuMonitor>();
            if (!gpu_monitor_->Init())
            {
                gpu_monitor_.reset();
            }
        }
        else
        {
            gpu_monitor_.reset();
        }
    }

    {
        base::AutoLock lock(pb_locker_);
        if (!switch_on)
        {
            if (pb_writer_)
            {
                pb_writer_.reset();
            }
        }

        if (AppFunctionController::GetInstance())
        {
            // APM有效才进行数据采样和上报
            auto apm_ctrl = AppFunctionController::GetInstance()->apm_ctrl();
            if (apm_ctrl.valid)
            {
                apm_statistic_.reset(new ApmStatisticAnalysisService);
            }
            else
            {
                apm_statistic_.reset();
            }
        }
    }
}

bool StreamingReportService::IsGpuValid() const
{
    return !!gpu_monitor_;
}

void StreamingReportService::AnalysisLiveQuality()
{
    const auto& qmcs = AppFunctionController::GetInstance()->streaming_qmcs_ctrl();

	uint32_t ret = FINE;

	//网络平均丢帧大于40%
	if (net_queue_.GetAverageQuality() > qmcs.local_net_lose_rate_threshold)
	{
		ret |= NET_LOSS;
        live_quality_report_info_.push_lag_count++;
	}
	//CPU占用平均高于80
    float cpu_ret = cpu_queue_.GetAverageQuality();
	if (cpu_ret > 80)
	{
		ret |= CPU_HIGH;
	}
	if (cpu_ret > 90)
	{
        live_quality_report_info_.cpu_lag_count++;
	}
	//编码器丢帧大于40%
	if (enc_queue_.GetAverageQuality() > qmcs.local_edf_lose_rate_threshold)
	{
		ret |= ENC_LOSS;
	}
	//渲染丢帧大于40%
	if (rdr_queue_.GetAverageQuality() > qmcs.local_rdf_lose_rate_threshold)
	{
		ret |= RENDER_LOSS;
        live_quality_report_info_.render_lag_count++;
	}
	//GPU占用高于90
	if (gpu_queue_.GetAverageQuality() > 90)
	{
		ret |= GPU_HIGH;
        live_quality_report_info_.gpu_lag_count++;
	}

	last_quality_result_ = ret;

    // 开关没开不分析质量
    if (!qmcs.switch_on || !is_analysis_live_quality_)
    {
        return;
    }
    //CPU高且用的软件编码，提醒用户换硬件编码
    if ((ret & CPU_HIGH) && encode_info_.encoder_name == "x264")
    {
        if (!is_warned_[WARNING_USE_HARD_ENCODE])
        {
            LOG(INFO) << "streaming warning:CPU High and use x264";
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(&StreamingReportService::Warning, this,
                    WARNING_USE_HARD_ENCODE));
        }
    }

    //网络丢帧 提醒用户降低码率或者检测网络状况
    if (ret & NET_LOSS)
    {
        if (!is_warned_[WARNING_NET_LOSS])
        {
            LOG(INFO) << "streaming warning:net loss";
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(&StreamingReportService::Warning, this,
                    WARNING_NET_LOSS));
        }
    }

    //编码器丢帧，且是硬件编码，提醒用户降低游戏设置
    if ((ret & ENC_LOSS) && encode_info_.encoder_name != "x264")
    {
        if (!is_warned_[WARNING_CHANGE_GAME_SETTING])
        {
            LOG(INFO) << "streaming warning:encode loss and use hardware encode";
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(&StreamingReportService::Warning, this,
                    WARNING_CHANGE_GAME_SETTING));
        }
    }

    //编码器丢帧，但是是用户选了高FPS,高画质，高分辨率，建议用户降低直播姬编码器设置
    if ((ret & ENC_LOSS) && encode_info_.width == 1920 && encode_info_.height == 1080 &&
        encode_info_.encoder_quality == 3 && encode_info_.fps == 60)
    {
        if (!is_warned_[WARNING_CHANGE_LIVEHIME_SETTING])
        {
            LOG(INFO) << "streaming warning:livehime setting high";
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(&StreamingReportService::Warning, this,
                    WARNING_CHANGE_LIVEHIME_SETTING));
        }
    }
}

void StreamingReportService::SetAnalysisLiveQuality(bool enable)
{
    is_analysis_live_quality_ = enable;
    if (!is_analysis_live_quality_ && !live_quality_report_info_.is_report)
    {
        total_encode_loss_ = 0;
        total_render_loss_ = 0;
        total_net_loss_ = 0;
        total_net_need_send_ = 0;
        net_queue_.Clear();
        enc_queue_.Clear();
        rdr_queue_.Clear();
        cpu_queue_.Clear();
        gpu_queue_.Clear();
    }
}

std::list<StreamingReportInfo> StreamingReportService::LastCacheSamples()
{
    base::AutoLock lock(pb_locker_);
    return last_cache_samples_;
}

uint64 StreamingReportService::GetTotalRenderLoss()
{
    return total_render_loss_;
}

uint64 StreamingReportService::GetTotalEncodeLoss()
{
    return total_encode_loss_;
}

uint64 StreamingReportService::GetTotalNetLoss()
{
    return total_net_loss_;
}

void StreamingReportService::OnAllAppKVCtrlInfoCompleted()
{
    const std::string & report_paras = AppFunctionController::GetInstance()->get_live_quality_report_paras();
    //std::string report_paras = "open:1#interval:20";

    std::vector<std::string> items;
    base::SplitString(report_paras,'#', &items);
    for (auto &item : items)
    {
		std::vector<std::string> values;
		base::SplitString(item, ':', &values);
        if (values.size() == 2 && !values[0].empty() && !values[1].empty())
        {
            if (values[0] == "open")
            {
                live_quality_report_info_.is_report = values[1] == "1";
            }else if (values[0] == "interval")
            {
                base::StringToUint(values[1], &(live_quality_report_info_.time_interval_second));
            }
        }
    }

    warn_memory_ = AppFunctionController::GetInstance()->get_memory_occupy();

    report_time_ = AppFunctionController::GetInstance()->get_apptime_kv_control().report_time;
}

void StreamingReportService::ReportLiveQualityInfo(const StreamingReportInfo& info, bool force/* = false*/)
{
    if (!live_quality_report_info_.is_report)
    {
        return;//开关关闭返回
    }
    if (!force)
    {
		if (!live_quality_report_info_.is_living)
		{
			return;//没有直播时不上报
		}
        live_quality_report_info_.time_count += kSampleRate;
		if (live_quality_report_info_.time_count < live_quality_report_info_.time_interval_second)
		{
			return;//时间未到不上报
        }
        else
        {
            live_quality_report_info_.time_count = 0;
        }
    }
	base::StringPairs event_ext;
    //开播信息
	event_ext.push_back(std::pair<std::string, std::string>("app_key", "pc_link"));

    LiveQualityReportInfo::StreamType stream_type = LiveQualityReportInfo::StreamType::Voice;
	auto obs_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
    if (obs_proxy)
    {
        obs_proxy::Scene* scene = obs_proxy->GetCurrentSceneCollection()->current_scene();
        if (scene)
        {
            bool have_vtuber = false;
            bool have_game = false;
            bool have_video = false;
            for (auto item : scene->GetItems())
            {
                if (item)
                {
					if (item->type() == obs_proxy::SceneItemType::Scene)
					{
						have_vtuber = true;
					}
					else if (item->type() == obs_proxy::SceneItemType::GameCapture)
					{
						have_game = true;
					}
					else if (item->type() == obs_proxy::SceneItemType::VideoCaptureDevice)
					{
						have_video = true;
					}
                }
            }
            if (have_vtuber)
            {
                stream_type = LiveQualityReportInfo::StreamType::Vtuber;
            }else if (have_game)
            {
                stream_type = LiveQualityReportInfo::StreamType::Game;
			}
			else if (have_video)
			{
                stream_type = LiveQualityReportInfo::StreamType::Video;
			}
        }
    }
    event_ext.push_back(std::pair<std::string, std::string>("stream_type", base::IntToString((int)stream_type)));
    event_ext.push_back(std::pair<std::string, std::string>("index_report", base::IntToString(live_quality_report_info_.index_report)));

    const std::string& sub_session_key = GetBililiveProcess()->secret_core()->anchor_info().sub_session_key();
    if (!sub_session_key.empty())
    {
        live_quality_report_info_.session_id = sub_session_key;
    }
    //这里不用sub_session_key，使用stream_guid
    event_ext.push_back(std::pair<std::string, std::string>("session_id", live_quality_report_info_.stream_guid));
    //CPU
    event_ext.push_back(std::pair<std::string, std::string>("cpu_process_rate", base::StringPrintf("%.3f", info.data[StreamingSampleType::P_CPU].data/100.0f)));
    event_ext.push_back(std::pair<std::string, std::string>("cpu_total_rate", base::StringPrintf("%.3f", info.data[StreamingSampleType::CPU].data/100.0f)));
    event_ext.push_back(std::pair<std::string, std::string>("cpu_process_threads", base::IntToString(GetAppThreads())));
    event_ext.push_back(std::pair<std::string, std::string>("cpu_size", base::IntToString(GetNumberOfProcessors())));
    //卡顿
    event_ext.push_back(std::pair<std::string, std::string>("caton_gpu_fps", base::IntToString(live_quality_report_info_.gpu_lag_count)));
    event_ext.push_back(std::pair<std::string, std::string>("caton_main", base::IntToString(live_quality_report_info_.cpu_lag_count)));
    event_ext.push_back(std::pair<std::string, std::string>("caton_frame", base::IntToString(live_quality_report_info_.render_lag_count)));
    event_ext.push_back(std::pair<std::string, std::string>("caton_push", base::IntToString(live_quality_report_info_.push_lag_count)));
    //温度
    event_ext.push_back(std::pair<std::string, std::string>("temperature", "45"));
    //内存
    if (live_quality_report_info_.memory_total_size <= 0)
    {
		MEMORYSTATUSEX memory;
		memory.dwLength = sizeof(memory);
		if (GlobalMemoryStatusEx(&memory))
		{
            DWORDLONG gb = memory.ullTotalPhys / 1024 / 1024 / 1024;
            live_quality_report_info_.memory_total_size = gb;
			if (live_quality_report_info_.memory_total_size % 2 != 0)
			{
                live_quality_report_info_.memory_total_size++;
			}
		}
    }
    double app_mem = info.data[StreamingSampleType::P_MEM].data;
    double app_mem_rate = 0.0f;
    double sys_mem_rate = info.data[StreamingSampleType::MEM].data;
    if (live_quality_report_info_.memory_total_size > 0)
    {
        app_mem_rate = app_mem / (double)(live_quality_report_info_.memory_total_size*1024);
    }
    event_ext.push_back(std::pair<std::string, std::string>("mem_process_value", base::StringPrintf("%.0f", app_mem)));
    event_ext.push_back(std::pair<std::string, std::string>("mem_process_rate", base::StringPrintf("%.3f", app_mem_rate)));
    event_ext.push_back(std::pair<std::string, std::string>("mem_total_rate", base::StringPrintf("%.3f", sys_mem_rate/100.0f)));
    event_ext.push_back(std::pair<std::string, std::string>("mem_rom_size", base::IntToString(live_quality_report_info_.memory_total_size)));
    //推流质量
    event_ext.push_back(std::pair<std::string, std::string>("drop_frame_rate", base::StringPrintf("%.3f", info.data[StreamingSampleType::RDF])));
    event_ext.push_back(std::pair<std::string, std::string>("drop_push_rate", base::StringPrintf("%.3f", info.data[StreamingSampleType::NDF])));
    double push_up_stream = info.data[StreamingSampleType::SRT_SEND_RATE_KBPS].data;
    push_up_stream = push_up_stream / 8.0f;//转成KB
    event_ext.push_back(std::pair<std::string, std::string>("push_up_stream", base::StringPrintf("%.0f", push_up_stream)));
    event_ext.push_back(std::pair<std::string, std::string>("push_quality_type", base::IntToString(live_quality_report_info_.resolution_height)));
    event_ext.push_back(std::pair<std::string, std::string>("push_break_num", base::IntToString(live_quality_report_info_.send_rate_zero_count)));
	livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::LiveQualityInfoReport, event_ext);

    live_quality_report_info_.index_report++;
    live_quality_report_info_.gpu_lag_count = 0;
    live_quality_report_info_.cpu_lag_count = 0;
    live_quality_report_info_.render_lag_count = 0;
    live_quality_report_info_.push_lag_count = 0;
    live_quality_report_info_.send_rate_zero_count = 0;
}


void StreamingReportService::ReportLiveAppUseTime(int64 seconds) {
    base::StringPairs event_ext;

    event_ext.push_back(std::pair<std::string, std::string>("duration", base::Int64ToString(seconds)));

    if (GetBililiveProcess()->bililive_obs() && GetBililiveProcess()->bililive_obs()->obs_view()) {
        if (GetBililiveProcess()->bililive_obs()->main_window_is_minimized()) {
            event_ext.push_back(std::pair < std::string, std::string>("type", "background"));
        }
        else {
            event_ext.push_back(std::pair < std::string, std::string>("type", "front"));
        }
    }
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::AppUseTimeTracker, event_ext);
}

//由外部调用
void StreamingReportService::ReportLiveAppUseTime() {

    if (apptime_count_ <= 0 || apptime_count_ == report_time_)
        return;

    base::StringPairs event_ext;
    event_ext.push_back(std::pair<std::string, std::string>("duration", base::Int64ToString(apptime_count_)));
    event_ext.push_back(std::pair < std::string, std::string>("type", "user_quit"));

    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::AppUseTimeTracker, event_ext);
}