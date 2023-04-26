#include "bililive/bililive/first_run/first_run_tasks.h"

#include <windows.h>

#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/win/windows_version.h"
#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"

#include "grit/generated_resources.h"

namespace {

const int kResultPassed = 0;
const int kResultNotFound = 1;
const int kResultInvalid = 2;
const int kMaxValidVersion = 49;
const int kMinValidVersion = 40;

base::FilePath GetRealSystemDirectory()
{
    auto wow64_status = base::win::OSInfo::GetInstance()->wow64_status();
    if (wow64_status != base::win::OSInfo::WOW64_ENABLED) {
        base::FilePath system_dir;
        PathService::Get(base::DIR_SYSTEM, &system_dir);
        return system_dir;
    }

    const UINT kBufSize = MAX_PATH + 1;
    wchar_t buf[kBufSize] { 0 };
    GetSystemWow64DirectoryW(buf, kBufSize);
    return base::FilePath(buf);
}

}   // namespace

namespace bililive {

int ValidateD3DComponent()
{
    bool any_found = false;
    auto sys_dir = GetRealSystemDirectory();
    for (auto ver = kMaxValidVersion; ver >= kMinValidVersion; --ver) {
        auto dll_name = base::StringPrintf("D3DCompiler_%02d.dll", ver);
        auto dll_path = sys_dir.AppendASCII(dll_name);
        if (base::PathExists(dll_path)) {
            any_found = true;
            DLOG(INFO) << dll_name << " found!";
            HMODULE module = LoadLibraryW(dll_path.value().c_str());
            if (module) {
                FreeLibrary(module);
                DLOG(INFO) << dll_name << " accepted!";
                return kResultPassed;
            }
        }
    }

    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    std::wstring dlg_title = rb.GetLocalizedString(IDS_D3D_COMPONENT_ERROR_DLG_TITLE);
    std::wstring error_msg;
    int result_code;
    if (any_found) {
        error_msg = rb.GetLocalizedString(IDS_D3D_COMPONENT_INVALID);
        result_code = kResultInvalid;
    } else {
        error_msg = rb.GetLocalizedString(IDS_D3D_COMPONENT_NOT_FOUND);
        result_code = kResultNotFound;
    }

    LOG(ERROR) << error_msg << "; error code: " << result_code;
    MessageBoxW(nullptr, error_msg.c_str(), dlg_title.c_str(), MB_OK | MB_ICONEXCLAMATION);

    return result_code;
}

}   // namespace bililive