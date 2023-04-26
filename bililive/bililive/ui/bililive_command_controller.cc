#include "bililive/bililive/ui/bililive_command_controller.h"

#include "base/logging.h"

#include "bililive/bililive/ui/bililive_command_handler_livehime.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_command_ids.h"

namespace {

template<typename T>
T* receiver_as(BililiveCommandReceiver* raw_receiver)
{
    return static_cast<T*>(raw_receiver);
}

}   // namespace

namespace bililive {

BililiveCommandController::BililiveCommandController(
    BililiveCommandReceiver *bililive_commands_receiver)
    : commands_receiver_(bililive_commands_receiver),
    command_updater_(this)
{
    InitCommandState();
}

BililiveCommandController::~BililiveCommandController()
{}

void BililiveCommandController::ExecuteCommandWithParams(int id, const CommandParamsDetails &params)
{
    DCHECK(command_updater_.IsCommandEnabled(id)) << "Invalid/disabled command " << id;

    if (IDC_LIVEHIME_COMMAND_START <= id && id < IDC_LIVEHIME_COMMAND_END) {
        DCHECK(BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime));
        ExecuteCommandWithParamsLivehime(receiver_as<BililiveOBS>(commands_receiver_), id, params);
    } else {
        NOTREACHED();
    }
}

void BililiveCommandController::InitCommandState()
{
    switch (BililiveContext::Current()->GetApplicationMode()) {
        case ApplicationMode::BililiveLivehime:
            InitLiveHimeIntlCommands();
            break;

        default:
            NOTREACHED();
    }
}

void BililiveCommandController::InitLiveHimeIntlCommands()
{
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_CLOSE_APP, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_CLOSE_APP_SILENTLY, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_LOGOUT, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_TOKEN_INVALID, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_MINIMIZE_WINDOW, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_MAXIMIZE_WINDOW, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_RESTORE_WINDOW, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ACTIVE_MAIN_WINDOW, true);

    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_SOUND_EFFECT, true);

    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_NOVICE_GUIDE, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_CHECK_FOR_UPDATE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SETUP, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHARE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_LOG_FILE, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_GIFTBOX, true);

    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_START_RECORDING, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_STOP_RECORDING, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_START_LIVE_STREAMING, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_STOP_LIVE_STREAMING, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_STOP_STUDIO_LIVE_STREAMING, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_LIVE_ROOM_LOCKED, true);

    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE, true);

    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ADD_SOURCE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ADD_COLOR_SOURCE_FROM_MENU, true);

    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ADD_AUDIO_INPUT_FROM_MENU, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ADD_AUDIO_OUTPUT_FROM_MENU, true);

    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SYSVOL_ENABLE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SENCE1, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SENCE2, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SENCE3, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE1, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE2, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SWITCH_LIVE, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_SWITCH_RECORD, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_HOTKEY_FAST_FORWARD, true);

    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_DELETE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_ALL_SCENE_ITEM_DELETE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_TOTOP, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_MOVEUP, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_MOVEDOWN, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_INTERACTION, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_PROPERTY, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_SELECT, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_DESELECT, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_VISIBLE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_VISUALIZE_BY_SOURCESHOW, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_USER_ADDED, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_LOCK, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCENE_ITEM_REORDER, true);

    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_BILIBILI_COMMON_WEB_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_COLIVE_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_MELEE_SEASON_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_LOTTERY_WEB_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_FEEDBACK_WEB_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_VTUBER_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_THREE_VTUBER_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_ANCHOR_NOTICE, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SHOW_SYS_BALLOON, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_BROWSER_SCENE_SET, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_PLUGIN_ITEM_DELETE, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_OPEN_USER_CARD, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SCHEME_CMD, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_SELECT_VTUBER_WINDOW, true);
    //command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_CREATE_CHAT_USERCARD, true);
    command_updater_.UpdateCommandEnabled(IDC_LIVEHIME_LANGUAGE_SETTING_CHANGED, true);
}

}   // namespace bililive
