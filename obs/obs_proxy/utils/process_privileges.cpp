#include "obs/obs_proxy/utils/process_privileges.h"

#include "base/win/scoped_handle.h"

namespace {

enum class PrivilegeOption {
    ENABLE,
    DISABLE
};

bool SetProcessSEPrivilege(HANDLE process, PrivilegeOption option)
{
    DWORD flags = TOKEN_ADJUST_PRIVILEGES;
    base::win::ScopedHandle token;
    if (!OpenProcessToken(process, flags, token.Receive()))
    {
        return false;
    }

    LUID uid;
    if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &uid))
    {
        return false;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = uid;
    tp.Privileges[0].Attributes = (option == PrivilegeOption::ENABLE) ? SE_PRIVILEGE_ENABLED : 0;
    BOOL rv = AdjustTokenPrivileges(token.Get(), FALSE, &tp, sizeof(tp), nullptr, nullptr);
    return !!rv;
}

}   // namespace

namespace obs_proxy {

bool EnableProcessSEPrivilege(HANDLE process)
{
    return SetProcessSEPrivilege(process, PrivilegeOption::ENABLE);
}

bool DisableProcessSEPrivilege(HANDLE process)
{
    return SetProcessSEPrivilege(process, PrivilegeOption::DISABLE);
}

}   // namespace obs_proxy