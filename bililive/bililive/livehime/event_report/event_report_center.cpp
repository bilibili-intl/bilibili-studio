#include "bililive/bililive/livehime/event_report/event_report_center.h"

#include <map>
#include <shellapi.h>
#include <string>

#include "base/bind.h"
#include "base/ext/callable_callback.h"
#include "base/json/json_writer.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/bililive/livehime/event_report/log_file_collection.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/account_info.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "third_party/zlib/zlib.h"

#include "obs/obs_proxy/public/common/pref_names.h"

namespace
{
    // gzip, copy from gzip_filter_unittest.cc
    enum EncodeMode
    {
        ENCODE_GZIP,      // Wrap the deflate with a GZip header.
        ENCODE_DEFLATE    // Raw deflate.
    };

    // The GZIP header (see RFC 1952):
    //   +---+---+---+---+---+---+---+---+---+---+
    //   |ID1|ID2|CM |FLG|     MTIME     |XFL|OS |
    //   +---+---+---+---+---+---+---+---+---+---+
    //     ID1     \037
    //     ID2     \213
    //     CM      \010 (compression method == DEFLATE)
    //     FLG     \000 (special flags that we do not support)
    //     MTIME   Unix format modification time (0 means not available)
    //     XFL     2-4? DEFLATE flags
    //     OS      ???? Operating system indicator (255 means unknown)
    //
    // Header value we generate:
    const char kGZipHeader[] = { '\037', '\213', '\010', '\000', '\000',
                                 '\000', '\000', '\000', '\002', '\377' };

    int CompressAll(EncodeMode mode, const char* source, int source_size,
        char* dest, int* dest_len)
    {
        z_stream zlib_stream;
        memset(&zlib_stream, 0, sizeof(zlib_stream));
        int code;

        // Initialize zlib
        if (mode == ENCODE_GZIP)
        {
            code = deflateInit2(&zlib_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                -MAX_WBITS,
                8,  // DEF_MEM_LEVEL
                Z_DEFAULT_STRATEGY);
        }
        else
        {
            code = deflateInit(&zlib_stream, Z_DEFAULT_COMPRESSION);
        }

        if (code != Z_OK)
            return code;

        // Fill in zlib control block
        zlib_stream.next_in = bit_cast<Bytef*>(source);
        zlib_stream.avail_in = source_size;
        zlib_stream.next_out = bit_cast<Bytef*>(dest);
        zlib_stream.avail_out = *dest_len;

        // Write header if needed
        if (mode == ENCODE_GZIP)
        {
            if (zlib_stream.avail_out < sizeof(kGZipHeader))
                return Z_BUF_ERROR;
            memcpy(zlib_stream.next_out, kGZipHeader, sizeof(kGZipHeader));
            zlib_stream.next_out += sizeof(kGZipHeader);
            zlib_stream.avail_out -= sizeof(kGZipHeader);
        }

        // Do deflate
        code = deflate(&zlib_stream, Z_FINISH);
        *dest_len = *dest_len - zlib_stream.avail_out;

        deflateEnd(&zlib_stream);
        return code;
    }

    std::string StreamingSettingsForERVS()
    {
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

        // {code_rate,frame_rate,resolution_ratio}
        std::unique_ptr<base::DictionaryValue> fields(new base::DictionaryValue());

        fields->SetString("code_rate", base::IntToString(pref->GetInteger(prefs::kVideoBitRate)));
        fields->SetString("frame_rate", pref->GetString(prefs::kVideoFPSCommon));
        fields->SetString("resolution_ratio", base::StringPrintf("%dx%d",
            pref->GetInteger(prefs::kVideoOutputCX), pref->GetInteger(prefs::kVideoOutputCY)));

        std::string extended_fields;
        base::JSONWriter::Write(fields.get(), &extended_fields);

        return extended_fields;
    }

    volatile std::atomic_int g_pending_log_upload_task_count = 0;
    std::map<std::string, std::tuple<std::string, int, int>> g_upload_info_map;

    void OnUploadLogFiles(bool valid, int code, const secret::AppService::UploadInfo& info,
        const std::string& log_key, int file_length)
    {
        // 有没有上传成功都埋点，以便我们在后台查看用户数据的完整上传记录
        std::string url = info.url;
        if (url.empty())
        {
            url = "n/a";
        }

        auto it = g_upload_info_map.find(log_key);
        if (it != g_upload_info_map.end()) {
            std::string cur_url = std::get<0>(it->second);
            url = cur_url.append("&&").append(url);
        }

        g_upload_info_map[log_key] = std::make_tuple(url, code, file_length);


        --g_pending_log_upload_task_count;

        if (0 == g_pending_log_upload_task_count)
        {
            auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
            std::vector<std::string> ps;

            std::string res_result;
            for (auto& iter : g_upload_info_map)
            {
                ps.push_back(iter.first + ":" + std::get<0>(iter.second));

                res_result += iter.first
                    + "="
                    + std::to_string(std::get<1>(iter.second))
                    + "+"
                    + std::to_string(std::get<2>(iter.second))
                    + ",";
            }
            res_result.pop_back();

            ps.push_back("res_result:" + res_result);
            ps.push_back("opportunity:0");  //TODO 0为用户主动上传 1为后台回捞

            auto mid = GetBililiveProcess()->secret_core()->account_info().mid();
            GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
                secret::LivehimeBehaviorEvent::LivehimeLogFilesUpload,
                mid, JoinString(ps, ';')).Call();
        }
    }

}

namespace livehime
{
    void TechnologyEventReport(secret::LivehimeBehaviorEvent type, const std::string& msg)
    {
        auto secret_core = GetBililiveProcess()->secret_core();
        auto mid = secret_core->account_info().mid();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            type, mid, msg).Call();
    }

    void UploadLogFilesAndReport(bool user_report)
    {
        if (g_pending_log_upload_task_count != 0)
        {
            return;
        }

        g_upload_info_map.clear();
    }

    void BehaviorEventReport(secret::LivehimeBehaviorEventV2 type, const std::string& msg)
    {
        base::StringPairs params;
        base::SplitStringIntoKeyValuePairs(msg, ':', ';', &params);

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEventV2(
            type, params).Call();
    }

    void BehaviorEventReport(const std::string& event_id, const std::string& msg)
    {
        base::StringPairs params;
        base::SplitStringIntoKeyValuePairs(msg, ':', ';', &params);

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEventV2(
            event_id, params).Call();
    }

    void ApmEventReport(const std::string& msg)
    {
        // gzip
        /*int gzip_encode_len = msg.length() * 1.5f;
        std::string post_data(gzip_encode_len, 0);
        int code = CompressAll(ENCODE_GZIP, msg.data(), msg.length(),
            &post_data[0], &gzip_encode_len);
        if (code == Z_STREAM_END)
        {
            auto secret_core = GetBililiveProcess()->secret_core();
            secret_core->event_tracking_service()->ReportLivehimeApmEventV2(
                std::string(post_data.data(), gzip_encode_len)).Call();
        }
        else
        {
            LOG(WARNING) << "[apm] compress failed.";
        }*/

        auto secret_core = GetBililiveProcess()->secret_core();
        if (secret_core && secret_core->event_tracking_service())
        {
			secret_core->event_tracking_service()->ReportLivehimeApmEventV2(
				msg).Call();
        }
    }

    base::StringPairs CommonFieldsForERVS()
    {
        base::StringPairs common_params;

        base::Time::Exploded time_exploded;
        base::Time::Now().LocalExplode(&time_exploded);
        common_params.push_back({ "ctime", base::StringPrintf("%d/%02d/%02d %02d:%02d:%02d",
            time_exploded.year,
            time_exploded.month,
            time_exploded.day_of_month,
            time_exploded.hour,
            time_exploded.minute,
            time_exploded.second) });
        //common_params.push_back({ "type_status", "" });   // 接口层填、H5自填
        common_params.push_back({ "ruid", base::Int64ToString(GetBililiveProcess()->secret_core()->account_info().mid()) });
        common_params.push_back({ "platform", "pc_link" });
        common_params.push_back({ "version", BililiveContext::Current()->GetExecutableVersionAsASCII() });
        common_params.push_back({ "is_obs", LivehimeLiveRoomController::GetInstance()->IsInTheThirdPartyStreamingMode() ? "1" : "0" });
        common_params.push_back({ "broad_type", "0" });
        common_params.push_back({ "screen_status", (!LiveModelController::GetInstance() || LiveModelController::GetInstance()->IsLandscapeModel()) ? "1" : "2" });
        common_params.push_back({ "is_simple", (GetBililiveProcess()->secret_core()->anchor_info().get_join_slide() == 0) ? "0" : "1" });
        common_params.push_back({ "title", GetBililiveProcess()->secret_core()->anchor_info().room_title() });
        common_params.push_back({ "cover", GetBililiveProcess()->secret_core()->anchor_info().room_cover() });
        common_params.push_back({ "definition", StreamingSettingsForERVS() });
        //common_params.push_back({ "background", "" });    // PC直播姬无此值
        //common_params.push_back({ "makeup_type", "" });   // 业务层自填
        //common_params.push_back({ "pk_id", "" });         // 业务层自填
        //common_params.push_back({ "pk_type", "" });       // 业务层自填
        //common_params.push_back({ "line_id", "" });       // 业务层自填
        //common_params.push_back({ "line_type", "" });     // 业务层自填
        //common_params.push_back({ "draw_id", "" });       // H5自填
        //common_params.push_back({ "order_id", "" });      // H5自填
        //common_params.push_back({ "screencast_type", "" });// 业务层自填

        return common_params;
    }

    void BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent type, const std::string& msg)
    {
        // 通过服务端上报的数据中有json格式数据，不能用常用的冒号“:”作为kv分割符
        base::StringPairs params;
        base::SplitStringIntoKeyValuePairs(msg, '=', ';', &params);

        base::StringPairs common_params(CommonFieldsForERVS());

        // 上层业务带数据的，以上层业务数据为准，覆盖此处自动填充的字段数据
        for (auto& param : params)
        {
            auto ret = std::find_if(common_params.begin(), common_params.end(),
                [param](const std::pair<std::string, std::string>& pr)->bool {
                return param.first == pr.first;
            });
            if (ret == common_params.end())
            {
                common_params.push_back(param);
            }
            else
            {
                ret->second = param.second;
            }
        }

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEventViaSrv(
            type, common_params).Call();
    }


    void BehaviorEventReportViaServerNew(secret::LivehimeViaServerBehaviorEventNew type, const std::string& msg)
    {
        base::StringPairs common_params;

        base::Time::Exploded time_exploded;
        base::Time::Now().LocalExplode(&time_exploded);
        common_params.push_back({ "ctime", base::StringPrintf("%d/%02d/%02d %02d:%02d:%02d",
            time_exploded.year,
            time_exploded.month,
            time_exploded.day_of_month,
            time_exploded.hour,
            time_exploded.minute,
            time_exploded.second) });


        common_params.push_back({ "uid", base::Int64ToString(GetBililiveProcess()->secret_core()->account_info().mid()) });
        common_params.push_back({ "room_id", base::Int64ToString(GetBililiveProcess()->secret_core()->user_info().room_id()) });

        common_params.push_back({ "platform", "pc_link" });
        common_params.push_back({ "version", BililiveContext::Current()->GetExecutableVersionAsASCII() });
        common_params.push_back({ "is_obs", LivehimeLiveRoomController::GetInstance()->IsInTheThirdPartyStreamingMode() ? "1" : "0" });
        common_params.push_back({ "broad_type", "0" });
        common_params.push_back({ "screen_status", (!LiveModelController::GetInstance() || LiveModelController::GetInstance()->IsLandscapeModel()) ? "1" : "2" });
        common_params.push_back({ "is_simple", (GetBililiveProcess()->secret_core()->anchor_info().get_join_slide() == 0) ? "0" : "1" });
        common_params.push_back({ "definition", StreamingSettingsForERVS() });

        common_params.push_back({ "live_key", GetBililiveProcess()->secret_core()->anchor_info().live_key() });
        common_params.push_back({ "sub_session_key", GetBililiveProcess()->secret_core()->anchor_info().sub_session_key() });

        common_params.push_back({ "cover", GetBililiveProcess()->secret_core()->anchor_info().room_cover() });
        common_params.push_back({ "title", GetBililiveProcess()->secret_core()->anchor_info().room_title() });
        common_params.push_back({ "area", std::to_string(GetBililiveProcess()->secret_core()->anchor_info().current_area())});

        common_params.push_back({ "event_id", std::to_string((int)type) });
        common_params.push_back({ "event_detail", msg });

        auto secret_core = GetBililiveProcess()->secret_core();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEventViaSrvNew(
            type, common_params).Call();

    }

    void PolarisEventReport(secret::LivehimePolarisBehaviorEvent type, const std::string& msg)
    {
        base::StringPairs params;
        base::SplitStringIntoKeyValuePairs(msg, ':', ';', &params);
        PolarisEventReport(type, params);
    }

    void PolarisEventReport(secret::LivehimePolarisBehaviorEvent event_id, const base::StringPairs& msg)
    {
        auto secret_core = GetBililiveProcess()->secret_core();
        if (secret_core->network_info().os_ver().empty())
        {
            secret_core->network_info().set_os_ver(obs_get_cpu_and_os_version().os_version);
        }
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEventPolaris(
            event_id, msg).Call();
    }

    void PolarisEventReport(const std::string& event_id, const std::string& msg)
    {
        auto secret_core = GetBililiveProcess()->secret_core();
        if (secret_core->network_info().os_ver().empty())
        {
            secret_core->network_info().set_os_ver(obs_get_cpu_and_os_version().os_version);
        }
        base::StringPairs params;
        base::SplitStringIntoKeyValuePairs(msg, ':', ';', &params);
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEventPolaris(
            event_id, params).Call();
    }



    // SupportsEventReportV2
    SupportsEventReportV2::SupportsEventReportV2()
    {
    }

    void SupportsEventReportV2::ReportBehaviorEvent(const std::string* msg/* = nullptr*/)
    {
        if (!IsEnableReport())
        {
            return;
        }

        for (auto& iter : report_tasks_)
        {
            if (msg)
            {
                iter.event_msg = *msg;
            }

            switch (iter.event_type)
            {
            case ReportTask::V2:
                {
                    BehaviorEventReport(iter.event_id.v2_type, iter.event_msg);
                }
                break;
            case ReportTask::Polaris:
                {
                    base::StringPairs params;
                    base::SplitStringIntoKeyValuePairs(iter.event_msg, ':', ';', &params);
                    PolarisEventReport(iter.event_id.polaris_type, params);
                }
                break;
            default:
                break;
            }
        }
    }

    void SupportsEventReportV2::SetReportEventDetails(secret::LivehimeBehaviorEventV2 event_type, const std::string& event_msg,
        bool enable/* = true*/)
    {
        auto match_item = std::find_if(report_tasks_.begin(), report_tasks_.end(), [&](const ReportTask& task)->bool {
            return (task.event_type == ReportTask::V2) && (task.event_id.v2_type == event_type);
        });
        if (match_item == report_tasks_.end())
        {
            ReportTask task;
            task.event_type = ReportTask::V2;
            task.event_msg = event_msg;
            task.event_id.v2_type = event_type;
            report_tasks_.push_back(task);
        }
        else
        {
            match_item->event_msg = event_msg;
            match_item->event_id.v2_type = event_type;
        }
        EnableReport(enable);
    }

    void SupportsEventReportV2::SetReportEventDetails(secret::LivehimePolarisBehaviorEvent event_type, const std::string& event_msg,
        bool enable /*= true*/)
    {
        auto match_item = std::find_if(report_tasks_.begin(), report_tasks_.end(), [&](const ReportTask& task)->bool {
            return (task.event_type == ReportTask::Polaris) && (task.event_id.polaris_type == event_type);
        });
        if (match_item == report_tasks_.end())
        {
            ReportTask task;
            task.event_type = ReportTask::Polaris;
            task.event_msg = event_msg;
            task.event_id.polaris_type = event_type;
            report_tasks_.push_back(task);
        }
        else
        {
            match_item->event_msg = event_msg;
            match_item->event_id.polaris_type = event_type;
        }
        EnableReport(enable);
    }

    void SupportsEventReportV2::EnableReport(bool enable)
    {
        enable_report_ = enable;
    }

    bool SupportsEventReportV2::IsEnableReport() const
    {
        return enable_report_ && !report_tasks_.empty();
    }

}