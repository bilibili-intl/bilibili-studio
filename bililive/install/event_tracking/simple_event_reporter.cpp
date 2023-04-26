/*
 @ 0xCCCCCCCC
*/

#include "bililive/install/event_tracking/simple_event_reporter.h"

#include <Windows.h>

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/guid.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/win/windows_version.h"

#include "bililive/install/event_tracking/http_request.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/common/bililive_context.h"

namespace {

using bililive::EventReporter;
using bililive::HttpRequest;

const char kURLGetBuvid[] = "";
const char kURLLancerDataCenter[] = "";

const wchar_t kCredentialFileName[] = L"ugc_credential";
const char kUGCTaskID[] = "000073";
const char kUGCProid[] = "1";
const char kUGCPid[] = "1";

const std::map<EventReporter::Event, std::pair<std::string, std::string>> kBehaviorEventTable {
    {EventReporter::Install, {"base", "install_done"}},
    {EventReporter::Uninstall, {"base", "unistall_done"}},
};

std::string GetBuvid(std::chrono::seconds timeout)
{
    int response_code = 0;
    std::string response_data;

    bool rv = HttpRequest::Send(kURLGetBuvid, HttpRequest::Get, "", "", timeout, &response_data,
                                &response_code);

    if (rv && !response_data.empty()) {
        return response_data;
    }

    return std::string();
}

std::string Now()
{
    auto now = base::Time::Now();
    base::Time::Exploded exploded;
    now.LocalExplode(&exploded);

    return base::StringPrintf("%d%02d%02d%02d%02d%02d",
                              exploded.year,
                              exploded.month,
                              exploded.day_of_month,
                              exploded.hour,
                              exploded.minute,
                              exploded.second);
}

std::string GenerateBudid()
{
    std::string guid = base::GenerateGUID();
    std::string now_date = Now();
    return base::StringPrintf("%s-%s-local", guid.c_str(), now_date.c_str());
}

base::FilePath GetCredentialSaveDir()
{
    base::FilePath local_app_data;
    if (!PathService::Get(base::DIR_LOCAL_APP_DATA, &local_app_data)) {
        return base::FilePath();
    }

    auto save_dir = local_app_data.AppendASCII("bililive\\User Data\\Global");
    if (!base::PathExists(save_dir)) {
        file_util::CreateDirectory(save_dir);
    }

    return save_dir;
}

std::string GetUnixTimestamp()
{
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    return std::to_string(timestamp);
}

std::string GetOSVerName()
{
    using namespace base;
    auto version = win::GetVersion();

    switch (version) {
    case win::Version::VERSION_VISTA:
        return "Windows vista";
    case win::Version::VERSION_WIN7:
        return "Windows 7";
    case win::Version::VERSION_WIN8:
        return "Windows 8";
    case win::Version::VERSION_WIN8_1:
        return "Windows 8.1";
    case win::Version::VERSION_WIN10:
        return "Windows 10";
    default:
        return "unknown version";
    }
}

std::string FormatUGCEventCookie(const std::string& buvid, const std::string& budid)
{
    return base::StringPrintf("Cookie: DedeUserID=0; buvid3=%s; budid=%s\r\n",
                              buvid.c_str(), budid.c_str());
}

std::string FormatUGCEventParams(EventReporter::Event event_id)
{
    std::string content;
    content.append(kUGCTaskID).append(GetUnixTimestamp());
    auto event_info = kBehaviorEventTable.at(event_id);
    std::string fields = JoinString({"0",
                                     kUGCProid,
                                     kUGCPid,
                                     GetOSVerName(),
                                     "0",
                                     BililiveContext::Current()->GetExecutableVersionAsASCII(),
                                     "",
                                     event_info.first,
                                     event_info.second,
                                     "",
                                     ""},
                                    "|");

    content.append(fields);

    return content;
}

}   // namespace

namespace bililive {

EventReporter::EventReporter()
{
    auto cmdline = CommandLine::ForCurrentProcess();
    std::string timeout_str = cmdline->GetSwitchValueASCII(kSwitchReportTimeout);
    int num = 0;
    if (base::StringToInt(timeout_str, &num)) {
        timeout_ = std::chrono::seconds(num);
    }
}

void EventReporter::InitializeCredential()
{
    auto buvid = GetBuvid(timeout_);
    if (buvid.empty()) {
        LOG(WARNING) << "Failed to get buvid from server! Making a user-genearted one for compensation!";
        buvid_ = base::StringPrintf("%s%duser", base::GenerateGUID().c_str(), GetCurrentProcessId());
    } else {
        buvid_ = buvid;
    }

    budid_ = GenerateBudid();

    auto save_dir = GetCredentialSaveDir();
    auto credential_path = save_dir.Append(kCredentialFileName);
    auto credential_data = buvid_ + "\n" + budid_;
    file_util::WriteFile(credential_path, credential_data.data(), static_cast<int>(credential_data.size()));
}

bool EventReporter::RestoreCredential()
{
    auto save_dir = GetCredentialSaveDir();
    auto credential_path = save_dir.Append(kCredentialFileName);
    if (credential_path.ReferencesParent()) {
        credential_path = base::MakeAbsoluteFilePath(credential_path);
    }

    std::string credential_data;
    if (base::PathExists(credential_path) &&
        file_util::ReadFileToString(credential_path, &credential_data)) {
        std::vector<std::string> ids;
        base::SplitString(credential_data, '\n', &ids);
        if (ids.size() >= 2) {
            buvid_ = ids[0];
            budid_ = ids[1];
            return true;
        }
    }

    return false;
}

EventReporter& EventReporter::GetInstance()
{
    static EventReporter instance;
    return instance;
}

void EventReporter::Report(Event event_id) const
{
    auto cookie = FormatUGCEventCookie(buvid_, budid_);
    auto params = FormatUGCEventParams(event_id);

    int response_code = 0;
    bool rv = HttpRequest::Send(kURLLancerDataCenter, HttpRequest::Get, cookie, params, timeout_, nullptr,
                                &response_code);
    DCHECK(rv && response_code == 200);
}

}   // namespace bililive
