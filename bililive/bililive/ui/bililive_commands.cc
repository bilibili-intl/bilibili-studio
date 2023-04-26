#include "bililive/bililive/ui/bililive_commands.h"

#include "base/command_line.h"
#include "base/metrics/histogram.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/bililive_command_controller.h"


namespace bililive
{
    bool IsCommandEnabled(BililiveCommandReceiver *bililive_receiver, int command)
    {
        return bililive_receiver->command_controller()->command_updater()->IsCommandEnabled(
                   command);
    }

    bool SupportsCommand(BililiveCommandReceiver *bililive_receiver, int command)
    {
        return bililive_receiver->command_controller()->command_updater()->SupportsCommand(
                   command);
    }

    bool ExecuteCommand(BililiveCommandReceiver *bililive_receiver, int command)
    {
        return bililive_receiver->command_controller()->command_updater()->ExecuteCommand(
                   command);
    }

    bool ExecuteCommandWithParams(BililiveCommandReceiver *bililive_receiver,
                                       int command,
                                       const CommandParamsDetails &params)
    {
        return bililive_receiver->command_controller()->command_updater()->
          ExecuteCommandWithParams(command, params);
    }

    void UpdateCommandEnabled(BililiveCommandReceiver *bililive_receiver, int command, bool enabled)
    {
        bililive_receiver->command_controller()->command_updater()->UpdateCommandEnabled(
            command, enabled);
    }

    void AddCommandObserver(BililiveCommandReceiver *bililive_receiver,
                            int command,
                            CommandObserver *observer)
    {
        bililive_receiver->command_controller()->command_updater()->AddCommandObserver(
            command, observer);
    }

    void RemoveCommandObserver(BililiveCommandReceiver *bililive_receiver,
                               int command,
                               CommandObserver *observer)
    {
        bililive_receiver->command_controller()->command_updater()->RemoveCommandObserver(
            command, observer);
    }
}