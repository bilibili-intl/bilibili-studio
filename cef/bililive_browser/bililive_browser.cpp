// bililive_browser.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"

#include <string>
#include <thread>

#include "bililive_browser.h"

#include "renderer/livehime_cef_app_render.h"
#include "renderer/livehime_cef_app_other.h"

#include "cef_core/include/cef_file_util.h"
#include "cef_proxy_dll/public/livehime_cef_proxy_constants.h"

#include "public/bililive_browser_switches.h"

#include <fstream>

namespace
{
    HANDLE hShutdownEvent = nullptr;
    std::thread g_detect_browser_exit_thread;

    void DetectBrowserExitThreadProc(DWORD browser_process_id, DWORD main_thread_id)
    {
        HANDLE process_handle = ::OpenProcess(SYNCHRONIZE, false, browser_process_id);
        if (process_handle)
        {
            HANDLE handles[2] = { process_handle , hShutdownEvent };
            DWORD dwRet = WaitForMultipleObjects(2, handles, false, INFINITE);
            if (dwRet == WAIT_OBJECT_0)
            {
                // 父进程退了，向我们自己的主线程（CEF主线程）发退出消息
                PostThreadMessage(main_thread_id, WM_QUIT, 0, 0);
                // 停顿个几秒钟，让CEF按自己的方式退出，不然就直接强退了（虽然CEF自己也是强退）
                dwRet = WaitForSingleObject(hShutdownEvent, 5000);
                if (dwRet != WAIT_OBJECT_0)
                {
                    TerminateProcess(GetCurrentProcess(), (UINT)-1);
                }
            }

            ::CloseHandle(process_handle);
        }
    }

    std::wstring CreateDummyComSpecFile()
    {
        CefString temp_dir;
        if (CefGetTempDirectory(temp_dir))
        {
            temp_dir = temp_dir.ToWString() + L"\\Bililive";
            if (CefCreateDirectory(temp_dir))
            {
                temp_dir = temp_dir.ToWString() + L"\\livehime_comspec.exe";
                if (!(GetFileAttributes(temp_dir.ToWString().c_str()) != INVALID_FILE_ATTRIBUTES))
                {
                    std::ofstream(temp_dir.ToWString().c_str(), std::ios_base::binary | std::ios_base::trunc);
                }
            }
        }
        return temp_dir.ToWString();
    }

	DECLARE_HANDLE(OBS_DPI_AWARENESS_CONTEXT);
#define OBS_DPI_AWARENESS_CONTEXT_UNAWARE              ((OBS_DPI_AWARENESS_CONTEXT)-1)
#define OBS_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((OBS_DPI_AWARENESS_CONTEXT)-2)
#define OBS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((OBS_DPI_AWARENESS_CONTEXT)-3)
#define OBS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((OBS_DPI_AWARENESS_CONTEXT)-4)

	bool SetHighDPIv2Scaling()
	{
		static BOOL(WINAPI *func)(OBS_DPI_AWARENESS_CONTEXT) = nullptr;
		func = reinterpret_cast<decltype(func)>(GetProcAddress(
			GetModuleHandleW(L"USER32"),
			"SetProcessDpiAwarenessContext"));
		if (!func) {
			return false;
		}

		return !!func(OBS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CefMainArgs main_args(nullptr);

	// Enable High-DPI support on Windows 7 or newer.
	if (!SetHighDPIv2Scaling())
		CefEnableHighDPISupport();

    void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif

    // Parse command-line arguments.
    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->InitFromString(::GetCommandLineW());

    // Create a ClientApp of the correct type.
    CefRefPtr<CefApp> app;
    LivehimeCefApp::ProcessType process_type = LivehimeCefApp::GetProcessType(command_line);
    switch (process_type)
    {
    case LivehimeCefApp::RendererProcess:
        app = new LivehimeCefAppRender();
        //::MessageBox(nullptr, L"Attach Render process to Debuger", std::to_wstring(::GetCurrentProcessId()).c_str(), MB_OK);
        break;
    case LivehimeCefApp::GpuProcess:
    case LivehimeCefApp::PPAPIProcess:
    case LivehimeCefApp::OtherProcess:
        {
            app = new LivehimeCefAppOther();

            if (LivehimeCefApp::PPAPIProcess == process_type)
            {
                // CEF加载Flash插件时如果不是采用sandbox方式的话那么在第一次加载的时候会弹出命令行cmd.exe窗口
                // 其上显示“NOT SANDBOXED”，交互体验太差，这个算是Flash的BUG，Flash的源码是从环境变量里查ComSpec路径，
                // 找到就运行它并显示这行文案。网上的资料大致是这几种方式解决：
                // 1、修改Flash的源码，别弹窗，生成的Flash插件随程序一起发布；
                // 2、以sandbox方式运行CEF，这种方式有可能无法创建子进程，Gooogle Chrome自己都不用这种方式运行；
                // 3、HOOK系统api的方式（需要引入巨硬的Detours库或是第三方的开源库Esayhook等等），挂勾CreateProcessA/W，发现命令行
                //    参数是“echo NOT SANDBOXED”的直接拒绝执行；
                //    （这个方案试过，不管用Detours还是Esayhook都会出现ESP错误，暂时放弃了）
                // 4、将当前进程的环境变量中的ComSpec（cmd.exe的路径）指向一个不做事的exe，我们采用这个方式，此种方式最为便捷。
                // 不懂OBS是怎么做的，源码没找到蛛丝马迹。
                ::SetEnvironmentVariableW(L"ComSpec", CreateDummyComSpecFile().c_str());
            }
        }
        break;
    default:
        // 不允许直接运行该exe，需要通过cef_proxy_dll附带命令行参数启动
        return 0;
        break;
    }

    // 运行browser进程状态监测线程，browser进程一旦退出就把所有CEF子进程关掉；
    // CEF内部是有类似的监测退出机制的，两种方式并行：
    //      1、browser进程正常关闭的时候会把所有子进程Terminate；
    //      2、browser进程异常关闭的时候（比如被Terminate），子进程监测到browser进程关闭了，自己进行ExitProcess；
    // CEF自己的这两种方式都是直接粗暴的，但有时候browser进程关闭之后并不会按此预期执行，
    // 所以我们自己再加一层监测，这么做相当于三重保险，为的是主进程退出了子进程能尽快退出
    CefString id_str = command_line->GetSwitchValue(switches::kBililiveBrowserProcessId);
    if (!id_str.empty())
    {
        hShutdownEvent = ::CreateEvent(nullptr, false, false, nullptr);
        if (!!hShutdownEvent)
        {
            g_detect_browser_exit_thread = std::thread(DetectBrowserExitThreadProc, 
                std::stoul(id_str.c_str()), GetCurrentThreadId());
        }
    }

    // Execute the secondary process, if any.
    int ret = CefExecuteProcess(main_args, app, sandbox_info);

    // 有些情况下CEF子进程会进行优雅的退出，此时一切都是运行正常，子进程的退出也是正常的
    // 这时要将监测线程退掉
    if (hShutdownEvent)
    {
        ::SetEvent(hShutdownEvent);

        if (g_detect_browser_exit_thread.joinable())
        {
            g_detect_browser_exit_thread.join();
        }

        ::CloseHandle(hShutdownEvent);
    }

    return ret;
}

