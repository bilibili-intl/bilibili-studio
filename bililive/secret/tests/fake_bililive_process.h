/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_TESTS_FAKE_BILILIVE_PROCESS_H_
#define BILILIVE_SECRET_TESTS_FAKE_BILILIVE_PROCESS_H_

#include "base/logging.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/public/bililive/bililive_process.h"

class FakeBililiveProcess : public BililiveProcess {
public:
    FakeBililiveProcess();

    ~FakeBililiveProcess();

    // Override BililiveProcess

    // We don't pass this fake process instance across modules.

    unsigned AddRefModule() override
    {
        NOTIMPLEMENTED();
        return 0;
    }

    unsigned ReleaseModule() override
    {
        NOTIMPLEMENTED();
        return 0;
    }

    bool IsShuttingDown() override
    {
        NOTIMPLEMENTED();
        return false;
    }

    const std::string& GetApplicationLocale() override;

    void SetApplicationLocale(const std::string& locale) override;

    BililiveSecret* secret_core() override
    {
        DCHECK(secret_);
        return secret_;
    }

    void set_secret_core(BililiveSecret* secret)
    {
        secret_ = secret;
    }

    Profile* global_profile() override
    {
        NOTIMPLEMENTED();
        return nullptr;
    }

    Profile* profile() override
    {
        NOTIMPLEMENTED();
        return nullptr;
    }

    BililiveDatabase* database() const override
    {
        NOTIMPLEMENTED();
        return nullptr;
    }

    void set_bililive_obs(BililiveOBS*) override
    {
        NOTIMPLEMENTED();
    }

    scoped_refptr<BililiveOBS> bililive_obs() override
    {
        NOTIMPLEMENTED();
        return nullptr;
    }

    LivehimeThemeService* ThemeService() override
    {
        NOTIMPLEMENTED();
        return nullptr;
    }

    bool ExitIsLogout() override
    {
        return exit_as_logout_;
    }

    void SetLogout() override
    {
        exit_as_logout_ = true;
    }

    void SetTokenInvalid() override
    {
        NOTIMPLEMENTED();
    }
    bool first_run() const override
    {
        return first_run_;
    }

    bool updated_run() const override
    {
        return updated_run_;
    }

    void set_first_run(bool first_run)
    {
        first_run_ = first_run;
    }

    void set_updated_run(bool updated_run)
    {
        updated_run_ = updated_run;
    }

private:
    bool first_run_;
    bool updated_run_;
    bool exit_as_logout_;
    BililiveSecret* secret_;
};

#endif  // BILILIVE_SECRET_TESTS_FAKE_BILILIVE_PROCESS_H_
