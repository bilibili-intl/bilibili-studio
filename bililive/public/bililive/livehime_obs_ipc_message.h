#ifndef BILILIVE_PUBLIC_BILILIVE_LIVEHIME_OBS_IPC_MESSAGE_H_
#define BILILIVE_PUBLIC_BILILIVE_LIVEHIME_OBS_IPC_MESSAGE_H_

namespace ipc_message
{
    const wchar_t kInstanceMutexLiveHimeOBSPlugins[] = L"{6814311E-C833-4105-B1CE-6DA02C516F37}";
    const char kIPCChannelLiveHimeOBSPlugins[] = "livehimeintl_obs_plugin_ipc_channel";

    enum LivehimeOBSPluginIPCMessageType : unsigned int
    {
        IPC_MSG_BEGIN = 100,

        // test[
        IPC_LIVEHIME_TO_OBS,
        IPC_OBS_TO_LIVEHIME,
        IPC_BOTHWAY,
        // ]

        IPC_OBS_TO_LIVEHIME_START_LIVE,
        IPC_LIVEHIME_TO_OBS_STREAM_SETTINGS,
        IPC_LIVEHIME_TO_OBS_STOP_STREAMING,

        IPC_MSG_END,
    };

}

#endif
