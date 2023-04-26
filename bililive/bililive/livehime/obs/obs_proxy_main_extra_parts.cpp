/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/livehime/obs/obs_proxy_main_extra_parts.h"

#include <Windows.h>
#include <shellapi.h>

#include "base/logging.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_handle.h"

#include "ui/base/resource/resource_bundle.h"

#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
//#include "bililive/bililive/livehime/vtuber/vtuber_ui_presenter.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/common/pref_names.h"

#include "grit/generated_resources.h"



namespace {

const wchar_t kSuppressionMarkName[] = L"bililive_window_capture_disable_performance_mode";
const char kMicRecordFilterName[] = "mic_record_filter_echo_test";
base::win::ScopedHandle suppresssion_mark;

void DisableWindowCapturePerformanceMode()
{
    suppresssion_mark.Set(CreateMutexW(nullptr, FALSE, kSuppressionMarkName));
}

std::wstring LocalStr(int res_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
}

void NotifyVideoHardwareNotSupported()
{
    auto title = LocalStr(IDS_INITIALIZE_OBS_CORE_ERROR_TITLE);
    auto content = LocalStr(IDS_VIDEO_HARDWARE_NOT_SUPPORTED);
    MessageBoxW(nullptr, content.c_str(), title.c_str(), MB_OK | MB_ICONEXCLAMATION);
}

void NotifyCoreInitializationFailure()
{
    auto title = LocalStr(IDS_INITIALIZE_OBS_CORE_ERROR_TITLE);
    auto content = LocalStr(IDS_INITIALIZE_OBS_CORE_ERROR_MSG);
    MessageBoxW(nullptr, content.c_str(), title.c_str(), MB_OK | MB_ICONEXCLAMATION);

    base::FilePath log_dir;
    PathService::Get(bililive::DIR_LOGS, &log_dir);
    if (!log_dir.empty()) {
        auto log_path = log_dir.Append(bililive::kBililiveLogFileName);
        std::wstring shell_cmd(L"/select,");
        shell_cmd += log_path.value();
        ShellExecuteW(nullptr, L"open", L"explorer", shell_cmd.c_str(), nullptr, SW_SHOWDEFAULT);
    }
}

}   // namespace

namespace livehime {

void OBSProxyMainExtraParts::PreBililiveStart()
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();
    if (!global_prefs->GetBoolean(prefs::kWindowCaptureOptimizeForPerformance)) {
        DisableWindowCapturePerformanceMode();
    }

    // Double-insurance: current directory must be the main directory; otherwise the obs-core
    // will end up in unexpted state.
    base::FilePath main_dir;
    PathService::Get(bililive::DIR_APP, &main_dir);
    if (!main_dir.empty()) {
        SetCurrentDirectoryW(main_dir.value().c_str());
    }

    auto core_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();

    // There is nothing we can do, just notify the user then die nobly.
    try {
        core_proxy->InitOBSCore();
    } catch (const obs_proxy::VideoHardwareNotSupported&) {
        NotifyVideoHardwareNotSupported();
        LOG(ERROR) << "Failed to initialize obs core: Hardware not supported!";
        exit(1);
    } catch (const obs_proxy::CoreInitError& ex) {
        NotifyCoreInitializationFailure();
        LOG(ERROR) << "Failed to initialize obs core: " << ex.what();
        exit(1);
    }

    bililive::OutputController::GetInstance()->Bind(core_proxy);
}

void OBSProxyMainExtraParts::PostBililiveStart()
{
    // Load last scene collection.
    // Note: collection name might contain non-ascii characters.
    auto core_proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();

    auto user_profile = GetBililiveProcess()->profile();
    std::string last_collection_name = user_profile->GetPrefs()->GetString(prefs::kLastSceneCollectionName);
    base::FilePath collection_file(user_profile->GetPath());
    collection_file = collection_file.Append(UTF8ToWide(last_collection_name));
    if (!core_proxy->LoadSceneCollection(collection_file))
    {
        core_proxy->LoadDefaultSceneCollection();
        core_proxy->GetCurrentSceneCollection()->Save();
    }

    OBSProxyService::GetInstance().obs_ui_proxy()->Initialize();

    //检测是否有回声检测的filter，有则删除
    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection)
    {
        std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

        auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
            return dev->name() == obs_proxy::kDefaultInputAudio; });
        if (mic != vct.end())
        {
            obs_proxy::Filter* filter = (*mic)->GetFilter(kMicRecordFilterName);
            if (filter)
            {
                (*mic)->RemoveFilter(kMicRecordFilterName);
            }
        }
    }
}

void OBSProxyMainExtraParts::BililiveTearDown()
{
    bililive::OutputController::GetInstance()->UnBind();
}

}   // namespace livehime
