#include "bililive/bililive/livehime/obs/obs_proxy_service.h"

#include "base/logging.h"
#include "base/path_service.h"

#include "bililive/bililive/bililive_main_loop.h"
#include "bililive/bililive/livehime/obs/obs_proxy_main_extra_parts.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_paths.h"

#include "obs/obs_proxy/public/obs_proxy_import.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

namespace {

OBSProxyService* g_proxy_service_instance = nullptr;

}   // namespace

OBSProxyService::OBSProxyService()
    : bililive_obs_view_(nullptr)
{}

OBSProxyService::~OBSProxyService()
{}

// static
bool OBSProxyService::InitInstance(BililiveMainParts* main_parts)
{
    DCHECK(g_proxy_service_instance == nullptr) << "OBSProxyService initialized twice";
    g_proxy_service_instance = new OBSProxyService();

    BindOBSProxy(g_proxy_service_instance);

    main_parts->AddParts(new livehime::OBSProxyMainExtraParts());

    return true;
}

// static
void OBSProxyService::CleanupInstance()
{
    if (g_proxy_service_instance)
    {
        delete g_proxy_service_instance;
        g_proxy_service_instance = nullptr;
    }
}

// static
bool OBSProxyService::HasInstance()
{
    return g_proxy_service_instance != nullptr;
}

// static
OBSProxyService& OBSProxyService::GetInstance()
{
    CHECK(g_proxy_service_instance != nullptr);
    return *g_proxy_service_instance;
}


obs_proxy::OBSCoreProxy* OBSProxyService::GetOBSCoreProxy() const
{
    DCHECK(obs_proxy_ != nullptr);
    return obs_proxy_.get();
}

BililiveProcess* OBSProxyService::GetBililiveProcess()
{
    return ::GetBililiveProcess();
}

BililiveMainParts* OBSProxyService::GetBililiveMainParts()
{
    return BililiveMainLoop::GetInstance()->main_parts();
}

base::NotificationService* OBSProxyService::GetNotificationService()
{
    return base::NotificationService::current();
}

BililiveOBSView* OBSProxyService::GetBililiveOBSView()
{
    return bililive_obs_view_;
}

void OBSProxyService::SetCoreProxy(obs_proxy::OBSCoreProxy* proxy)
{
    DCHECK(obs_proxy_ == nullptr);
    obs_proxy_.reset(proxy);
}

void OBSProxyService::SetUIProxy(obs_proxy_ui::OBSUIProxy* proxy_ui)
{
    DCHECK(obs_proxy_ui_ == nullptr);
    obs_proxy_ui_.reset(proxy_ui);
}

// static
void OBSProxyService::RegisterProfilePrefs(PrefRegistrySimple* registry)
{
    registry->RegisterIntegerPref(prefs::kAudioSampleRate, prefs::kDefaultAudioSampleRate);
    registry->RegisterStringPref(prefs::kAudioChannelSetup, prefs::kDefaultAudioChannelSetup);
    registry->RegisterIntegerPref(prefs::kAudioBitRate, prefs::kDefaultAudioBitRate);
    registry->RegisterIntegerPref(prefs::kAudioDenoise, prefs::kDefaultAudioDenoise);
    registry->RegisterStringPref(prefs::kAudioMonoChannelSetup, prefs::kAudioMonoNoMix);
    registry->RegisterStringPref(prefs::kAudioMicDeviceInUse, prefs::kNoAudioDevice);
    registry->RegisterStringPref(prefs::kAudioSpeakerDeviceInUse, prefs::kNoAudioDevice);

    registry->RegisterIntegerPref(prefs::kVideoBitRate, prefs::kDefaultVideoBitRate);
    registry->RegisterStringPref(prefs::kVideoBitRateControl, prefs::kDefaultVideoBitRateControl);
    registry->RegisterIntegerPref(prefs::kVideoFPSType, prefs::kDefaultVideoFPSType);
    registry->RegisterStringPref(prefs::kVideoFPSCommon, prefs::kDefaultFPSCommon);
    registry->RegisterIntegerPref(prefs::kVideoFPSInt, prefs::kDefaultVideoInt);
    registry->RegisterIntegerPref(prefs::kVideoFPSNum, prefs::kDefaultVideoNum);
    registry->RegisterIntegerPref(prefs::kVideoFPSDen, prefs::kDefaultVideoDen);
    registry->RegisterStringPref(prefs::kVideoRenderer, prefs::kDefaultVideoRenderer);

    registry->RegisterIntegerPref(prefs::kVideoOutputCX, prefs::kDefaultVideoOutputWidth);
    registry->RegisterIntegerPref(prefs::kVideoOutputCY, prefs::kDefaultVideoOutputHeight);

    registry->RegisterStringPref(prefs::kVideoColorFormat, prefs::kDefaultVideoColorFormat);
    registry->RegisterStringPref(prefs::kVideoColorSpace, prefs::kDefaultVideoColorSpace);
    registry->RegisterStringPref(prefs::kVideoColorRange, prefs::kDefaultVideoColorRange);
    registry->RegisterStringPref(prefs::kVideoScaleType, prefs::kDefaultVideoScaleType);

    registry->RegisterStringPref(prefs::kOutputStreamVideoEncoder,
        prefs::kDefaultOutputStreamVideoEncoder);
    registry->RegisterIntegerPref(prefs::kOutputStreamVideoQuality,
        prefs::kVideoStreamQualityDefault);
    registry->RegisterStringPref(prefs::kOutputRecordVideoEncoder,
        prefs::kDefaultOutputRecordVideoEncoder);
    registry->RegisterStringPref(prefs::kOutputRecordVideoQuality,
        prefs::kDefaultOutputRecordVideoQuality);
    registry->RegisterBooleanPref(prefs::kOutputReconnect, prefs::kDefaultOutputReconnect);
    registry->RegisterIntegerPref(prefs::kOutputRetryDelay, prefs::kDefaultOutputRetryDelay);
    registry->RegisterIntegerPref(prefs::kOutputMaxRetries, prefs::kDefaultOutputMaxRetries);
    registry->RegisterBooleanPref(prefs::kOutputUseDelay, prefs::kDefaultOutputUseDelay);
    registry->RegisterIntegerPref(prefs::kOutputDelaySec, prefs::kDefaultOutputDelaySec);
    registry->RegisterBooleanPref(prefs::kOutputDelayPreserve, prefs::kDefaultOutputDelayPreserve);

    registry->RegisterStringPref(prefs::kLastSceneCollectionName, prefs::kDefaultSceneCollectionName);

    base::FilePath user_video_dir(prefs::kFallbackOutputDir);    // use current directory as a fallback.
    PathService::Get(bililive::DIR_USER_VIDEOS, &user_video_dir);
    registry->RegisterFilePathPref(prefs::kOutputFolder, user_video_dir);
    registry->RegisterStringPref(prefs::kOutputRecordingFormat, prefs::kDefaultRecordingFormat);
    registry->RegisterBooleanPref(prefs::kOutputRecordingSync, prefs::kDefaultRecordingSync);

    registry->RegisterBooleanPref(prefs::kCenterSnapping, prefs::kDefaultCenterSnapping);
    registry->RegisterBooleanPref(prefs::kScreenSnapping, prefs::kDefaultScreenSnapping);
    registry->RegisterBooleanPref(prefs::kSourceSnapping, prefs::kDefaultSourceSnapping);
    registry->RegisterBooleanPref(prefs::kSnappingEnabled, prefs::kDefaultSnappingEnabled);
    registry->RegisterDoublePref(prefs::kSnapDistance, prefs::kDefaultSnapDistance);
    registry->RegisterStringPref(prefs::kSourceDragMode, prefs::kSourceDragModeZoom);
    registry->RegisterBooleanPref(prefs::kAnalysisLiveQuality, prefs::kDefaultAnalysisLiveQuality);
}

void OBSProxyService::TweakProfilePrefs(PrefService* prefs)
{}
