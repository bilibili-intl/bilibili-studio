#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_PRESENTER_IMPL_H_

#include "base/memory/weak_ptr.h"

#include "bililive/bililive/livehime/sources_properties/source_camera_property_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"

#include "obs/obs-studio/libobs/obs.hpp"


const int kChromaKeySimilarMin = 1;
const int kChromaKeySimilarMax = 1000;

const int kChromaKeySmoothMin = 1;
const int kChromaKeySmoothMax = 1000;

const int kChromaKeySpillMin = 1;
const int kChromaKeySpillMax = 1000;


class SourceCameraPropertyPresenterImpl
    : public contracts::SourceCameraPropertyPresenter
{
public:
    explicit SourceCameraPropertyPresenterImpl(obs_proxy::SceneItem* scene_item,
        contracts::SourceCameraPropertyView* view = nullptr,
        contracts::SourceCameraCallback* callback = nullptr,
        bool need_regist_signal = false, const std::string& tab_type ="");

    ~SourceCameraPropertyPresenterImpl()
    {
        if (need_regist_signal_)
        {
            camera_signal_.Disconnect();
        }
    }

    bool Initialize() override;

    float GetVolumeValue() override;
    void SetVolumeValue(float value) override;

    bool IsMuted() override;
    void SetMuted(bool muted) override;

    int GetAudioMonitoringType() override;
    void SetAudioMonitoringType(int status) override;

    PropertyList<std::string> GetCameraList(const string16& default_item) override;

    std::string GetSelectedCamera() override;
    void SetSelectedCamera(const std::string& value) override;

    std::string SelectedAudioDevice() override;
    void SelectedAudioDevice(std::string val) override;

    bool IsCustomAudioDevice() override;
    void SetCustomAudioDevice(bool val) override;

    PropertyList<std::string> CustomAudioDeviceList() override;

    void LaunchConfig() override;

    void VerticalFlip() override;

    void HorizontalFlip() override;

    PropertyList<std::string> GetResolutionList(const string16& default_item) override;
    std::string GetSelectedResolution() override;
    void SetSelectedResolution(const std::string& value) override;
    PropertyList<int64> GetAudioOutputList() override;
    int64 GetSelectedAudioOutput() override;
    void SetSelectedAudioOutput(int64 value) override;
    void RegisterSceneItemDeferredUpdate() override;

    std::vector<std::wstring> LoadColorFilterList(const string16& filter_str) override;

    void CameraSnapshot() override;
    void CameraRestore() override;

    //顺时针旋转
    float GetSourcesItemRotateVal() override;
    void ClockwiseRotateCamera() override;

    //垂直翻转保存
    void SetVerticalFlip(bool val) override;
    bool GetVerticalFlip() override;

    //水平翻转保存
    void SetHorizontalFlip(bool val) override;
    bool GetHorizontalFlip() override;

    //90度旋转保存
    void SetCameraRotate(int val) override;
    int GetCameraRotate() override;

    void SetCameraXScale(bool val) override;
    bool GetCameraXScale() override;
    void SetCameraYScale(bool val) override;
    bool GetCameraYScale() override;

    static void OnCameraSignal(void* data, calldata_t* params);

private:
    void RegistSignal(const obs_source_t* source);
    OBSSignal camera_signal_;
    contracts::SourceCameraCallback* callback_;
    bool need_regist_signal_;
    CameraSceneItemHelper camera_scene_item_;
    std::unique_ptr<livehime::CameraPropertySnapshot> camera_snapshot_;
    std::unique_ptr<livehime::BeautyPropertySnapshot> beauty_snapshot_;
    //std::unique_ptr<livehime::ColorKeyPropertySnapshot> color_snapshot_;
    std::unique_ptr<livehime::BeautyShapePropertySnapshot> beauty_shape_snapshot_;

    contracts::SourceCameraPropertyView* view_;

    std::shared_ptr<int> canary_;
    base::WeakPtrFactory<SourceCameraPropertyPresenterImpl> wptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(SourceCameraPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_PRESENTER_IMPL_H_

