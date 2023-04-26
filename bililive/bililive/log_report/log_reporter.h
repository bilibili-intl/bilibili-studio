
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

        // �������ڣ���&&�գ���ʱ�䣨ʱ���룩��ȡ������־���ϴ���
        // ��־������������־��CEFģ�����־
        void UploadLog(const base::Time& head, const base::Time& tail,
            LogUploadCompletedHandler reply_handler);

        // �ϴ�����˴����н�ֹ��ǰ��ȫ����־
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
