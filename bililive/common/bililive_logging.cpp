#include "bililive/common/bililive_logging.h"

#include <Windows.h>
#include <shlobj.h>
#include <fstream>
#include <regex>

#include "base/file_util.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/public/common/bililive_constants.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_constants.h"

// Logging is one of the most important infrastructures, which has no doubt been widely used in the
// entire project. We need to be cautious here, to avoid having logging module unleased from the cage
// before we have done our setup.

namespace {

bool GetLocalAppDataDirectory(base::FilePath& dir_path)
{
    const size_t kBufSize = MAX_PATH + 1;
    wchar_t path_buf[kBufSize] {0};
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path_buf)))
    {
        return false;
    }

    dir_path = base::FilePath(path_buf);
    return true;
}

// Get the C:\Program Files\Common Files 
bool GetCommonFilesDirectory(base::FilePath& dir_path)
{
	const size_t kBufSize = MAX_PATH + 1;
	wchar_t path_buf[kBufSize]{ 0 };
	if (FAILED(SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES_COMMON, nullptr, SHGFP_TYPE_CURRENT, path_buf)))
	{
		return false;
	}

	dir_path = base::FilePath(path_buf);
	return true;
}

// Hope no one has ever messed up with the directory.
// we don't do error checking here, for the sake of the god.
void CreateDirectoryIfNecessary(const base::FilePath& dir_path)
{
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    if (!base::PathExists(dir_path))
    {
        CreateDirectoryW(dir_path.value().c_str(), nullptr);
    }
}

int64 g_startup_log_file_offset = 0;
int64 g_startup_cef_log_file_offset = 0;
int64 g_curday_log_file_offset = 0;
int64 g_curday_cef_log_file_offset = 0;
bool first_start = true;

}   // namespace

namespace bililive {

base::FilePath GetBililiveDirectory()
{
    base::FilePath data_dir;
    if (!GetLocalAppDataDirectory(data_dir))
    {
        return base::FilePath();
    }

    data_dir = data_dir.AppendASCII(kBililiveIntl);
    CreateDirectoryIfNecessary(data_dir);

    return data_dir;
}

base::FilePath GetBililiveUserDataDirectory()
{
    auto data_dir = GetBililiveDirectory();
    if (data_dir.empty())
    {
        return base::FilePath();
    }

    auto user_data_dir = data_dir.Append(kUserDataDirname);
    CreateDirectoryIfNecessary(user_data_dir);

    return user_data_dir;
}

base::FilePath GetBililiveLogDirectory()
{
    return GetBililiveUserDataDirectory();
}

base::FilePath GetFilesCommonDirectory()
{
    base::FilePath data_dir;
    if (!GetCommonFilesDirectory(data_dir))
    {
        return base::FilePath();
    }
 
    return data_dir;
}

void BindStandardDevices()
{
    errno_t err;
    FILE* sout;
    FILE* serr;
    err = freopen_s(&sout, "CONOUT$", "w", stdout);
    LOG_IF(WARNING, err) << "Failed to bind stdout with console; Error: " << err;
    err = freopen_s(&serr, "CONOUT$", "w", stderr);
    LOG_IF(WARNING, err) << "Failed to bind stderr with console; Error: " << err;
}

void RequestConsole()
{
    auto rv = AllocConsole();
    PLOG_IF(WARNING, !rv) << "Failed to allocate a new console for the calling process.";
}

bool ConsoleExists()
{
    return GetConsoleWindow() != nullptr;
}

void InitBililiveLogging(bool debug_mode)
{
    logging::LoggingSettings logging_settings;

    if (debug_mode) {
        logging_settings.logging_dest = logging::LoggingDestination::LOG_TO_ALL;
    }

    auto log_dir = GetBililiveLogDirectory();

    if (first_start && !debug_mode) {
        first_start = false;
        InitRetainBililiveLog(log_dir);
    }

    auto log_file_path = log_dir.Append(kBililiveLogFileName);
    logging_settings.log_file = log_file_path.value().c_str();

    logging::InitLogging(logging_settings);
    logging::SetLogItems(false, true, true, false);

    // Record the end offset of the log file when the program is running, and upload the contents
    // read from this offset to the end of the file when it crashes
    {
        std::ifstream log_in(log_file_path.value(), std::ios_base::in | std::ios_base::ate);
        if (log_in)
        {
            g_startup_log_file_offset = log_in.tellg();
        }
    }

    // Record the end offset of the log file when the program is running, and upload the contents 
    // read from this offset to the end of the file when it crashes
    {
        log_file_path = log_dir.Append(cef_proxy::kBililiveBrowserLogFileName);
        std::ifstream log_in(log_file_path.value(), std::ios_base::in | std::ios_base::ate);
        if (log_in)
        {
            g_startup_cef_log_file_offset = log_in.tellg();
        }
    }
}

void InitBililiveLogging()
{
    // Always output into the output panel in visual studio when in debug.
#if !defined(NDEBUG)
    bool debug_mode = true;
#else
    bool debug_mode = false;
#endif

    InitBililiveLogging(debug_mode);
}

int64 GetStartupLogFileOffset()
{
    return g_startup_log_file_offset;
}

int64 GetStartupCEFLogFileOffset()
{
    return g_startup_cef_log_file_offset;
}

int64 GetCurdayLogFileOffset()
{
    return g_curday_log_file_offset;
}

int64 GetCurdayCEFLogFileOffset()
{
    return g_curday_cef_log_file_offset;
}

std::string GetCurrentLogText()
{
    base::FilePath log_file = bililive::GetBililiveLogDirectory();
    log_file = log_file.Append(bililive::kBililiveLogFileName);

    std::ifstream log_in(log_file.value(), std::ios_base::in | std::ios_base::ate);
    if (log_in)
    {
        auto eof_pos = log_in.tellg();
        int64 startup_offset = bililive::GetStartupLogFileOffset();
        if (startup_offset > eof_pos)
        {
            startup_offset = 0;
        }
        int64 buffer_size = eof_pos - startup_offset;
        if (buffer_size > bililive::kMaxFileSizeAllowedToUpload)
        {
            buffer_size = bililive::kMaxFileSizeAllowedToUpload;
            startup_offset = eof_pos - buffer_size;
        }
        log_in.seekg(0, log_in.beg);
        log_in.seekg(startup_offset);
        std::string buffer(buffer_size, 0);
        log_in.read(&buffer[0], buffer_size);
        DCHECK(log_in);
        if (log_in)
        {
            return buffer;
        }
    }

    return {};
}

std::vector<std::string> GetCurrentUserLogText()
{
    base::FilePath log_file = bililive::GetBililiveLogDirectory();
    log_file = log_file.Append(bililive::kBililiveLogFileName);
    std::vector<std::string> v_curday_log;

    std::ifstream log_in(log_file.value(), std::ios_base::in | std::ios_base::ate);
    if (log_in)
    {
        auto eof_pos = log_in.tellg();
        int log_count = 0;

        // Current log size level
        log_in.seekg(0, log_in.beg);
        int64 startup_offset = 0;
        if (eof_pos - bililive::kMaxFileSizeAllowedToUpload * 4 > 0) {
            startup_offset = eof_pos - bililive::kMaxFileSizeAllowedToUpload * 4;
        }

        while (log_count<2) {
            if (startup_offset >= eof_pos) {
                break;
            }

            int64 buffer_size = eof_pos - startup_offset;
            if (buffer_size > bililive::kMaxFileSizeAllowedToUpload * 2) {
                buffer_size = bililive::kMaxFileSizeAllowedToUpload * 2;
            }

            log_in.seekg(startup_offset);
            std::string buffer(buffer_size, 0);
            log_in.read(&buffer[0], buffer_size);
            DCHECK(log_in);
            if (log_in) {
                if (!buffer.empty())
                {
                    v_curday_log.push_back(buffer);
                    log_count++;
                }
            }
            else {
                break;
            }
            startup_offset += buffer_size;
        }

        // The partial logs of bilive_debug_1 were uploaded and reserved
        if (log_count < 2) {
            base::FilePath re_log_file = bililive::GetBililiveLogDirectory();
            bool exist = false;
            re_log_file = re_log_file.Append(bililive::kBililiveLog1FileName);
            {
                base::ThreadRestrictions::ScopedAllowIO allow_io;
                if (base::PathExists(re_log_file)) {
                    exist = true;
                }
            }
            if (exist) {
                std::ifstream re_log_in(re_log_file.value(), std::ios_base::in | std::ios_base::ate);
                if (re_log_in) {
                    auto eof_pos = re_log_in.tellg();

                    int64 buffer_size = bililive::kMaxFileSizeAllowedToUpload * 2;
                    startup_offset = eof_pos - bililive::kMaxFileSizeAllowedToUpload * 2;
                    if (startup_offset < 0) {
                        startup_offset = 0;
                        buffer_size = eof_pos;
                    }

                    re_log_in.seekg(startup_offset);
                    std::string buffer(buffer_size, 0);
                    re_log_in.read(&buffer[0], buffer_size);
                    DCHECK(re_log_in);
                    if (re_log_in) {
                        if (!buffer.empty())
                        {
                            v_curday_log.insert(v_curday_log.begin(), buffer);
                        }
                    }
                }
            }
        }
    }

    return v_curday_log;
}

std::string GetCurrentCEFLogText()
{
    base::FilePath log_file = bililive::GetBililiveLogDirectory();
    log_file = log_file.Append(cef_proxy::kBililiveBrowserLogFileName);

    std::ifstream log_in(log_file.value(), std::ios_base::in | std::ios_base::ate);
    if (log_in)
    {
        auto eof_pos = log_in.tellg();
        int64 startup_offset = bililive::GetStartupCEFLogFileOffset();
        if (startup_offset > eof_pos)
        {
            startup_offset = 0;
        }

        int64 buffer_size = eof_pos - startup_offset;
        if (buffer_size > bililive::kMaxFileSizeAllowedToUpload)
        {
            buffer_size = bililive::kMaxFileSizeAllowedToUpload;
            startup_offset = eof_pos - buffer_size;
        }

        log_in.seekg(0, log_in.beg);
        log_in.seekg(startup_offset);
        std::string buffer(buffer_size, 0);
        log_in.read(&buffer[0], buffer_size);
        DCHECK(log_in);
        if (log_in)
        {
            return buffer;
        }
    }

    return {};
}

std::vector<std::string> GetCurrentDayLogText()
{
    base::FilePath log_file = bililive::GetBililiveLogDirectory();
    log_file = log_file.Append(bililive::kBililiveLogFileName);
    std::vector<std::string> v_curday_log;

    std::ifstream log_in(log_file.value(), std::ios_base::in | std::ios_base::ate);
    if (log_in)
    {
        auto eof_pos = log_in.tellg();
        int64 startup_offset = bililive::GetCurdayLogFileOffset();
        if (startup_offset > eof_pos){
            startup_offset = 0;
        }

        log_in.seekg(0, log_in.beg);
        bool out_max = true;
        while (out_max) {
            int64 buffer_size = eof_pos - startup_offset;
            if (buffer_size > bililive::kMaxFileSizeAllowedToUpload){
                buffer_size = bililive::kMaxFileSizeAllowedToUpload;
            }
            else {
                out_max = false;
            }

            log_in.seekg(startup_offset);
            std::string buffer(buffer_size, 0);
            log_in.read(&buffer[0], buffer_size);
            DCHECK(log_in);
            if (log_in) {
                if (!buffer.empty())
                {
                    v_curday_log.push_back(buffer);
                }
            }

            startup_offset += buffer_size;
        }
    }

    return v_curday_log;
}

std::vector<std::string> GetCurrentDayCEFLogText()
{
    base::FilePath log_file = bililive::GetBililiveLogDirectory();
    log_file = log_file.Append(cef_proxy::kBililiveBrowserLogFileName);
    std::vector<std::string> v_curday_log;

    std::ifstream log_in(log_file.value(), std::ios_base::in | std::ios_base::ate);
    if (log_in)
    {
        auto eof_pos = log_in.tellg();
        int64 startup_offset = bililive::GetCurdayCEFLogFileOffset();
        if (startup_offset > eof_pos) {
            startup_offset = 0;
        }

        log_in.seekg(0, log_in.beg);
        bool out_max = true;
        while (out_max) {
            int64 buffer_size = eof_pos - startup_offset;
            if (buffer_size > bililive::kMaxFileSizeAllowedToUpload) {
                buffer_size = bililive::kMaxFileSizeAllowedToUpload;
            }
            else {
                out_max = false;
            }

            log_in.seekg(startup_offset);
            std::string buffer(buffer_size, 0);
            log_in.read(&buffer[0], buffer_size);
            DCHECK(log_in);
            if (log_in) {
                if (!buffer.empty())
                {
                    v_curday_log.push_back(buffer);
                }
            }

            startup_offset += buffer_size;
        }
    }

    return v_curday_log;
}

void InitRetainBililiveLog(const base::FilePath& log_path)
{
    auto log_dir = log_path;
    auto log_file_path = log_dir.Append(kBililiveLogFileName);
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    if (base::PathExists(log_file_path)) {
        // Record the end offset of the log file when the program is running, and upload the contents
        // read from this offset to the end of the file when it crashes
        {
            std::ifstream log_in(log_file_path.value(), std::ios_base::in | std::ios_base::ate);
            if (log_in)
            {
                g_startup_log_file_offset = log_in.tellg();
            }
        }

        if (g_startup_log_file_offset > bililive::kMaxFileSizeAllowedToUpload * 4) {
            auto retain_log_file_1 = log_dir.Append(kBililiveLog1FileName);
            auto retain_log_file_2 = log_dir.Append(kBililiveLog2FileName);
            bool retain_1 = false;
            base::ThreadRestrictions::ScopedAllowIO allow_io;
            if (base::PathExists(retain_log_file_1)) {
                retain_1 = true;
            }
            if (retain_1) {
                // If standby 2 exists, delete it
                if (base::PathExists(retain_log_file_2)) {
                    base::DeleteFileW(retain_log_file_2, true);
                }

                // Standby 1 was renamed Standby 2
                int ret = std::rename(retain_log_file_1.AsUTF8Unsafe().c_str(), retain_log_file_2.AsUTF8Unsafe().c_str());
                if (ret != 0) {
                    DCHECK(ret);
                }
            }

            // Change the current log to standby 1
            int ret = std::rename(log_file_path.AsUTF8Unsafe().c_str(), retain_log_file_1.AsUTF8Unsafe().c_str());
            if (ret != 0) {
                DCHECK(ret);
            }
        }
    }
}

void InitCurrentBililiveLogging(const base::FilePath& log_record_path) {
    std::string old_date;
    int64_t bilive_offset = 0;
    int64_t browser_offset = 0;
    std::ifstream off_in(log_record_path.value(), std::ios_base::in | std::ios_base::ate);
    if (off_in)
    {
        auto eof_pos = off_in.tellg();
        int length = eof_pos;
        std::string buffer(length, 0);
        off_in.seekg(0, off_in.beg);
        off_in.read(&buffer[0], length);

        std::vector<std::string> parts;
        if (buffer.length() > 0) {
            base::SplitString(buffer, ';', &parts);
        }

        if (parts.size() > 2) {
            old_date = parts.at(0);
            static std::regex expression(R"((\d+)(?:\((.+)\))?)");
            if (std::regex_match(parts.at(1), expression)) {
                base::StringToInt64(parts.at(1), &bilive_offset);
            }
            if (std::regex_match(parts.at(2), expression)) {
                base::StringToInt64(parts.at(2), &browser_offset);
            }
        }
    }

    auto now = base::Time::Now();
    base::Time::Exploded exploded;
    now.LocalExplode(&exploded);
    std::string cur_date = base::StringPrintf("%d%02d%02d",
        exploded.year,
        exploded.month,
        exploded.day_of_month);

    if (cur_date != old_date) {
        cur_date = cur_date.append(";").append(std::to_string(g_startup_log_file_offset)).append(";").append(std::to_string(g_startup_cef_log_file_offset));

        std::ofstream out(log_record_path.value(), std::ios::out | std::ios::trunc);
        if (out.is_open()) {
            out << cur_date << " ";
            out.close();
        }
    }
    else {
        g_curday_log_file_offset = (bilive_offset != 0 && bilive_offset < g_startup_log_file_offset) ? bilive_offset : g_startup_log_file_offset;
        g_curday_cef_log_file_offset = (browser_offset != 0 && browser_offset < g_startup_cef_log_file_offset) ? browser_offset : g_startup_cef_log_file_offset;
    }
}

}   // namespace bililive