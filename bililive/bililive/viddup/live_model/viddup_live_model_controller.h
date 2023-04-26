#pragma once

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"


class ViddupLiveModelController : LiveModelController
{
public:
    ViddupLiveModelController();
    ~ViddupLiveModelController();

    static LiveModelController* GetInstance();

    void AddObserver(LiveModelControllerObserver *observer) override
    {
        observer_list_.AddObserver(observer);
    }

    void RemoveObserver(LiveModelControllerObserver *observer) override
    {
        observer_list_.RemoveObserver(observer);
    }

    bool ChangeLayoutModel(ModelType type, bool user_invoke = true) override;
    ModelType layout_model() const override;
    bool IsLandscapeModel() const override;

    void ShowFauxAudientEffect(bool show, bool landscape) override;

    bool ChangeCurrentScene(const std::string& scene_name) override;
    std::string current_model_scene_name() const override;

    void Uninitialize() override;

protected:
    // OBSUIProxyObserver
    void OnOBSUIProxyInitialized() override;

private:
    void Initialize();

private:
    friend class BililiveOBS;
    friend struct std::default_delete<ViddupLiveModelController>;

    ObserverList<LiveModelControllerObserver> observer_list_;

    ModelType model_type_ = ModelType::Unknown;
    std::string current_model_scene_name_;
    std::string last_landscape_model_scene_name_;
    std::string last_portrait_model_scene_name_;

    base::WeakPtrFactory<ViddupLiveModelController> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ViddupLiveModelController);
};