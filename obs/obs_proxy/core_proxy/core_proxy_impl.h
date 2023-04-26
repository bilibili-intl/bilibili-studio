#ifndef OBS_OBS_PROXY_CORE_PROXY_CORE_PROXY_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_CORE_PROXY_IMPL_H_

#include <memory>
#include <list>

#include "base/files/file_path.h"

#include "bilibase/basic_macros.h"

#include "obs/obs_proxy/core_proxy/common/basic_types.h"
#include "obs/obs_proxy/core_proxy/output_services/obs_output_service_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_scene_collection_impl.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"

namespace obs_proxy {

class CoreProxyImpl : public OBSCoreProxy {
public:
    CoreProxyImpl();

    ~CoreProxyImpl();

    DISABLE_COPY(CoreProxyImpl);

    // OBSCoreProxy
    void InitOBSCore() override;

    void ShutdownOBSCore() override;

    bool UpdateAudioSettings() override;

    bool UpdateVideoSettings() override;

    bool LoadSceneCollection(const base::FilePath& collection) override;

    void LoadDefaultSceneCollection() override;

    void SetCameraFrameHideStatus(bool status) override;

    OBSOutputService* GetOBSOutputService() const override;

    SceneCollection* GetCurrentSceneCollection() const override;

    bool EncoderIsSupported(const char *id) const override;

    bool PostTaskToTick(std::function<void()> procedure) override;

    void SetPublishBitrate(int bitrate) override;
    //todo: sure: temp, should be at right place
    static void BindSignals();

private:
    void ResetAudio();

    void ResetVideo(uint32_t out_width, uint32_t out_height);

    void BuildOutputService();

private:
    std::unique_ptr<SceneCollectionImpl> scene_collection_;
    std::unique_ptr<OBSOutputService> obs_output_service_;
};

// Returns a non-owning pointer to the core proxy instance.
// Use this function only in the obs-proxy module.
CoreProxyImpl* GetCoreProxy();

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_CORE_PROXY_IMPL_H_