/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/bililive_single_instance_guarantor.h"

#include "base/ext/bind_lambda.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/bililive_commands.h"
#include "ui/bililive_obs.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/livehime/pluggable/pluggable_controller.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"


namespace {

    const wchar_t kInstanceMutexLiveHime[] = L"{12E493A8-BEC2-446D-BE09-CE9CC582D1E6}";
    const wchar_t kMsgActivateStrLiveHime[] = L"{FA99D2D4-BA27-4A92-95CC-A8AD1F0FCFD5}";
    const wchar_t kMsgTransmitCmdline[] = L"{46418A59-EA18-4B6E-8B6F-FDEC49C5D6A9}";

    const wchar_t* mutex_name = kInstanceMutexLiveHime;

    std::map<UINT, DWORD> g_reg_msg_respons;

}   // namespace

BililiveSingleInstanceGuarantor::BililiveSingleInstanceGuarantor()
{
}

BililiveSingleInstanceGuarantor::~BililiveSingleInstanceGuarantor()
{}

bool BililiveSingleInstanceGuarantor::Install()
{
    msg_activate_id_ = ::RegisterWindowMessageW(kMsgActivateStrLiveHime);
    msg_transmit_cmdline_id_ = ::RegisterWindowMessageW(kMsgTransmitCmdline);
    if (msg_activate_id_ == 0 || 
        msg_transmit_cmdline_id_ == 0)
    {
        PLOG(WARNING) << "Register activate window message failure!";
        return false;
    }

    g_reg_msg_respons.insert({ msg_activate_id_, 0 });
    g_reg_msg_respons.insert({ msg_transmit_cmdline_id_, 0 });

    instance_mutex_.Set(::CreateMutexW(nullptr, false, mutex_name));
    if (!instance_mutex_) {
        PLOG(WARNING) << "Failed to create instance mutex!";
        return false;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        PostMessageW(HWND_BROADCAST, msg_activate_id_, 0, 0);
        return false;
    }

    return true;
}

void BililiveSingleInstanceGuarantor::UnInstall()
{
    if (monitoring_) {
        base::MessageLoopForUI::current()->RemoveObserver(this);
    }

    instance_mutex_.Close();
}

void BililiveSingleInstanceGuarantor::StartMonitor()
{
    DCHECK(!monitoring_);

    base::MessageLoopForUI::current()->AddObserver(this);
    monitoring_ = true;
}

void BililiveSingleInstanceGuarantor::TransmitCommandLine()
{
    auto process_type = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(bililive::kSwitchProcessType);
    if (process_type == bililive::kProcessScheme)
    {
        std::string args = base::UTF16ToUTF8(CommandLine::ForCurrentProcess()->GetCommandLineString());

        LOG(INFO) << "Transmit cmdline to main livehime process, \n\t" << args;

        // Write the command line arguments to the disk file, create && write successfully and send a broadcast message
        base::ThreadRestrictions::ScopedAllowIO allow;
        base::FilePath path;
        if (file_util::GetTempDir(&path))
        {
            path = path.Append(bililive::kCmdlineTempFileName);
            if (file_util::WriteFile(path, args.data(), args.length()) == (int)args.length())
            {
                PostMessageW(HWND_BROADCAST, msg_transmit_cmdline_id_, 0, 0);
            }
        }
    }
}

// static
void BililiveSingleInstanceGuarantor::ActivateMainWindow()
{
    auto main_window = GetBililiveProcess()->bililive_obs();
    if (!main_window) {
        return;
    }

    //auto weak_bound = main_window->AsWeakPtr();
    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([main_window] {
            if (main_window) {
                LOG(INFO) << "single instance activate";
                bililive::ExecuteCommand(main_window, IDC_LIVEHIME_ACTIVE_MAIN_WINDOW);
            }
        }));
}

base::EventStatus BililiveSingleInstanceGuarantor::WillProcessEvent(const base::NativeEvent& event)
{
    // We may receive several same messages from different windows, filter them out.
    static const DWORD kRefractoryPeriod = 1000;
    if (g_reg_msg_respons.count(event.message) != 0)
    {
        if ((event.time - g_reg_msg_respons[event.message]) > kRefractoryPeriod)
        {
            if (event.message == msg_activate_id_)
            {
                ActivateMainWindow();
            }
            else if (event.message == msg_transmit_cmdline_id_)
            {
                PluggableController::GetInstance()->ScheduleTransmitCmdline();
            }

            g_reg_msg_respons[event.message] = event.time;
        }
    }

    return base::EVENT_CONTINUE;
}

void BililiveSingleInstanceGuarantor::DidProcessEvent(const base::NativeEvent& event)
{}
