#ifndef BILILIVE_BILILIVE_UI_BILILIVE_COMMANDS_H
#define BILILIVE_BILILIVE_UI_BILILIVE_COMMANDS_H

#include <string>

class BililiveCommandReceiver;
class CommandObserver;
class CommandParamsDetails;

namespace bililive
{
    bool IsCommandEnabled(BililiveCommandReceiver *bililive_receiver, int command);
    bool SupportsCommand(BililiveCommandReceiver *bililive_receiver, int command);
    bool ExecuteCommand(BililiveCommandReceiver *bililive_receiver, int command);
    bool ExecuteCommandWithParams(BililiveCommandReceiver *bililive_receiver,
                                       int command,
                                       const CommandParamsDetails &params);
    void UpdateCommandEnabled(BililiveCommandReceiver *bililive_receiver, int command, bool enabled);
    void AddCommandObserver(BililiveCommandReceiver *bililive_receiver, int command, CommandObserver *observer);
    void RemoveCommandObserver(BililiveCommandReceiver *bililive_receiver, int command, CommandObserver *observer);
}

#endif