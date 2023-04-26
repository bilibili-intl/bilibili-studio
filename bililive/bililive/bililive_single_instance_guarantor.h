/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_BILILIVE_SINGLE_INSTANCE_GUARANTOR_H_
#define BILILIVE_BILILIVE_BILILIVE_SINGLE_INSTANCE_GUARANTOR_H_

#include "base/basictypes.h"
#include "base/message_loop/message_loop.h"
#include "base/win/scoped_handle.h"

class BililiveSingleInstanceGuarantor : public base::MessageLoopForUI::Observer {
public:
    BililiveSingleInstanceGuarantor();

    ~BililiveSingleInstanceGuarantor();

    bool Install();

    void UnInstall();

    void StartMonitor();

    void TransmitCommandLine();

private:
    static void ActivateMainWindow();

    // Overrides base::MessageLoopForUI::Observer.

    base::EventStatus WillProcessEvent(const base::NativeEvent& event) override;

    void DidProcessEvent(const base::NativeEvent& event) override;

    void EventSpared() override {}

    DISALLOW_COPY_AND_ASSIGN(BililiveSingleInstanceGuarantor);

private:
    UINT msg_activate_id_ = 0;
    UINT msg_transmit_cmdline_id_ = 0;
    base::win::ScopedHandle instance_mutex_;
    bool monitoring_ = false;
};

#endif  // BILILIVE_BILILIVE_BILILIVE_SINGLE_INSTANCE_GUARANTOR_H_
