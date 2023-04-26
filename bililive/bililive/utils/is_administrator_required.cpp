#include "is_administrator_required.h"

#include <Windows.h>
#include <TlHelp32.h>
#include  <psapi.h>

#include "base/logging.h"
#pragma comment(lib,"psapi.lib")


namespace bililive_utils
{
    int GetProcessIdByName(const wchar_t* processName)
    {
        int result = 0;
        HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32W processEntry;
            processEntry.dwSize = sizeof(processEntry);
            if (Process32FirstW(hProcessSnapshot, &processEntry) == TRUE)
            {
                do {
                    if (wcsicmp(processEntry.szExeFile, processName) == 0)
                    {
                        result = processEntry.th32ProcessID;
                        break;
                    }
                } while (Process32NextW(hProcessSnapshot, &processEntry));
            }
            CloseHandle(hProcessSnapshot);
        }

        return result;
    }

    bool CheckIfSucceedToOpenProcess(int pid)
    {
        bool bMayNeedAdmin = false;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (hProcess)
        {
            CloseHandle(hProcess);
        }
        else
        {
            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                bMayNeedAdmin = true;
            }
        }

        return !bMayNeedAdmin;
    }
    void KillProcessByPid(int pid)
    {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess){
            TerminateProcess(hProcess, (UINT)-1);
            CloseHandle(hProcess);
        }
    }
    bool CheckProcessIsOpen(int pid, base::string16& process_name,bool check_name)
    {
        bool result = false;
        HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32W processEntry;
            processEntry.dwSize = sizeof(processEntry);
            if (Process32FirstW(hProcessSnapshot, &processEntry) == TRUE)
            {
                do {
                    if ((int)processEntry.th32ProcessID == pid) {
                        process_name = processEntry.szExeFile;
                        if (check_name) {
                            if (process_name.find(L"start.exe") != process_name.npos) {
                                result = true;
                            }
                            else {
                                LOG(WARNING) << "[GameOpen] check game pid is start but game_name is::" <<          process_name;
                            }
                        }
                        else {
                            result = true;
                        }
                        
                        break;
                    }

                } while (Process32NextW(hProcessSnapshot, &processEntry));
            }
            CloseHandle(hProcessSnapshot);
        }
        return result;
    }

    bool CheckPluginProcess(int timer)
    {
        bool result = false;
        HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32W processEntry;
            processEntry.dwSize = sizeof(processEntry);
            if (Process32FirstW(hProcessSnapshot, &processEntry) == TRUE)
            {
                do {
                    base::string16 process_name = processEntry.szExeFile;
                    if (process_name == L"bililive_browser.exe") {
                        PROCESS_MEMORY_COUNTERS pmc;

                        HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, processEntry.th32ProcessID);
                        result = GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
                        if (result) {
                            int64 work_size = pmc.WorkingSetSize;
                            if (work_size > static_cast<int64>(1024) * 1024 * timer) {

                                TerminateProcess(handle, (UINT)-1);
                                CloseHandle(handle);
                                LOG(INFO) << "[plugin] close process name : " << work_size;
                                return false;
                            }
                        }
                        CloseHandle(handle);
                    }
                } while (Process32NextW(hProcessSnapshot, &processEntry));
            }
            CloseHandle(hProcessSnapshot);
        }
        return true;
    }


};
