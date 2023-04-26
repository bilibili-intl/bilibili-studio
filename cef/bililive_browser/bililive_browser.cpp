// bililive_browser.cpp : ����Ӧ�ó������ڵ㡣
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
                // ���������ˣ��������Լ������̣߳�CEF���̣߳����˳���Ϣ
                PostThreadMessage(main_thread_id, WM_QUIT, 0, 0);
                // ͣ�ٸ������ӣ���CEF���Լ��ķ�ʽ�˳�����Ȼ��ֱ��ǿ���ˣ���ȻCEF�Լ�Ҳ��ǿ�ˣ�
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
                // CEF����Flash���ʱ������ǲ���sandbox��ʽ�Ļ���ô�ڵ�һ�μ��ص�ʱ��ᵯ��������cmd.exe����
                // ������ʾ��NOT SANDBOXED������������̫��������Flash��BUG��Flash��Դ���Ǵӻ����������ComSpec·����
                // �ҵ�������������ʾ�����İ������ϵ����ϴ������⼸�ַ�ʽ�����
                // 1���޸�Flash��Դ�룬�𵯴������ɵ�Flash��������һ�𷢲���
                // 2����sandbox��ʽ����CEF�����ַ�ʽ�п����޷������ӽ��̣�Gooogle Chrome�Լ����������ַ�ʽ���У�
                // 3��HOOKϵͳapi�ķ�ʽ����Ҫ�����Ӳ��Detours����ǵ������Ŀ�Դ��Esayhook�ȵȣ����ҹ�CreateProcessA/W������������
                //    �����ǡ�echo NOT SANDBOXED����ֱ�Ӿܾ�ִ�У�
                //    ����������Թ���������Detours����Esayhook�������ESP������ʱ�����ˣ�
                // 4������ǰ���̵Ļ��������е�ComSpec��cmd.exe��·����ָ��һ�������µ�exe�����ǲ��������ʽ�����ַ�ʽ��Ϊ��ݡ�
                // ����OBS����ô���ģ�Դ��û�ҵ���˿����
                ::SetEnvironmentVariableW(L"ComSpec", CreateDummyComSpecFile().c_str());
            }
        }
        break;
    default:
        // ������ֱ�����и�exe����Ҫͨ��cef_proxy_dll���������в�������
        return 0;
        break;
    }

    // ����browser����״̬����̣߳�browser����һ���˳��Ͱ�����CEF�ӽ��̹ص���
    // CEF�ڲ��������Ƶļ���˳����Ƶģ����ַ�ʽ���У�
    //      1��browser���������رյ�ʱ���������ӽ���Terminate��
    //      2��browser�����쳣�رյ�ʱ�򣨱��类Terminate�����ӽ��̼�⵽browser���̹ر��ˣ��Լ�����ExitProcess��
    // CEF�Լ��������ַ�ʽ����ֱ�Ӵֱ��ģ�����ʱ��browser���̹ر�֮�󲢲��ᰴ��Ԥ��ִ�У�
    // ���������Լ��ټ�һ���⣬��ô���൱�����ر��գ�Ϊ�����������˳����ӽ����ܾ����˳�
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

    // ��Щ�����CEF�ӽ��̻�������ŵ��˳�����ʱһ�ж��������������ӽ��̵��˳�Ҳ��������
    // ��ʱҪ������߳��˵�
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

