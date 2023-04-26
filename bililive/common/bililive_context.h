#ifndef BILILIVE_COMMON_BILILIVE_CONTEXT_H_
#define BILILIVE_COMMON_BILILIVE_CONTEXT_H_

#include "base/files/file_path.h"
#include "base/strings/string16.h"
#include "base/time/time.h"

enum class ApplicationMode {
    BililiveLivehime
};

class BililiveContext {
public:
    static BililiveContext* Current();

    void Init();

    const base::FilePath& GetExecutablePath() const;

    const base::FilePath& GetExecutableDirectory() const;

    // Main directory is the one that contains main dll.
    const base::FilePath& GetMainDirectory() const;

    const base::FilePath& GetMainDLLPath() const;

    const base::string16& GetExecutableVersion() const;

    std::string GetExecutableVersionAsASCII() const;

    unsigned short GetExecutableBuildNumber() const;

    ApplicationMode GetApplicationMode() const;

    bool InApplicationMode(ApplicationMode mode) const;

    std::string GetDevicePlatform() const;

    std::string GetDevicename() const;

    void ApplaunchStartTime();
    
    base::Time GetApplaunchStartTime(){
        return app_startup_time_;
    }

private:
    BililiveContext();

    ~BililiveContext();

    DISALLOW_COPY_AND_ASSIGN(BililiveContext);

private:
    bool inited_;
    ApplicationMode app_mode_;
    base::FilePath exe_path_;
    base::FilePath exe_dir_;
    string16 exe_ver_;
    base::FilePath main_dir_;
    base::FilePath main_dll_path_;
    std::string device_platform_;
    std::string device_name_;
    base::Time app_startup_time_;
};

#endif  // BILILIVE_COMMON_BILILIVE_CONTEXT_H_
