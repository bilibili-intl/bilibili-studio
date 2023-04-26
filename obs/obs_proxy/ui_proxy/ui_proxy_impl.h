#ifndef BILILIVE_OBS_OBS_PROXY_UI_PROXY_UI_PROXY_IMPL_H_
#define BILILIVE_OBS_OBS_PROXY_UI_PROXY_UI_PROXY_IMPL_H_

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.hpp"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"

#include "base/observer_list.h"

namespace obs_proxy {
    class Scene;
}

namespace obs_proxy_ui {

class OBSPreview;
class SceneItemOpt;

class UIProxyImpl : public OBSUIProxy {
public:
    UIProxyImpl();
    virtual ~UIProxyImpl();

    static obs_scene_t* GetRawCurrentScene();
    static obs_scene_t* GetRawCurrentColiveScene();
    static void BindSignals(const std::vector<obs_proxy::Scene*>& scenes);
    static void InstallSignal(obs_proxy::Scene* scene);

    void ResetSourcesPos(obs_video_info& old_ovi);
    OBSPreview* main_obs_preview() { return main_obs_preview_.get(); }

    // OBSUIProxy
    bool is_initialized() const override;
    void Initialize() override;
    void ExecuteCommandWithParams(int cmd_id, const CommandParamsDetails& params) override;
    void ResizeMainOBSPreview(uint32_t base_cx, uint32_t base_cy) override;
    void AddObserver(OBSUIProxyObserver *observer) override;
    void RemoveObserver(OBSUIProxyObserver *observer) override;
    // 非主界面预览弹框绑定
    OBSPreview* ConnectOBSPReview(OBSPreviewController *preview_controller, PreviewType type) override;
    void DisonnectOBSPReview(OBSPreviewController *preview_controller) override;

private:
    void ChangeCurrentScene(size_t scene_id);

    // scene-collection callback
    static void OnCurrentSceneChangedHandlerRedirect(int scene_index);
    static void OnSourceShowHandlerRedirect(const std::string& item_name);
    static void OnAudioSourceActivateHandlerRedirect(obs_proxy::VolumeController* audio_source);
    static void OnAudioSourceDeactivateHandlerRedirect(const std::string & name);
    static void OnSourceRenameHandlerRedirect(const std::string& prev_name, const std::string& new_name);
    // scene callback
    static void OnSceneItemAddHandlerRedirect(const std::string& scene_name, const std::string& item_name, obs_source_t* source, int item_type);
    static void OnSceneItemRemoveHandlerRedirect(const std::string& scene_name, const std::string& item_name,
                                                 obs_source_t* underlying_source);
    static void OnSceneItemSelectedHandlerRedirect(const std::string& scene_name, const std::string& item_name);
    static void OnSceneItemDeselectHandlerRedirect(const std::string& scene_name, const std::string& item_name);
    static void OnSceneItemReorderedHandlerRedirect(const std::string& scene_name);
    static void OnSceneItemVisibleHandlerRedirect(const std::string& scene_name, const std::string& item_name,
                                                  bool visible);
    static void OnSceneItemLockedHandlerRedirect(const std::string& scene_name, const std::string& item_name,
                                                  bool locked);

private:
    bool is_initialized_ = false;
    std::unique_ptr<OBSPreview> main_obs_preview_;   // 主界面预览区使用的OBSPreview
    std::vector<std::unique_ptr<OBSPreview>> obs_previews_;// 预览映射以及其他需要预览弹框所使用的OBSPreview

    std::unique_ptr<SceneItemOpt> sceneitem_opt_;

    ObserverList<OBSUIProxyObserver> observer_list_;

    DISABLE_COPY(UIProxyImpl);
};

UIProxyImpl* GetUIProxy();

}   //namespace obs_proxy_ui

#endif // BILILIVE_OBS_OBS_PROXY_UI_PROXY_UI_PROXY_IMPL_H_