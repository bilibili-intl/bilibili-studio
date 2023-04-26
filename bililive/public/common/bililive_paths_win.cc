#include <windows.h>
#include <knownfolders.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/win/metro.h"
#include "base/win/registry.h"
#include "base/win/scoped_co_mem.h"
#include "base/strings/utf_string_conversions.h"
#include "bililive_paths_internal.h"
#include "bililive_constants.h"


namespace bililive
{

    namespace
    {
        bool GetUserDirectory(int csidl_folder, base::FilePath *result)
        {
            wchar_t path_buf[MAX_PATH];
            path_buf[0] = 0;
            if (FAILED(SHGetFolderPath(NULL, csidl_folder, NULL,
                                       SHGFP_TYPE_CURRENT, path_buf)))
            {
                return false;
            }
            *result = base::FilePath(path_buf);
            return true;
        }
    }

    bool GetDefaultUserDataDirectory(base::FilePath *result)
    {
        if (!PathService::Get(base::DIR_LOCAL_APP_DATA, result))
        {
            return false;
        }

        *result = result->Append(base::ASCIIToUTF16(bililive::kBililiveIntl));
        *result = result->Append(bililive::kUserDataDirname);
        return true;
    }

    void GetUserCacheDirectory(const base::FilePath &profile_dir,
                               base::FilePath *result)
    {
        *result = profile_dir;
    }

    bool GetUserDocumentsDirectory(base::FilePath *result)
    {
        return GetUserDirectory(CSIDL_MYDOCUMENTS, result);
    }

    bool GetUserDownloadsDirectorySafe(base::FilePath *result)
    {
        if (!GetUserDocumentsDirectory(result))
        {
            return false;
        }

        *result = result->Append(L"Downloads");
        return true;
    }

    bool GetUserDownloadsDirectory(base::FilePath *result)
    {
        typedef HRESULT (WINAPI * GetKnownFolderPath)(
            REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *);
        GetKnownFolderPath f = reinterpret_cast<GetKnownFolderPath>(
                                   GetProcAddress(GetModuleHandle(L"shell32.dll"), "SHGetKnownFolderPath"));
        base::win::ScopedCoMem<wchar_t> path_buf;
        if (f && SUCCEEDED(f(FOLDERID_Downloads, 0, NULL, &path_buf)))
        {
            *result = base::FilePath(std::wstring(path_buf));
            return true;
        }
        return GetUserDownloadsDirectorySafe(result);
    }

    bool GetUserMusicDirectory(base::FilePath *result)
    {
        return GetUserDirectory(CSIDL_MYMUSIC, result);
    }

    bool GetUserPicturesDirectory(base::FilePath *result)
    {
        return GetUserDirectory(CSIDL_MYPICTURES, result);
    }

    bool GetUserVideosDirectory(base::FilePath *result)
    {
        return GetUserDirectory(CSIDL_MYVIDEO, result);
    }

    bool ProcessNeedsProfileDir(const std::string &process_type)
    {
        return process_type.empty();
    }

    bool GetBililiveAccountConfigDirectory(const std::string &account_name,
                                           base::FilePath *result)
    {
        if (!bililive::GetDefaultUserDataDirectory(result))
            return false;

        *result = result->Append(UTF8ToWide(account_name));
        return true;
    }

	VOID SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
	{
		if (NULL == lpSystemInfo)    return;
		typedef VOID(WINAPI* LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
		LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandle(L"kernel32"), "GetNativeSystemInfo");;
		if (NULL != fnGetNativeSystemInfo)
		{
			fnGetNativeSystemInfo(lpSystemInfo);
		}
		else
		{
			GetSystemInfo(lpSystemInfo);
		}
	}
	// 获取操作系统位数  
	std::string  GetSystemBits()
	{
        std::string  system_bits_type = "x64";
		SYSTEM_INFO si;
		SafeGetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		{
            system_bits_type = "x64";
        }
        else
        {
            system_bits_type = "win32";
        }
        return system_bits_type;
	}

    base::FilePath QueryInstalledDirectoryFromRegistry()
    {
#ifdef  _WIN64
		const wchar_t kSubkey[] =
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{06BB8CDE-F4AC-4B22-A680-0F3699CB9912}_is1"; //x64
#else
		const wchar_t kSubkey[] =
			L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{81F6F736-F774-4965-A593-1AFD31ABBB35}_is1";

#endif // _WIN64

        base::win::RegKey entry(HKEY_LOCAL_MACHINE, kSubkey, KEY_READ);
        if (!entry.Valid())
        {
            NOTREACHED();
            PLOG(WARNING) << "Failed open installation registry key!";
            return {};
        }

        std::wstring installed_dir;
        auto rv = entry.ReadValue(L"InstallLocation", &installed_dir);
        if (installed_dir.empty())
        {
            NOTREACHED();
            PLOG(WARNING) << "Failed to read installed dir (Result code: " << rv << ")";
            return {};
        }

        return base::FilePath(installed_dir);
    }





}