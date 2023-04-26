#ifndef BILILIVE_PUBLIC_BILILIVE_BILILIVE_COMMAND_IDS_H_
#define BILILIVE_PUBLIC_BILILIVE_BILILIVE_COMMAND_IDS_H_

enum CommandBililiveLivehime : int {
    IDC_LIVEHIME_COMMAND_START = 10000,

    IDC_LIVEHIME_CLOSE_APP,
    IDC_LIVEHIME_CLOSE_APP_SILENTLY,
    IDC_LIVEHIME_LOGOUT,
    IDC_LIVEHIME_TOKEN_INVALID,
    IDC_LIVEHIME_MINIMIZE_WINDOW,
    IDC_LIVEHIME_MAXIMIZE_WINDOW,
    IDC_LIVEHIME_RESTORE_WINDOW,
    IDC_LIVEHIME_ACTIVE_MAIN_WINDOW,

    IDC_LIVEHIME_SHOW_SOUND_EFFECT,

    IDC_LIVEHIME_SHOW_NOVICE_GUIDE,
    IDC_LIVEHIME_CHECK_FOR_UPDATE,
    IDC_LIVEHIME_SETUP,
    IDC_LIVEHIME_SHARE,
    IDC_LIVEHIME_SHOW_LOG_FILE,
    IDC_LIVEHIME_SHOW_GIFTBOX,

    IDC_LIVEHIME_START_RECORDING,
    IDC_LIVEHIME_STOP_RECORDING,
    IDC_LIVEHIME_START_LIVE_STREAMING,
    IDC_LIVEHIME_STOP_LIVE_STREAMING,
    IDC_LIVEHIME_STOP_STUDIO_LIVE_STREAMING,
    IDC_LIVEHIME_LIVE_ROOM_LOCKED,

    IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE,
    IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE,

    IDC_LIVEHIME_SHOW_BILIBILI_COMMON_WEB_WINDOW,

    IDC_LIVEHIME_SHOW_FEEDBACK_WEB_WINDOW,

    IDC_LIVEHIME_ADD_SOURCE,

    // Commands issued by menu(popup or context) cannot carry any paramter.
    IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU,
    IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU,
    IDC_LIVEHIME_ADD_COLOR_SOURCE_FROM_MENU,

    IDC_LIVEHIME_ADD_AUDIO_INPUT_FROM_MENU,
    IDC_LIVEHIME_ADD_AUDIO_OUTPUT_FROM_MENU,

    IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE,
    IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN,
    IDC_LIVEHIME_SCENE_ITEM_TOTOP,
    IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM,
    IDC_LIVEHIME_SCENE_ITEM_MOVEUP,
    IDC_LIVEHIME_SCENE_ITEM_MOVEDOWN,
    IDC_LIVEHIME_SCENE_ITEM_INTERACTION,
    IDC_LIVEHIME_SCENE_ITEM_PROPERTY,
    IDC_LIVEHIME_SCENE_ITEM_SELECT,
    IDC_LIVEHIME_SCENE_ITEM_DESELECT,
    IDC_LIVEHIME_SCENE_ITEM_VISIBLE,
    IDC_LIVEHIME_SCENE_ITEM_DELETE,
    IDC_LIVEHIME_ALL_SCENE_ITEM_DELETE,
    IDC_LIVEHIME_SCENE_ITEM_LOCK,
    IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT,
    IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH,
    IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM,
    IDC_LIVEHIME_SCENE_ITEM_USER_ADDED,
    IDC_LIVEHIME_SCENE_ITEM_VISUALIZE_BY_SOURCESHOW,
    IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW,
    IDC_LIVEHIME_SCENE_ITEM_REORDER,

    IDC_LIVEHIME_DANMAKU_USERNAME,
    IDC_LIVEHIME_DANMAKU_REPORT,
    IDC_LIVEHIME_DANMAKU_ADD_TO_SLIENT,
    IDC_LIVEHIME_DANMAKU_ADD_TO_BLACKLIST,
    IDC_LIVEHIME_DANMAKU_ADD_TO_ROOMMNG,
    IDC_LIVEHIME_DANMAKU_ADD_TO_SENIOR_ROOMMNG,
    IDC_LIVEHIME_DANMAKU_ADD_TO_REMOVE_ROOMMNG,

    IDC_LIVEHIME_HOTKEY_SYSVOL_ENABLE,
    IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME,
    IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME,
    IDC_LIVEHIME_HOTKEY_SENCE1,
    IDC_LIVEHIME_HOTKEY_SENCE2,
    IDC_LIVEHIME_HOTKEY_SENCE3,
    IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE1,
    IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE2,
    IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3,
    IDC_LIVEHIME_HOTKEY_SWITCH_LIVE,
    IDC_LIVEHIME_HOTKEY_SWITCH_RECORD,
    IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE,
    IDC_LIVEHIME_HOTKEY_FAST_FORWARD,  // �л�������ݼ���Ĭ��д�� ctrl+shift+F10

    IDC_LIVEHIME_SHOW_SYS_BALLOON,

    IDC_LIVEHIME_SHOW_ANCHOR_NOTICE,
    IDC_LIVEHIME_BROWSER_SCENE_SET,
    IDC_LIVEHIME_PLUGIN_ITEM_DELETE,
    IDC_LIVEHIME_OPEN_USER_CARD,
    IDC_LIVEHIME_SCHEME_CMD,
    IDC_LIVEHIME_CREATE_CHAT_USERCARD,
    IDC_LIVEHIME_LANGUAGE_SETTING_CHANGED,

    IDC_LIVEHIME_COMMAND_END
};

enum CommandBililiveUploader : int {
    IDC_UPLOADER_COMMAND_START = 30000,

    IDC_UPLOADER_ADD_ALBUM,
    IDC_UPLOADER_ADD_ALBUM_WITHPATHS,
    IDC_UPLOADER_EXIT,
    IDC_UPLOADER_EXIT_WITHOUTASK,
    IDC_UPLOADER_LOGOUT,
    IDC_UPLOADER_ALL_VIDEOS_COMPLETED,
    IDC_UPLOADER_CANCEL_SHUTDOWN,
    IDC_UPLOADER_CHECK_UPDATE,
    IDC_UPLOADER_ALL_ALBUM_COMPLETED,
    IDC_UPLOADER_RETURN_TO_HISTORY_ALBUMLIST,
    IDC_UPLOADER_START_ALL_TASK,
    IDC_UPLOADER_PAUSE_ALL_TASK,
    IDC_UPLOADER_ACTIVATE_MAIN_WINDOW,

    IDC_UPLOADER_COMMAND_END,
};

#endif  // BILILIVE_PUBLIC_BILILIVE_BILILIVE_COMMAND_IDS_H_