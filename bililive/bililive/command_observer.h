#ifndef BILILIVE_BILILIVE_COMMAND_OBSERVER_H_
#define BILILIVE_BILILIVE_COMMAND_OBSERVER_H_

class CommandObserver {
public:
    virtual void EnabledStateChangedForCommand(int id, bool enabled) = 0;

protected:
    virtual ~CommandObserver() {}
};

#endif  // BILILIVE_BILILIVE_COMMAND_OBSERVER_H_
