#include "bililive/common/bililive_context.h"

#include <memory>

#include <Windows.h>

#include "base/file_util.h"
#include "base/file_version_info.h"
#include "base/strings/string_util.h"
#include "base/win/scoped_handle.h"
#include "base/version.h"
#include "base/sys_info.h"
#include "base/strings/utf_string_conversions.h"
#include "bililive/public/common/bililive_constants.h"

namespace {

std::wstring MakeAbsoluteFilePath(const std::wstring& input)
{
    std::wstring path = input;
    //path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
    wchar_t file_path[MAX_PATH] = { 0 };
    if (!_wfullpath(file_path, path.c_str(), MAX_PATH))
        return std::wstring();
    return std::wstring(file_path);
}

base::FilePath GetModuleExecutablePath()
{
    const size_t kMaxPathBuf = MAX_PATH + 1;
    base::FilePath::CharType exe_path[kMaxPathBuf];
    GetModuleFileNameW(nullptr, exe_path, kMaxPathBuf);

    std::wstring abs_path = MakeAbsoluteFilePath(exe_path);
    if (!abs_path.empty())
    {
        return base::FilePath(abs_path);
    }

    return base::FilePath(exe_path);
}

base::string16 GetFileVersion(const base::FilePath& file_path)
{
    std::unique_ptr<FileVersionInfo> version_info(
        FileVersionInfo::CreateFileVersionInfo(file_path));
    if (version_info) {
        base::string16 version_string(version_info->file_version());
        if (Version(WideToASCII(version_string)).IsValid()) {
            return version_string;
        }
    }

    return base::string16();
}

// Indicates whether a file can be opened using the same flags that LoadLibrary() uses to open modules.
bool ModuleCanBeLoaded(const base::FilePath& module_path)
{
    base::win::ScopedHandle module(CreateFileW(module_path.value().c_str(),
                                               GENERIC_READ,
                                               FILE_SHARE_READ,
                                               nullptr,
                                               OPEN_EXISTING,
                                               0,
                                               nullptr));
    return module.IsValid();
}

base::FilePath GuessMainDLLPath(const base::FilePath& exe_dir, const base::string16& exe_ver)
{
    auto main_dll_path = exe_dir.Append(bililive::kBililiveMainDll);
    if (ModuleCanBeLoaded(main_dll_path)) {
        return main_dll_path;
    }

    main_dll_path = exe_dir.Append(exe_ver).Append(bililive::kBililiveMainDll);
    DCHECK(ModuleCanBeLoaded(main_dll_path));

    return main_dll_path;
}

ApplicationMode GuessApplicationMode(const base::FilePath&)
{
    return ApplicationMode::BililiveLivehime;
}

std::string GetDeviceName()
{
	unsigned long size = 255;
    wchar_t pc_name[MAX_PATH] = { 0 };
    GetComputerName(pc_name, &size);
    return base::WideToUTF8(pc_name);
}

}   // namespace

// static
BililiveContext* BililiveContext::Current()
{
    static BililiveContext instance;
    return &instance;
}

BililiveContext::BililiveContext()
    : inited_(false), app_mode_(ApplicationMode::BililiveLivehime)
{}

BililiveContext::~BililiveContext()
{}

void BililiveContext::Init()
{
    DCHECK(!inited_) << "BililiveContext can't be initialized twice!";

    exe_path_ = GetModuleExecutablePath();
    exe_ver_ = GetFileVersion(exe_path_);
    exe_dir_ = exe_path_.DirName();

    main_dll_path_ = GuessMainDLLPath(exe_dir_, exe_ver_);
    main_dir_ = main_dll_path_.DirName();

    std::string diagnose_data;
	  const char item_win_ver[] = "Windows Version: ";
	  auto os_version = base::SysInfo::OperatingSystemVersion();
	  auto os_arch = base::SysInfo::OperatingSystemArchitecture();
    diagnose_data.append(item_win_ver).append(os_version).append(" ").append(os_arch);
    device_platform_ = diagnose_data;
    device_name_ = GetDeviceName();

    app_mode_ = GuessApplicationMode(main_dir_);

    inited_ = true;
}

const base::FilePath& BililiveContext::GetExecutablePath() const
{
    DCHECK(!exe_path_.empty());
    return exe_path_;
}

const base::FilePath& BililiveContext::GetExecutableDirectory() const
{
    DCHECK(!exe_dir_.empty());
    return exe_dir_;
}

const base::FilePath& BililiveContext::GetMainDirectory() const
{
    DCHECK(!main_dir_.empty());
    return main_dir_;
}

const base::FilePath& BililiveContext::GetMainDLLPath() const
{
    DCHECK(!main_dll_path_.empty());
    return main_dll_path_;
}

const base::string16& BililiveContext::GetExecutableVersion() const
{
    DCHECK(!exe_ver_.empty());
    return exe_ver_;
}

std::string BililiveContext::GetExecutableVersionAsASCII() const
{
    return WideToASCII(exe_ver_);
}

unsigned short BililiveContext::GetExecutableBuildNumber() const
{
    base::Version ver(GetExecutableVersionAsASCII());
    const auto& components = ver.components();
    if (components.empty()) {
        return 0;
    }

    return components.back() == 0 ? 9999 : components.back();
}

ApplicationMode BililiveContext::GetApplicationMode() const
{
    return app_mode_;
}

bool BililiveContext::InApplicationMode(ApplicationMode mode) const
{
    return app_mode_ == mode;
}

std::string BililiveContext::GetDevicename() const
{
    return device_name_;
}

std::string BililiveContext::GetDevicePlatform() const
{
    return device_platform_;
}

void BililiveContext::ApplaunchStartTime() {
    app_startup_time_ = base::Time().Now();
}