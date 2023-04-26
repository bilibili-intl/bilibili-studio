#include "bililive_obs_plugin.h"

#include <util/threading.h>
#include <util/platform.h>
#include <util/util.hpp>
#include <util/dstr.hpp>
#include <obs-module.h>
#include <obs.hpp>
#include "UI/obs-frontend-api/obs-frontend-api.h"

#include <libloaderapi.h>
#include <mutex>
#include <windows.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/prefs/json_pref_store.h"
#include "base/win/registry.h"

#include "bililive/public/bililive/livehime_obs_ipc_message.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"

#include "obs_livehime_ipc/obs_livehime_ipc_service.h"
#include "profiles/obs_livehime_profile.h"
#include "public/obs_livehime_pref_constants.h"
#include "threads/obs_livehime_thread_manager.h"

// 为了多级菜单需要接入QT库，要改构建机、每个人都要装QT才能编，算了，暂时一级菜单[
//#include <QAction>
//#include <QString>
//#include <QMenu>
//#include <QPointer>
// ]

namespace
{
    const wchar_t kLivehimeDebugName[] = L"bililiveintl.exe";
    const wchar_t kLivehimeReleaseName[] = L"bililiveintl.exe";

    const char kPluginID[] = "bililive_obs_plugin";
    const char kPluginName[] = "bililive_obs_plugin";
    const char kPluginVersion[] = "1.0.0.0";

    const char kLimitOBSVersionStr[] = "23.1.0";
    const uint32_t kLimitOBSVersion = 0x17010000;

    const wchar_t kMenuItemName[] = L"哔哩哔哩直播";
    //const wchar_t kStartLiveItemName[] = L"开始直播";

    std::unique_ptr<base::AtExitManager> g_exit_manager;

    std::unique_ptr<LivehimeOBSPluginImpl> g_livehime_obs_plugin;

    void InitChromiumBaseCore()
    {
        DCHECK(!g_exit_manager);

        CommandLine::Init(0, nullptr);
        g_exit_manager = std::make_unique<base::AtExitManager>();
    }

    void UnintChromiumBaseCore()
    {
        if (g_exit_manager)
        {
            CommandLine::Reset();
            g_exit_manager.reset();
        }
    }

    std::wstring GetLivehimeInstallPath()
    {
#ifdef  _WIN64
		static const wchar_t kAppId[] = L"{06BB8CDE-F4AC-4B22-A680-0F3699CB9912}_is1";
		static const wchar_t kLiveHimeUninstallEntryRegPath[] = LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\)";
#else

		static const wchar_t kAppId[] = L"{81F6F736-F774-4965-A593-1AFD31ABBB35}_is1";
		static const wchar_t kLiveHimeUninstallEntryRegPath[] = LR"(SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\)";

#endif // _WIN64
        static const wchar_t kAppExeName[] = L"Bilibili Studio.exe";
        
        auto get_install_location = [](const wchar_t* subkey)->std::wstring
        {
            std::wstring app_key(subkey);
            app_key.append(kAppId);

            std::wstring livehime_path;
            base::win::RegKey reg(HKEY_LOCAL_MACHINE, app_key.c_str(), KEY_READ | KEY_WOW64_32KEY);
            if (reg.Valid())
            {
                if (ERROR_SUCCESS == reg.ReadValue(L"InstallLocation", &livehime_path))
                {
                    livehime_path.append(kAppExeName);
                }
                else
                {
                    blog(LOG_INFO, "can't find livehime install path in reg '%ls'.", subkey);
                }
            }
            else
            {
                blog(LOG_INFO, "can't find livehime install infos in reg '%ls'.", subkey);
            }
            return livehime_path;
        };

        std::wstring livehime_path = get_install_location(kLiveHimeUninstallEntryRegPath);
        return livehime_path;
    }

    base::FilePath GetGlobalProfileDir()
    {
        base::FilePath user_data_dir;
        PathService::Get(bililive::DIR_USER_DATA, &user_data_dir);
        auto global_profile_dir = user_data_dir.Append(bililive::kGlobalProfileDirName);
        return global_profile_dir;
    }

    bool LunchLivehime(const std::vector<std::string>& args)
    {
        std::wstring livehime_path = GetLivehimeInstallPath();
        if (!livehime_path.empty())
        {
            base::FilePath path(livehime_path);
            if (base::PathExists(path))
            {
                CommandLine cmd(path);
                cmd.AppendSwitch(bililive::kSwitchOBSPlugin);

                for (auto& iter : args)
                {
                    cmd.AppendSwitch(iter);
                }

                STARTUPINFOW si = { sizeof(si) };
                si.dwFlags = STARTF_USESTDHANDLES;
                si.hStdInput = INVALID_HANDLE_VALUE;
                si.hStdOutput = INVALID_HANDLE_VALUE;
                si.hStdError = INVALID_HANDLE_VALUE;
                PROCESS_INFORMATION pi = {};
                if (::CreateProcessW(nullptr, (LPWSTR)cmd.GetCommandLineString().c_str(), 0, 0, FALSE, CREATE_NO_WINDOW, 0, 0, &si, &pi))
                {
                    ::CloseHandle(pi.hThread);
                    ::CloseHandle(pi.hProcess);
                    blog(LOG_INFO, "run livehime successed.");

                    return true;
                }
                else
                {
                    blog(LOG_WARNING, "run livehime failed, code = %d.", ::GetLastError());
                }
            }
            else
            {
                //char *str = NULL;
                //os_wcs_to_utf8_ptr(livehime_path.c_str(), 0, &str);
                //bfree(str);
                blog(LOG_WARNING, "run livehime failed, livehime not exists in the path '%ls'.", livehime_path.c_str());
            }
        }

        return false;
    }

    void OnStartLiveMenuItemClick(void* private_data)
    {
        if (!g_livehime_obs_plugin || !g_livehime_obs_plugin->IsLivehimeConnected())
        {
            // 以命令行参数形式指示直播姬启动完成后就开播
            LunchLivehime({ bililive::kSwitchOBSPluginStartLive });
        }

        // 发IPC消息，没连上的时候IPC消息会暂存在发送队列中的，连上之后会发过去的，
        // 也就是说直播姬可能会收到多个累积的开播消息，直播姬要做状态判断
        if (g_livehime_obs_plugin && g_livehime_obs_plugin->ipc_service())
        {
            // 如果OBS正在推流要将其停止推流，是否需要msgbox提示一下？
            if (obs_frontend_streaming_active())
            {
                int ret = ::MessageBoxW(nullptr, L"您正在推流，要通过直播姬开播需要先停止当前推流，\n确认停止推流吗？",
                    L"哔哩哔哩直播", MB_SYSTEMMODAL|MB_YESNO);
                if (ret != IDYES)
                {
                    return;
                }
                obs_frontend_streaming_stop();
            }

            g_livehime_obs_plugin->ipc_service()->SendLivehimeMessage(ipc_message::IPC_OBS_TO_LIVEHIME_START_LIVE);
        }
    }
}

// LivehimeOBSPlugin
LivehimeOBSPluginImpl* LivehimeOBSPlugin()
{
    DCHECK(g_livehime_obs_plugin);
    return g_livehime_obs_plugin.get();
}

LivehimeOBSPluginImpl::LivehimeOBSPluginImpl()
{
    Initialize();
}

LivehimeOBSPluginImpl::~LivehimeOBSPluginImpl()
{
    Uninitialize();
}

bool LivehimeOBSPluginImpl::IsLivehimeConnected() const
{
    if (ipc_service_ && ipc_service_->livehime_connected())
    {
        return true;
    }

    return false;
}

void LivehimeOBSPluginImpl::Initialize()
{
    PathService::Get(bililive::DIR_USER_DATA, &user_data_dir_);
    auto global_profile_dir = GetGlobalProfileDir();
    if (!base::DirectoryExists(global_profile_dir))
    {
        file_util::CreateDirectory(global_profile_dir);
    }

    thread_manager_.reset(new OBSPluginThreadManager(this));
    thread_manager_->Initialize();
}

void LivehimeOBSPluginImpl::Uninitialize()
{
    if (ipc_service_)
    {
        ipc_service_->Uninitialize();
        ipc_service_ = nullptr;
    }
    // 配置文件的未决写入要放到插件主线程（即Profile.PrefService的create线程）去做
    if (profile_)
    {
        if (thread_manager_ && thread_manager_->main_message_loop_proxy())
        {
            thread_manager_->main_message_loop_proxy()->PostTask(FROM_HERE,
                base::Bind(&Profile::CommitPendingWrite, profile_));
            profile_ = nullptr;
        }
    }
    if (thread_manager_)
    {
        thread_manager_->Uninitialize();
        thread_manager_ = nullptr;
    }
}

// OBSPluginThreadDelegate
void LivehimeOBSPluginImpl::PostMainThreadStarted()
{
}

void LivehimeOBSPluginImpl::PostIPCThreadStarted()
{
    ipc_service_ = new OBSPluginLivehimeIPCService(thread_manager_->ipc_message_loop_proxy());
    ipc_service_->Initialize();
}

void LivehimeOBSPluginImpl::PostWorkerPoolStarted()
{
    if (!profile_)
    {
        auto global_profile_dir = GetGlobalProfileDir();

        scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner = JsonPrefStore::GetTaskRunnerForFile(
            global_profile_dir.AppendASCII(obs_plugin_prefs::kProfileFilename), thread_manager_->GetWorkerPool());
        profile_ = Profile::CreateGlobalProfile(thread_manager_->main_message_loop_proxy(),
            global_profile_dir, sequenced_task_runner);
    }
}


// obs
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(kPluginID, "en-US")

bool obs_module_load(void)
{
    blog(LOG_INFO, "[%s]: Version %s", kPluginName, kPluginVersion);

    uint32_t iver = obs_get_version();
    if (iver < kLimitOBSVersion)
    {
        blog(LOG_WARNING, "[%s] need obs version at least v%s, current obs version is v%s",
            kPluginName, kLimitOBSVersionStr, obs_get_version_string());

        return false;
    }

    InitChromiumBaseCore();

    auto cmd = CommandLine::ForCurrentProcess();
    std::wstring path = cmd->GetProgram().BaseName().value();
    if (_wcsicmp(path.c_str(), kLivehimeDebugName) == 0 || 
        _wcsicmp(path.c_str(), kLivehimeReleaseName) == 0)
    {
        // 直播姬自己不加载这个插件，直播姬自己的IPC服务中最好也再过滤一下
        UnintChromiumBaseCore();
        return false;
    }

    blog(LOG_INFO, "[%s] initializing...", kPluginName);

    bililive::RegisterPathProvider();

    g_livehime_obs_plugin = std::make_unique<LivehimeOBSPluginImpl>();

    char *str = NULL;
    os_wcs_to_utf8_ptr(kMenuItemName, 0, &str);
    obs_frontend_add_tools_menu_item(str, OnStartLiveMenuItemClick, 0);
    /*QAction* action = (QAction*)obs_frontend_add_tools_menu_qaction(str);
    if (action)
    {
        QPointer<QMenu> menu = new QMenu(QString::fromUtf8("bililive menu"));
        action->setMenu(menu);

        {
            QPointer<QAction> sub_action = new QAction(QString::fromWCharArray(kStartLiveItemName));
            QObject::connect(sub_action, &QAction::triggered, OnStartLiveMenuItemClick);

            menu->addAction(sub_action);
        }
    }*/
    bfree(str);

    blog(LOG_INFO, "[%s] Initialize completed.", kPluginName);
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "[%s] Uninitializing...", kPluginName);

    /*if (security_dll_loader)
    {
        security_dll_loader.reset(nullptr);
    }*/

    if (g_livehime_obs_plugin)
    {
        g_livehime_obs_plugin.reset();
    }

    UnintChromiumBaseCore();

    blog(LOG_INFO, "[%s]: Uninitialized.", kPluginName);
}
