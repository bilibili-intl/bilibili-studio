#include "bililive/bililive/livehime/pluggable/pluggable_controller.h"

#include <map>
#include <regex>
#include <string>

#include "base/file_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"

#include "net/base/escape.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/utils/net_util.h"
//#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/account_info.h"
#include "bililive/secret/public/event_tracking_service.h"
//#include "bililive/bililive/livehime/vtuber/three_vtuber/three_vtuber_ui_presenter.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"

namespace
{
    const std::map<std::wstring, PluggableType> kSupportSchemeCmd
    {
        {L"mainui.active", PluggableType::Active},

        {L"live.start", PluggableType::StartLive},
        //{L"live.stop", PluggableType::StopLive},
        {L"vtuber.addfigma", PluggableType::OpenUnity2D},
        {L"zstar.selectgear", PluggableType::OpenUnityZstar},
        {L"interactive.game", PluggableType::OPenInteractivePlay},
        {L"together.play", PluggableType::OPenTogetherPlay},
        {L"voice.link", PluggableType::OPenVoiceLink},
        {L"preview.debug", PluggableType::OpenPreviewDebug},
    };
}


PluggableController* PluggableController::GetInstance()
{
    return Singleton<PluggableController>::get();
}

PluggableController::PluggableController()
{

}

PluggableController::~PluggableController() = default;

void PluggableController::ScheduleTransmitCmdline()
{
    base::ThreadRestrictions::ScopedAllowIO allow;
    base::FilePath path;
    if (file_util::GetTempDir(&path))
    {
        path = path.Append(bililive::kCmdlineTempFileName);
        std::string args;
        if (file_util::ReadFileToString(path, &args))
        {
            std::wstring argsw = base::UTF8ToUTF16(args);
            CommandLine cmdline = CommandLine::FromString(argsw);

            LOG(INFO) << "[PGB] schedule cmdline from other process, \n\t" << args;
            ScheduleCmdline(&cmdline);

            base::DeleteFile(path, false);
        }
    }
}

void PluggableController::ScheduleCurrentProcessCmdline()
{
    ScheduleCmdline(CommandLine::ForCurrentProcess());
}

void PluggableController::ScheduleCmdline(CommandLine* cmdline)
{
    auto process_type = cmdline->GetSwitchValueASCII(bililive::kSwitchProcessType);
    if (process_type == bililive::kProcessScheme)
    {
        auto scheme = cmdline->GetSwitchValueASCII("scheme");
        net::UnescapeRule::Type flags =
            net::UnescapeRule::SPACES | net::UnescapeRule::URL_SPECIAL_CHARS;
        base::string16 ss = net::UnescapeAndDecodeUTF8URLComponent(scheme, flags, NULL);
        LOG(INFO)<<"[PGB] ScheduleCmdline:cmd "<< ss;
        // url=livehime://test-live=%E7%99%EB%76, cmd=livehime://test-live?key=百度&%ww|``&ss=[]\';/.,#33
        // 按照直播姬规则匹配scheme，不合规的不处理
        // livehime://com.bilibili.livehime/module.cmd?referer=xxxx&key0=汉字&key1=value1&key2=value2&........
        //static const wchar_t kURLSchemesRegexPattern[] = LR"(^livehime://com\.bilibili\.livehime/([a-z0-9]+)\.([a-z0-9]+)\?(\S+))";
        static const wchar_t kURLSchemesRegexPattern[] = LR"(^livehime://com\.bilibili\.livehime/([a-z0-9]+)\.([a-z0-9]+)\?(.+))";
        std::wsmatch match;
        std::wregex regex(kURLSchemesRegexPattern);
        if (std::regex_match(ss, match, regex))
        {
            std::wstring module = match[1];
            std::wstring cmd = match[2];
            std::wstring query = match[3];
            parameter_cmd_ = query;
            base::StringPairs params;
            base::SplitStringIntoKeyValuePairs(base::UTF16ToUTF8(query), '=', '&', &params);
            auto iter = std::find_if(params.begin(), params.end(), [](const std::pair<std::string, std::string>& pair)->bool {
                if (strcmp(pair.first.c_str(), "referer") == 0)
                {
                    return true;
                }
                return false;
            });
            if (iter != params.end() && !iter->second.empty())
            {
                std::wstring mc = module + L"." + cmd;
                if (kSupportSchemeCmd.find(mc) != kSupportSchemeCmd.end())
                {
                    PluggableType pt = kSupportSchemeCmd.at(mc);
                    SchedulePluggableOperate(pt, params);
                }
                else
                {
                    LOG(WARNING) << "[PGB] unsupport scheme cmd, \n\t" << scheme;
                }
            }
            else
            {
                LOG(WARNING) << "[PGB] scheme args miss valid referer field, \n\t" << scheme;
            }
        }
        else
        {
            LOG(WARNING) << "[PGB] scheme args not in valid format, \n\t" << scheme;
        }
    }
}

void PluggableController::SchedulePluggableOperate(PluggableType type, const base::StringPairs& params)
{
    base::StringPairs param;
    param.push_back({ "side_type", "1" });
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::JumpAgreementTrack, param);

    switch (type)
    {
    case PluggableType::Active:
        LOG(INFO) << "[PGB] active main wnd.";
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
        break;
    case PluggableType::StartLive:
    {
        if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
        {
            LOG(INFO) << "[PGB] start live.";
            bool hotkey = false;
            bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_START_LIVE_STREAMING,
                CommandParams<bool>(&hotkey));
        }
    }break;
    case PluggableType::OpenUnity2D: {
        PluggableInfo info;
        info.type = PluggableType::OpenUnity2D;
        info.params = base::UTF16ToUTF8(parameter_cmd_);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SCHEME_CMD, CommandParams<PluggableInfo>(&info));
    }break;
    case PluggableType::OpenUnityZstar:{
        PluggableInfo info;
        info.type = PluggableType::OpenUnityZstar;
        info.params = base::UTF16ToUTF8(parameter_cmd_);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SCHEME_CMD, CommandParams<PluggableInfo>(&info));
    }break;
    case PluggableType::OPenInteractivePlay:{
        PluggableInfo info;
        info.type = PluggableType::OPenInteractivePlay;
        info.params = base::UTF16ToUTF8(parameter_cmd_);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SCHEME_CMD, CommandParams<PluggableInfo>(&info));
    }
    break;
    case PluggableType::OPenTogetherPlay: {
        PluggableInfo info;
        info.type = PluggableType::OPenTogetherPlay;
        info.params = base::UTF16ToUTF8(parameter_cmd_);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SCHEME_CMD, CommandParams<PluggableInfo>(&info));
    }
    break;
    case PluggableType::OPenVoiceLink: {
        PluggableInfo info;
        info.type = PluggableType::OPenVoiceLink;
        info.params = base::UTF16ToUTF8(parameter_cmd_);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SCHEME_CMD, CommandParams<PluggableInfo>(&info));
    }
    break;
    case PluggableType::OpenPreviewDebug: {
        PluggableInfo info;
        info.type = PluggableType::OpenPreviewDebug;
        info.params = base::UTF16ToUTF8(parameter_cmd_);
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SCHEME_CMD, CommandParams<PluggableInfo>(&info));
    }
    break;
    default:
        break;
    }
}
