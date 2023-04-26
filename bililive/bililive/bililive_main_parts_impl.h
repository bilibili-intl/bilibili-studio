#ifndef BILILIVE_BILILIVE_BILILIVE_MAIN_PARTS_IMPL_H_
#define BILILIVE_BILILIVE_BILILIVE_MAIN_PARTS_IMPL_H_

#include <memory>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/ui/startup/startup_bililive_creator.h"
#include "bililive/public/bililive/bililive_main_parts.h"
#include "bililive/public/bililive/bililive_main_extra_parts.h"

namespace bililive {


}   // namespace bililive

class BililiveMainPartsImpl
    : public BililiveMainParts {
public:
    BililiveMainPartsImpl();

    virtual ~BililiveMainPartsImpl();

    void AddParts(BililiveMainExtraParts* parts) override;

    void PreEarlyInitialization() override;

    void PostEarlyInitialization() override;

    void ToolkitInitialized() override;

    void PreMainMessageLoopStart() override;

    void PostMainMessageLoopStart() override;

    int PreCreateThreads() override;

    void PreMainMessageLoopRun() override;

    bool MainMessageLoopRun(int* looper_result_code) override;

    void PostMainMessageLoopRun() override;

    void PostDestroyThreads() override;

private:
    int PreCreateThreadsImpl();

    int PreMainMessageLoopRunImpl();

    virtual void PreProfileInit();

    virtual void PostProfileInit();

    virtual void PreBililiveStart();

    virtual void PostBililiveStart();

    const CommandLine& parsed_command_line() const
    {
        return parsed_command_line_;
    }

    const PrefService* local_state() const
    {
        return local_state_;
    }

private:
    const CommandLine& parsed_command_line_;
    int result_code_;

    std::vector<BililiveMainExtraParts*> bililive_extra_parts_;
    scoped_ptr<BililiveProcessImpl> bililive_process_;
    scoped_ptr<StartupBililiveCreator> bililive_creator_;

    Profile* profile_;
    bool run_message_loop_;
    bool bililive_started_;

    PrefService* local_state_;
    base::FilePath user_data_dir_;

    DISALLOW_COPY_AND_ASSIGN(BililiveMainPartsImpl);
};

#endif  // BILILIVE_BILILIVE_BILILIVE_MAIN_PARTS_IMPL_H_