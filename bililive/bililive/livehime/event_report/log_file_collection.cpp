#include "bililive/bililive/livehime/event_report/log_file_collection.h"

#include <Windows.h>
#include <shlobj.h>
#include <fstream>

#include "base/file_util.h"
#include "base/logging.h"
#include "base/ext/bind_lambda.h"
#include "base/files/file_enumerator.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/public/common/bililive_constants.h"

#include "third_party/zlib/google/zip.h"

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

bool LogFileFilter(const std::vector<std::wstring>& filter_strs,
    int64_t limit_len,
    int64_t limit_count,
    int64_t file_count,
    const base::FilePath& file_path)
{
    //过滤文件名
    bool name_fliter_succ = true;
    std::wstring file_name = file_path.value();
    int filter_size = filter_strs.size();
    int filter_succ_count = 0;

    if (filter_size > 0) {
        for (int i = 0; i < filter_size; ++i) {
            int ret = file_name.find(filter_strs[i]);
            if (ret >= 0) {
                filter_succ_count++;
            }
        }

        if (filter_succ_count == filter_size) {
            name_fliter_succ = true;
        }
        else {
            name_fliter_succ = false;
        }
    }

    // 过滤条件判断
    base::PlatformFileInfo info;
    if (file_util::GetFileInfo(file_path, &info)){
        if (base::Time::Now().LocalMidnight() == info.last_modified.LocalMidnight()
            &&info.size <= limit_len 
            &&file_count<= limit_count
            && name_fliter_succ) {
            return true;
        }
    }
    return false;
}

std::string GetFileModifiedTime(const base::FilePath& file_path) {
    base::PlatformFileInfo info;
    file_util::GetFileInfo(file_path, &info);

    base::Time::Exploded exploded;
    base::Time::FromTimeT(info.last_modified.ToTimeT()).LocalExplode(&exploded);

    char times_str[64] = { 0 };
    sprintf_s(times_str, "%4d-%02d-%02d %02d:%02d:%02d",
              exploded.year, exploded.month, exploded.day_of_month,
              exploded.hour, exploded.minute, exploded.second);
    return times_str;
}

std::string GetTodayLogText(const base::FilePath& src_dir,
                            const std::vector<std::wstring>& filter_strs,
                            int64_t limit_len, 
                            int64_t limit_count,
                            bool compress = false)
{
    base::ThreadRestrictions::ScopedAllowIO allow;

    std::vector<base::FilePath> files;

    base::FileEnumerator file_enum(src_dir, false, base::FileEnumerator::FILES);

    for (base::FilePath name = file_enum.Next(); !name.empty(); name = file_enum.Next()) {

        if (!LogFileFilter(filter_strs, limit_len, limit_count, files.size(), name)) {
            continue;
        }

        files.push_back(name);
    }

    //拼接文件数据
    std::string file_data  = "";
    int file_count = files.size();

    if (file_count > 0) {
        for (int i = 0; i < file_count; ++i) {
            std::string content;
            if (file_util::ReadFileToString(files[i], &content)) {
                file_data += files[i].BaseName().MaybeAsASCII();
                file_data.append("        ");
                file_data += GetFileModifiedTime(files[i]);
                file_data.append("\r\n\r\n");
                file_data += content;
                file_data.append("\r\n\r\n***********************************************************\r\n\r\n");
            }
        }
    }
    return file_data;
}

}   // namespace

namespace livehime {

std::string GetTodayAgoraLogText(bool compress/* = false*/)
{
    static const int kMaxAgoraLogFileLength = 2* 1024 * 1024;
    static const int kMaxAgoraLogFileCount = 5;

    // 遍历 AppData/Local/Agora/livehime 目录文件
    base::FilePath log_data_dir;
    if (!GetLocalAppDataDirectory(log_data_dir))
    {
        return {};
    }

    std::vector<std::wstring> filter_strs;
    filter_strs.push_back(L"agorasdk");
    filter_strs.push_back(L".log");

    log_data_dir = log_data_dir.Append(FILE_PATH_LITERAL("Agora\\livehime"));
    return GetTodayLogText(log_data_dir, filter_strs, kMaxAgoraLogFileLength, kMaxAgoraLogFileCount);
}

std::string GetTodayWebrtcLogText(bool compress/* = false*/)
{
    static const int kMaxWebrtcLogFileLength = 2* 1024 * 1024;
    static const int kMaxWebrtcLogFileCount = 30;

    // 遍历 AppData/Local/bililive/webrtc_log 目录文件
    base::FilePath log_data_dir;
    if (!GetLocalAppDataDirectory(log_data_dir))
    {
        return {};
    }

    std::vector<std::wstring> filter_strs;
    filter_strs.push_back(L"log_0");
    log_data_dir = log_data_dir.Append(FILE_PATH_LITERAL("bililive\\webrtc_log"));
    return GetTodayLogText(log_data_dir, filter_strs, kMaxWebrtcLogFileLength, kMaxWebrtcLogFileCount);
}

}   // namespace bililive