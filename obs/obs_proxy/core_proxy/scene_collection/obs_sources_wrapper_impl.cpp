#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"

#include <algorithm>
#include <unordered_map>

#include "base/logging.h"

#include "bilibase/error_exception_util.h"
#include "bilibase/scope_guard.h"

#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/common/id_table_utils.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_filter_wrapper_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_property_values_impl.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

#include "obs/obs-studio/libobs/graphics/matrix4.h"

#pragma warning(disable:4838)

namespace {

using obs_proxy::CoreProxyImpl;
using obs_proxy::FilterImpl;
using obs_proxy::impl_cast;
using obs_proxy::Scene;
using obs_proxy::SceneImpl;
using obs_proxy::SceneItemType;
using obs_proxy::SceneItemImpl;

// The index of an item id must match the value in SceneItemType.
const char* const item_id_table[]{
    obs_proxy::kImageItemID,
    obs_proxy::kSliderItemID,
    obs_proxy::kMediaSourceItemID,
    obs_proxy::kTextItemID,
    obs_proxy::kSceneID,
    obs_proxy::kColiveSourceID,
    obs_proxy::kVoiceLinkSourceID,
    obs_proxy::kDmkhimeSourceID,
    obs_proxy::kDisplayCaptureItemID,
    obs_proxy::kWindowCaptureItemID,
    obs_proxy::kGameCaptureItemID,
    obs_proxy::kVideoCaptureDeviceItemID,
    obs_proxy::kAudioInputCaptureItemID,
    obs_proxy::kAudioOutputCaptureItemID,
    obs_proxy::kColorSourceItemID,
    obs_proxy::kReceiverSourceItemID,
    obs_proxy::kBrowserSourceItemID,
    obs_proxy::kRtcColiveSourceID,
    obs_proxy::kTextureRendererSourceID,
    obs_proxy::kVtuberInputItemID,
    //obs_proxy::kOrnamentInputItemID,
    obs_proxy::kVtuberRendererItemID,
    obs_proxy::kRtcVoiceLinkSourceID,
	obs_proxy::kFilterSingIdentifyID,
    obs_proxy::kThreeVtuberSourceID,
    obs_proxy::kMainSceneRendererSourceID,
};

const char* ItemTypeToID(SceneItemType type)
{
    return obs_proxy::EnumValueToTableEntry(type, item_id_table);
}

SceneItemType ItemIDToType(const char* id)
{
    return obs_proxy::TableEntryToEnumValue<SceneItemType>(id, item_id_table);
}

class CreatingSceneItemError : public std::runtime_error {
public:
    explicit CreatingSceneItemError(const char* message)
        : runtime_error(message)
    {}

    explicit CreatingSceneItemError(const std::string& message)
        : runtime_error(message)
    {}
};

struct text_item_tag {};
struct media_item_tag {};
struct album_item_tag {};
struct browser_item_tag {};

obs_data_t* GeneratePresetPropertyValues(text_item_tag)
{
    const char* const kMsg = "Welcome to Bilibili";
    const char* const kFontFace = "Arial";

    const int kFontSize = 32;

    obs_data_t* preset_values = obs_data_create();

    obs_data_set_string(preset_values, "text", kMsg);
    obs_data_t* font_settings = obs_data_create();
    obs_data_set_string(font_settings, "face", kFontFace);
    obs_data_set_int(font_settings, "size", kFontSize);
    obs_data_set_obj(preset_values, "font", font_settings);
    obs_data_release(font_settings);

    return preset_values;
}

obs_data_t* GeneratePresetPropertyValues(media_item_tag)
{
    obs_data_t* preset_values = obs_data_create();
    obs_data_set_bool(preset_values, "looping", true);
    obs_data_set_bool(preset_values, "restart_on_activate", false);
    return preset_values;
}

obs_data_t* GeneratePresetPropertyValues(album_item_tag)
{
    obs_data_t* preset_values = obs_data_create();
    obs_data_set_string(preset_values, "transition", "cut");
    obs_data_set_int(preset_values, "slide_time", 5000);
    obs_data_set_int(preset_values, "transition_speed", 1000);

    return preset_values;
}

obs_data_t* GeneratePresetPropertyValues(browser_item_tag)
{
    obs_data_t* preset_values = obs_data_create();
    obs_data_set_string(preset_values, "url", "http://bilibili.tv");
    return preset_values;
}

obs_data_t* PresetPropertyValuesForItem(SceneItemType type)
{
    switch (type)
    {
        case SceneItemType::Text:
            return GeneratePresetPropertyValues(text_item_tag());

        case SceneItemType::MediaSource:
            return GeneratePresetPropertyValues(media_item_tag());

        case SceneItemType::Slider:
            return GeneratePresetPropertyValues(album_item_tag());

        case SceneItemType::BrowserSource:
            return GeneratePresetPropertyValues(browser_item_tag());

        default:
            return nullptr;
    }
}

struct SceneItemBundle {
    SceneItemBundle(obs_source_t* source, obs_sceneitem_t** item, bool visible)
        : raw_source(source), target_item(item), visible(visible)
    {}

    obs_source_t* raw_source;
    obs_sceneitem_t** target_item;
    bool visible;
};

void TurnSourceIntoSceneItem(void* bundle, obs_scene_t* scene)
{
    auto item_bundle = static_cast<SceneItemBundle*>(bundle);
    *item_bundle->target_item = obs_scene_add(scene, item_bundle->raw_source);
    obs_sceneitem_set_visible(*item_bundle->target_item, item_bundle->visible);
}

bool OnAttachedSceneItemFound(obs_scene_t*, obs_sceneitem_t* item, void* param)
{
    std::string item_name = obs_source_get_name(obs_sceneitem_get_source(item));
    SceneItemType item_type = ItemIDToType(obs_source_get_id(obs_sceneitem_get_source(item)));
    auto scene_item = std::make_unique<SceneItemImpl>(item, item_type, item_name);
    auto scene = static_cast<SceneImpl*>(param);
    scene->AttachExistingItem(std::move(scene_item));
    return true;
}

void OnBoundFilterFound(obs_source_t*, obs_source_t* raw_filter, void* param)
{
    std::string filter_name = obs_source_get_name(raw_filter);
    auto filter_type = obs_proxy::FilterIDToType(obs_source_get_id(raw_filter));
    auto filter = std::make_unique<FilterImpl>(raw_filter, filter_type, filter_name);

    auto scene_item = static_cast<SceneItemImpl*>(param);
    scene_item->AddExistingFilter(std::move(filter));
}

vec2 GetItemSourceSize(obs_sceneitem_t *item)
{
    vec2 size;
    obs_source_t *source = obs_sceneitem_get_source(item);

    size.x = obs_source_get_width(source);
    size.y = obs_source_get_height(source);

    return size;
}

// transform
// copied from obs-studio: window-basic-main.cpp
#ifdef _MSC_VER
#pragma region copied from obs-studio window-basic-main.cpp
#endif

void GetItemBox(obs_sceneitem_t *item, vec3 &tl, vec3 &br)
{
    matrix4 boxTransform;
    obs_sceneitem_get_box_transform(item, &boxTransform);

    vec3_set(&tl, M_INFINITE, M_INFINITE, 0.0f);
    vec3_set(&br, -M_INFINITE, -M_INFINITE, 0.0f);

    auto GetMinPos = [&](float x, float y)
    {
        vec3 pos;
        vec3_set(&pos, x, y, 0.0f);
        vec3_transform(&pos, &pos, &boxTransform);
        vec3_min(&tl, &tl, &pos);
        vec3_max(&br, &br, &pos);
    };

    GetMinPos(0.0f, 0.0f);
    GetMinPos(1.0f, 0.0f);
    GetMinPos(0.0f, 1.0f);
    GetMinPos(1.0f, 1.0f);
}

vec3 GetItemTL(obs_sceneitem_t *item)
{
    vec3 tl, br;
    GetItemBox(item, tl, br);
    return tl;
}

void SetItemTL(obs_sceneitem_t *item, const vec3 &tl)
{
    vec3 newTL;
    vec2 pos;

    obs_sceneitem_get_pos(item, &pos);
    newTL = GetItemTL(item);
    pos.x += tl.x - newTL.x;
    pos.y += tl.y - newTL.y;
    obs_sceneitem_set_pos(item, &pos);
}

bool RotateSelectedSources(obs_scene_t *scene, obs_sceneitem_t *item,
    void *param)
{
    if (!obs_sceneitem_selected(item))
        return true;

    float rot = *reinterpret_cast<float*>(param);

    vec3 tl = GetItemTL(item);

    rot += obs_sceneitem_get_rot(item);
    if (rot >= 360.0f)       rot -= 360.0f;
    else if (rot <= -360.0f) rot += 360.0f;
    obs_sceneitem_set_rot(item, rot);

    SetItemTL(item, tl);

    UNUSED_PARAMETER(scene);
    UNUSED_PARAMETER(param);
    return true;
};

bool RotateSources(obs_scene_t* scene, obs_sceneitem_t* item,
    void* param){
    float rot = *reinterpret_cast<float*>(param);

    vec3 tl = GetItemTL(item);

    rot += obs_sceneitem_get_rot(item);
    if (rot >= 360.0f)       rot -= 360.0f;
    else if (rot <= -360.0f) rot += 360.0f;
    obs_sceneitem_set_rot(item, rot);

    SetItemTL(item, tl);

    UNUSED_PARAMETER(scene);
    UNUSED_PARAMETER(param);
    return true;
}

float SourcesItemRotateVal(obs_scene_t* scene, obs_sceneitem_t* item) {
    float rot = obs_sceneitem_get_rot(item);

    UNUSED_PARAMETER(scene);
    return rot;
}

#ifdef _MSC_VER
#pragma endregion
#endif

bool sceneitem_is_width_or_height_zero(obs_sceneitem_t* item)
{
    obs_source_t* source = obs_sceneitem_get_source(item);
    if (!source)
        return true;

    if (obs_source_get_width(source) == 0)
        return true;
    if (obs_source_get_height(source) == 0)
        return true;

    return false;
}

void sceneitem_do_actual_vertical_flip(obs_sceneitem_t* item)
{
    vec3 tl = GetItemTL(item);
    vec2 scaleVec;
    obs_sceneitem_get_scale(item, &scaleVec);
    scaleVec.y = -scaleVec.y;
    obs_sceneitem_set_scale(item, &scaleVec);
    SetItemTL(item, tl);
}

void sceneitem_do_actual_horizontal_flip(obs_sceneitem_t* item)
{
    vec3 tl = GetItemTL(item);
    vec2 scaleVec;
    obs_sceneitem_get_scale(item, &scaleVec);
    scaleVec.x = -scaleVec.x;
    obs_sceneitem_set_scale(item, &scaleVec);
    SetItemTL(item, tl);
}

// scene item operate callback
void OnItemAddHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_item = static_cast<obs_sceneitem_t*>(calldata_ptr(params, "item"));
    obs_source_t* underlying_source = obs_sceneitem_get_source(raw_item);
    std::string item_name = obs_source_get_name(underlying_source);
    obs_scene_t* raw_scene = obs_sceneitem_get_scene(raw_item);
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    SceneItemType item_type = ItemIDToType(obs_source_get_id(obs_sceneitem_get_source(raw_item)));
    auto& fn = *static_cast<Scene::ItemAddHandler*>(data);
    fn(scene_name, item_name, obs_sceneitem_get_source(raw_item), (int)item_type);
}

void OnItemRemoveHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_item = static_cast<obs_sceneitem_t*>(calldata_ptr(params, "item"));
    obs_source_t* underlying_source = obs_sceneitem_get_source(raw_item);
    std::string item_name = obs_source_get_name(underlying_source);
    obs_scene_t* raw_scene = obs_sceneitem_get_scene(raw_item);
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    auto& fn = *static_cast<Scene::ItemRemoveHandler*>(data);
    fn(scene_name, item_name, underlying_source);
}

void OnItemSelectedHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_scene = static_cast<obs_scene_t*>(calldata_ptr(params, "scene"));
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    auto* raw_scene_item = static_cast<obs_sceneitem_t*>(calldata_ptr(params, "item"));
    std::string item_name = obs_source_get_name(obs_sceneitem_get_source(raw_scene_item));
    auto& fn = *static_cast<Scene::ItemSelectedHandler*>(data);
    fn(scene_name, item_name);
}

void OnItemDeselectedHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_scene = static_cast<obs_scene_t*>(calldata_ptr(params, "scene"));
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    auto* raw_scene_item = static_cast<obs_sceneitem_t*>(calldata_ptr(params, "item"));
    std::string item_name = obs_source_get_name(obs_sceneitem_get_source(raw_scene_item));
    auto& fn = *static_cast<Scene::ItemDeselectedHandler*>(data);
    fn(scene_name, item_name);
}

void OnItemReorderedHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_scene = static_cast<obs_scene_t*>(calldata_ptr(params, "scene"));
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    auto scene_collection = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    auto scene = impl_cast(scene_collection->GetScene(scene_name));
    scene->SynchronizeItemOrder();
    auto& fn = *static_cast<Scene::ItemReorderedHandler*>(data);
    fn(scene_name);
}

void OnItemVisibleHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_scene = static_cast<obs_scene_t*>(calldata_ptr(params, "scene"));
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    auto* raw_scene_item = static_cast<obs_sceneitem_t*>(calldata_ptr(params, "item"));
    std::string item_name = obs_source_get_name(obs_sceneitem_get_source(raw_scene_item));
    bool visible = calldata_bool(params, "visible");
    auto& fn = *static_cast<Scene::ItemVisibleHandler*>(data);
    fn(scene_name, item_name, visible);
}

}   // namespace

namespace obs_proxy {

SceneImpl::SceneImpl(const std::string& name, bool custom_size, int cx, int cy)
    : name_(name)
{
    obs_data_t* settings = nullptr;
    if (custom_size) {
        settings = obs_data_create();
        obs_data_set_bool(settings, "custom_size", custom_size);
        obs_data_set_int(settings, "cx", cx);
        obs_data_set_int(settings, "cy", cy);
    }

    // Function obs_scene_create() has a potential risk of dereferencing a null pointer.
    auto* raw_source = obs_source_create(kSceneID, name.c_str(), settings, nullptr);
    scene_ = obs_scene_from_source(raw_source);
    ENSURE(scene_ != nullptr)(name).Require<CreatingSceneError>();

    if (settings) {
        obs_data_release(settings);
    }
}

SceneImpl::SceneImpl(obs_scene_t* raw_scene, const std::string& name)
    : scene_(raw_scene), name_(name)
{}

SceneImpl::~SceneImpl()
{
    DismissSignalHandlers();

    items_.clear();

    obs_source_release(AsSource());
}

// static
std::unique_ptr<SceneImpl> SceneImpl::FromExistingScene(obs_scene_t* raw_scene)
{
    DCHECK(raw_scene != nullptr);
    std::string scene_name = obs_source_get_name(obs_scene_get_source(raw_scene));
    std::unique_ptr<SceneImpl> scene(new SceneImpl(raw_scene, scene_name));

    // Enumerate and attach all existing sub items.
    obs_scene_enum_items(raw_scene, OnAttachedSceneItemFound, scene.get());

    return scene;
}

SceneItem* SceneImpl::AttachNewItem(SceneItemType type, const std::string& name, bool visible)
{
    SceneItem* item_view;
    try
    {
        auto new_item = std::make_unique<SceneItemImpl>(*this, type, name, visible);
        item_view = new_item.get(); // Take the view.
        items_.push_back(std::move(new_item));
    }
    catch (const CreatingSceneItemError& ex)
    {
        LOG(WARNING) << ex.what();
        item_view = nullptr;
    }

    return item_view;
}

SceneItem* SceneImpl::AttachNewColiveRemoteSource(uint64_t uid, const std::string& name, bool visible, bool use_rtc_colive, bool is_little_video) {
    SceneItem* item_view;
    try
    {
        auto new_item = std::make_unique<SceneItemImpl>(*this, name, uid, visible, use_rtc_colive,is_little_video);
        item_view = new_item.get(); // Take the view.
        items_.push_back(std::move(new_item));
    }
    catch (const CreatingSceneItemError& ex)
    {
        LOG(WARNING) << ex.what();
        item_view = nullptr;
    }

    return item_view;
}

void SceneImpl::AttachExistingItem(std::unique_ptr<SceneItemImpl> scene_item)
{
    items_.push_back(std::move(scene_item));
}

void SceneImpl::AttachExistingScene(const std::string& scene_name, bool visible) {
    try {
        auto new_item = std::make_unique<SceneItemImpl>(*this, scene_name, visible);
        auto item_view = new_item.get(); // Take the view.
        items_.push_back(std::move(new_item));
    } catch (const CreatingSceneItemError& ex) {
        LOG(WARNING) << ex.what();
    }
}

SceneItem* SceneImpl::AddRawSource(obs_source_t* raw_source, const std::string& name, SceneItemType type)
{
    SceneItem* item_view;
    try
    {
        obs_sceneitem_t* raw_item = obs_scene_add(scene_, raw_source);
        auto new_item = std::make_unique<SceneItemImpl>(raw_item, type, name);
        item_view = new_item.get(); // Take the view.
        items_.push_back(std::move(new_item));
    }
    catch (const CreatingSceneItemError& ex)
    {
        LOG(WARNING) << ex.what();
        item_view = nullptr;
    }

    return item_view;
}

void SceneImpl::RemoveItem(const std::string& name)
{
    auto it = std::find_if(items_.begin(), items_.end(),
                                  [&name](const std::unique_ptr<SceneItemImpl>& item) {
                                      return item->name() == name;
                                  });
    if (it != items_.end())
    {
        it->reset();
        items_.erase(it);
    }
}

//void SceneImpl::RemoveItemLeak(const std::string& name) {
//    auto new_end = std::remove_if(items_.begin(), items_.end(),
//        [&name, this](std::unique_ptr<SceneItemImpl>& item) {
//            if (item->name() == name)
//            {
//                item.release();
//                return true;
//            }
//            return false;
//        });
//    items_.erase(new_end, items_.end());
//}

SceneItem* SceneImpl::GetItem(const std::string& name) const
{
    auto it = std::find_if(items_.begin(), items_.end(),
                           [&name](const std::unique_ptr<SceneItemImpl>& item) {
                                if (item)
                                {
                                     return item->name() == name;
                                }
                                else
                                {
                                    return false;
                                }
                           });
    return it == items_.end() ? nullptr : it->get();
}

obs_proxy::SceneItem* SceneImpl::GetSelectedItem() const
{
    auto it = std::find_if(items_.begin(), items_.end(),
                           [](const std::unique_ptr<SceneItemImpl>& item)->bool {
                               return item->IsSelected();
                           });
    return it == items_.end() ? nullptr : it->get();
}

std::vector<SceneItem*> SceneImpl::GetItems() const
{
    return AcquireSourceViews<SceneItemImpl, SceneItem>(items_);
}

const std::string& SceneImpl::name() const
{
    return name_;
}

void SceneImpl::RegisterItemAddHandler(ItemAddHandler handler)
{
    signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_add_handler_.first = handler;
    item_add_handler_.second.Connect(raw_signal, kSignalItemAdd, OnItemAddHandlerRedirect,
                                     &item_add_handler_.first);
}

void SceneImpl::RegisterItemRemoveHandler(ItemRemoveHandler handler)
{
    signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_remove_handler_.first = handler;
    item_remove_handler_.second.Connect(raw_signal, kSignalItemRemove, OnItemRemoveHandlerRedirect,
                                        &item_remove_handler_.first);
}

void SceneImpl::RegisterItemSelectedHandler(ItemSelectedHandler handler)
{
    signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_selected_handler_.first = handler;
    item_selected_handler_.second.Connect(raw_signal,
                                          kSignalItemSelect,
                                          OnItemSelectedHandlerRedirect,
                                          &item_selected_handler_.first);
}

void SceneImpl::RegisterItemDeselectHandler(ItemDeselectedHandler handler)
{
    signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_deselected_handler_.first = handler;
    item_deselected_handler_.second.Connect(raw_signal,
                                            kSignalItemDeselect,
                                            OnItemDeselectedHandlerRedirect,
                                            &item_deselected_handler_.first);
}

void SceneImpl::RegisterItemReorderedHandler(ItemReorderedHandler handler)
{
    signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_reordered_handler_.first = handler;
    item_reordered_handler_.second.Connect(raw_signal,
                                           kSignalItemReorder,
                                           OnItemReorderedHandlerRedirect,
                                           &item_reordered_handler_.first);
}

void SceneImpl::RegisterItemVisibleHandler(ItemVisibleHandler handler)
{
    signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_visible_handler_.first = handler;
    item_visible_handler_.second.Connect(raw_signal,
        kSignalItemVisible,
        OnItemVisibleHandlerRedirect,
        &item_visible_handler_.first);
}

void SceneImpl::RegisterItemLockedHandler(ItemLockedHandler handler)
{
    // 当前版本的obs对于item的lock是没有signal通知的，仅仅只是置一个标识位而已，
    // 需要我们的自己的业务层自行决定什么时候该触发回调
    //signal_handler_t* raw_signal = obs_source_get_signal_handler(AsSource());
    item_locked_handler_.first = handler;
    /*item_locked_handler_.second.Connect(raw_signal,
        kSignalItemVisible,
        OnItemVisibleHandlerRedirect,
        &item_locked_handler_.first);*/
}

void SceneImpl::SignaledItemLockedChanged(const std::string& item_name)
{
    auto item = GetItem(item_name);
    if (item && item_locked_handler_.first)
    {
        item_locked_handler_.first(name(), item_name, item->IsLocked());
    }
}

void SceneImpl::SynchronizeItemOrder()
{
    std::vector<std::string> items_in_real_order;
    items_in_real_order.reserve(items_.size());
    obs_scene_enum_items(scene_,
                         [](obs_scene_t*, obs_sceneitem_t* item, void* param) {
                             auto name = obs_source_get_name(obs_sceneitem_get_source(item));
                             static_cast<std::vector<std::string>*>(param)->emplace_back(name);
                             return true;
                         },
                         &items_in_real_order);

    DCHECK(items_in_real_order.size() == items_.size());
    for (size_t i = 0; i < items_in_real_order.size(); ++i)
    {
        const std::string& name = items_in_real_order[i];
        auto it = std::find_if(items_.begin() + i, items_.end(),
                               [&name](const std::unique_ptr<SceneItemImpl>& item) {
                                   return item->name() == name;
                               });
        DCHECK(it != items_.end());
        using std::swap;
        swap(*it, items_[i]);
    }
}

void SceneImpl::DismissSignalHandlers()
{
    item_add_handler_.second.Disconnect();
    item_remove_handler_.second.Disconnect();
    item_selected_handler_.second.Disconnect();
    item_deselected_handler_.second.Disconnect();
    item_reordered_handler_.second.Disconnect();
    item_visible_handler_.second.Disconnect();
}



// -*- SceneItemImpl implementation -*-
SceneItemImpl::SceneItemImpl(SceneImpl& scene, SceneItemType type, const std::string& name,
                             bool visible)
    : type_(type), name_(name), scene_item_(nullptr)
{
    const char* id = ItemTypeToID(type);
    auto preset_values = PresetPropertyValuesForItem(type);
    auto* raw_source = obs_source_create(id, name.c_str(), preset_values, nullptr);

    ON_SCOPE_EXIT
    {
        obs_data_release(preset_values);
        obs_source_release(raw_source);
    };

    auto bundle = SceneItemBundle(raw_source, &scene_item_, visible);
    obs_scene_atomic_update(scene.LeakUnderlyingSource(), TurnSourceIntoSceneItem, &bundle);
    ENSURE(scene_item_ != nullptr)(id)(name).Require<CreatingSceneItemError>();

    properties_ = std::make_unique<PropertiesImpl>(AsSource());

    obs_data_t* item_property_values = obs_source_get_settings(AsSource());
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(item_property_values);
}

SceneItemImpl::SceneItemImpl(obs_sceneitem_t* raw_scene_item, SceneItemType type,
                             const std::string& name)
    : type_(type), name_(name), scene_item_(raw_scene_item)
{
    properties_ = std::make_unique<PropertiesImpl>(AsSource());

    obs_data_t* item_property_values = obs_source_get_settings(AsSource());
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(item_property_values);

    // Incorporates all filters that have already bound with the scene item.
    obs_source_enum_filters(AsSource(), OnBoundFilterFound, this);
}

SceneItemImpl::SceneItemImpl(SceneImpl& host_scene, const std::string& scene_name, bool visible)
    : type_(SceneItemType::Scene), name_(scene_name), scene_item_(nullptr) {

    const char* id = ItemTypeToID(type_);
    obs_source_t* source = obs_get_source_by_name(name_.c_str());

    ON_SCOPE_EXIT
    {
        obs_source_release(source);
    };

    auto bundle = SceneItemBundle(source, &scene_item_, visible);
    obs_enter_graphics();
    obs_scene_atomic_update(host_scene.LeakUnderlyingSource(), TurnSourceIntoSceneItem, &bundle);
    obs_leave_graphics();
    ENSURE(scene_item_ != nullptr)(id)(name_).Require<CreatingSceneItemError>();

    properties_ = std::make_unique<PropertiesImpl>(AsSource());

    obs_data_t* item_property_values = obs_source_get_settings(AsSource());
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(item_property_values);
}

SceneItemImpl::SceneItemImpl(
    SceneImpl& scene, const std::string& name, uint64_t uid, bool visible, bool use_rtc_colive, bool is_little_video)
    : type_(SceneItemType::ColiveSource), name_(name), scene_item_(nullptr) {

    if (use_rtc_colive)
    {
        type_ = SceneItemType::RtcColiveSource;
    }
    const char* id = ItemTypeToID(type_);

    obs_data_t* preset_values = obs_data_create();
    obs_data_set_int(preset_values, "uid", uid);
    if (use_rtc_colive && is_little_video)
    {
        obs_data_set_bool(preset_values, "is_little_video", true);
    }

    auto* raw_source = obs_source_create(id, name.c_str(), preset_values, nullptr);

    obs_source_set_monitoring_type(
        raw_source, obs_monitoring_type::OBS_MONITORING_TYPE_MONITOR_ONLY);

    ON_SCOPE_EXIT
    {
        obs_data_release(preset_values);
        obs_source_release(raw_source);
    };

    auto bundle = SceneItemBundle(raw_source, &scene_item_, visible);
    obs_scene_atomic_update(scene.LeakUnderlyingSource(), TurnSourceIntoSceneItem, &bundle);
    ENSURE(scene_item_ != nullptr)(id)(name).Require<CreatingSceneItemError>();

    properties_ = std::make_unique<PropertiesImpl>(AsSource());

    obs_data_t* item_property_values = obs_source_get_settings(AsSource());
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(item_property_values);
}

SceneItemImpl::~SceneItemImpl()
{
    obs_source_uninstall_deferred_updated_handler(AsSource());
    obs_sceneitem_remove(scene_item_);
}

void SceneItemImpl::MoveOrder(OrderMovement movement)
{
    obs_sceneitem_set_order(scene_item_, obs_order_movement(movement));
}

void SceneItemImpl::SetOrder(int position)
{
    obs_sceneitem_set_order_position(scene_item_, position);
}

void SceneItemImpl::AddExistingFilter(std::unique_ptr<FilterImpl> filter)
{
    filters_.push_back(std::move(filter));
}

Filter* SceneItemImpl::AddNewFilter(FilterType type, const std::string& name)
{
    Filter* filter_view = nullptr;
    auto* existing_filter = obs_source_get_filter_by_name(AsSource(), name.c_str());
    if (existing_filter)
    {
        LOG(WARNING) << "Filter with name " << name << " already exists!";
        obs_source_release(existing_filter);
        return filter_view;
    }

    try
    {
        auto filter = std::make_unique<FilterImpl>(type, name);
        obs_source_filter_add(AsSource(), filter->LeakUnderlyingSource());
        filter_view = filter.get();
        filters_.push_back(std::move(filter));
    }
    catch (const CreatingFilterError& ex)
    {
        LOG(WARNING) << ex.what();
        filter_view = nullptr;
    }

    return filter_view;
}

void SceneItemImpl::RemoveFilter(const std::string& name)
{
    // Dirty workaround that being compatible with the dark case where there are multiple
    // filters with the same name.
    auto it = std::partition(filters_.begin(), filters_.end(),
                             [&name](const std::unique_ptr<FilterImpl>& filter) {
                                 return filter->name() != name;
                             });

    DCHECK(std::distance(it, filters_.end()) <= 1);
    if (it == filters_.end())
    {
        return;
    }

    std::for_each(it, filters_.end(), [this](const std::unique_ptr<FilterImpl>& filter) {
                      obs_source_filter_remove(AsSource(), filter->LeakUnderlyingSource());
                  });

    filters_.erase(it, filters_.end());
}

Filter* SceneItemImpl::GetFilter(const std::string& name) const
{
    auto it = std::find_if(filters_.cbegin(), filters_.cend(),
                           [&name](const std::unique_ptr<FilterImpl>& filter) {
                               return filter->name() == name;
                           });
    return it == filters_.cend() ? nullptr : it->get();
}

std::vector<Filter*> SceneItemImpl::GetFilters() const
{
    return AcquireSourceViews<FilterImpl, Filter>(filters_);
}

bool SceneItemImpl::ReorderFilters(const std::vector<Filter*>& filters)
{
    // this function is copied to AudioDeviceImpl::ReorderFilters
    std::unordered_map<Filter*, size_t> new_filter_indices;
    for (size_t i = 0; i < filters.size(); ++i) {
        new_filter_indices.insert({filters[i], i});
    }

    bool filters_matched = filters_.size() == new_filter_indices.size() &&
                           std::all_of(filters_.cbegin(), filters_.cend(),
                                       [&new_filter_indices](const std::unique_ptr<FilterImpl>& filter) {
                                            return new_filter_indices.count(filter.get()) != 0;
                                       });
    if (!filters_matched) {
        NOTREACHED();
        return false;
    }

    // Using bubble-sort to reorder filters.
    bool swapped = false;
    for (size_t i = 0; i < filters_.size(); ++i) {
        for (size_t j = 0; j < filters_.size() - i - 1; ++j) {
            if (new_filter_indices[filters_[j].get()] > new_filter_indices[filters_[j+1].get()]) {
                obs_source_filter_set_order(AsSource(), filters_[j]->LeakUnderlyingSource(),
                                            obs_order_movement::OBS_ORDER_MOVE_DOWN);
                using std::swap;
                swap(filters_[j], filters_[j+1]);
                swapped = true;
            }
        }

        if (!swapped) {
            break;
        }

        swapped = false;
    }

    return true;
}

bool SceneItemImpl::IsDeferUpdateItem() const
{
    auto flags = obs_source_get_output_flags(AsSource());
    return !!(flags & OBS_SOURCE_VIDEO);
}

void SceneItemImpl::InstallDeferredUpdatedHandler(DeferredUpdatedHanlder handler, void* param)
{
    obs_source_install_deferred_updated_handler(AsSource(), handler, param);
}

bool SceneItemImpl::IsDeferredUpdatedHanlderInstalled() const
{
    return obs_source_is_deferred_updated_handler_installed(AsSource());
}

PropertyValue& SceneItemImpl::GetPropertyValues()
{
    return property_values_;
}

void SceneItemImpl::UpdatePropertyValues()
{
    auto* new_properties = property_values_.impl_->LeakUnderlyingValue();
    obs_source_update(AsSource(), new_properties);
}

const Properties& SceneItemImpl::GetProperties(PropertiesRefreshMode mode) const
{
    if (mode != PropertiesRefreshMode::NoRefresh)
    {
        properties_->Refresh();
    }

    return *properties_;
}

bool SceneItemImpl::IsVisible() const
{
    return obs_sceneitem_visible(scene_item_);
}

void SceneItemImpl::SetVisible(bool visible)
{
    if (visible != IsVisible())
    {
        obs_sceneitem_set_visible(scene_item_, visible);
    }
}

bool SceneItemImpl::IsSelected() const
{
    return obs_sceneitem_selected(scene_item_);
}

void SceneItemImpl::Setlect(bool select)
{
    if (IsSelected() != select)
    {
        obs_sceneitem_select(scene_item_, select);
    }
}

void SceneItemImpl::SetMuted(bool mute) {
    if (IsMuted() != mute) {
        obs_source_set_muted(AsSource(), mute);
    }
}

bool SceneItemImpl::IsMuted() {
    return obs_source_muted(AsSource());
}

void SceneItemImpl::SetLocked(bool lock)
{
    obs_sceneitem_set_locked(scene_item_, lock);
}

bool SceneItemImpl::IsLocked()
{
    return obs_sceneitem_locked(scene_item_);
}

void SceneItemImpl::SetFitToScreen(bool fit_to_screen)
{
    obs_sceneitem_set_fit_to_screen(scene_item_, fit_to_screen);
}

bool SceneItemImpl::IsFitToScreen() const
{
    return obs_sceneitem_fit_to_screen(scene_item_);
}

void SceneItemImpl::SizeToActualSize()
{
    // 选择原始尺寸就立刻取消全屏适配标识
    SetFitToScreen(false);

    int item_w = obs_source_get_base_width(AsSource());
    int item_h = obs_source_get_base_height(AsSource());

    obs_transform_info item_info;
    obs_sceneitem_get_info(scene_item_, &item_info);

    vec2_set(&item_info.pos, 0, 0);
    vec2_set(&item_info.scale, 1, 1);
    vec2_set(&item_info.bounds, item_w, item_h);
    item_info.bounds_type = OBS_BOUNDS_NONE;
    //item_info.rot = 0.0f;

    obs_sceneitem_set_info(scene_item_, &item_info);
}

void SceneItemImpl::SizeToScreenSize()
{
    OBSScene scene = obs_sceneitem_get_scene(scene_item_);
    if (!scene)
    {
        return;
    }

    int item_w = obs_source_get_width(AsSource());
    int item_h = obs_source_get_height(AsSource());
    if ((0 == item_w) || (0 == item_h))
    {
        return;
    }

    int scene_w = obs_source_get_width(obs_scene_get_source(scene));
    int scene_h = obs_source_get_height(obs_scene_get_source(scene));
    float scene_ratio = float(scene_w) / float(scene_h);
    if ((0 == scene_w) || (0 == scene_h))
    {
        NOTREACHED() << "unexcepted";
        return;
    }

    auto get_pos_scale_proc = [&](int &new_item_w, int &new_item_h, int &item_x, int &item_y, float &scale)
    {
        // OBS的全屏幕比例适配在item的rotate变更的时候直接无效，我们采取的做法是：
        // 当item处于全屏适配状态时，旋转角度后直接以当前角度重新计算坐标尺寸进行全屏适配，
        // 不像OBS那样直接取消全屏适配状态
        // 拿当前的旋转角度，180°/360°的不用重计算尺寸，90°/270°的要重计算
        float rot = obs_sceneitem_get_rot(scene_item_);
        rot /= 90.0f;
        int nRot = static_cast<int>(std::round(std::abs(rot)));
        if (nRot % 2 == 0)
        {
            float item_ratio = float(item_w) / float(item_h);
            if (item_ratio > scene_ratio)
            {
                new_item_w = scene_w;
                new_item_h = int(float(new_item_w) / item_ratio);

                scale = float(new_item_w) / float(item_w);
            }
            else
            {
                new_item_h = scene_h;
                new_item_w = int(double(new_item_h) * item_ratio);

                scale = float(new_item_h) / float(item_h);
            }
            item_x = scene_w / 2 - new_item_w / 2;
            item_y = scene_h / 2 - new_item_h / 2;
        }
        else
        {
            float item_ratio = float(item_h) / float(item_w);
            if (item_ratio > scene_ratio)
            {
                new_item_w = scene_w;
                new_item_h = int(float(new_item_w) / item_ratio);

                // 90°/270°的时候屏幕的高度用来容纳源的宽度，屏宽用来容纳源高
                scale = float(new_item_w) / float(item_h);
            }
            else
            {
                new_item_h = scene_h;
                new_item_w = int(double(new_item_h) * item_ratio);

                // 90°/270°的时候屏幕的高度用来容纳源的宽度，屏宽用来容纳源高
                scale = float(new_item_h) / float(item_w);
            }
            item_x = scene_w / 2 - new_item_w / 2;
            item_y = scene_h / 2 - new_item_h / 2;
        }
    };

    int new_item_w = 0;
    int new_item_h = 0;
    int item_x = 0;
    int item_y = 0;
    float scale = 1.0f;
    get_pos_scale_proc(new_item_w, new_item_h, item_x, item_y, scale);

    obs_transform_info item_info;
    obs_sceneitem_get_info(scene_item_, &item_info);

    vec2_set(&item_info.pos, item_x, item_y);
    vec2_set(&item_info.scale,
        item_info.scale.x / std::abs(item_info.scale.x) * scale, // 保持当前的水平翻转状态
        item_info.scale.y / std::abs(item_info.scale.y) * scale); // 保持当前的垂直翻转状态
    vec2_set(&item_info.bounds, new_item_w, new_item_h);
    item_info.bounds_type = OBS_BOUNDS_NONE;

    obs_sceneitem_set_info(scene_item_, &item_info);

    // 这里计算得到的item_x、item_y是针对未旋转时的item的左上角而言的，
    // 比如当item旋转了90°时，从视觉角度来看，item的右上角坐标就是item_x、item_y；
    // 所以要使旋转后的item能正确显示到给其指定的bounds中需要拿到其旋转后的矩阵信息，再偏移其坐标
    // 拿到item旋转后的矩阵信息，然后移动坐标使item矩阵完全挪到可视区
    vec3 newTL = GetItemTL(scene_item_);
    vec2 pos;
    obs_sceneitem_get_pos(scene_item_, &pos);
    pos.x += pos.x - newTL.x;
    pos.y += pos.y - newTL.y;
    obs_sceneitem_set_pos(scene_item_, &pos);
}

void obs_proxy::SceneItemImpl::SetBounds(float width, float height)
{
    obs_transform_info item_info;
    obs_sceneitem_get_info(scene_item_, &item_info);

    vec2_set(&item_info.pos, 0, 0);
    vec2_set(&item_info.scale, 1, 1);
    vec2_set(&item_info.bounds, width, height);
    //OBS_BOUNDS_STRETCH 会影响镜头旋转的功能？？？(；′⌒`)
    item_info.bounds_type = OBS_BOUNDS_SCALE_OUTER;
    item_info.rot = 0.0f;

    obs_sceneitem_set_info(scene_item_, &item_info);
}

int obs_proxy::SceneItemImpl::GetWidth()
{
    return obs_source_get_width(AsSource());
}

int obs_proxy::SceneItemImpl::GetHeight()
{
    return obs_source_get_height(AsSource());
}

void obs_proxy::SceneItemImpl::SetScale(float x_scale, float y_scale)
{
    vec2 scalar{};
    scalar.x = x_scale;
    scalar.y = y_scale;
    obs_sceneitem_set_scale(scene_item_, &scalar);
}

void SceneItemImpl::DoVerticalFlip()
{
    if (sceneitem_is_width_or_height_zero(scene_item_))
        return;

    float rot = obs_sceneitem_get_rot(scene_item_);
    rot /= 90.0f;
    int nRot = static_cast<int>(std::round(std::abs(rot)));
    if (nRot % 2 == 0)
    {
        sceneitem_do_actual_vertical_flip(scene_item_);
    }
    else
    {
        sceneitem_do_actual_horizontal_flip(scene_item_);
    }
}

void SceneItemImpl::DoHorizontalFlip()
{
    if (sceneitem_is_width_or_height_zero(scene_item_))
        return;

    float rot = obs_sceneitem_get_rot(scene_item_);
    rot /= 90.0f;
    int nRot = static_cast<int>(std::round(std::abs(rot)));
    if (nRot % 2 == 0)
    {
        sceneitem_do_actual_horizontal_flip(scene_item_);
    }
    else
    {
        sceneitem_do_actual_vertical_flip(scene_item_);
    }
}

void SceneItemImpl::DoLeftRotate()
{
    if (sceneitem_is_width_or_height_zero(scene_item_))
        return;

    float rot = -90.0;
    RotateSelectedSources(nullptr, scene_item_, &rot);
}

void SceneItemImpl::DoRightRotate()
{
    if (sceneitem_is_width_or_height_zero(scene_item_))
        return;

    float rot = 90.0;
    RotateSelectedSources(nullptr, scene_item_, &rot);
}

void SceneItemImpl::DoSourceItemRotateLeft() {
    if (sceneitem_is_width_or_height_zero(scene_item_))
        return;

    float rot = -90.0;
    RotateSources(nullptr, scene_item_, &rot);
}

void SceneItemImpl::DoSourceItemRotateRight() {
    if (sceneitem_is_width_or_height_zero(scene_item_))
        return;

    float rot = 90.0;
    RotateSources(nullptr, scene_item_, &rot);
}

float SceneItemImpl::GetSourcesItemRotateVal() {
    return SourcesItemRotateVal(nullptr, scene_item_);
}

TransformState SceneItemImpl::GetTransformState()
{
    TransformState result;
    vec2 scaleVec, posVec;
    obs_sceneitem_get_scale(scene_item_, &scaleVec);
    obs_sceneitem_get_pos(scene_item_, &posVec);
    result.rot = obs_sceneitem_get_rot(scene_item_);
    result.posx = posVec.x;
    result.posy = posVec.y;
    result.scalex = scaleVec.x;
    result.scaley = scaleVec.y;
    return result;
}

void SceneItemImpl::SetTransformState(TransformState state)
{
    vec2 scaleVec, posVec;
    float rot;
    scaleVec.x = state.scalex;
    scaleVec.y = state.scaley;
    posVec.x = state.posx;
    posVec.y = state.posy;
    rot = state.rot;

    obs_sceneitem_set_scale(scene_item_, &scaleVec);
    obs_sceneitem_set_pos(scene_item_, &posVec);
    obs_sceneitem_set_rot(scene_item_, rot);
}

SceneItemType SceneItemImpl::type() const
{
    return type_;
}

const std::string& SceneItemImpl::type_name() const
{
    return kSceneTypeTable.at(type_);
}

const std::string& SceneItemImpl::name() const
{
    return name_;
}

bool SceneItemImpl::Rename(const std::string& new_name)
{
    auto source = obs_get_source_by_name(new_name.c_str());

    // A source with given name already exists.
    if (source)
    {
        obs_source_release(source);
        return false;
    }

    name_ = new_name;
    obs_source_set_name(AsSource(), new_name.c_str());
    return true;
}

void SceneItemImpl::CropAndSetBounds(const CropInfo &crop_info, const CropInfo &bounds_info)
{
    float x_offset = crop_info.x;
    float y_offset = crop_info.y;
    float width = crop_info.w;
    float height = crop_info.h;

    DCHECK(x_offset >= 0 && y_offset >= 0 && width > 0 && height > 0);

    if (x_offset + width > 1.0f)
    {
        x_offset = 1.0f - width;
    }
    if (y_offset + height > 1.0f)
    {
        y_offset = 1.0f - height;
    }

    OBSSceneItem scene_item(LeakUnderlyingSource());

    vec2 scene_item_size = GetSourceSize();

    vec2 newPos;
    vec2_zero(&newPos);
    //vec2 newBounds;
    //vec2_zero(&newBounds);
    obs_video_info new_ovi;
    if (obs_get_video_info(&new_ovi))
    {
        newPos.x = new_ovi.output_width * bounds_info.x;
        newPos.y = new_ovi.output_height * bounds_info.y;
        //newBounds.x = new_ovi.output_width * 0.4f;
        //newBounds.y = new_ovi.output_height * 0.2f;
    }
    else
    {
        DCHECK(false);
    }

    float x = x_offset * scene_item_size.x;
    float y = y_offset * scene_item_size.y;
    float right_insets = 1.0f - (x_offset + width);
    float bottom_insets = 1.0f - (y_offset + height);

    obs_sceneitem_crop crop;
    crop.left = x;
    crop.top = y;
    crop.right = right_insets * scene_item_size.x;
    crop.bottom = bottom_insets * scene_item_size.y;

    int cx = scene_item_size.x - crop.left - crop.right;
    int cy = scene_item_size.y - crop.top - crop.bottom;

    vec2 zoom_scale;
    vec2_zero(&zoom_scale);
    //zoom_scale.x = (new_ovi.output_width * 0.5f) / (cx * 1.0f);
    //zoom_scale.y = new_ovi.output_height / (cy * 1.0f);
    zoom_scale.x = zoom_scale.y = 1.0f;

    obs_sceneitem_defer_update_begin(scene_item);
    obs_sceneitem_set_crop(scene_item, &crop);
    obs_sceneitem_set_pos(scene_item, &newPos);
    //obs_sceneitem_set_bounds(scene_item, &newBounds);
    obs_sceneitem_set_scale(scene_item, &zoom_scale);
    obs_sceneitem_defer_update_end(scene_item);

}

void SceneItemImpl::Crop(const CropInfo &crop_info, const BoundsInfo* bounds_info/* = nullptr*/)
{
    OBSSceneItem scene_item(LeakUnderlyingSource());
    vec2 scene_item_size = { 0 };
    if (!bounds_info)
    {
        scene_item_size = GetSourceSize();
        DCHECK(scene_item_size.x != 0 && scene_item_size.y != 0);
    }
    else
    {
        scene_item_size.x = bounds_info->w;
        scene_item_size.y = bounds_info->h;
    }

    // obs_sceneitem_crop参数的意义是裁减区四周的缩进量，不是中间实际裁剪区的尺寸
    obs_sceneitem_crop crop = { crop_info.x, crop_info.y,  };
    crop.left = crop_info.x;
    crop.top = crop_info.y;
    crop.right = scene_item_size.x - crop_info.x - crop_info.w;
    crop.bottom = scene_item_size.y - crop_info.y - crop_info.h;

    obs_sceneitem_defer_update_begin(scene_item);
    obs_sceneitem_set_crop(scene_item, &crop);
    obs_sceneitem_defer_update_end(scene_item);
}

void SceneItemImpl::EnableFixSize(int x, int y, int w, int h, FixType fixType)
{
    obs_sceneitem_t* sceneItem = LeakUnderlyingSource();

    if (sceneItem)
    {
        vec2 pos = { x, y }, bounds = { w, h };

        obs_sceneitem_set_bounds(sceneItem, &bounds);
        obs_sceneitem_set_pos(sceneItem, &pos);

        switch (fixType)
        {
        case FixType::INNER_RECT:
            obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_INNER);
            break;

        case FixType::OUTER_RECT:
            obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_OUTER);
            break;

        case FixType::FILL_WIDTH:
            obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_TO_WIDTH);
            break;

        case FixType::FILL_HEIGHT:
            obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_SCALE_TO_HEIGHT);
            break;

        default:
            obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_NONE);
            NOTREACHED();
            break;
        }
    }
}

void SceneItemImpl::DisableFixSize()
{
    obs_sceneitem_t* sceneItem = LeakUnderlyingSource();

    if (sceneItem)
    {
        obs_sceneitem_set_bounds_type(sceneItem, OBS_BOUNDS_NONE);
    }
}

vec2 SceneItemImpl::GetSourceSize() const
{
    OBSSceneItem scene_item(LeakUnderlyingSource());
    vec2 scene_item_size = GetItemSourceSize(scene_item);
    return scene_item_size;
}

matrix4 obs_proxy::SceneItemImpl::GetBoxTransform() const
{
    obs_sceneitem_t* sceneItem = LeakUnderlyingSource();
    matrix4 m;
    obs_sceneitem_get_draw_transform(sceneItem, &m);
    return m;
}

// -*- Transition implementation -*-

Transition::Transition(obs_source_t* raw_transition)
    : transition_(raw_transition)
{
    const char* name = obs_source_get_name(transition_);
    if (name)
    {
        name_ = name;
    }
}

Transition::~Transition()
{
    transition_stop_signal_.Disconnect();
    obs_source_release(transition_);
}

obs_source_t* Transition::LeakUnderlyingSource() const
{
    return transition_;
}

Transition::operator bool() const
{
    return transition_ != nullptr;
}

const std::string& Transition::name() const
{
    return name_;
}

void Transition::RegisterTransitionStopHandler(signal_callback_t callback, void* data)
{
    signal_handler_t* handler = obs_source_get_signal_handler(transition_);
    transition_stop_signal_.Connect(handler, kSignalTransitionVideoStop, callback, data);
}

}   // namespace obs_proxy