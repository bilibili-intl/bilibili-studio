#ifndef BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_CONTROLLER_H_
#define BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_CONTROLLER_H_

#include "bililive/bililive/command_updater.h"
#include "bililive/bililive/command_updater_delegate.h"

class BililiveCommandReceiver;

namespace bililive {

class BililiveCommandController
    : public CommandUpdaterDelegate {
public:
    explicit BililiveCommandController(BililiveCommandReceiver* bililive_commands_receiver);

    virtual ~BililiveCommandController();

    CommandUpdater *command_updater()
    {
        return &command_updater_;
    }

private:
    void InitCommandState();

    void InitLiveHimeIntlCommands();

    // Override CommandUpdaterDelegate.
    void ExecuteCommandWithParams(int id, const CommandParamsDetails &params) override;

private:
    BililiveCommandReceiver* commands_receiver_;
    CommandUpdater command_updater_;

    DISALLOW_COPY_AND_ASSIGN(BililiveCommandController);
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_CONTROLLER_H_
