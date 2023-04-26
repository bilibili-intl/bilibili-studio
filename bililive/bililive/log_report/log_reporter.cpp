
#include "bililive/bililive/log_report/log_reporter.h"

#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"

//#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/secret/bililive_secret.h"

namespace
{
    //const char kFallbackFileUrl[] = "n/a";

    livehime::LogReporter* g_single_instance = nullptr;
        
}   // namespace

namespace livehime
{
    LogReporter* LogReporter::GetInstance()
    {
        return g_single_instance;
    }

    LogReporter::LogReporter()
        : weakptr_factory_(this)
    {
        g_single_instance = this;
    }

    LogReporter::~LogReporter()
    {
        g_single_instance = nullptr;
    }

    void LogReporter::UploadCurrentLog(LogUploadCompletedHandler reply_handler)
    {
        UploadLog(base::Time(), base::Time(), reply_handler);
    }

    void LogReporter::UploadLog(const base::Time& head, const base::Time& tail,
        LogUploadCompletedHandler reply_handler)
    {
        BililiveThread::PostTask(BililiveThread::FILE,
            FROM_HERE,
            base::Bind(&LogReporter::UploadLogInternal, weakptr_factory_.GetWeakPtr(),
                head, tail, reply_handler));
    }

    void LogReporter::UploadLogInternal(const base::Time& head, const base::Time& tail,
        LogUploadCompletedHandler reply_handler)
    {
        std::string log;

        if (head.is_null() && tail.is_null())
        {
            log = bililive::GetCurrentLogText();
            std::string cef_log = bililive::GetCurrentCEFLogText();
            log.append("\n" + std::string(bililive::kLogBoundary) + "\n");
            log.append(cef_log);
        }
        else
        {
            // 按起止时间截取日志，过后再做
            //////////////////////////////////////////////////////////////////////////
        }

        if (!log.empty())
        {

        }
        else
        {
            if (reply_handler)
            {
                reply_handler(true, "", log);
            }
        }
    }

    void LogReporter::OnFileUpload(bool valid, int code, const secret::AppService::UploadInfo& info,
        LogUploadCompletedHandler handler, const std::string& log)
    {
        if (handler)
        {
            handler(valid && (code == 0), info.url, log);
        }
    }

}
