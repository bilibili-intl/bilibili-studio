#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCES_WRAPPER_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCES_WRAPPER_H_

#include <functional>
#include <map>
#include <string>

#include "obs/obs_proxy/public/proxy/obs_source_properties.h"
#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"
#include "obs/obs-studio/libobs/obs.h"

namespace obs_proxy {

// Must keep compatible with obs definitions.
enum class OrderMovement {
    MoveUp,
    MoveDown,
    MoveTop,
    MoveBottom
};

enum class SceneItemType : size_t {
    Image = 0,
    Slider,
    MediaSource,
    Text,
    Scene,
    ColiveSource,
    VoiceLinkSource,
    DmkhimeSource,
    DisplayCapture,
    WindowCapture,
    GameCapture,
    VideoCaptureDevice,
    AudioInputCapture,
    AudioOutputCapture,
    ColorSource,
    ReceiverSource,
    BrowserSource,
    RtcColiveSource,
    TextureRendererSource,
    RtcVoiceLinkSource,
    SingIdentifySource,
    ThreeVtuberSource,
    MainSceneRendererSource,
};

const std::map<SceneItemType, std::string> kSceneTypeTable
{
    { SceneItemType::Image, "Image" },
    { SceneItemType::Slider, "Slider" },
    { SceneItemType::MediaSource, "MediaSource" },
    { SceneItemType::Text, "Text" },
    { SceneItemType::Scene, "Scene" },
    { SceneItemType::ColiveSource, "ColiveSource" },
    { SceneItemType::VoiceLinkSource, "VoiceLinkSource" },
    { SceneItemType::DmkhimeSource, "DmkhimeSource" },
    { SceneItemType::DisplayCapture, "DisplayCapture" },
    { SceneItemType::WindowCapture, "WindowCapture" },
    { SceneItemType::GameCapture, "GameCapture" },
    { SceneItemType::VideoCaptureDevice, "VideoCaptureDevice" },
    { SceneItemType::AudioInputCapture, "AudioInputCapture" },
    { SceneItemType::AudioOutputCapture, "AudioOutputCapture" },
    { SceneItemType::ColorSource, "ColorSource" },
    { SceneItemType::ReceiverSource, "ReceiverSource" },
    { SceneItemType::BrowserSource, "BrowserSource" },
    { SceneItemType::RtcColiveSource, "RtcColiveSource" },
    { SceneItemType::TextureRendererSource, "TextureRendererSource" },
    { SceneItemType::RtcVoiceLinkSource, "RtcVoiceLinkSource" },
    { SceneItemType::SingIdentifySource, "SingIdentifySource" },
    { SceneItemType::ThreeVtuberSource, "ThreeVtuberSource" },
    { SceneItemType::MainSceneRendererSource, "MainSceneRendererSource" },
};

enum class FilterType : size_t {
    Mask = 0,
    Crop,
    Gain,
    Color,
    Scroll,
    ColorKey,
    Sharpness,
    ChromaKey,
    AsyncDelay,
    NoiseSuppress,
    Beauty,
    ForceMono,
    Reverb,
    Unpremultiply,
    FlipSetting,
    MP4Motion,
    SingIdentify,
	MicRecordFilter,
};

class Filter {
public:
    virtual ~Filter() {}

    virtual const std::string& name() const = 0;

    virtual FilterType type() const = 0;

    virtual PropertyValue& GetPropertyValues() = 0;

    virtual void UpdatePropertyValues() = 0;
};

struct TransformState
{
    float rot;
    float posx, posy;
    float scalex, scaley;
};

struct CropInfo
{
    CropInfo()
        : x(0.5f)
        , y(0)
        , w(0.5f)
        , h(1.0f)
    {
    }

    void zero()
    {
        x = y = w = h = 0;
    }

    void InitForModel(bool for_landscape)
    {
        if (for_landscape)
        {
            x = 0.5f;
            y = 0;
            w = 0.5f;
            h = 1.0f;
        }
        else
        {
            x = 0;
            y = 0.18359375f;
            w = 1.0f;
            h = 0.6328125f;
        }
    }

    bool operator==(const CropInfo &rhs) const
    {
        return x == rhs.x &&
            y == rhs.y &&
            w == rhs.w &&
            h == rhs.h;
    }

    bool operator!=(const CropInfo &rhs) const
    {
        return
            x != rhs.x ||
            y != rhs.y ||
            w != rhs.w ||
            h != rhs.h;
    }

    float x;
    float y;
    float w;
    float h;
};

using BoundsInfo = CropInfo;

class SceneItem {
public:
    using DeferredUpdatedHanlder = void(*)(void*);

    virtual ~SceneItem() {}

    virtual void MoveOrder(OrderMovement movement) = 0;

    virtual void SetOrder(int position) = 0;

    virtual Filter* AddNewFilter(FilterType type, const std::string& name) = 0;

    virtual void RemoveFilter(const std::string& name) = 0;

    virtual Filter* GetFilter(const std::string& name) const = 0;

    virtual std::vector<Filter*> GetFilters() const = 0;

    virtual bool ReorderFilters(const std::vector<Filter*>& filters) = 0;

    virtual bool IsDeferUpdateItem() const = 0;

    virtual bool IsDeferredUpdatedHanlderInstalled() const = 0;

    virtual void InstallDeferredUpdatedHandler(DeferredUpdatedHanlder handler, void* param) = 0;

    virtual PropertyValue& GetPropertyValues() = 0;

    virtual void UpdatePropertyValues() = 0;

    // Some itemes, such as window-capture, need refresh their properties each time.
    virtual const Properties& GetProperties(PropertiesRefreshMode mode) const = 0;

    virtual bool IsVisible() const = 0;

    virtual void SetVisible(bool visible) = 0;

    virtual bool IsSelected() const = 0;

    // `ItemSelectedHandler` and `ItemDeselectedHandler` are invoked only when selection status
    // changes.
    virtual void Setlect(bool select) = 0;

    virtual void SetMuted(bool mute) = 0;

    virtual bool IsMuted() = 0;

    virtual void SetLocked(bool lock) = 0;

    virtual bool IsLocked() = 0;

    // Flip the scene item.

    virtual void DoVerticalFlip() = 0;

    virtual void DoHorizontalFlip() = 0;

    virtual void DoLeftRotate() = 0;

    virtual void DoRightRotate() = 0;

    virtual void DoSourceItemRotateLeft() = 0;

    virtual void DoSourceItemRotateRight() = 0;

    virtual float GetSourcesItemRotateVal() = 0;

    virtual TransformState GetTransformState() = 0;

    virtual void SetTransformState(TransformState state) = 0;

    virtual void SetFitToScreen(bool fit_to_screen) = 0;
    virtual bool IsFitToScreen() const = 0;

    virtual void SizeToActualSize() = 0;

    virtual void SizeToScreenSize() = 0;

    virtual int GetWidth() = 0;

    virtual int GetHeight() = 0;

    virtual void SetBounds(float width, float height) = 0;

    virtual void SetScale(float x_scale, float y_scale) = 0;

    // Returns true, if succeed;
    // Returns false, if there already is a source with the `new_name`.
    virtual bool Rename(const std::string& new_name) = 0;

    virtual const std::string& name() const = 0;

    virtual SceneItemType type() const = 0;

    virtual const std::string& type_name() const = 0;

    virtual void CropAndSetBounds(const CropInfo &crop_info, const CropInfo &bounds_info) = 0;
    virtual void Crop(const CropInfo &crop_info, const BoundsInfo* bounds_info = nullptr) = 0;

    enum FixType { INNER_RECT = 0, OUTER_RECT, FILL_HEIGHT, FILL_WIDTH };

    virtual void EnableFixSize(int x, int y, int w, int h, FixType fixType = INNER_RECT) = 0;

    virtual void DisableFixSize() = 0;

    virtual matrix4 GetBoxTransform() const = 0;
};

class Scene {
public:
    using ItemAddHandler = std::function<void(const std::string& scene_name,
                                              const std::string& item_name, obs_source_t*, int item_type)>;
    using ItemRemoveHandler = std::function<void(const std::string& scene_name,
                                                 const std::string& item_name,
                                                 obs_source_t* underlying_source)>;
    using ItemSelectedHandler = std::function<void(const std::string& scene_name,
                                                   const std::string& item_name)>;
    using ItemDeselectedHandler = std::function<void(const std::string& scene_name,
                                                     const std::string& item_name)>;
    using ItemReorderedHandler = std::function<void(const std::string& scene_name)>;
    using ItemVisibleHandler = std::function<void(const std::string& scene_name,
                                                  const std::string& item_name,
                                                  bool visible)>;
    using ItemLockedHandler = std::function<void(const std::string& scene_name,
                                                  const std::string& item_name,
                                                  bool locked)>;

    virtual ~Scene() {}

    // Create a new sub item in the current scene.
    // Returns a view pointer (no ownership transfer) to the caller if the function succeeds;
    // Returns nullptr, otherwise.
    virtual SceneItem* AttachNewItem(SceneItemType type, const std::string& name, bool visible) = 0;

    // Create a new colive_source in the current scene.
    // Returns a view pointer (no ownership transfer) to the caller if the function succeeds;
    // Returns nullptr, otherwise.
    virtual SceneItem* AttachNewColiveRemoteSource(uint64_t uid, const std::string& name, bool visible,bool use_rtc_colive = false,bool is_little_video = false) = 0;

    // Attach a scene indicated by name into the current scene.
    // If you attach some scenes recursively, the result is undefined.
    // Attaching the same scene multiple times will cause mouse-picking issues.
    virtual void AttachExistingScene(const std::string& name, bool visible) = 0;

    virtual SceneItem* AddRawSource(obs_source_t* source, const std::string& name, SceneItemType type) = 0;

    virtual void RemoveItem(const std::string& name) = 0;

    virtual SceneItem* GetItem(const std::string& name) const = 0;

    virtual SceneItem* GetSelectedItem() const = 0;

    virtual std::vector<SceneItem*> GetItems() const = 0;

    virtual const std::string& name() const = 0;

    virtual void RegisterItemAddHandler(ItemAddHandler handler) = 0;

    virtual void RegisterItemRemoveHandler(ItemRemoveHandler handler) = 0;

    virtual void RegisterItemSelectedHandler(ItemSelectedHandler handler) = 0;

    virtual void RegisterItemDeselectHandler(ItemDeselectedHandler handler) = 0;

    virtual void RegisterItemReorderedHandler(ItemReorderedHandler handler) = 0;

    virtual void RegisterItemVisibleHandler(ItemVisibleHandler handler) = 0;

    virtual void RegisterItemLockedHandler(ItemLockedHandler handler) = 0;
    // 当前版本的obs对于item的lock是没有signal通知的，仅仅只是置一个标识位而已，
    // 需要我们的自己的业务层自行决定什么时候该触发回调，在此设置一个接口给item自己调用
    // 过后如果obs自己提供了signal的通知绑定则应该把此接口去掉
    virtual void SignaledItemLockedChanged(const std::string& item_name) = 0;
};

} // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCES_WRAPPER_H_
