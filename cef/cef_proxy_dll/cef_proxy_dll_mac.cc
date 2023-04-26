
#include "cef_proxy_dll.h"

#include <thread>
#include <functional>
#include <stdlib.h>

#include "util/cef_proxy_util.h"

#include "livehime/livehime_cef_client_browser.h"

#include "public/livehime_cef_proxy_constants.h"

#include "bililive_browser/livehime/livehime_cef_app_browser.h"
#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"

#include "cef_core/include/cef_waitable_event.h"

namespace
{
    volatile bool cef_initialize_succeeded = false;

    bool GetLocalAppDataDirectory(std::wstring& dir_path)
    {
        const size_t kBufSize = MAX_PATH + 1;
        wchar_t path_buf[kBufSize]{ 0 };
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path_buf)))
        {
            return false;
        }

        dir_path = path_buf;

        return true;
    }

    // Hope no one has ever messed up with the directory.
    // we don't do error checking here, for the sake of the god.
    void CreateDirectoryIfNecessary(const std::wstring& dir_path)
    {
        if (!(GetFileAttributes(dir_path.c_str()) != INVALID_FILE_ATTRIBUTES))
        {
            CreateDirectoryW(dir_path.c_str(), nullptr);
        }
    }

    std::wstring GetBililiveDirectory()
    {
        std::wstring data_dir;
        if (!GetLocalAppDataDirectory(data_dir))
        {
            return L"";
        }

        data_dir = data_dir.append(L"\\bililive");
        CreateDirectoryIfNecessary(data_dir);

        return data_dir;
    }

    std::wstring GetBililiveUserDataDirectory()
    {
        auto data_dir = GetBililiveDirectory();
        if (data_dir.empty())
        {
            return L"";
        }

        auto user_data_dir = data_dir.append(L"\\User Data");
        CreateDirectoryIfNecessary(user_data_dir);

        return user_data_dir;
    }

    using cef_init_params = std::tuple<CefMainArgs, CefSettings, CefRefPtr<CefApp>, void*>;
    bool InitCef(cef_init_params* params)
    {
        cef_init_params& tp = *params;
        return CefInitialize(std::get<0>(tp), std::get<1>(tp), std::get<2>(tp), std::get<3>(tp));
    }

    cef_proxy::CefInitResult InitCefSEH(cef_init_params* params)
    {
        cef_proxy::CefInitResult code = cef_proxy::CefInitResult::CefInitializeFailed;
        __try
        {
            if (!InitCef(params))
            {
                code = cef_proxy::CefInitResult::CefInitializeFailed;
            }
            else
            {
                code = cef_proxy::CefInitResult::CefInitializeSuccess;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            code = cef_proxy::CefInitResult::CefInitializeCrash;
        }
        return code;
    }

    bool ShutdownCefSEH()
    {
        bool ret = false;
        __try
        {
            CefShutdown();
            ret = true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
        return ret;
    }

}

namespace internal
{
    void BrowserManagerThread(void)
    {
        std::string path = obs_get_module_binary_path(obs_current_module());
        path = path.substr(0, path.find_last_of('/') + 1);
        path += cef_proxy::kCefSubProcesses;

        /* On non-windows platforms, ie macOS, we'll want to pass thru flags to
         * CEF */
        struct obs_cmdline_args cmdline_args = obs_get_cmdline_args();
        CefMainArgs args(cmdline_args.argc, cmdline_args.argv);

        CefSettings settings;
        settings.log_severity = LOGSEVERITY_DISABLE;
        settings.windowless_rendering_enabled = true;
        settings.no_sandbox = true;

#if defined(__APPLE__) && !defined(BROWSER_DEPLOY)
        CefString(&settings.framework_dir_path) = CEF_LIBRARY;
#endif

        /*std::string obs_locale = obs_get_locale();
        std::string accepted_languages;
        if (obs_locale != "en-US")
        {
            accepted_languages = obs_locale;
            accepted_languages += ",";
            accepted_languages += "en-US,en";
        }
        else
        {
            accepted_languages = "en-US,en";
        }*/

        BPtr<char> conf_path = obs_module_config_path("");
        os_mkdir(conf_path);
        BPtr<char> conf_path_abs = os_get_abs_path_ptr(conf_path);
        //CefString(&settings.locale) = obs_get_locale();
        //CefString(&settings.accept_language_list) = accepted_languages;
        CefString(&settings.cache_path) = conf_path_abs;
        CefString(&settings.browser_subprocess_path) = path;
        CefString(&settings.locale) = cef_proxy::kCefLocale;

        app = new LivehimeCefAppBrowser();
        CefExecuteProcess(args, app, nullptr);

        CefInitialize(args, settings, app, nullptr);

        // 通知上层初始化状态
        LivehimeCefProxyEventsDispatcher::DispatchShutdownStatus(cef_shutdown_ret);
    }

    bool IsCefInitializeSucceeded()
    {
        return cef_initialize_succeeded;
    }
}
