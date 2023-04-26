/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_INSTALL_EVENT_TRACKING_SIMPLE_EVENT_REPORTER_H_
#define BILILIVE_INSTALL_EVENT_TRACKING_SIMPLE_EVENT_REPORTER_H_

#include <chrono>
#include <string>

#include "bilibase/basic_macros.h"

namespace bililive {

class EventReporter {
public:
    enum Event {
        Install,
        Uninstall
    };

    ~EventReporter() = default;

    DISABLE_COPY(EventReporter);

    static EventReporter& GetInstance();

    void InitializeCredential();

    bool RestoreCredential();

    void Report(Event event_id) const;

private:
    EventReporter();

private:
    std::chrono::seconds timeout_;
    std::string buvid_;
    std::string budid_;
};

}   // namespace bililive

#endif  // BILILIVE_INSTALL_EVENT_TRACKING_SIMPLE_EVENT_REPORTER_H_
