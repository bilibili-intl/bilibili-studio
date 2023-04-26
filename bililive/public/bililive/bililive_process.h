#ifndef BILILIVE_PUBLIC_BILILIVE_BILILIVE_PROCESS_H_
#define BILILIVE_PUBLIC_BILILIVE_BILILIVE_PROCESS_H_

#include "bililive/bililive/profiles/profile.h"

class Profile;
class BililiveMainPartsImpl;
class BililiveSecret;
class BililiveOBS;
class BililiveProcess;
class BililiveDatabase;
class LivehimeThemeService;

//namespace bililive {
//
//class LocalVideoArchiveManager;
//class VideoCompressionService;
//class VideoUploadManager;
//
//}   // namespace bililive
//
//using bililive::LocalVideoArchiveManager;
//using bililive::VideoCompressionService;
//using bililive::VideoUploadManager;

// We should always prefer using this function over direct access to the global instance.
BililiveProcess* GetBililiveProcess();

class BililiveProcess {
public:
    BililiveProcess() {}

    virtual ~BililiveProcess() {}

    virtual unsigned int AddRefModule() = 0;

    virtual unsigned int ReleaseModule() = 0;

    virtual bool IsShuttingDown() = 0;

    virtual const std::string& GetApplicationLocale() = 0;

    virtual void SetApplicationLocale(const std::string& locale) = 0;

    virtual BililiveSecret* secret_core() = 0;

    virtual Profile* global_profile() = 0;

    virtual Profile* profile() = 0;

    virtual BililiveDatabase* database() const = 0;

    virtual LivehimeThemeService* ThemeService() = 0;

    virtual void set_bililive_obs(BililiveOBS* bililive_obs) = 0;

    virtual scoped_refptr<BililiveOBS> bililive_obs() = 0;


    virtual bool ExitIsLogout() = 0;

    virtual void SetLogout() = 0;

    virtual void SetTokenInvalid() = 0;

    /*virtual void set_bililive_uploader(BililiveUploader* bililive_uploader) = 0;
    virtual BililiveUploader* bililive_uploader() = 0;

    virtual VideoCompressionService* video_compression_service() const = 0;

    virtual VideoUploadManager* video_upload_manager() const = 0;

    virtual LocalVideoArchiveManager* local_video_archive_manager() const = 0;*/

    // Both are process-wide and process-boundary.
    // first_run() is reliable only since BililiveProcessImpl::PreMainMessageLoopRun(), whereas
    // updated_run() is reliable only since BililiveProcessImpl::PostBililiveStart().

    virtual bool first_run() const = 0;

    virtual bool updated_run() const = 0;

    virtual void set_last_crash_exit(bool is_crash_exit) = 0;

    virtual bool is_last_crash_exit() = 0;

    virtual bool ExitIsRestart() = 0;

    virtual void SetRestart() = 0;
};

#endif  // BILILIVE_PUBLIC_BILILIVE_BILILIVE_PROCESS_H_