#include "bililive/app/main_dll_loader.h"

#include <memory>

#include <windows.h>
#include <shlwapi.h>

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "base/strings/string_util.h"

#include "bililive/common/bililive_context.h"

namespace {

using DLL_MAIN = int (*)(HINSTANCE);

HMODULE LoadBililiveMainDLL(const base::FilePath& main_dll_path)
{
    return ::LoadLibraryExW(main_dll_path.value().c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
}

void AddMainDirectoryIntoPathEnv(const base::FilePath& main_directory)
{
    std::unique_ptr<base::Environment> env(base::Environment::Create());
    std::string path_env;
    bool succeeded = env->GetVar("path", &path_env);
    if (!succeeded) {
        PLOG(WARNING) << "Failed to read path env";
        return;
    }

    std::string patched_path_env = main_directory.AsUTF8Unsafe();
    patched_path_env.append(";").append(path_env);
    succeeded = env->SetVar("path", patched_path_env);
    PLOG_IF(WARNING, !succeeded) << "Failed to update path env";
}

} // namespace

MainDllLoader::MainDllLoader()
    : dll_(nullptr)
{}

MainDllLoader::~MainDllLoader()
{}

HMODULE MainDllLoader::Load(base::FilePath* module_dir)
{
    auto dll_path = BililiveContext::Current()->GetMainDLLPath();
    *module_dir = dll_path.DirName();
    HMODULE dll = LoadBililiveMainDLL(dll_path);
    if (!dll) {
        PLOG(ERROR) << "Failed to load bililive main dll from " << dll_path.AsUTF8Unsafe();
        return nullptr;
    }

    return dll;
}

int MainDllLoader::Launch(HINSTANCE instance)
{
    base::FilePath main_dir;
    dll_ = Load(&main_dir);
    if (!dll_) {
        return 0;
    }

    // Make main directory as current directory to ensure all dependency dlls being loaded successfully.
    DCHECK(!main_dir.empty());
    SetCurrentDirectoryW(main_dir.value().c_str());

    // In some as-yet unknown circumstances, SetCurrentDirectory() still failed to guarantee all
    // dependency dlls being loaded successfully, especially for those who are imported implicitly.
    // Add main directory into PATH environment variable as the last resort.
    AddMainDirectoryIntoPathEnv(main_dir);

    OnBeforeLaunch(main_dir);

    DLL_MAIN entry_point = reinterpret_cast<DLL_MAIN>(::GetProcAddress(dll_, "BililiveMain"));
    if (!entry_point) {
        return 0;
    }

    int rc = entry_point(instance);

    return OnBeforeExit(rc, main_dir);
}

void MainDllLoader::OnBeforeLaunch(const base::FilePath& main_dir)
{}

int MainDllLoader::OnBeforeExit(int return_code, const base::FilePath& main_dir)
{
    return return_code;
}

// -*- BililiveDllLoader -*-

class BililiveDllLoader
    : public MainDllLoader {
public:
    string16 GetRegistryPath() override
    {
        return string16();
    }

    void OnBeforeLaunch(const base::FilePath& main_dir) override
    {}

    int OnBeforeExit(int return_code, const base::FilePath& main_dir) override
    {
        return 0;
    }
};

MainDllLoader *MakeMainDllLoader()
{
    return new BililiveDllLoader();
}