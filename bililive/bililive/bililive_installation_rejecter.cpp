#include "bililive/bililive/bililive_installation_rejecter.h"

#include "base/win/scoped_handle.h"


namespace {
    const wchar_t kInstallerMutex[] = L"{A64AEECF-DF0F-46AD-BEA5-8E674FBB1F7E}";
}

bool BililiveInstallationRejecter::Reject() {
    base::win::ScopedHandle instance_mutex;

    instance_mutex.Set(::OpenMutexW(SYNCHRONIZE, FALSE, kInstallerMutex));
    if (instance_mutex.IsValid()) {
        PLOG(WARNING) << "Installer is running!";
        return true;
    }

    if (::GetLastError() == ERROR_FILE_NOT_FOUND) {
        return false;
    }

    return false;
}