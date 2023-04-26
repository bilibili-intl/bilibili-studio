// cef_proxy_dll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "cef_proxy_dll.h"

#include <shlobj.h>
#include <thread>
#include <functional>
#include <stdlib.h>

#include "util/cef_proxy_util.h"

#include "browser/livehime_cef_app_browser.h"
#include "browser/livehime_cef_client_browser.h"
#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"
#include "public/livehime_cef_proxy_constants.h"

#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "cef_core/include/cef_waitable_event.h"


extern HMODULE cef_proxy_module;

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

        data_dir = data_dir.append(L"\\bililiveintl");
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

        // copy from obs-browser\obs-browser-plugin.cpp:BrowserInit [
        /* Massive (but amazing) hack to prevent chromium from modifying our
         * process tokens and permissions, which caused us problems with winrt,
         * used with window capture.  Note, the structure internally is just
         * two pointers normally.  If it causes problems with future versions
         * we'll just switch back to the static library but I doubt we'll need
         * to. */
        uintptr_t zeroed_memory_lol[32] = {};
        // ]
        return CefInitialize(std::get<0>(tp), std::get<1>(tp), std::get<2>(tp), zeroed_memory_lol);
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
        // Enable High-DPI support on Windows 7 or newer.
        CefEnableHighDPISupport();

        // Initialize CEF.
        void* sandbox_info = NULL;
        // Provide CEF with command-line arguments.
        CefMainArgs main_args(GetModuleHandle(nullptr));

        // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
        // that share the same executable. This function checks the command-line and,
        // if this is a sub-process, executes the appropriate logic.
        int exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
        if (exit_code >= 0)
        {
            // The sub-process has completed so return here.
            //return !!exit_code;
            return;
        }

        wchar_t dll_path[MAX_PATH] = { 0 };
        ::GetModuleFileName(cef_proxy_module, dll_path, MAX_PATH);
        std::wstring absolute_dll_path = MakeAbsoluteFilePath(dll_path);
        std::wstring cef_proxy_dir = absolute_dll_path.substr(0, absolute_dll_path.rfind('\\') + 1);
        std::wstring a_browser_path = cef_proxy_dir + cef_proxy::kCefSubProcesses;
        a_browser_path += L".exe";

        // Specify CEF global settings here.
        CefSettings settings;
        settings.no_sandbox = true;
        settings.windowless_rendering_enabled = true;
        // browser-source����־���ڵ�ǰ��������־Ŀ¼��
        settings.log_severity = LOGSEVERITY_DISABLE;
        auto log_dir = GetBililiveUserDataDirectory();
        if (!log_dir.empty())
        {
            // ��3538����3770����3770д��cache 3538ʶ���ˣ������û����¾ɰ汾���л�ʹ��ʱ�ɰ汾���ܱ�����
            // ����3770�汾�Ȳ�дcache���ȵ����ϵ�3770���ǵò������дcache
            CefString(&settings.cache_path) = log_dir + L"\\" + std::wstring(cef_proxy::kBililiveBrowserCacheDirName);

            CefString(&settings.log_file) = log_dir + L"\\" + std::wstring(cef_proxy::kBililiveBrowserLogFileName);
            settings.log_severity = LOGSEVERITY_INFO;
        }
        CefString(&settings.locale) = cef_proxy::kCefLocale;
        CefString(&settings.browser_subprocess_path) = a_browser_path;

        // �����հ�obs-browser�Ĳ������ã���Ϊ���ǵ�ǰ��������򷢲�obs-plugins��data��Ҳ����û��locale����һ��.ini�����ļ�����
        // �������������֧�ֵ������б��ڴ򿪷�����ҳ��ʱCEF�ײ�����
        /*std::string accepted_languages;
        if (std::string(cef_proxy::kCefLocale) != "en-US")
        {
            accepted_languages = cef_proxy::kCefLocale;
            accepted_languages += ",";
            accepted_languages += "en-US,en";
        }
        else
        {
            accepted_languages = "en-US,en";
        }
        CefString(&settings.accept_language_list) = accepted_languages;*/

        // ��Ҫֱ���������޸�ȫ�ֵ�UA���������õ�UA����Ӱ������ҳ�棬�������ض���ҵ��ҳ����ҪαװUA��Ӧ����CefRequest��̬����UA
        // ����������αװ��Android�Ļ���վ��ҳ��Ͳ��ܿ��ˣ��򿪱��ҳ��Ҳ�����ƶ��˵ķ����ʽ����ʾ
        //CefString(&settings.user_agent) = "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36 Android";

        // SimpleApp implements application-level callbacks for the browser process.
        // It will create the first browser instance in OnContextInitialized() after
        // CEF has initialized.
        CefRefPtr<LivehimeCefAppBrowser> browser_app(new LivehimeCefAppBrowser());

        cef_init_params tp_data = std::make_tuple(main_args, settings, CefRefPtr<CefApp>(browser_app), sandbox_info);
        cef_proxy::CefInitResult code = InitCefSEH(&tp_data);
        cef_initialize_succeeded = (code == cef_proxy::CefInitResult::CefInitializeSuccess);

        // ֪ͨ�ϲ��ʼ��״̬
        LivehimeCefProxyEventsDispatcher::DispatchInitStatus(cef_initialize_succeeded, code);

        if (cef_initialize_succeeded)
        {
            CefRunMessageLoop();
        }

        bool cef_shutdown_ret = ShutdownCefSEH();

        // ֪ͨ�ϲ��ʼ��״̬
        LivehimeCefProxyEventsDispatcher::DispatchShutdownStatus(cef_shutdown_ret);
    }

    bool IsCefInitializeSucceeded()
    {
        return cef_initialize_succeeded;
    }
}
