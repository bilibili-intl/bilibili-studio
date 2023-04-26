#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"

#include <thread>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"

#include "bilibase/basic_macros.h"
#include "bilibase/error_exception_util.h"
#include "bilibase/scope_guard.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/obs_logging_hook.h"
#include "obs/obs_proxy/public/proxy/obs_proxy_delegate.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"
#include "obs/obs_proxy/utils/obs_type_scope_wrapper.h"
#include "obs/obs_proxy/utils/obs_video_setup.h"
#include "obs/obs_proxy/utils/process_privileges.h"

#include "grit/generated_resources.h"
#include "ui/base/resource/resource_bundle.h"
//#include "bililive/bililive/livehime/crash_warning/crash_warning.h"

bool obs_module_inited = false;
base::WaitableEvent cef_module_loaded_event(true, false);

namespace {

using std::placeholders::_1;
using obs_proxy::OBSCoreProxy;

const char kLocale[] = "en-US";
const char kModuleConfigPath[] = "data/plugins/config";

static std::once_flag s_task_queue_callback_flag_;
static std::recursive_mutex s_task_queue_mutex_;
static std::list<std::function<void()>> s_task_queue_;

// Note that this pointer doesn't own the core-proxy-impl instance.
obs_proxy::CoreProxyImpl* g_core_proxy_instance = nullptr;

typedef void(*SETOBSCAMERAFRAMEHIDESTATUS)(bool);
SETOBSCAMERAFRAMEHIDESTATUS g_set_obs_camera_frame_hide_status = nullptr;

std::wstring LocalStr(int res_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
}

void StartOBS()
{
    bool rv = obs_startup(kLocale, kModuleConfigPath, nullptr);
    ENSURE(rv).Require<obs_proxy::StartOBSError>();
}

void LoadOBSModules()
{
    obs_load_all_modules();
}

void UnloadBrowserModules()
{
    HMODULE hMod = GetModuleHandleW(L"obs-browser.dll");
    if (hMod)
    {
        typedef void(*OBSBROWSERMODULEUNLOAD)(void);
        OBSBROWSERMODULEUNLOAD obs_module_unload = (OBSBROWSERMODULEUNLOAD)GetProcAddress(hMod, "obs_module_unload");
        if (obs_module_unload)
        {
            obs_module_unload();
        }
    }
}

void UnloadDmkhimeSourceModules()
{
    HMODULE hMod = GetModuleHandleW(L"obs-dmkhime.dll");
    if (hMod)
    {
        typedef void(*OBSDMKHIMEMODULEUNLOAD)(void);
        OBSDMKHIMEMODULEUNLOAD obs_module_unload = (OBSDMKHIMEMODULEUNLOAD)GetProcAddress(hMod, "obs_module_unload");
        if (obs_module_unload)
        {
            obs_module_unload();
        }
    }
}

}   // namespace

namespace obs_proxy {

CoreProxyImpl::CoreProxyImpl()
{
    DCHECK(g_core_proxy_instance == nullptr);
    g_core_proxy_instance = this;
}

CoreProxyImpl::~CoreProxyImpl()
{
    DCHECK(g_core_proxy_instance == this);
    g_core_proxy_instance = nullptr;
}

void CoreProxyImpl::InitOBSCore()
{
    // OBS模块开始加载前要确保CEF模块的加载流程全部完成了（不管加载成功或失败）
    cef_module_loaded_event.Wait();

    if (!EnableProcessSEPrivilege(GetCurrentProcess())) {
        PLOG(WARNING) << "Failed to enable SE_DEBUG privilege";
    }

    HookOBSLogging();

    StartOBS();

    ResetAudio();

    __try
    {
        uint32_t out_width, out_height;
        std::tie(out_width, out_height) = GetConfiguredVideoOutputMetrics(GetPrefs());
        ResetVideo(out_width, out_height);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LOG(ERROR) << "ResetVideo crash";
        logging::FlushLogFile();
        //bililive::LoadGpuCrashWarning();

        exit(1);
    }
    LoadOBSModules();

    BuildOutputService();

    std::call_once(s_task_queue_callback_flag_, []() {
        obs_add_tick_callback([](void* param, float seconds) {
            std::lock_guard<std::recursive_mutex> lg(s_task_queue_mutex_);
            if (s_task_queue_.empty())
                return;
            std::list<std::function<void()>> queue;
            queue.swap(s_task_queue_);
            for (auto& x : queue)
                x();
        }, nullptr);
    });

    //
    obs_module_inited = true;
}

void CoreProxyImpl::ShutdownOBSCore()
{
    scene_collection_->Cleanup();
    UnloadBrowserModules();
    UnloadDmkhimeSourceModules();
}

bool CoreProxyImpl::UpdateAudioSettings()
{
    try {
        ResetAudio();
    } catch (const ResetAudioError& ex) {
        LOG(ERROR) << ex.what();
        return false;
    }

    return true;
}

bool CoreProxyImpl::UpdateVideoSettings()
{
    if (!obs_output_service_ || obs_output_service_->IsOutputActive())
    {
        LOG(WARNING) << "reset video setting when video currently active.";
        return false;
    }

    try {
        uint32_t out_width, out_height;
        std::tie(out_width, out_height) = GetConfiguredVideoOutputMetrics(GetPrefs());
        ResetVideo(out_width, out_height);
    } catch (const ResetVideoError& ex) {
        LOG(ERROR) << ex.what();
        return false;
    }

    return true;
}

bool CoreProxyImpl::LoadSceneCollection(const base::FilePath& collection)
{
    scene_collection_ = SceneCollectionImpl::FromCollectionFile(collection);
    return !!scene_collection_;
}

void CoreProxyImpl::LoadDefaultSceneCollection()
{
    base::FilePath collection_file(GetProfile()->GetPath());
    collection_file = collection_file.Append(UTF8ToWide(prefs::kDefaultSceneCollectionName));
    scene_collection_ = SceneCollectionImpl::ForDefault(collection_file);
}

void CoreProxyImpl::SetCameraFrameHideStatus(bool status)
{
    if (!g_set_obs_camera_frame_hide_status)
    {
        HMODULE hMod = GetModuleHandleW(L"win-dshow.dll");
        if (hMod)
        {
            g_set_obs_camera_frame_hide_status = (SETOBSCAMERAFRAMEHIDESTATUS)GetProcAddress(hMod, "SetObsCameraFrameHideStatus");
        }
    }
    if (g_set_obs_camera_frame_hide_status)
    {
        g_set_obs_camera_frame_hide_status(status);
    }
}

OBSOutputService* CoreProxyImpl::GetOBSOutputService() const
{
    DCHECK(obs_output_service_);
    return obs_output_service_.get();
}

SceneCollection* CoreProxyImpl::GetCurrentSceneCollection() const
{
    DCHECK(scene_collection_);
    return scene_collection_.get();
}

bool CoreProxyImpl::EncoderIsSupported(const char *id) const
{
    if (!obs_get_encoder_codec(id))
    {
        return false;
    }
    return true;
}

void CoreProxyImpl::ResetAudio()
{
    FORCE_AS_NON_CONST_MEMBER_FUNCTION();

    obs_audio_info audio_info;
    audio_info.samples_per_sec = GetPrefs()->GetInteger(prefs::kAudioSampleRate);
    std::string preferred_audio_channel = GetPrefs()->GetString(prefs::kAudioChannelSetup);
    bool mono_speaker = base::EqualsCaseInsensitiveASCII(preferred_audio_channel,
                                                         prefs::kAudioSpeakMono);
    audio_info.speakers = mono_speaker ? SPEAKERS_MONO : SPEAKERS_STEREO;

    bool rv = obs_reset_audio(&audio_info);
    ENSURE(rv).Require<ResetAudioError>();
}

void CoreProxyImpl::ResetVideo(uint32_t out_width, uint32_t out_height)
{
    FORCE_AS_MEMBER_FUNCTION();

    obs_video_info video_info;
    video_info.adapter = 0;
    video_info.gpu_conversion = true;
    video_info.graphics_module = GetConfiguredRendererModule(GetPrefs());
    video_info.output_format = GetConfiguredVideoFormat(GetPrefs());
    video_info.colorspace = GetConfiguredVideoColorspace(GetPrefs());
    video_info.range = GetConfiguredVideoRange(GetPrefs());
    video_info.scale_type = GetConfiguredScaleType(GetPrefs());
    std::tie(video_info.fps_num, video_info.fps_den) = GetConfiguredFPS(GetPrefs());
    std::tie(video_info.base_width, video_info.base_height) = GetConfiguredVideoOutputMetrics(GetPrefs());
    video_info.output_width = out_width;
    video_info.output_height = out_height;

    obs_video_info old_ovi;
    obs_get_video_info(&old_ovi);

    auto rv = obs_reset_video(&video_info);
    if (rv != OBS_VIDEO_SUCCESS)
    {
        LOG(ERROR) << "ResetVideo failed, code=" << rv;
    }
    if (rv == OBS_VIDEO_NOT_SUPPORTED) {
        throw VideoHardwareNotSupported("Video hardware doesn't meet DirectX requirements!");
    }

    ENSURE(rv == OBS_VIDEO_SUCCESS)(rv).Require<ResetVideoError>();

    if (obs_proxy_ui::GetUIProxy())
    {
        obs_proxy_ui::GetUIProxy()->ResizeMainOBSPreview(video_info.base_width, video_info.base_height);
    }
}

void CoreProxyImpl::BuildOutputService()
{
    try {
        obs_output_service_.reset(CreateOBSOutputService());
    } catch (const std::runtime_error& ex) {
        throw ResetOutputsError(ex.what());
    }
}

bool CoreProxyImpl::PostTaskToTick(std::function<void()> procedure)
{
    std::lock_guard<std::recursive_mutex> lg(s_task_queue_mutex_);
    s_task_queue_.push_back(procedure);
    return true;
}

void CoreProxyImpl::SetPublishBitrate(int bitrate)
{
    if (obs_output_service_)
    {
        obs_output_service_->SetPublishBitrate(bitrate);
    }
}

// static
void CoreProxyImpl::BindSignals()
{
    std::vector<Scene*> scenes = GetCoreProxy()->scene_collection_->GetScenes();
    obs_proxy_ui::UIProxyImpl::BindSignals(scenes);
}

CoreProxyImpl* GetCoreProxy()
{
    DCHECK(g_core_proxy_instance != nullptr);
    return g_core_proxy_instance;
}

}   // namespace obs_proxy
