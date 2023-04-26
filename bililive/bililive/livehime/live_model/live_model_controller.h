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
    // �������Ѿ����
    virtual void OnLiveLayoutModelChanged(bool user_invoke) {}

    // ֱ����Ԥ��״̬�ѱ��
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

    // �ı������ģʽ
    virtual bool ChangeLayoutModel(ModelType type, bool user_invoke = true);
    virtual ModelType layout_model() const;
    virtual bool IsLandscapeModel() const;

    // ����/�ر�ֱ����Ԥ��
    virtual void ShowFauxAudientEffect(bool show, bool landscape);

    // UI��ĳ����л�ͳһͨ������������
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