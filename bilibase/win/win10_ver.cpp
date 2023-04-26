#include "win10_ver.h"

#include <Windows.h>
#include <VersionHelpers.h>


namespace bilibase {
namespace win {

    bool IsWin10Ver(DWORD build, BYTE condition) {
        OSVERSIONINFOEXW osi;
        osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        osi.dwBuildNumber = build;
        osi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN10);
        osi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN10);
        osi.wServicePackMajor = 0;
        osi.wServicePackMinor = 0;

        auto condition_mask = VerSetConditionMask(
            VerSetConditionMask(
                VerSetConditionMask(
                    VerSetConditionMask(
                        VerSetConditionMask(0, VER_BUILDNUMBER, condition),
                        VER_MAJORVERSION, condition),
                    VER_MINORVERSION, condition),
                VER_SERVICEPACKMAJOR, condition),
            VER_SERVICEPACKMINOR, condition);

        BOOL result = VerifyVersionInfoW(
            &osi,
            VER_BUILDNUMBER |
            VER_MAJORVERSION |
            VER_MINORVERSION |
            VER_SERVICEPACKMAJOR |
            VER_SERVICEPACKMINOR,
            condition_mask);

        return (result != FALSE);
    }

    bool IsWindows10V2004OrGreater() {
        return IsWin10Ver(19041, VER_GREATER_EQUAL);
    }

    bool IsWindowsVersionOrGreater(DWORD build)
    {
        return IsWin10Ver(build, VER_GREATER_EQUAL);
    }
}
}