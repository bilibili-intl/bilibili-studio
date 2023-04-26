#include "bililive/bililive/bililive_main_parts_impl.h"

#include <dxgi.h>

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/prefs/pref_service.h"
#include "base/run_loop.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/sys_info.h"
#include "base/win/windows_version.h"

#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_win.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_bundle_win.h"
#include "ui/gfx/platform_font_win.h"
#include "ui/views/focus/accelerator_handler.h"

#include "bililive/bililive/bililive_installation_rejecter.h"
#include "bililive/bililive/first_run/first_run.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/ui/examples/examples_runner.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_result_codes.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/common/pref_constants.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/log_ext/log_constants.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "grit/app_locale_settings.h"

#include "obs/obs-studio/libobs/util/windows/win-version.h"
#include "obs/obs-studio/plugins/win-dshow/libdshowcapture/source/ComPtr.hpp"
#include "bililive/common/bililive_logging.h"


namespace {

void AdjustUIFont(LOGFONT* logfont)
{
    l10n_util::AdjustUIFont(logfont);
}

int GetMinimumFontSize()
{
    int min_font_size;
    base::StringToInt(l10n_util::GetStringUTF16(IDS_MINIMUM_UI_FONT_SIZE),
                      &min_font_size);
    return min_font_size;
}

void LogSystemInformation()
{
    const size_t kPresumedSize = 256;
    const char kFenceBar[] = "------------SYS INFO------------";

    std::string diagnose_data;
    diagnose_data.reserve(kPresumedSize);
    diagnose_data.append("\n").append(kFenceBar).append("\n");

    const char item_app_ver[] = "Application Version: ";
    auto app_version = base::WideToUTF8(BililiveContext::Current()->GetExecutableVersion());
    diagnose_data.append(item_app_ver).append(app_version).append("\n");

    const char item_win_ver[] = "Windows Version: ";
    auto os_version = base::SysInfo::OperatingSystemVersion();
    auto os_arch = base::SysInfo::OperatingSystemArchitecture();
    diagnose_data.append(item_win_ver).append(os_version).append(" ").append(os_arch).append("\n");

    const char item_exe_path[] = "EXE Path: ";
    base::FilePath exe_path;
    PathService::Get(base::FILE_EXE, &exe_path);
    diagnose_data.append(item_exe_path).append(base::WideToUTF8(exe_path.value())).append("\n");

    diagnose_data.append(kFenceBar);

    LOG(INFO) << diagnose_data;

    LOG(INFO) << app_log::kLogAppLaunch <<
        base::StringPrintf("appver:%s; osver:%s %s; cpu:%s; memory:%d MB", app_version.c_str(), os_version.c_str(), os_arch.c_str(),
            base::SysInfo::CPUModelName().c_str(), base::SysInfo::AmountOfPhysicalMemoryMB());
}

void EnumD3DAdapters(std::vector<std::string>& graphic_adapters)
{
    ComPtr<IDXGIFactory1> factory;
    ComPtr<IDXGIAdapter1> adapter;
    HRESULT hr;
    UINT i = 0;

    auto fnGetWinVer = []()->uint32_t {
        struct win_version_info ver;
        get_win_ver(&ver);

        return (ver.major << 8) | ver.minor;
    };

    static const IID dxgiFactory2 =
    { 0x50c83a1c, 0xe072, 0x4c48, {0x87, 0xb0, 0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0} };

    uint32_t win_ver_num = fnGetWinVer();
    IID factoryIID = (win_ver_num >= 0x602) ? dxgiFactory2 : __uuidof(IDXGIFactory1);

    hr = CreateDXGIFactory1(factoryIID, (void**)factory.Assign());
    if (FAILED(hr))
    {
        return;
    }

    while (factory->EnumAdapters1(i++, adapter.Assign()) == S_OK)
    {
        DXGI_ADAPTER_DESC desc;
        char name[512] = "";

        hr = adapter->GetDesc(&desc);
        if (FAILED(hr))
            continue;

        /* ignore Microsoft's 'basic' renderer' */
        if (desc.VendorId == 0x1414 && desc.DeviceId == 0x8c)
            continue;

        graphic_adapters.push_back(base::WideToUTF8(desc.Description));
        /*os_wcs_to_utf8(desc.Description, 0, name, sizeof(name));
        blog(LOG_INFO, "\tAdapter %u: %s", i, name);
        blog(LOG_INFO, "\t  Dedicated VRAM: %u",
             desc.DedicatedVideoMemory);
        blog(LOG_INFO, "\t  Shared VRAM:    %u",
             desc.SharedSystemMemory);*/
    }
}

void RequestWindowsInfo(bool is_Polaris)
{
    std::string event_msg;
    auto os_version = base::SysInfo::OperatingSystemVersion();
    auto os_arch = base::SysInfo::OperatingSystemArchitecture();
    auto build_no = base::win::OSInfo::GetInstance()->version_number().build;
    auto cpu_name = base::SysInfo::CPUModelName();
    auto cpu_count = base::SysInfo::NumberOfProcessors();

    event_msg.append("windows:").append(os_version).append(";");
    event_msg.append("architecture:").append(os_arch).append(";");
    event_msg.append("build:").append(base::IntToString(build_no)).append(";");

    // physics device
    event_msg.append("cpu:").append(cpu_name).append(";");
    event_msg.append("cpu_count:").append(base::IntToString(cpu_count)).append(";");
    // graphic adapters
    std::string adapters;
    std::vector<std::string> graphic_adapters;
    EnumD3DAdapters(graphic_adapters);
    if (!graphic_adapters.empty())
    {
        for (auto& adapter_name : graphic_adapters)
        {
            adapters.append(adapter_name).append(",");
        }
        adapters.pop_back();
    }
    event_msg.append("gpu:").append(adapters).append(";");

    if (is_Polaris) {
        event_msg.append("ram_capacity:").append(base::IntToString(base::SysInfo::AmountOfPhysicalMemoryMB())).append(";");

        event_msg.append("first_install:").append(base::IntToString(GetBililiveProcess()->first_run())).append(";");

        livehime::PolarisEventReport(
            secret::LivehimePolarisBehaviorEvent::StartUpShow, event_msg);
    }
    else {
        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeComputerSystem,
            secret_core->account_info().mid(), event_msg).Call();
    }
}

}   // namespace

BililiveMainPartsImpl::BililiveMainPartsImpl()
    : parsed_command_line_(*CommandLine::ForCurrentProcess()),
      result_code_(bililive::ResultCodeNormalExit),
      profile_(nullptr),
      run_message_loop_(true),
      bililive_started_(false),
      local_state_(nullptr)
{}

BililiveMainPartsImpl::~BililiveMainPartsImpl()
{
    for (auto it = bililive_extra_parts_.rbegin(); it != bililive_extra_parts_.rend(); ++it)
    {
        delete *it;
    }

    bililive_extra_parts_.clear();
}

void BililiveMainPartsImpl::PreEarlyInitialization()
{
    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PreEarlyInitialization();
    }
}

void BililiveMainPartsImpl::PostEarlyInitialization()
{
    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PostEarlyInitialization();
    }
}

void BililiveMainPartsImpl::ToolkitInitialized()
{
    gfx::PlatformFontWin::adjust_font_callback = &AdjustUIFont;
    gfx::PlatformFontWin::get_minimum_font_size_callback = &GetMinimumFontSize;

    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->ToolkitInitialized();
    }
}

void BililiveMainPartsImpl::PreMainMessageLoopStart()
{
    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PreMainMessageLoopStart();
    }
}

void BililiveMainPartsImpl::PostMainMessageLoopStart()
{
    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PostMainMessageLoopStart();
    }
}

int BililiveMainPartsImpl::PreCreateThreads()
{
    result_code_ = PreCreateThreadsImpl();

    if (result_code_ == bililive::ResultCodeNormalExit)
    {
        for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
        {
            bililive_extra_parts_[i]->PreCreateThreads();
        }
    }

    return result_code_;
}

int BililiveMainPartsImpl::PreCreateThreadsImpl()
{
    run_message_loop_ = false;

    CHECK(PathService::Get(bililive::DIR_USER_DATA, &user_data_dir_));

    ui::SetResourcesDataDLL(GetModuleHandleW(bililive::kBililiveResourcesDll));

    auto locale_loaded = ResourceBundle::InitSharedInstanceWithLocale(prefs::kLocaleEnUS, nullptr);
    DCHECK_EQ(locale_loaded, prefs::kLocaleEnUS);

    bililive_process_.reset(new BililiveProcessImpl(user_data_dir_, parsed_command_line()));

    bililive_process_->SetApplicationLocale(locale_loaded);
    bililive_process_->InitGlobalProfile();
    
    // reset language from: 
    // 1.command line args  OR
    // 2.pref langauge
    std::string startupLanguage{};
    if (parsed_command_line().HasSwitch("language")) {
        std::vector<std::pair<std::string, std::string>> codes{
            {"en_us", "en-US"},
            {"zh_cn", "zh-CN"},
            {"id_id", "id-ID"},
            {"th_th", "th-TH"},
            // and more
        };
        auto code = parsed_command_line().GetSwitchValueASCII("language");
        std::transform(code.begin(), code.end(), code.begin(),
            [](unsigned char c) { return std::tolower(c); });
        for (auto pair : codes) {
            if (pair.first == code) {
                startupLanguage = pair.second;
                // LOG(INFO) << "===found peer langaue code " << startupLanguage;
                break;
            }
        }
    }
    else {
        // 
        auto prefs = GetBililiveProcess()->global_profile()->GetPrefs();
        startupLanguage = prefs->GetString(prefs::kLivehimeAppLocale);
        // LOG(INFO) << "===read language from prefs " << startupLanguage;
    }

    if (!startupLanguage.empty()) {
        if (startupLanguage != locale_loaded) {
            locale_loaded = ResourceBundle::GetSharedInstance().ReloadLocaleResources(startupLanguage);
            DCHECK_EQ(locale_loaded, startupLanguage);
            bililive_process_->SetApplicationLocale(startupLanguage);
        }
       
        GetBililiveProcess()->global_profile()->GetPrefs()->SetString(prefs::kLivehimeAppLocale, startupLanguage);
        // LOG(INFO) << "===set string " << startupLanguage;
    }

    bililive_creator_.reset(new StartupBililiveCreator());

    base::FilePath resources_pack_path;
    PathService::Get(bililive::FILE_RESOURCES_PACK, &resources_pack_path);
    if (base::PathExists(resources_pack_path))
        ResourceBundle::GetSharedInstance().AddDataPackFromPath(
            resources_pack_path, ui::SCALE_FACTOR_NONE);

    bililive_process_->PreCreateThreads();

    return bililive::ResultCodeNormalExit;
}

void BililiveMainPartsImpl::PreMainMessageLoopRun()
{
    result_code_ = PreMainMessageLoopRunImpl();

    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PreMainMessageLoopRun();
    }
}

// PreMainMessageLoopRun calls these extra stages in the following order:
//  PreMainMessageLoopRunImpl()
//   ... initial setup, including bililive_process_ setup.
//   PreProfileInit()
//   ... additional setup, including CreateProfile()
//   PostProfileInit()
//   ... additional setup
//   PreBililiveStart()
//   ... bililive_creator_->Start (OR parameters().ui_task->Run())
//   PostBililiveStart()

void BililiveMainPartsImpl::PreProfileInit()
{
    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PreProfileInit();
    }
}

void BililiveMainPartsImpl::PostProfileInit()
{
    bililive_process_->PostProfileInit();

    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PostProfileInit();
    }
}

void BililiveMainPartsImpl::PreBililiveStart()
{
    bililive_process_->PreBililiveStart();

    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PreBililiveStart();
    }
}

void BililiveMainPartsImpl::PostBililiveStart()
{
    bililive_process_->PostBililiveStart();

    for (size_t i = 0; i < bililive_extra_parts_.size(); ++i)
    {
        bililive_extra_parts_[i]->PostBililiveStart();
    }
}

int BililiveMainPartsImpl::PreMainMessageLoopRunImpl()
{
    if (BililiveInstallationRejecter::Reject()) {
        return bililive::ResultCodeErrorOccurred;
    }

    if (!bililive_process_->SetupSingleInstance()) {
        // Now that the main program instance is open, analyze the startup parameters to see 
        // if you need to pass them to the started main process
        bililive_process_->TransmitCommandLine();
        return bililive::ResultCodeErrorOccurred;
    }

    if (!bililive_process_->PreMainMessageLoopRun()) {
        return bililive::ResultCodeErrorOccurred;
    }

    LogSystemInformation();

    // When to install crash client is a little bit tricky, we must ensure that:
    // (1) installing is not prior to mutiple application instances check.
    // (2) installing is prior to main work stuff.

    // This is where the detection update process is handled
    if (!bililive_process_->InitCheckVersion()) {
        LOG(INFO) << "check update over";
        return bililive::ResultCodeLoginCancelled;
    }

    PreProfileInit();

#ifndef NDEBUG
    if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchLaunchUIExamples)) {
        ExamplesRunner runner;
        runner.Run();
        return bililive::ResultCodeLoginCancelled;
    }
#endif
    RequestWindowsInfo(true);
    if (!bililive_process_->InitSecretCore()) {
        return bililive::ResultCodeLoginCancelled;
    }

    RequestWindowsInfo(false);

    PostProfileInit();

    PreBililiveStart();

    int result_code;
    if (bililive_creator_->Start(parsed_command_line(), base::FilePath(), &result_code)) {
        run_message_loop_ = true;
    } else {
        run_message_loop_ = false;
    }

    PostBililiveStart();

    bililive_started_ = true;

    return bililive::ResultCodeNormalExit;
}

bool BililiveMainPartsImpl::MainMessageLoopRun(int* looper_result_code)
{
    *looper_result_code = result_code_;
    if (!run_message_loop_)
    {
        return true;  // Don't run the default message loop.
    }

    DCHECK_EQ(base::MessageLoop::TYPE_UI, base::MessageLoop::current()->type());
    views::AcceleratorHandler accelerator_handler;
    base::RunLoop run_loop(&accelerator_handler);
    run_loop.Run();

    if (bililive_process_->ExitIsLogout()) {
        result_code_ = bililive::ResultCodeSignOut;
        *looper_result_code = result_code_;
    } else if (bililive_process_->ExitIsRestart()) {
        result_code_ = bililive::ResultCodeRestart;
        *looper_result_code = result_code_;
    }

    return true;
}

void BililiveMainPartsImpl::PostMainMessageLoopRun()
{
    // Tear-down application layer module only when it was started.
    if (bililive_started_) {
        for (auto it = bililive_extra_parts_.rbegin();
            it != bililive_extra_parts_.rend(); ++it)
        {
            (*it)->BililiveTearDown();
        }
    }

    for (auto it = bililive_extra_parts_.rbegin();
        it != bililive_extra_parts_.rend(); ++it)
    {
        (*it)->PostMainMessageLoopRun();
    }

    bililive_process_->StartTearDown();
}

void BililiveMainPartsImpl::PostDestroyThreads()
{
    bililive_process_->PostDestroyThreads();
    bililive_process_.reset(nullptr);
}

void BililiveMainPartsImpl::AddParts(BililiveMainExtraParts *parts)
{
    bililive_extra_parts_.push_back(parts);
}