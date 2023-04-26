#pragma once

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"


enum UISceneIndex
{
    UISI_NOTHING = -1,
    UISI_LANDSCAPE_SCENE0,
    UISI_LANDSCAPE_SCENE1,
    UISI_LANDSCAPE_SCENE2,
    UISI_VERTICAL_SCENE0,
    UISI_VERTICAL_SCENE1,
    UISI_VERTICAL_SCENE2,
    UISI_COUNT,
};


class LiveModelControllerObserver
{
public:
    // 横竖屏已经变更
    virtual void OnLiveLayoutModelChanged(bool user_invoke) {}

    // 直播间预览状态已变更
    virtual void OnLiveRoomPreviewStatusChanged(bool show, bool landscape) {}

protected:
    virtual ~LiveModelControllerObserver() = default;
};

class LiveModelController : public obs_proxy_ui::OBSUIProxyObserver
{
public:
    LiveModelController();
    virtual ~LiveModelController();

    enum class ModelType
    {
        Unknown = -1,
        Landscape,
        Portrait,
    };

    static LiveModelController* GetInstance();

    virtual void AddObserver(LiveModelControllerObserver *observer)
    {
        observer_list_.AddObserver(observer);
    }

    virtual void RemoveObserver(LiveModelControllerObserver *observer)
    {
        observer_list_.RemoveObserver(observer);
    }

    // 改变横竖屏模式
    virtual bool ChangeLayoutModel(ModelType type, bool user_invoke = true);
    virtual ModelType layout_model() const;
    virtual bool IsLandscapeModel() const;

    // 开启/关闭直播间预览
    virtual void ShowFauxAudientEffect(bool show, bool landscape);

    // UI层的场景切换统一通过这里来操作
    virtual bool ChangeCurrentScene(const std::string& scene_name);
    virtual std::string current_model_scene_name() const;

    virtual void Uninitialize();

protected:
    // OBSUIProxyObserver
    void OnOBSUIProxyInitialized() override;

private:
    void Initialize();

private:
    friend class BililiveOBS;
    friend struct std::default_delete<LiveModelController>;

    ObserverList<LiveModelControllerObserver> observer_list_;

    ModelType model_type_ = ModelType::Unknown;
    std::string current_model_scene_name_;
    std::string last_landscape_model_scene_name_;
    std::string last_portrait_model_scene_name_;

    base::WeakPtrFactory<LiveModelController> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LiveModelController);
};