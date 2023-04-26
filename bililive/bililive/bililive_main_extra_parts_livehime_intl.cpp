#include "bililive/bililive/bililive_main_extra_parts_livehime_intl.h"

#include "base/command_line.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"

#include "ui/bililive_commands.h"
#include "ui/bililive_obs.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/text_to_speech/tts_helper.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

MainExtraPartsLivehimeIntl::MainExtraPartsLivehimeIntl() {}

MainExtraPartsLivehimeIntl::~MainExtraPartsLivehimeIntl() {}

void MainExtraPartsLivehimeIntl::PreEarlyInitialization()
{
    // Suppress installer-ready alert.
    //if (!CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchIgnoreUpdate) &&
    //    bililive::ClientUpdater::GetInstance().CheckIfUpdatesAvailable())
    //{
    //    if (!bililive::ClientUpdater::GetInstance().LaunchUpdates())
    //    {
    //        // UAC access refused.
    //        if (GetLastError() == ERROR_CANCELLED)
    //        {
    //            // We haven't got resource bundle ready yet, resort to Windows Messagebox instead.
    //            ::MessageBoxW(nullptr, kDlgMustApplyUpdateFirst, kDlgMustApplyUpdateFirstTitle,
    //                        MB_OK | MB_ICONEXCLAMATION);
    //            exit(0);
    //        }

    //        PLOG(WARNING) << "Failed to launch the installer; retry in next time";
    //    }
    //    else
    //    {
    //        const char kNormalExitTag[] = "--- Application Exit ---";
    //        LOG(INFO) << kNormalExitTag << "\nExit result code: 0";
    //        exit(0);
    //    }
    //}
}

void MainExtraPartsLivehimeIntl::PreProfileInit()
{
}

void MainExtraPartsLivehimeIntl::PostProfileInit() {
    auto secret_core = GetBililiveProcess()->secret_core();
    auto login_params = base::StringPrintf("firstrun:%d", GetBililiveProcess()->first_run());
    secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeLogin,
        secret_core->account_info().mid(),
        login_params).Call();
}

void MainExtraPartsLivehimeIntl::PreBililiveStart()
{
    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        bililive::InitTextToSpeech();
        DCHECK(bililive::GetLastInitTextToSpeechResult());
    }
}

void MainExtraPartsLivehimeIntl::PostBililiveStart()
{
}

void MainExtraPartsLivehimeIntl::BililiveTearDown()
{
    obs_proxy::OBSCoreProxy* core_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
    DCHECK(core_proxy);
    core_proxy->GetCurrentSceneCollection()->Save();
    core_proxy->ShutdownOBSCore();
}