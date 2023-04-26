#ifndef BILILIVE_BILILIVE_LIVEHIME_KV_KV_COLIVE_SETTINGS_H_
#define BILILIVE_BILILIVE_LIVEHIME_KV_KV_COLIVE_SETTINGS_H_

#include <string>


namespace livehime {

    class KVColiveSettings {
    public:
        struct Settings {
            bool colive_anchor_voice_switch = false;

            /**
             * 用于给个别主播设置强制走/不走本地合流的开关。
             * 0: 根据总开关来；
             * 1: 强制走本地合流，不管计算机资源使用情况；
             * 2: 强制不走本地合流；
             * 其他：和 0 一样。
             */
            int force_self_publish = 0;

            /**
             * 本地合流总开关，影响线上所有主播（除了强制设置的）。
             * true: 根据计算机资源使用情况和阈值比较决定是否本地合流；
             * false: 不走本地合流。
             */
            bool is_self_publish = true;

            bool phantom_gpu_result = false;
            int max_sample_count = 0;
            int max_cpu_rate = -1;
            int max_gpu_rate = -1;
            int max_mem_occupied = -1;
            int max_ndf_rate = -1;
            int max_rdf_rate = -1;
            int max_edf_rate = -1;

            bool reduce_bitrate_enabled = false;
            // 连麦自推流时降的直播间码率，单位 Kbps
            int reduce_bitrate = 0;
            // 连麦自推流时降直播间码率的下限，单位 Kbps
            int min_reduced_bitrate = 0;

            /**
             * 指定在竖屏直播和本地合流连麦之间切换时，
             * 是否应用不断流机制。
             */
            bool is_spp_continued = false;

            //是否带电脑系统声音
            bool system_voice = false;

            bool chat_system_voice = false;

            //是否带系统声音根据系统版本号
            bool system_voice_windows_ver_switch = false;

            int64_t windows_build_ver = 0;
        };

        static void RequestColiveSettings();
        static const Settings& GetSettings();

        static void SetChatRoomStatus(bool open);
    private:
        static void OnGetColiveSettings(
            bool valid, int code, const std::string& err_msg, const std::string& data);

        static void ResetSettings(Settings* settings);

        static Settings colive_settings_;
        static bool chat_system_white_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_KV_KV_COLIVE_SETTINGS_H_