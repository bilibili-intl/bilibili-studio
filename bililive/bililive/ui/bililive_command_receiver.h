#ifndef BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_RECEIVER_H_
#define BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_RECEIVER_H_

#include <memory>

#include "base/basictypes.h"

#include "bililive/bililive/ui/bililive_command_controller.h"

class BililiveCommandReceiver {
public:
    BililiveCommandReceiver();

    virtual ~BililiveCommandReceiver();

    bililive::BililiveCommandController* command_controller() const
    {
        return command_controller_.get();
    }

private:
    std::unique_ptr<bililive::BililiveCommandController> command_controller_;

    DISALLOW_COPY_AND_ASSIGN(BililiveCommandReceiver);
};

#endif  // BILILIVE_BILILIVE_UI_BILILIVE_COMMAND_RECEIVER_H_
