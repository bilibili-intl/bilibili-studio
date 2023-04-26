#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_CONTRACT_H_

#include <vector>

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {
struct CameraReportInfo
{
    int light = 0;
    int blur = 0;
    int sharpening = 0;
    int face_lefting = 0;
    int bigger_eyes = 0;
    int chin = 0;
    int brow = 0;
    int nose = 0;
    int shape_face = 0;
    int filter = 0;
    int face_sticker = 0;
};

class SourceCameraPropertyPresenter
    : public SourcePublicPropertyPresenter
{
public:
    explicit SourceCameraPropertyPresenter(obs_proxy::SceneItem* scene_item)
        : SourcePublicPropertyPresenter(scene_item){}

    virtual ~SourceCameraPropertyPresenter() {}

    virtual bool Initialize() = 0;

    virtual float GetVolumeValue() = 0;
    virtual void SetVolumeValue(float value) = 0;

    virtual bool IsMuted() = 0;
    virtual void SetMuted(bool muted) = 0;

    virtual int GetAudioMonitoringType() = 0;
    virtual void SetAudioMonitoringType(int status) = 0;

    virtual PropertyList<std::string> GetCameraList(const string16& default_item) = 0;

    virtual std::string GetSelectedCamera() = 0;
    virtual void SetSelectedCamera(const std::string& value) = 0;

    virtual std::string SelectedAudioDevice() = 0;
    virtual void SelectedAudioDevice(std::string val) = 0;

    virtual bool IsCustomAudioDevice() = 0;
    virtual void SetCustomAudioDevice(bool val) = 0;

    virtual PropertyList<std::string> CustomAudioDeviceList() = 0;

    virtual void LaunchConfig() = 0;

    virtual void VerticalFlip() = 0;

    virtual PropertyList<std::string> GetResolutionList(const string16& default_item) = 0;
    virtual std::string GetSelectedResolution() = 0;
    virtual void SetSelectedResolution(const std::string& value) = 0;
    virtual PropertyList<int64> GetAudioOutputList() = 0;
    virtual int64 GetSelectedAudioOutput() = 0;
    virtual void SetSelectedAudioOutput(int64 value) = 0;
    virtual void HorizontalFlip() = 0;  

    virtual void RegisterSceneItemDeferredUpdate() = 0;

    virtual std::vector<std::wstring> LoadColorFilterList(const string16& filter_str) = 0;

    virtual void CameraSnapshot() = 0;
    virtual void CameraRestore() = 0;

    //垂直翻转保存
    virtual void SetVerticalFlip(bool val) = 0;
    virtual bool GetVerticalFlip() = 0;

    //水平翻转保存
    virtual void SetHorizontalFlip(bool val) = 0;
    virtual bool GetHorizontalFlip() = 0;

    virtual void ClockwiseRotateCamera() = 0;

    virtual float GetSourcesItemRotateVal() = 0;

    //90度旋转保存
    virtual void SetCameraRotate(int val) = 0;
    virtual int GetCameraRotate() = 0;

    //把scal传下去
    virtual void SetCameraXScale(bool val) = 0;
    virtual bool GetCameraXScale() = 0;
    virtual void SetCameraYScale(bool val) = 0;
    virtual bool GetCameraYScale() = 0;

};

class SourceCameraCallback
{
public:
    virtual ~SourceCameraCallback() {}

    virtual void CameraShowBeauty(bool show) = 0;
};

class SourceCameraPropertyView {
public:
    virtual ~SourceCameraPropertyView() {}

    virtual void RefreshResolutionListCallback() = 0;
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_CAMERA_PROPERTY_CONTRACT_H_