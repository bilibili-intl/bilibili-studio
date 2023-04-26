#include "bililive/bililive/command_updater.h"

#include "base/logging.h"
#include "base/observer_list.h"

class CommandUpdater::Command {
public:
    bool enabled;
    ObserverList<CommandObserver> observers;

    Command() : enabled(true) {}
};

CommandUpdater::CommandUpdater(CommandUpdaterDelegate* delegate)
    : delegate_(delegate)
{}

CommandUpdater::~CommandUpdater()
{}

bool CommandUpdater::SupportsCommand(int id) const
{
    return commands_.find(id) != commands_.end();
}

bool CommandUpdater::IsCommandEnabled(int id) const
{
    auto command = commands_.find(id);
    if (command == commands_.end()) {
        return false;
    }

    return command->second->enabled;
}

bool CommandUpdater::ExecuteCommand(int id)
{
    return ExecuteCommandWithParams(id, EmptyCommandParams());
}

bool CommandUpdater::ExecuteCommandWithParams(int id, const CommandParamsDetails& params)
{
    if (SupportsCommand(id) && IsCommandEnabled(id)) {
        delegate_->ExecuteCommandWithParams(id, params);
        return true;
    }

    return false;
}

void CommandUpdater::AddCommandObserver(int id, CommandObserver* observer)
{
    GetCommand(id, true)->observers.AddObserver(observer);
}

void CommandUpdater::RemoveCommandObserver(int id, CommandObserver* observer)
{
    GetCommand(id, false)->observers.RemoveObserver(observer);
}

void CommandUpdater::RemoveCommandObserver(CommandObserver* observer)
{
    for (const auto& cmd_pair : commands_) {
        Command* command = cmd_pair.second.get();
        if (command) {
            command->observers.RemoveObserver(observer);
        }
    }
}

void CommandUpdater::UpdateCommandEnabled(int id, bool enabled)
{
    Command* command = GetCommand(id, true);
    if (command->enabled == enabled) {
        // Nothing to do.
        return;
    }

    command->enabled = enabled;

    FOR_EACH_OBSERVER(CommandObserver, command->observers, EnabledStateChangedForCommand(id, enabled));
}

CommandUpdater::Command* CommandUpdater::GetCommand(int id, bool create)
{
    bool supported = SupportsCommand(id);
    if (supported){
        return commands_[id].get();
    }

    DCHECK(create);
    std::unique_ptr<Command>& entry = commands_[id];
    entry = std::make_unique<Command>();

    return entry.get();
}