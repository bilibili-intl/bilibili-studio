// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/win/windows_version.h"

#include <windows.h>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"

// bililive {

#define CONCATENATE_IMPL(part1, part2) part1##part2
#define CONCATENATE(part1, part2) CONCATENATE_IMPL(part1, part2)
#define ANONYMOUS_VAR(tag) CONCATENATE(tag, __LINE__)

#define DECLARE_DLL_FUNCTION(fn, type, dll)                     \
    auto ANONYMOUS_VAR(_module_) = GetModuleHandleW(L##dll);    \
    auto fn = ANONYMOUS_VAR(_module_) ?                         \
                reinterpret_cast<type>(GetProcAddress(ANONYMOUS_VAR(_module_), #fn)) : nullptr

namespace {

using base::win::Version;

using ServicePack = base::win::OSInfo::ServicePack;
using VersionNumber = base::win::OSInfo::VersionNumber;

Version IdentifySystemVersion(const VersionNumber& version_number)
{
    Version version = Version::VERSION_PRE_XP;

    if ((version_number.major == 5) && (version_number.minor > 0)) {
        // Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
        version = (version_number.minor == 1) ? Version::VERSION_XP : Version::VERSION_SERVER_2003;
    } else if (version_number.major == 6) {
        switch (version_number.minor) {
            case 0:
                // Treat Windows Server 2008 the same as Windows Vista.
                version = Version::VERSION_VISTA;
                break;
            case 1:
                // Treat Windows Server 2008 R2 the same as Windows 7.
                version = Version::VERSION_WIN7;
                break;
            case 2:
                // Treat Windows Server 2012 the same as Windows 8.
                version = Version::VERSION_WIN8;
                break;
            case 3:
                // Treat Windows Server 2012 R2 the same as Windows 8.1.
                version = Version::VERSION_WIN8_1;
                break;
            default:
                NOTREACHED();
                version = Version::VERSION_WIN_LAST;
                break;
        }
    } else if (version_number.major == 10) {
        version = Version::VERSION_WIN10;
    } else {
        NOTREACHED();
        version = Version::VERSION_WIN_LAST;
    }

    return version;
}

// We use conventional API as a fallback.
Version GetSystemVersionFallback(VersionNumber* version_number, ServicePack* service_pack, bool* server_build)
{
    OSVERSIONINFOEX version_info {sizeof version_info};
    GetVersionExW(reinterpret_cast<OSVERSIONINFO*>(&version_info));

    version_number->major = version_info.dwMajorVersion;
    version_number->minor = version_info.dwMinorVersion;
    version_number->build = version_info.dwBuildNumber;

    service_pack->major = version_info.wServicePackMajor;
    service_pack->minor = version_info.wServicePackMinor;

    *server_build = version_info.wProductType == VER_NT_SERVER;

    return IdentifySystemVersion(*version_number);
}

Version GetSystemVersion(VersionNumber* version_number, ServicePack* service_pack, bool* server_build)
{
    typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
    const NTSTATUS kStatusSuccess = static_cast<NTSTATUS>(0L);
    DECLARE_DLL_FUNCTION(RtlGetVersion, NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW), "ntdll.dll");

    if (!RtlGetVersion) {
        return GetSystemVersionFallback(version_number, service_pack, server_build);
    }

    RTL_OSVERSIONINFOEXW ovi {sizeof(ovi)};
    if (RtlGetVersion(reinterpret_cast<RTL_OSVERSIONINFOW*>(&ovi)) != kStatusSuccess) {
        return GetSystemVersionFallback(version_number, service_pack, server_build);
    }

    version_number->major = ovi.dwMajorVersion;
    version_number->minor = ovi.dwMinorVersion;
    version_number->build = ovi.dwBuildNumber;

    service_pack->major = ovi.wServicePackMajor;
    service_pack->minor = ovi.wServicePackMinor;

    *server_build = ovi.wProductType == VER_NT_SERVER;

    return IdentifySystemVersion(*version_number);
}

}   // namespace

// } bililive

namespace base {
namespace win {

// static
OSInfo* OSInfo::GetInstance() {
  // Note: we don't use the Singleton class because it depends on AtExitManager,
  // and it's convenient for other modules to use this classs without it. This
  // pattern is copied from gurl.cc.
  static OSInfo* info;
  if (!info) {
    OSInfo* new_info = new OSInfo();
    if (InterlockedCompareExchangePointer(
        reinterpret_cast<PVOID*>(&info), new_info, NULL)) {
      delete new_info;
    }
  }
  return info;
}

OSInfo::OSInfo()
    : version_(VERSION_PRE_XP),
      architecture_(OTHER_ARCHITECTURE),
      wow64_status_(GetWOW64StatusForProcess(GetCurrentProcess())) {
  version_ = GetSystemVersion(&version_number_, &service_pack_, &is_server_);

  SYSTEM_INFO system_info = { 0 };
  GetNativeSystemInfo(&system_info);
  switch (system_info.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL: architecture_ = X86_ARCHITECTURE; break;
    case PROCESSOR_ARCHITECTURE_AMD64: architecture_ = X64_ARCHITECTURE; break;
    case PROCESSOR_ARCHITECTURE_IA64:  architecture_ = IA64_ARCHITECTURE; break;
    default: NOTREACHED();
  }
  processors_ = system_info.dwNumberOfProcessors;
  allocation_granularity_ = system_info.dwAllocationGranularity;
}

OSInfo::~OSInfo() {
}

std::string OSInfo::processor_model_name() {
  if (processor_model_name_.empty()) {
    const wchar_t kProcessorNameString[] =
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
    base::win::RegKey key(HKEY_LOCAL_MACHINE, kProcessorNameString, KEY_READ);
    string16 value;
    key.ReadValue(L"ProcessorNameString", &value);
    processor_model_name_ = UTF16ToUTF8(value);
  }
  return processor_model_name_;
}

// static
OSInfo::WOW64Status OSInfo::GetWOW64StatusForProcess(HANDLE process_handle) {
  typedef BOOL (WINAPI* IsWow64ProcessFunc)(HANDLE, PBOOL);
  IsWow64ProcessFunc is_wow64_process = reinterpret_cast<IsWow64ProcessFunc>(
      GetProcAddress(GetModuleHandle(L"kernel32.dll"), "IsWow64Process"));
  if (!is_wow64_process)
    return WOW64_DISABLED;
  BOOL is_wow64 = FALSE;
  if (!(*is_wow64_process)(process_handle, &is_wow64))
    return WOW64_UNKNOWN;
  return is_wow64 ? WOW64_ENABLED : WOW64_DISABLED;
}

Version GetVersion() {
  return OSInfo::GetInstance()->version();
}

}  // namespace win
}  // namespace base
