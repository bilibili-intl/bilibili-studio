#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCES_WRAPPER_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCES_WRAPPER_IMPL_H_

#include <memory>
#include <vector>

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/core_proxy/common/basic_types.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_properties_impl.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

namespace obs_proxy {

template<typename T, typename R>
std::vector<R*> AcquireSourceViews(const std::vector<std::unique_ptr<T>>& sources)
{
    static_assert(std::is_base_of<R, T>::value, "Types of source and view don't match");
    std::vector<R*> views;
    views.reserve(sources.size());
    std::transform(sources.begin(), sources.end(), std::back_inserter(views),
                   [](const std::unique_ptr<T>& source) {
                       return source.get();
                   });
    return views;
}

class FilterImpl;
class SceneImpl;

class SceneItemImpl : public SceneItem {
public:
    SceneItemImpl(SceneImpl& scene, SceneItemType type, const std::string& name, bool visible);

    SceneItemImpl(obs_sceneitem_t* raw_scene_item, SceneItemType type, const std::string& name);

    // For scene -> scene_item
    SceneItemImpl(SceneImpl& host_scene, const std::string& scene_name, bool visible);

    // For colive_source
    SceneItemImpl(SceneImpl& scene, const std::string& name, uint64_t uid, bool visible, bool use_rtc_colive = false,bool is_little_video = false);

    ~SceneItemImpl();

    DISABLE_COPY(SceneItemImpl);

    void MoveOrder(OrderMovement movement) override;

    void SetOrder(int position) override;

    Filter* AddNewFilter(FilterType type, const std::string& name) override;

    void AddExistingFilter(std::unique_ptr<FilterImpl> filter);

    void RemoveFilter(const std::string& name) override;

    Filter* GetFilter(const std::string& name) const override;

    std::vector<Filter*> GetFilters() const override;

    bool ReorderFilters(const std::vector<Filter*>& filters) override;

    bool IsDeferUpdateItem() const override;

    bool IsDeferredUpdatedHanlderInstalled() const override;

    void InstallDeferredUpdatedHandler(DeferredUpdatedHanlder handler, void* param) override;

    PropertyValue& GetPropertyValues() override;

    void UpdatePropertyValues() override;

    const Properties& GetProperties(PropertiesRefreshMode mode) const override;

    SceneItemType type() const override;

    const std::string& type_name() const override;

    const std::string& name() const override;

    bool IsVisible() const override;

    void SetVisible(bool visible) override;

    bool IsSelected() const override;

    void Setlect(bool select) override;

    void SetMuted(bool mute) override;

    bool IsMuted() override;

    void SetLocked(bool lock) override;

    bool IsLocked() override;

    void DoVerticalFlip() override;

    void DoHorizontalFlip() override;

    void DoLeftRotate() override;

    void DoRightRotate() override;

    void DoSourceItemRotateLeft() override;

    void DoSourceItemRotateRight() override;

    float GetSourcesItemRotateVal() override;

    TransformState GetTransformState() override;

    void SetTransformState(TransformState state) override;

    void SetFitToScreen(bool fit_to_screen) override;
    bool IsFitToScreen() const override;

    void SizeToActualSize() override;

    void SizeToScreenSize() override;

    void SetBounds(float width, float height) override;

    int GetWidth() override;

    int GetHeight() override;

    void SetScale(float x_scale, float y_scale) override;

    bool Rename(const std::string& new_name) override;

    void CropAndSetBounds(const CropInfo &crop_info, const CropInfo &bounds_info) override;
    void Crop(const CropInfo &crop_info, const BoundsInfo* bounds_info = nullptr) override;

    void EnableFixSize(int x, int y, int w, int h, FixType fixType) override;

    void DisableFixSize() override;

    // These two functions are for workaround for some edge cases.
    // Be cautious when you use them.

    obs_sceneitem_t* LeakUnderlyingSource() const
    {
        return scene_item_;
    }

    obs_source_t* AsSource() const
    {
        return obs_sceneitem_get_source(scene_item_);
    }

    vec2 GetSourceSize() const;

    matrix4 GetBoxTransform() const override;

private:
    SceneItemType type_;
    std::string name_;
    obs_sceneitem_t* scene_item_;
    std::unique_ptr<PropertiesImpl> properties_;
    PropertyValue property_values_;
    std::vector<std::unique_ptr<FilterImpl>> filters_;
};

class SceneImpl : public Scene {
public:
    explicit SceneImpl(const std::string& name, bool custom_size, int cx, int cy);

    ~SceneImpl();

    DISABLE_COPY(SceneImpl);

    // A complete `Scene` object contains its sub scene-items, so we need to dig and attach
    // sub items during constrution. However, if we do all works in constructor, we have to
    // do something underhanded, which is due to weird design of obs-interfaces, so this creator
    // function comes to rescue.
    static std::unique_ptr<SceneImpl> FromExistingScene(obs_scene_t* raw_scene);

    SceneItem* AttachNewItem(SceneItemType type, const std::string& name, bool visible) override;

    SceneItem* AttachNewColiveRemoteSource(uint64_t uid, const std::string& name, bool visible, bool use_rtc_colive, bool is_little_video) override;

    void AttachExistingItem(std::unique_ptr<SceneItemImpl> scene_item);

    void AttachExistingScene(const std::string& name, bool visible) override;

    SceneItem* AddRawSource(obs_source_t* source, const std::string& name, SceneItemType type) override;

    void RemoveItem(const std::string& name) override;

    //void RemoveItemLeak(const std::string& name);

    SceneItem* GetItem(const std::string& name) const override;

    SceneItem* GetSelectedItem() const override;

    std::vector<SceneItem*> GetItems() const override;

    const std::string& name() const override;

    void RegisterItemAddHandler(ItemAddHandler handler) override;

    void RegisterItemRemoveHandler(ItemRemoveHandler handler) override;

    void RegisterItemSelectedHandler(ItemSelectedHandler handler) override;

    void RegisterItemDeselectHandler(ItemDeselectedHandler handler) override;

    void RegisterItemReorderedHandler(ItemReorderedHandler handler) override;

    void RegisterItemVisibleHandler(ItemVisibleHandler handler) override;

    void RegisterItemLockedHandler(ItemLockedHandler handler) override;
    void SignaledItemLockedChanged(const std::string& item_name) override;

    // Force synchronizing order of scene items with underlying obs scene items.
    void SynchronizeItemOrder();

    // These two functions are for workaround for some edge cases.
    // Be cautious when you use them.

    obs_scene_t* LeakUnderlyingSource() const
    {
        return scene_;
    }

    obs_source_t* AsSource() const
    {
        return obs_scene_get_source(scene_);
    }

private:
    // It does only partial initialization, keep it out of public sight.
    explicit SceneImpl(obs_scene_t* raw_scene, const std::string& name);

    void DismissSignalHandlers();

private:
    obs_scene_t* scene_;
    std::string name_;
    SignalHandler<ItemAddHandler> item_add_handler_;
    SignalHandler<ItemRemoveHandler> item_remove_handler_;
    SignalHandler<ItemSelectedHandler> item_selected_handler_;
    SignalHandler<ItemDeselectedHandler> item_deselected_handler_;
    SignalHandler<ItemReorderedHandler> item_reordered_handler_;
    SignalHandler<ItemVisibleHandler> item_visible_handler_;
    SignalHandler<ItemLockedHandler> item_locked_handler_;
    std::vector<std::unique_ptr<SceneItemImpl>> items_;
};

// Don't know what transition is yet.
// Make it simply a RAII-wrapper for the time being.
class Transition {
public:
    // Takes over the ownership.
    explicit Transition(obs_source_t* raw_transition);

    ~Transition();

    DISABLE_COPY(Transition);

    obs_source_t* LeakUnderlyingSource() const;

    operator bool() const;

    const std::string& name() const;

    void RegisterTransitionStopHandler(signal_callback_t callback, void* data);

private:
    obs_source_t* transition_;
    std::string name_;
    OBSSignal transition_stop_signal_;
};

class CreatingSceneError : public std::runtime_error {
public:
    explicit CreatingSceneError(const char* message)
        : runtime_error(message)
    {}

    explicit CreatingSceneError(const std::string& message)
        : runtime_error(message)
    {}
};

class RemovingSceneError : public std::runtime_error {
public:
    explicit RemovingSceneError(const char* message)
        : runtime_error(message) {}

    explicit RemovingSceneError(const std::string& message)
        : runtime_error(message) {}
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCES_WRAPPER_IMPL_H_