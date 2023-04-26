#ifndef BILILIVE_BILILIVE_COMMAND_UPDATER_H_
#define BILILIVE_BILILIVE_COMMAND_UPDATER_H_

#include <memory>
#include <unordered_map>

#include "base/basictypes.h"

#include "bililive/bililive/command_observer.h"
#include "bililive/bililive/command_updater_delegate.h"

// This object manages the enabled state of a set of commands. Observers register to listen to
// changes in this state so they can update their presentation.
class CommandUpdater {
public:
    explicit CommandUpdater(CommandUpdaterDelegate* delegate);

    ~CommandUpdater();

    bool SupportsCommand(int id) const;

    bool IsCommandEnabled(int id) const;

    bool ExecuteCommand(int id);

    bool ExecuteCommandWithParams(int id, const CommandParamsDetails& params);

    void AddCommandObserver(int id, CommandObserver* observer);

    void RemoveCommandObserver(int id, CommandObserver* observer);

    void RemoveCommandObserver(CommandObserver* observer);

    void UpdateCommandEnabled(int id, bool state);

private:
    class Command;

    Command* GetCommand(int id, bool create);

    CommandUpdaterDelegate* delegate_;

    using CommandMap = std::unordered_map<int, std::unique_ptr<Command>>;
    CommandMap commands_;

    DISALLOW_COPY_AND_ASSIGN(CommandUpdater);
};

#endif  // BILILIVE_BILILIVE_COMMAND_UPDATER_H_
