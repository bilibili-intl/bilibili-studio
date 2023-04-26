#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_PROXY_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_PROXY_H_

#include <functional>

#include "bilibase/signals.h"

#include "obs/obs_proxy/public/proxy/obs_output_service.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

class BililiveMainExtraParts;
class PrefRegistrySimple;
class CommandParamsDetails;

namespace base {

using string16 = std::wstring;
class FilePath;

}   // namespace base

namespace obs_proxy {

class SceneCollection;
class VolumeController;

class CoreInitError : public std::runtime_error {
public:
    explicit CoreInitError(const std::string& msg)
        : runtime_error(msg)
    {}
};

class StartOBSError : public CoreInitError {
public:
    explicit StartOBSError(const std::string& msg)
        : CoreInitError(msg)
    {}
};

class ResetAudioError : public CoreInitError {
public:
    explicit ResetAudioError(const std::string& msg)
        : CoreInitError(msg)
    {}
};

class ResetVideoError : public CoreInitError {
public:
    explicit ResetVideoError(const std::string& msg)
        : CoreInitError(msg)
    {}
};

class VideoHardwareNotSupported : public CoreInitError {
public:
    explicit VideoHardwareNotSupported(const std::string& msg)
        : CoreInitError(msg)
    {}
};

class ResetOutputsError : public CoreInitError {
public:
    explicit ResetOutputsError(const std::string& msg)
        : CoreInitError(msg)
    {}
};

// OBSCoreProxy provides a clean abstraction for underlying obs-core.
class OBSCoreProxy {
public:
    virtual ~OBSCoreProxy() {}

    // Initializes the obs-core.
    virtual void InitOBSCore() = 0;

    virtual void ShutdownOBSCore() = 0;

    // Updates audio info in obs-core.
    // Don't call this function when recoding or streaming.
    virtual bool UpdateAudioSettings() = 0;

    // Updates video info in obs-core.
    // Don't call this function when recoding or streaming.
    virtual bool UpdateVideoSettings() = 0;

    // Loads a scene collection from a given data file.
    virtual bool LoadSceneCollection(const base::FilePath& collection) = 0;

    virtual void SetCameraFrameHideStatus(bool status) = 0;

    // Uses default/predefined scene collection.
    // Invoking this function would overwrite existing scene collection file with default name,
    // i.e. "untitled.json". Therefore, if it isn't the first run, you should better try to
    // load default collection file.
    virtual void LoadDefaultSceneCollection() = 0;

    virtual OBSOutputService* GetOBSOutputService() const = 0;

    virtual SceneCollection* GetCurrentSceneCollection() const = 0;

    virtual bool EncoderIsSupported(const char *id) const = 0;

    virtual bool PostTaskToTick(std::function<void()> procedure) = 0;

    virtual void SetPublishBitrate(int bitrate) = 0;
};

}   // namespace proxy

namespace obs_proxy_ui {

class OBSPreviewController;
class OBSPreview;

enum class PreviewType
{
    PREVIEW_MAIN,
    PREVIEW_PROJECTOR,
    PREVIEW_SOURCE,
    PREVIEW_MAIN_PREVIEW,
};

class TableItem
{
public:
    TableItem()
        : contents_()
        , selected_(false)
        , visible_(true)
        , locked_(false)
        , is_plugin_(false)
    {
    }

    ~TableItem() {}

    void set_visible(bool v)
    {
        visible_ = v;
    }

    void set_select(bool s)
    {
        selected_ = s;
    }

    void set_locked(bool lock)
    {
        locked_ = lock;
    }

    void set_contents(const base::string16 &contents)
    {
        contents_ = contents;
    }

    void set_scene_type(obs_proxy::SceneItemType type)
    {
        type_ = type;
    }

    //bool is_orna_type()
    //{
    //    return type_ == obs_proxy::SceneItemType::OrnamentSource;
    //}

    void set_browser_plugin()
    {
        is_plugin_ = true;
    }

    bool is_browser_plugin()
    {
        return is_plugin_;
    }

public:
    base::string16 contents_;
    obs_proxy::SceneItemType type_;
    bool visible_;
    bool selected_;
    bool locked_;
    bool is_plugin_;
};

class OBSUIProxyObserver
{
public:
    virtual ~OBSUIProxyObserver() = default;

    // obs_proxy层初始化完毕，包括但不限于scene_collection加载完毕、场景及其item项加载完毕
    virtual void OnOBSUIProxyInitialized() {}

    // scene correlation
    virtual void OnCurrentSceneChanged(int scene_index) {}

    // source correlation
    virtual void OnSourceRename(const base::string16& prev_item_name, const base::string16& new_item_name) {}

    virtual void OnAudioSourceActivate(obs_proxy::VolumeController* audio_source) {}

    virtual void OnAudioSourceDeactivate(const std::string& name) {}

    // scene item correlation
    virtual void OnSceneItemAdd(const base::string16& scene_name, const TableItem& table_item, obs_source_t *source, int source_type) {}

    virtual void OnSceneItemRemove(const base::string16& scene_name, const base::string16& item_name) {}

    virtual void OnSceneItemSelected(const base::string16& scene_name, const base::string16& item_name) {}

    virtual void OnSceneItemDeselect(const base::string16& scene_name, const base::string16& item_name) {}

    virtual void OnSceneItemReordered(const base::string16& scene_name, const std::vector<string16> &item_names) {}

    virtual void OnSceneItemVisible(const base::string16& scene_name, const base::string16& item_name, bool visible) {}

    virtual void OnSceneItemLocked(const base::string16& scene_name, const base::string16& item_name, bool locked) {}
};

class OBSUIProxy {

public:
    virtual ~OBSUIProxy() {}

    virtual bool is_initialized() const = 0;

    virtual void Initialize() = 0;

    virtual void ExecuteCommandWithParams(int cmd_id, const CommandParamsDetails &params) = 0;

    virtual void ResizeMainOBSPreview(uint32_t base_cx, uint32_t base_cy) = 0;

    virtual void AddObserver(OBSUIProxyObserver *observer) = 0;

    virtual void RemoveObserver(OBSUIProxyObserver *observer) = 0;

    // 绑定新窗口到obs
    virtual OBSPreview* ConnectOBSPReview(OBSPreviewController *preview_controller, PreviewType type) = 0;
    // 解绑窗口与obs
    virtual void DisonnectOBSPReview(OBSPreviewController *preview_controller) = 0;
};

} // namespace obs_proxy_ui

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_PROXY_H_