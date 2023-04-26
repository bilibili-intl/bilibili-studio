#include "kv_colive_settings.h"

#include "base/ext/callable_callback.h"
#include "base/json/json_reader.h"
#include "base/strings/string_number_conversions.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"


namespace livehime {

    KVColiveSettings::Settings KVColiveSettings::colive_settings_;
    bool KVColiveSettings::chat_system_white_ = false;

    // static
    void KVColiveSettings::RequestColiveSettings() {

    }

    // static
    const KVColiveSettings::Settings& KVColiveSettings::GetSettings() {
        return colive_settings_;
    }

    void KVColiveSettings::SetChatRoomStatus(bool open) {
        if (open && chat_system_white_) {
            colive_settings_.chat_system_voice = true;
        }
        else {
            colive_settings_.chat_system_voice = false;
        }
    }

    // static
    void KVColiveSettings::OnGetColiveSettings(
        bool valid, int code, const std::string& err_msg, const std::string& data)
    {
        ResetSettings(&colive_settings_);

        std::unique_ptr<base::Value> values(base::JSONReader::Read(data));
        const base::DictionaryValue* content = nullptr;
        if (!values || !values->GetAsDictionary(&content)) {
            return;
        }

        // 连麦旁路推流控制
        std::string val;
        if (content->GetString("colive_anchor_voice_switch_new", &val)) {
            colive_settings_.colive_anchor_voice_switch = val == "1";
        }

        if (content->GetString("force_colive_self_publish", &val)) {
            if (val == "1") {
                colive_settings_.force_self_publish = 1;
            } else if (val == "2") {
                colive_settings_.force_self_publish = 2;
            } else {
                colive_settings_.force_self_publish = 0;
            }
        }

        if (content->GetString("colive_self_publish", &val)) {
            colive_settings_.is_self_publish = val == "1";
        }

        if (content->GetString("phantom_gpu_result", &val)) {
            colive_settings_.phantom_gpu_result = val == "1";
        }

        if (content->GetString("max_sample_count", &val)) {
            int count;
            if (base::StringToInt(val, &count)) {
                colive_settings_.max_sample_count = count;
            }
        }

        if (content->GetString("max_cpu_rate", &val)) {
            int rate;
            if (base::StringToInt(val, &rate)) {
                colive_settings_.max_cpu_rate = rate;
            }
        }

        if (content->GetString("max_gpu_rate", &val)) {
            int rate;
            if (base::StringToInt(val, &rate)) {
                colive_settings_.max_gpu_rate = rate;
            }
        }

        if (content->GetString("max_mem_occupied", &val)) {
            int count;
            if (base::StringToInt(val, &count)) {
                colive_settings_.max_mem_occupied = count;
            }
        }

        if (content->GetString("max_ndf_rate", &val)) {
            int rate;
            if (base::StringToInt(val, &rate)) {
                colive_settings_.max_ndf_rate = rate;
            }
        }

        if (content->GetString("max_rdf_rate", &val)) {
            int rate;
            if (base::StringToInt(val, &rate)) {
                colive_settings_.max_rdf_rate = rate;
            }
        }

        if (content->GetString("max_edf_rate", &val)) {
            int rate;
            if (base::StringToInt(val, &rate)) {
                colive_settings_.max_edf_rate = rate;
            }
        }

        // 连麦自推流时降码率
        if (content->GetString("reduce_bitrate_enabled", &val)) {
            colive_settings_.reduce_bitrate_enabled = val == "1";
        }
        if (content->GetString("reduce_bitrate", &val)) {
            int bitrate;
            if (base::StringToInt(val, &bitrate)) {
                colive_settings_.reduce_bitrate = bitrate;
            }
        }
        if (content->GetString("min_reduced_bitrate", &val)) {
            int bitrate;
            if (base::StringToInt(val, &bitrate)) {
                colive_settings_.min_reduced_bitrate = bitrate;
            }
        }

        // 竖屏直播和本地合流连麦之间切换时不断流
        if (content->GetString("is_spp_continued", &val)) {
            colive_settings_.is_spp_continued = val == "1";
        }

        // 系统声音
        if (content->GetString("system_voice", &val)) {
            colive_settings_.system_voice = val == "1";
        }

        // 语聊房系统声音
        if (content->GetString("chat_system_voice", &val)) {
            chat_system_white_ = val == "1";
        }

        //是否带系统声音根据系统版本号
        if (content->GetString("system_voice_windows_ver_switch", &val))
        {
            colive_settings_.system_voice_windows_ver_switch = val == "1";
        }

        if (content->GetString("windows_build_ver", &val)) {
            int64 windows_build_ver = 0;
            if (base::StringToInt64(val, &windows_build_ver)) {
                colive_settings_.windows_build_ver = windows_build_ver;
            }
        }
    }

    // static
    void KVColiveSettings::ResetSettings(Settings* settings) {
        settings->force_self_publish = 0;

        settings->is_self_publish = true;
        settings->max_sample_count = 0;
        settings->max_cpu_rate = -1;
        settings->max_gpu_rate = -1;
        settings->max_mem_occupied = -1;
        settings->max_ndf_rate = -1;
        settings->max_rdf_rate = -1;
        settings->max_edf_rate = -1;

        settings->reduce_bitrate_enabled = false;
        settings->reduce_bitrate = 0;
        settings->min_reduced_bitrate = 0;

        settings->is_spp_continued = false;
        settings->system_voice = false;
        settings->system_voice_windows_ver_switch = false;
        settings->windows_build_ver = 0;
    }

}