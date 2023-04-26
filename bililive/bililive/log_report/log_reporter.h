
#ifndef BILILIVE_BILILIVE_LOG_UPLOADER_H_
#define BILILIVE_BILILIVE_LOG_UPLOADER_H_

#include "base/memory/weak_ptr.h"

#include "bililive/secret/public/app_service.h"

namespace livehime
{
    using LogUploadCompletedHandler = std::function<void(bool, const std::string& bfs_url, const std::string& log)>;

    class LogReporter
    {
    public:
        static LogReporter* GetInstance();

        LogReporter();
        ~LogReporter();

        // 根据日期（月&&日）和时间（时分秒）提取本地日志并上传，
        // 日志包括主程序日志和CEF模块的日志
        void UploadLog(const base::Time& head, const base::Time& tail,
            LogUploadCompletedHandler reply_handler);

        // 上传程序此次运行截止当前的全部日志
        void UploadCurrentLog(LogUploadCompletedHandler handler);

    private:
        void UploadLogInternal(const base::Time& head, const base::Time& tail,
            LogUploadCompletedHandler reply_handler);
        void OnFileUpload(bool valid, int code, const secret::AppService::UploadInfo& info,
            LogUploadCompletedHandler handler, const std::string& log);

    private:

        base::WeakPtrFactory<LogReporter> weakptr_factory_;

        DISALLOW_COPY_AND_ASSIGN(LogReporter);
    };

}   // namespace

#endif
