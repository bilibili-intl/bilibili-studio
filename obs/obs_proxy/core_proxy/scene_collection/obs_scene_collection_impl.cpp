#include "obs/obs_proxy/core_proxy/scene_collection/obs_scene_collection_impl.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_handle.h"

#include "bilibase/basic_types.h"
#include "bilibase/lambda_decay.h"
#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/resource_helper.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"

#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

#include "grit/generated_resources.h"



namespace {

using obs_proxy::AudioDeviceImpl;
using obs_proxy::Transition;
using obs_proxy::SceneCollectionImpl;
using obs_proxy::SceneImpl;
using obs_proxy::Scene;

// Scene collection data constants.
const char kKeyName[] = "name";
const char kKeySources[] = "sources";
const char kKeyCurrentTransition[] = "current_transition";
const char kKeyCurrentScene[] = "current_scene";
const char kKeyTransitionDuration[] = "transition_duration";
const char kKeySceneOrder[] = "scene_order";
const char kKeyTransitions[] = "transitions";
const char kKeyID[] = "id";
const char kKeySettings[] = "settings";
const uint32_t kDefaultTransDuration = 300;

const char kKeyDeviceID[] = "device_id";

const base::string16 kGameProcessName = L"bilibili大乱斗.exe";
const char kStormItemName[] = "storm-effect";

const std::vector<std::string> kPortraitScenes{
    prefs::kVerticalFirstDefaultSceneName,
    prefs::kVerticalSecondDefaultSceneName,
    prefs::kVerticalThirdDefaultSceneName,
};

const std::vector<std::string> kAllowableScenes
{
    prefs::kFirstDefaultSceneName,
    prefs::kSecondDefaultSceneName,
    prefs::kThirdDefaultSceneName,
    prefs::kVerticalFirstDefaultSceneName,
    prefs::kVerticalSecondDefaultSceneName,
    prefs::kVerticalThirdDefaultSceneName,
};

void ClearSceneData()
{
    // Know nothing about the channel id, just a shameless copycat from obs-studio.
    const uint32_t kMaxChannelID = 5;
    for (uint32_t i = 0; i <= kMaxChannelID; ++i)
    {
        obs_set_output_source(i, nullptr);
    }

    obs_enum_sources([](void*, obs_source_t* source) {
                         obs_source_release(source);
                         return true;
                     }, nullptr);
}

void SetupTransition(Transition& transition)
{
    Transition old_transition(obs_get_output_source(obs_proxy::kMainSceneChannel));
    if (old_transition && transition)
    {
        obs_transition_swap_begin(transition.LeakUnderlyingSource(),
                                  old_transition.LeakUnderlyingSource());
        obs_set_output_source(obs_proxy::kMainSceneChannel, transition.LeakUnderlyingSource());
        obs_transition_swap_end(transition.LeakUnderlyingSource(),
                                old_transition.LeakUnderlyingSource());
    }
    else
    {
        obs_set_output_source(obs_proxy::kMainSceneChannel, transition.LeakUnderlyingSource());
    }
}

bool HasAudioDeviceAttached(const std::string& audio_source_id)
{
    obs_properties_t* props = obs_get_source_properties(audio_source_id.c_str());
    if (!props)
    {
        return false;
    }

    ON_SCOPE_EXIT { obs_properties_destroy(props); };
    obs_property_t* devices = obs_properties_get(props, "device_id");
    size_t count = 0;
    if (devices)
    {
        count = obs_property_list_item_count(devices);
    }

    return count != 0;
}

// Audio output channel: 1, 2
// Audio input channel: 3, 4, 5
bool IsAudioOutputChannel(int channel)
{
    DCHECK(channel > 0);
    return channel < 3;
}

std::vector<std::string> QueryAttachedAudioDeviceID(const std::string& audio_source_id)
{
    std::vector<std::string> device_ids;

    obs_properties_t* props = obs_get_source_properties(audio_source_id.c_str());
    if (!props) {
        return device_ids;
    }

    ON_SCOPE_EXIT { obs_properties_destroy(props); };

    obs_property_t* device_list = obs_properties_get(props, kKeyDeviceID);
    if (!device_list) {
        return device_ids;
    }

    size_t device_count = obs_property_list_item_count(device_list);
    for (size_t i = 0; i < device_count; ++i) {
        device_ids.emplace_back(obs_property_list_item_string(device_list, i));
    }

    return device_ids;
}

std::string ExtractAudioDeviceIDFromData(obs_data_t* collection_data, const std::string& device_name)
{
    obs_data_t* device_data = obs_data_get_obj(collection_data, device_name.c_str());
    ON_SCOPE_EXIT { obs_data_release(device_data); };
    if (!device_data) {
        return std::string();
    }

    obs_data_t* settings = obs_data_get_obj(device_data, "settings");
    ON_SCOPE_EXIT { obs_data_release(settings); };
    if (!settings) {
        return std::string();
    }

    auto id = obs_data_get_string(settings, kKeyDeviceID);
    return id ? std::string(id) : std::string();
}

bool IsAudioSource(obs_source_t* source)
{
    auto flags = obs_source_get_output_flags(source);
    return !!(flags & OBS_SOURCE_AUDIO);
}

void OnCollectionSourceLoaded(void* data, obs_source_t* source)
{
    obs_scene_t* raw_scene = obs_scene_from_source(source);
    if (raw_scene)
    {
        // obs would internally decrement ref-count of the `source` after loading sources.
        // We increment ref-count to take over.
        obs_source_addref(source);
        auto scene = SceneImpl::FromExistingScene(raw_scene);

        // 不管什么原因，如果用户的obs配置文件多了我们不允许的场景记录，在加载时就不要加载进来，防止崩溃
        auto it = std::find_if(kAllowableScenes.begin(), kAllowableScenes.end(),
            [&scene](const std::string& name)
            {
                return name == scene->name();
            });
        if (it != kAllowableScenes.end())
        {
            SceneCollectionImpl* scene_collection = static_cast<SceneCollectionImpl*>(data);
            scene_collection->AddExistingScene(std::move(scene));
        }
        else
        {
            LOG(WARNING) << "not allow scene found, name='" << scene->name() << "'";
            NOTREACHED();
        }
    }
}

//void ReorderScene(std::vector<std::unique_ptr<SceneImpl>>& scenes, const std::string& name,
//                  size_t target_index)
//{
//    auto it = std::find_if(scenes.begin(), scenes.end(),
//                           [&name](const std::unique_ptr<SceneImpl>& scene) {
//                               return scene->name() == name;
//                           });
//    if (it != scenes.end())
//    {
//        target_index = std::min(target_index, scenes.size() - 1);
//        auto target = std::next(scenes.begin(), target_index);
//        if (it != target)
//        {
//            std::swap(*it, *target);
//        }
//    }
//    else
//    {
//        NOTREACHED() << "not allow scene found, name='" << name << "'";
//    }
//}

void MakeScenesOrdered(std::vector<std::unique_ptr<SceneImpl>>& scenes, obs_data_array_t* order_list)
{
    /*size_t count = obs_data_array_count(order_list);
    for (size_t i = 0; i < count; ++i)
    {
        obs_data_t* data = obs_data_array_item(order_list, i);
        const char* scene_name = obs_data_get_string(data, kKeyName);
        ReorderScene(scenes, scene_name, i);
        obs_data_release(data);
    }*/
    // 由于历史原因或是用户自行修改配置文件的原因，有可能出现在配置文件中场景的顺序记录多了
    // 一个或多个我们不允许的场景顺序记录项，就导致我们在对已经过滤好了的场景列表进行按名排序时可能乱序，
    // 所以这里的排序就不要按obs配置文件里面记录的来排序了，直接按场景列表里面的场景名来排序。
    // 注意！！这就要求如果以后要新增场景，那么在新场景的命名上要按照首字母ascii码顺序来命名
    std::sort(scenes.begin(), scenes.end(),
        [](std::unique_ptr<SceneImpl>& item1, std::unique_ptr<SceneImpl>& item2)->bool {
            return item1->name() < item2->name();
        }
    );
}

obs_data_array_t* GenerateSceneOrderData(const std::vector<Scene*>& scenes)
{
    obs_data_array_t* order_list = obs_data_array_create();
    for (auto scene : scenes)
    {
        //连麦场景不加进去
        if (scene->name() != prefs::kJoinMicSceneName)
        {
            obs_data_t* order_item = obs_data_create();
            obs_data_set_string(order_item, kKeyName, scene->name().c_str());
            obs_data_array_push_back(order_list, order_item);
            obs_data_release(order_item);
        }
    }

    return order_list;
}

obs_data_array_t* GenerateTransitionsData(const std::vector<Transition*>& transitions)
{
    obs_data_array_t* transitions_data = obs_data_array_create();
    for (auto transition : transitions)
    {
        obs_source_t* raw_transition = transition->LeakUnderlyingSource();
        if (!obs_source_configurable(raw_transition))
        {
            continue;
        }

        obs_data_t* data = obs_data_create();
        obs_data_t* settings = obs_source_get_settings(raw_transition);

        obs_data_set_string(data, kKeyName, transition->name().c_str());
        obs_data_set_string(data, kKeyID, obs_obj_get_id(raw_transition));
        obs_data_set_obj(data, kKeySettings, settings);

        obs_data_array_push_back(transitions_data, data);

        obs_data_release(settings);
        obs_data_release(data);
    }

    return transitions_data;
}

void GenerateAudioDeviceData(const char* name, int channel, obs_data_t* collection_data,
                             std::vector<obs_source_t*>& audio_sources)
{
    obs_source_t* source = obs_get_output_source(channel);
    if (!source)
    {
        return;
    }

    obs_data_t* data = obs_save_source(source);
    obs_data_set_obj(collection_data, name, data);
    obs_data_release(data);

    // Transfer to outside.
    audio_sources.push_back(source);
}

void AddSourceToVec(const char* name,
    std::vector<obs_source_t*>& sources_vec)
{
    obs_source_t* source = obs_get_source_by_name(name);
    if (source)
    {
        sources_vec.push_back(source);
    }
}

obs_data_array_t* GenerateSourcesData(const std::vector<obs_source_t*>& sources)
{
    using std::placeholders::_1;
    using SourceVec = std::vector<obs_source_t*>;
    obs_data_array_t* sources_data =
            obs_save_sources_filtered([](void* data, obs_source_t* source) {
                const auto& special_sources = *static_cast<SourceVec*>(data);
                return std::none_of(special_sources.cbegin(), special_sources.cend(),
                                    std::bind(std::equal_to<obs_source_t*>(), source, _1));
            }, const_cast<SourceVec*>(&sources));

    return sources_data;
}

bool NeedUpgradeSourceData()
{
    base::win::ScopedHandle mark(OpenMutexW(MUTEX_MODIFY_STATE, FALSE,
                                            bililive::kOBSSourceDataUpgradeMarkName));
    return mark.IsValid();
}

bool NeedItemVisualized(obs_proxy::SceneItemType item_type)
{
    return item_type != obs_proxy::SceneItemType::Text &&
        item_type != obs_proxy::SceneItemType::AudioInputCapture &&
        item_type != obs_proxy::SceneItemType::AudioOutputCapture;
}

}   // namespace

namespace obs_proxy {

SceneCollectionImpl::SceneCollectionImpl(const base::FilePath& path, const std::string& name,
                                         obs_data_t* collection_data)
    : collection_path_(path),
      name_(name),
      current_scene_(nullptr),
      fade_transition_(nullptr),
      transition_duration_(kDefaultTransDuration)
{
    ClearSceneData();
    InitDefaultTransitions();

    BindObsSignals();

    // Load sources.

    TryLoadAudioDevice(kDesktopAudio1, kAudioOutputCaptureItemID, kDesktopAudio1Channel, collection_data);
    TryLoadAudioDevice(kAuxAudio1, kAudioInputCaptureItemID, kAuxAudio1Channel, collection_data);

    obs_data_array_t* sources = obs_data_get_array(collection_data, kKeySources);
    ON_SCOPE_EXIT { obs_data_array_release(sources); };

    for (int i = 0; i < obs_data_array_count(sources); i++)
    {
        auto item = obs_data_array_item(sources, i);
        if (item)
        {
            std::string type = obs_data_get_string(item, kKeyID);
            if(type == "audio_capture")
            {
                obs_data_array_erase(sources, i);
            }
        }
    }
    obs_load_sources(sources, &OnCollectionSourceLoaded, this);

    obs_data_array_t* order_list = obs_data_get_array(collection_data, kKeySceneOrder);
    ON_SCOPE_EXIT { obs_data_array_release(order_list); };
    MakeScenesOrdered(scenes_, order_list);

    // 当前没有针对旧obs配置文件进行更新的机制，针对此次竖屏支持，
    // 直接在这里采取判断竖屏场景是否存在，不存在就增加的方式进行添加
    for (auto& iter : kAllowableScenes)
    {
        if (!GetScene(iter))
        {
            SceneCollectionImpl::AddNewScene(iter, false, 0, 0);
        }
    }

    // Set up transitions.

    base::StringPiece current_transition_name = obs_data_get_string(collection_data,
                                                                    kKeyCurrentTransition);
    if (current_transition_name.empty() && fade_transition_)
    {
        current_transition_name = fade_transition_->name();
    }

    int duration = obs_data_get_int(collection_data, kKeyTransitionDuration);
    if (duration != 0)
    {
        transition_duration_ = duration;
    }

    Transition* current_transition = fade_transition_;
    auto trans_it = std::find_if(transitions_.cbegin(), transitions_.cend(),
                           [current_transition_name](const std::unique_ptr<Transition>& trans) {
                               return current_transition_name == trans->name();
                           });
    if (trans_it != transitions_.cend())
    {
        current_transition = trans_it->get();
    }

    DCHECK(current_transition != nullptr);
    SetupTransition(*current_transition);

    // Set up scenes.

    const char* scene_name = obs_data_get_string(collection_data, kKeyCurrentScene);
    auto scene_it = std::find_if(scenes_.cbegin(), scenes_.cend(),
                                 [scene_name](const std::unique_ptr<SceneImpl>& scene) {
                                     return scene->name() == scene_name;
                                 });
    DCHECK(scene_it != scenes_.cend());
    SetCurrentScene(scene_it->get(), true);

    // Update last used collection.
    std::string collection_name = path.BaseName().AsUTF8Unsafe();
    GetPrefs()->SetString(prefs::kLastSceneCollectionName, collection_name);
}

SceneCollectionImpl::SceneCollectionImpl(const base::FilePath& path, const std::string& name)
    : collection_path_(path),
      name_(name),
      current_scene_(nullptr),
      fade_transition_(nullptr),
      transition_duration_(kDefaultTransDuration)
{
    ClearSceneData();
    InitDefaultTransitions();

    if (fade_transition_)
    {
        SetupTransition(*fade_transition_);
    }

    BindObsSignals();

    SetupDefaultAudioDevices();

    for (auto& iter : kAllowableScenes)
    {
        SceneCollectionImpl::AddNewScene(iter, false, 0, 0);
    }

    SetCurrentScene(scenes_.front().get(), true);

    // Update last used collection.
    std::string collection_name = path.BaseName().AsUTF8Unsafe();
    GetPrefs()->SetString(prefs::kLastSceneCollectionName, collection_name);
}

SceneCollectionImpl::~SceneCollectionImpl()
{}

void SceneCollectionImpl::PostInitialize(bool from_file)
{
    // Manually notify in loading stage.
    auto controls = GetVolumeControllers();
    for (auto volume_controller : controls)
    {
        OnAudioSourceActivated(volume_controller);
    }

    size_t index = 0;
    if(from_file)
    {
        auto&& scenes = GetScenes();
        auto cur_scene = current_scene();
        auto it = std::find_if(scenes.cbegin(), scenes.cend(),
                               [cur_scene](const Scene* scene) {
                                   return cur_scene == scene;
                               });
        DCHECK(it != scenes.cend());
        index = std::distance(scenes.cbegin(), it);
    }
    OnCurrentSceneChanged(index);
}

// static
std::unique_ptr<SceneCollectionImpl>
    SceneCollectionImpl::FromCollectionFile(const base::FilePath& collection_path)
{
    if (!base::PathExists(collection_path))
    {
        LOG(WARNING) << "Cannot locate collection file: " << collection_path.value();
        return nullptr;
    }

    auto path = collection_path.AsUTF8Unsafe();
    obs_data_t* collec_data = obs_data_create_from_json_file(path.c_str());
    ON_SCOPE_EXIT { obs_data_release(collec_data); };
    if (!collec_data)
    {
        LOG(WARNING) << "Unable to read collection file: " << collection_path.value()
                     << "; data maybe corrupted!";
        return nullptr;
    }

    std::string collection_name = obs_data_get_string(collec_data, kKeyName);
    if (collection_name.empty())
    {
        collection_name = collection_path.BaseName().AsUTF8Unsafe();
    }

    if (NeedUpgradeSourceData()) {}

    std::unique_ptr<SceneCollectionImpl> collection(new SceneCollectionImpl(collection_path, collection_name, collec_data));
    collection->PostInitialize(true);
    return collection;
}

// static
std::unique_ptr<SceneCollectionImpl>
    SceneCollectionImpl::ForDefault(const base::FilePath& collection_save_path)
{
    if (base::PathExists(collection_save_path))
    {
        LOG(INFO) << "Collection file " << collection_save_path.value()
                  << " already exists. Overwrite it!";
    }

    std::string collection_name = collection_save_path.BaseName().AsUTF8Unsafe();
    std::unique_ptr<SceneCollectionImpl> collection(new SceneCollectionImpl(collection_save_path, collection_name));
    collection->PostInitialize(false);
    return collection;
}

Scene* SceneCollectionImpl::AddNewScene(const std::string& name, bool custom_size, int cx, int cy)
{
    Scene* scene_view;
    try
    {
        auto scene = std::make_unique<SceneImpl>(name, custom_size, cx, cy);
        scene_view = scene.get();
        scenes_.push_back(std::move(scene));
    }
    catch (const CreatingSceneError& ex)
    {
        LOG(WARNING) << ex.what();
        scene_view = nullptr;
    }

    return scene_view;
}

void SceneCollectionImpl::AddExistingScene(std::unique_ptr<SceneImpl> scene)
{
    scenes_.push_back(std::move(scene));
}

bool SceneCollectionImpl::RemoveScene(const std::string& name)
{
    bool hit = false;
    for (auto it = scenes_.begin(); it != scenes_.end();)
    {
        if (it->get()->name() == name)
        {
            try
            {
                it->reset();
            }
            catch (const RemovingSceneError& ex)
            {
                LOG(WARNING) << ex.what();
            }

            it = scenes_.erase(it);
            hit = true;
        }
        else
        {
            ++it;
        }
    }

    return hit;
}

void SceneCollectionImpl::AddAudioDevice(std::unique_ptr<AudioDeviceImpl> audio_device)
{
    audio_devices_.push_back(std::move(audio_device));
}

obs_proxy::VolumeController* SceneCollectionImpl::AddVolumeController(obs_source_t* raw_source)
{
    // The source has died during notification.
    if (!raw_source)
    {
        return nullptr;
    }

    const char* raw_name = obs_source_get_name(raw_source);
    if (!raw_name)
    {
        NOTREACHED();
        return nullptr;
    }
    const char* type_id = obs_source_get_id(raw_source);
    //if (strcmp(raw_name, prefs::kJoinMicSceneRemoteSourceName) == 0) {
    if((strcmp(type_id,"rtc_colive_source") == 0) || (strcmp(type_id, "colive_source") == 0)){
        // 连麦远程源不要创建 VolumeController，因为移除远程源时没法同时移除对应的 VolumeController，
        // 并且对于连麦远程源来说，VolumeController 没用。
        return nullptr;
    }

    obs_proxy::VolumeController* volume_controller_ptr = GetVolumeController(raw_source);
    if (nullptr == volume_controller_ptr)
    {
        std::string source_name(raw_name);
        auto volume_controller = std::make_unique<obs_proxy::VolumeControllerImpl>(raw_source, source_name);
        volume_controller_ptr = volume_controller.get();

        AddVolumeController(std::move(volume_controller));

        obs_proxy::AudioDevice *audio_dev =
            static_cast<obs_proxy::AudioDevice*>(GetAudioSourceForVolumeController(volume_controller_ptr));
        if (audio_dev)
        {
            audio_dev->AddNewFilter(obs_proxy::FilterType::Gain, base::UTF16ToUTF8(bililive::LocalStr(IDS_GAIN_TXT)));
        }
    }
    return volume_controller_ptr;
}

void SceneCollectionImpl::AddVolumeController(std::unique_ptr<VolumeControllerImpl> volume_controller)
{
    volume_controllers_.push_back(std::move(volume_controller));
}

void SceneCollectionImpl::RemoveVolumeController(obs_source_t* bound_audio_source)
{
    auto nend = std::remove_if(volume_controllers_.begin(), volume_controllers_.end(),
                               [bound_audio_source](const std::unique_ptr<VolumeControllerImpl>& controller) {
                                   return controller->LeakBoundAudioSource() == bound_audio_source;
                               });
    volume_controllers_.erase(nend, volume_controllers_.end());
}

VolumeController* SceneCollectionImpl::GetVolumeController(obs_source_t* bound_audio_source) const
{
    auto it = std::find_if(volume_controllers_.begin(), volume_controllers_.end(),
                           [bound_audio_source](const std::unique_ptr<VolumeControllerImpl>& controller) {
                               return controller->LeakBoundAudioSource() == bound_audio_source;
                           });
    return it != volume_controllers_.end() ? it->get() : nullptr;
}

std::vector<VolumeController*> SceneCollectionImpl::GetVolumeControllers() const
{
    return AcquireSourceViews<VolumeControllerImpl, VolumeController>(volume_controllers_);
}

void SceneCollectionImpl::SceneItemRemove(obs_source_t* underlying_source)
{
    if (IsAudioSource(underlying_source))
    {
        RemoveVolumeController(underlying_source);
    }
}

const std::string& SceneCollectionImpl::name() const
{
    return name_;
}

void SceneCollectionImpl::set_name(const std::string& new_name)
{
    name_ = new_name;
}

Scene* SceneCollectionImpl::current_scene() const
{
    return current_scene_;
}

Scene* SceneCollectionImpl::TransitToScene(const std::string& name)
{
    auto target_scene = GetScene(name);
    DCHECK(target_scene != nullptr);
    if (target_scene)
    {
        SetCurrentScene(target_scene, false);

        // 回调通知
        auto it = std::find_if(scenes_.cbegin(), scenes_.cend(),
            [target_scene](const std::unique_ptr<SceneImpl>& scene)
        {
            return target_scene == scene.get();
        });
        if (it != scenes_.cend())
        {
            size_t index = std::distance(scenes_.cbegin(), it);
            current_scene_changed_handler_(index);
        }
        else
        {
            DCHECK(false);
        }
    }

    return target_scene;
}

SceneImpl* SceneCollectionImpl::GetScene(const std::string& name) const
{
    auto it = std::find_if(scenes_.begin(), scenes_.end(),
                           [&name](const std::unique_ptr<SceneImpl>& scene) {
                               return scene ? (name == scene->name()) : false;
                           });
    if (it != scenes_.end()) {
        return it->get();
    }

    return nullptr;
}

std::vector<Scene*> SceneCollectionImpl::GetScenes() const
{
    return AcquireSourceViews<SceneImpl, Scene>(scenes_);
}

AudioDeviceImpl* SceneCollectionImpl::GetAudioDevice(const std::string& name) const
{
    auto it = std::find_if(audio_devices_.begin(), audio_devices_.end(),
                           [&name](const std::unique_ptr<AudioDeviceImpl>& audio_device) {
                               return name == audio_device->name();
                           });
    return it == audio_devices_.end() ? nullptr : it->get();
}

std::vector<AudioDevice*> SceneCollectionImpl::GetAudioDevices() const
{
    return AcquireSourceViews<AudioDeviceImpl, AudioDevice>(audio_devices_);
}

VolumeController*
    SceneCollectionImpl::GetVolumeControllerForAudioSource(AudioSourceProxy audio_source) const
{
    if (!audio_source)
    {
        NOTREACHED() << "The audio source cannot be empty!";
        return nullptr;
    }

    obs_source_t* bound_source = nullptr;
    if (audio_source.source_type() == internal::SourceType::AudioDevice)
    {
        bound_source = impl_cast(static_cast<AudioDevice*>(audio_source))->LeakUnderlyingSource();
    }
    else
    {
        bound_source = impl_cast(static_cast<SceneItem*>(audio_source))->AsSource();
    }

    return GetVolumeController(bound_source);
}

AudioSourceProxy
    SceneCollectionImpl::GetAudioSourceForVolumeController(VolumeController* volume_controller) const
{
    std::string bound_source_name = volume_controller->GetBoundSourceName();

    auto audio_device = GetAudioDevice(bound_source_name);
    if (audio_device)
    {
        return AudioSourceProxy(audio_device);
    }

    // Seems that this audio source is in fact a scene-item.
    for (auto& scene : scenes_)
    {
        auto scene_item = scene->GetItem(bound_source_name);
        if (scene_item)
        {
            return AudioSourceProxy(scene_item);
        }
    }

    // Oops.
    return AudioSourceProxy(nullptr);
}

uint32_t SceneCollectionImpl::transition_duration() const
{
    return transition_duration_;
}

void SceneCollectionImpl::set_transition_duration(uint32_t duration)
{
    transition_duration_ = duration;
}

void SceneCollectionImpl::Save() const
{
    obs_data_t* collection_data = GenerateCollectionData();
    ON_SCOPE_EXIT { obs_data_release(collection_data); };
    auto collection_path = collection_path_.AsUTF8Unsafe();
    bool rv = obs_data_save_json(collection_data, collection_path.c_str());
    LOG_IF(WARNING, !rv) << "Could not save scene collection data to " << collection_path;
}

void SceneCollectionImpl::AddGameSource(obs_proxy::SceneImpl* joinmic_scene)
{
    base::string16 scene_name = L"bilibili游戏";
    auto source = joinmic_scene->AttachNewItem(SceneItemType::GameCapture, base::UTF16ToUTF8(scene_name), true);
    GameSceneItemHelper item(source);
    auto list = item.WindowList();
    std::string selected_data;
    bool find_game = false;
    for (auto it = list.begin(); it != list.end();) {
        selected_data = std::get<1>(*it);
        base::string16 game_name = base::UTF8ToUTF16(selected_data);
        if (game_name.find(kGameProcessName) != game_name.npos)
        {
            find_game = true;
            break;
        }
        else
        {
            ++it;
        }
    }

    if (find_game)
    {
        item.SetAdaptedGame(false);
        item.SelectedWindow(selected_data);
        item.IsAntiCheat(true);
        item.IsSliCompatible(true);
        item.SetFitToScreen(true);
        item.Update();

        int scene_width = obs_source_get_width(joinmic_scene->AsSource());
        int scene_height = obs_source_get_height(joinmic_scene->AsSource());

        source->EnableFixSize(0, 0, scene_width, scene_height,
            obs_proxy::SceneItem::FixType::OUTER_RECT);
    }
}

bool SceneCollectionImpl::AddGameDlgSource()
{
    obs_proxy::SceneImpl* joinmic_scene =
        static_cast<obs_proxy::SceneImpl*>(GetScene(prefs::kJoinMicSceneName));
    if (joinmic_scene)
    {
        base::string16 scene_name = L"bilibili窗口";
        auto source = joinmic_scene->AttachNewItem(SceneItemType::WindowCapture, base::UTF16ToUTF8(scene_name), true);
        WindowSceneItemHelper item(source);
        auto list = item.WindowList();
        std::string selected_data;
        bool find_game = false;
        for (auto it = list.begin(); it != list.end();) {
            selected_data = std::get<1>(*it);
            base::string16 game_name = base::UTF8ToUTF16(selected_data);
            if (game_name.find(kGameProcessName) != game_name.npos)
            {
                find_game = true;
                break;
            }
            else
            {
                ++it;
            }
        }
        if (find_game)
        {
            item.SelectedWindow(selected_data);
            item.SetCaptureModel(0);
            item.IsCaptureCursor(false);
            item.IsCompatibleMode(false);
            item.IsSliMode(false);
            item.Update();

            int scene_width = obs_source_get_width(joinmic_scene->AsSource());
            int scene_height = obs_source_get_height(joinmic_scene->AsSource());

            source->EnableFixSize(0, 0, scene_width, scene_height,
                obs_proxy::SceneItem::FixType::OUTER_RECT);

            return true;
        }
    }
    return false;
}

void SceneCollectionImpl::Cleanup()
{
    DismissSignalHandlers();

    scenes_.clear();

    volume_controllers_.clear();
}

void SceneCollectionImpl::BindObsSignals()
{
    signal_handler_t* raw_signal = obs_get_signal_handler();
    audio_source_activate_handler_.second.Connect(raw_signal,
                                                  kSignalSourceActivate,
                                                  SceneCollectionImpl::OnSourceActivateHandlerRedirect,
                                                  this);
    audio_source_deactivate_handler_.second.Connect(raw_signal,
                                                    kSignalSourceDeactivate,
                                                    SceneCollectionImpl::OnSourceDeactivateHandlerRedirect,
                                                    this);
    source_show_handler_.second.Connect(raw_signal,
                                        kSignalSourceShow,
                                        SceneCollectionImpl::OnSourceShowHandlerRedirect,
                                        this);
    source_rename_handler_.second.Connect(raw_signal,
                                          kSignalSourceRename,
                                          SceneCollectionImpl::OnSourceRenameHandlerRedirect,
                                          this);
    source_load_handler_.Connect(raw_signal,
                                 kSignalSourceLoad,
                                 SceneCollectionImpl::OnSourceLoadHandlerRedirect,
                                 this);
}

void SceneCollectionImpl::BindTransitionSignalHandler(Transition* transition)
{
    transition->RegisterTransitionStopHandler(&SceneCollectionImpl::OnTransitionStop, this);
}

void SceneCollectionImpl::DismissSignalHandlers()
{
    source_show_handler_.second.Disconnect();
    source_rename_handler_.second.Disconnect();
    audio_source_activate_handler_.second.Disconnect();
    audio_source_deactivate_handler_.second.Disconnect();
    source_load_handler_.Disconnect();
}

void SceneCollectionImpl::RegisterAudioSourceActivateHandler(AudioSourceActivateHandler handler)
{
    DCHECK(!audio_source_activate_handler_.first);
    audio_source_activate_handler_.first = handler;
}

void SceneCollectionImpl::RegisterAudioSourceDeactivateHandler(AudioSourceDeactivateHandler handler)
{
    DCHECK(!audio_source_deactivate_handler_.first);
    audio_source_deactivate_handler_.first = handler;
}

void SceneCollectionImpl::RegisterSourceShowHandler(SourceShowHandler handler)
{
    DCHECK(!source_show_handler_.first);
    source_show_handler_.first = handler;
}

void SceneCollectionImpl::RegisterSourceRenameHandler(SourceRenameHandler handler)
{
    DCHECK(!source_rename_handler_.first);
    source_rename_handler_.first = handler;
}

void SceneCollectionImpl::RegisterCurrentSceneChangedHandler(CurrentSceneChangedHandler handler)
{
    DCHECK(!current_scene_changed_handler_);
    current_scene_changed_handler_ = handler;
}

obs_data_t* SceneCollectionImpl::GenerateCollectionData() const
{
    obs_data_t* collection_data = obs_data_create();

    obs_data_set_string(collection_data, kKeyName, name().c_str());

    obs_data_set_string(collection_data, kKeyCurrentScene,
                        current_scene() ? current_scene()->name().c_str() : "");

    obs_data_array_t* scene_order_list = GenerateSceneOrderData(GetScenes());
    ON_SCOPE_EXIT { obs_data_array_release(scene_order_list); };
    obs_data_set_array(collection_data, kKeySceneOrder, scene_order_list);

    Transition current_transition(obs_get_output_source(kMainSceneChannel));
    obs_data_set_string(collection_data, kKeyCurrentTransition, current_transition.name().c_str());

    obs_data_array_t* transitions = GenerateTransitionsData(GetTransitions());
    ON_SCOPE_EXIT { obs_data_array_release(transitions); };
    obs_data_set_array(collection_data, kKeyTransitions, transitions);

    obs_data_set_int(collection_data, kKeyTransitionDuration, transition_duration());

    //把不保存到配置的源写到这里
    std::vector<obs_source_t*> not_save_sources;
    ON_SCOPE_EXIT { std::for_each(not_save_sources.begin(), not_save_sources.end(), obs_source_release); };
    GenerateAudioDeviceData(kDesktopAudio1, kDesktopAudio1Channel, collection_data, not_save_sources);
    GenerateAudioDeviceData(kDesktopAudio2, kDesktopAudio2Channel, collection_data, not_save_sources);
    GenerateAudioDeviceData(kAuxAudio1, kAuxAudio1Channel, collection_data, not_save_sources);
    GenerateAudioDeviceData(kAuxAudio2, kAuxAudio2Channel, collection_data, not_save_sources);
    GenerateAudioDeviceData(kAuxAudio3, kAuxAudio3Channel, collection_data, not_save_sources);

    //连麦相关的源加到不保存配置的vector里面
    AddSourceToVec(prefs::kJoinMicSceneName, not_save_sources);
    AddSourceToVec(prefs::kJoinMicSceneRemoteSourceName, not_save_sources);
    AddSourceToVec(prefs::kJoinMicSceneMaskSourceName, not_save_sources);
    obs_proxy::SceneImpl* joinmic_scene =
        static_cast<obs_proxy::SceneImpl*>(GetScene(prefs::kJoinMicSceneName));
    if (joinmic_scene)
    {
        for (auto iter : joinmic_scene->GetItems())
        {
            if (iter->type() == obs_proxy::SceneItemType::ColiveSource ||
                iter->type() == obs_proxy::SceneItemType::RtcColiveSource ||
                iter->type() == obs_proxy::SceneItemType::MainSceneRendererSource)
            {
                AddSourceToVec(iter->name().c_str(), not_save_sources);
            }
        }
    }
    //语音连麦不保存远程源
    obs_enum_sources([](void* source_vec, obs_source_t* source) {
        auto& vec = *static_cast<std::vector<obs_source_t*>*>(source_vec);
        std::string source_id = obs_source_get_id(source);

        if (source_id == "voice_link_source" || source_id == "rtc_voice_link_source" ||
            source_id == "audio_capture")
        {
            auto name = obs_source_get_name(source);
            if (name)
            {
                AddSourceToVec(name, vec);
            }
        }
        return true;
        }, &not_save_sources);

    AddSourceToVec(prefs::kColiveLiveRoomSceneName, not_save_sources);
    obs_proxy::SceneImpl* chat_room_scene =
        static_cast<obs_proxy::SceneImpl*>(GetScene(prefs::kColiveLiveRoomSceneName));
    if (chat_room_scene)
    {
        for (auto iter : chat_room_scene->GetItems())
        {
            AddSourceToVec(iter->name().c_str(), not_save_sources);
        }
    }

	AddSourceToVec(kStormItemName, not_save_sources);
    //回声测试相关
    AddSourceToVec(prefs::kMicRecordMediaSourceName, not_save_sources);


    //虚拟源不再保存
    obs_enum_sources([](void* source_vec, obs_source_t* source) {
        auto& vec = *static_cast<std::vector<obs_source_t*>*>(source_vec);
        std::string source_id = obs_source_get_id(source);
        if (source_id == "vtuber_input")
        {
            auto name = obs_source_get_name(source);
            if (name)
            {
                AddSourceToVec(name, vec);
            }
        }
        return true;
        }, &not_save_sources);


    obs_data_array_t* sources = GenerateSourcesData(not_save_sources);
    ON_SCOPE_EXIT { obs_data_array_release(sources); };
    obs_data_set_array(collection_data, kKeySources, sources);

    return collection_data;
}

void SceneCollectionImpl::InitDefaultTransitions()
{
    // Automatically add transitions that have no configuration (things such as cut/fade/etc).
    const char* id = nullptr;
    for (size_t i = 0; obs_enum_transition_types(i, &id); ++i)
    {
        if (!obs_is_source_configurable(id))
        {
            const char* name = obs_source_get_display_name(id);
            auto transition = std::make_unique<Transition>(obs_source_create_private(id,
                                                                                     name,
                                                                                     nullptr));
            BindTransitionSignalHandler(transition.get());
            if (strcmp(id, kFadeTransitionID) == 0)
            {
                fade_transition_ = transition.get();
            }

            transitions_.push_back(std::move(transition));
        }
    }

    if (!transitions_.empty())
    {
        SetupTransition(*transitions_.front());
    }
}

void SceneCollectionImpl::ResetAudioDevice(const std::string& audio_source_id, const std::string& device_id,
                                           const std::string& name, uint32_t channel)
{
    bool is_same_device = false;
    obs_source_t* channel_source = obs_get_output_source(channel);

    // If the channel is not empty, check if the associated device source is same as the one
    // we are going to set.
    if (channel_source) {
        obs_data_t* settings = obs_source_get_settings(channel_source);
        const char* channel_device_id = obs_data_get_string(settings, kKeyDeviceID);
        is_same_device = device_id == channel_device_id;
        obs_data_release(settings);
        obs_source_release(channel_source);
    }

    if (is_same_device) {
        return;
    }

    obs_set_output_source(channel, nullptr);

    if (device_id != "disabled") {
        obs_data_t* settings = obs_data_create();
        obs_data_set_string(settings, kKeyDeviceID, device_id.c_str());
        channel_source = obs_source_create(audio_source_id.c_str(), name.c_str(), settings, nullptr);
        DCHECK(channel_source != nullptr);

        AddAudioDevice(std::make_unique<AudioDeviceImpl>(channel_source, channel, name, true));
        obs_set_output_source(channel, channel_source);

        obs_data_release(settings);
    }
}

void SceneCollectionImpl::SetupDefaultAudioDevices()
{
    if (HasAudioDeviceAttached(kAudioOutputCaptureItemID)) {
        ResetAudioDevice(kAudioOutputCaptureItemID, "default", kDesktopAudio1, kDesktopAudio1Channel);
    }

    if (HasAudioDeviceAttached(kAudioInputCaptureItemID)) {
        ResetAudioDevice(kAudioInputCaptureItemID, "default", kAuxAudio1, kAuxAudio1Channel);
    }
}

void SceneCollectionImpl::TryLoadAudioDevice(const std::string& device_name,
                                             const std::string& audio_source_id,
                                             int channel,
                                             obs_data_t* collection_data)
{
    auto pref_key = IsAudioOutputChannel(channel) ? prefs::kAudioSpeakerDeviceInUse :
                                                    prefs::kAudioMicDeviceInUse;

    // No device attached; rectify prefs and discard last device source data.
    if (!HasAudioDeviceAttached(audio_source_id.c_str())) {
        if (GetPrefs()->GetString(pref_key) != prefs::kNoAudioDevice) {
            GetPrefs()->SetString(pref_key, prefs::kNoAudioDevice);
        }

        return;
    }

    auto attached_device_ids = QueryAttachedAudioDeviceID(audio_source_id);
    DCHECK(!attached_device_ids.empty());
    auto device_id_in_data = ExtractAudioDeviceIDFromData(collection_data, device_name);

    auto it = std::find(attached_device_ids.cbegin(), attached_device_ids.cend(), device_id_in_data);

    // Useless audio device data, discard it and resort to using default device.
    if (it == attached_device_ids.cend()) {
        GetPrefs()->SetString(pref_key, prefs::kDefaultAudioDevice);
        ResetAudioDevice(audio_source_id.c_str(), "default", device_name.c_str(), channel);
        return;
    }

    auto last_used_device_id = GetPrefs()->GetString(pref_key);
    if (last_used_device_id != device_id_in_data) {
        GetPrefs()->SetString(pref_key, device_id_in_data);
    }

    // ExtractAudioDeviceIDFromData() has already validated `device_data`.
    obs_data_t* device_data = obs_data_get_obj(collection_data, device_name.c_str());
    ON_SCOPE_EXIT { obs_data_release(device_data); };

    obs_source_t* source = obs_load_source(device_data);
    ON_SCOPE_EXIT { obs_source_release(source); };

    // WTF, we have to fallback.
    if (!source) {
        GetPrefs()->SetString(pref_key, prefs::kDefaultAudioDevice);
        ResetAudioDevice(audio_source_id.c_str(), "default", device_name.c_str(), channel);
        return;
    }

    AddAudioDevice(std::make_unique<AudioDeviceImpl>(source, channel, device_name, false));
    obs_set_output_source(channel, source);
}

void SceneCollectionImpl::SetCurrentScene(SceneImpl* scene, bool force)
{
    Transition transition(obs_get_output_source(kMainSceneChannel));
    auto* raw_transition = transition.LeakUnderlyingSource();
    obs_source_t* raw_source = obs_scene_get_source(scene->LeakUnderlyingSource());
    if (force)
    {
        obs_transition_set(raw_transition, raw_source);
    }
    else
    {
        obs_transition_start(raw_transition, OBS_TRANSITION_MODE_AUTO, transition_duration_,
                             raw_source);
    }

    current_scene_ = scene;
}

std::vector<Transition*> SceneCollectionImpl::GetTransitions() const
{
    return AcquireSourceViews<Transition, Transition>(transitions_);
}

// static
void SceneCollectionImpl::OnTransitionStop(void* data, calldata_t* cb_data)
{
    UNUSED_VAR(data);
    UNUSED_VAR(cb_data);
    DLOG(INFO) << "OnTransitionStop";
}

void SceneCollectionImpl::OnSourceShowHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_source = static_cast<obs_source_t*>(calldata_ptr(params, "source"));
    std::string source_name = obs_source_get_name(raw_source);
    auto scene_collection = static_cast<SceneCollectionImpl*>(data);

    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                             base::Bind(&SceneCollectionImpl::OnSourceShow,
                                        base::Unretained(scene_collection),
                                        source_name));
}

void SceneCollectionImpl::OnSourceActivateHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_source = static_cast<obs_source_t*>(calldata_ptr(params, "source"));
    if (IsAudioSource(raw_source))
    {
        auto scene_collection = static_cast<SceneCollectionImpl*>(data);
        auto weak_source = obs_source_get_weak_source(raw_source);

        BililiveThread::PostTask(BililiveThread::UI,
                                 FROM_HERE,
                                 base::Bind(&OnAudioSourceActivateHandle, weak_source, scene_collection));
    }
}

void SceneCollectionImpl::OnAudioSourceActivateHandle(obs_weak_source_t* weak_source,
                                                      SceneCollectionImpl* scene_collection)
{
    auto raw_source = obs_weak_source_get_source(weak_source);

    ON_SCOPE_EXIT {
        if (weak_source) {
            obs_weak_source_release(weak_source);
        }
        if (raw_source) {
            obs_source_release(raw_source);
        }
    };

    obs_proxy::VolumeController* volume_controller_ptr = scene_collection->GetVolumeController(raw_source);
    if (nullptr == volume_controller_ptr)
    {
        volume_controller_ptr = scene_collection->AddVolumeController(raw_source);
    }

    if (volume_controller_ptr)
    {
        scene_collection->OnAudioSourceActivated(volume_controller_ptr);
    }
}

void SceneCollectionImpl::OnSourceDeactivateHandlerRedirect(void* data, calldata_t* params)
{
    auto* raw_source = static_cast<obs_source_t*>(calldata_ptr(params, "source"));
    if (IsAudioSource(raw_source))
    {
        std::string source_name = obs_source_get_name(raw_source);
        auto scene_collection = static_cast<SceneCollectionImpl*>(data);

        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(&SceneCollectionImpl::OnAudioSourceDeactivate,
                       base::Unretained(scene_collection),
                       source_name));
    }
}

void SceneCollectionImpl::OnSourceRenameHandlerRedirect(void* data, calldata_t* params)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    std::string prev_name = calldata_string(params, "prev_name");
    std::string new_name = calldata_string(params, "new_name");
    auto scene_collection = static_cast<SceneCollectionImpl*>(data);

    if (scene_collection->source_rename_handler_.first)
    {
        scene_collection->source_rename_handler_.first(prev_name, new_name);
    }
}

void SceneCollectionImpl::OnSourceLoadHandlerRedirect(void* data, calldata_t* params)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    auto* raw_source = reinterpret_cast<obs_source_t*>(calldata_ptr(params, "source"));
    if (IsAudioSource(raw_source))
    {
        auto scene_collection = reinterpret_cast<SceneCollectionImpl*>(data);
        scene_collection->AddVolumeController(raw_source);
    }
}

void SceneCollectionImpl::OnCurrentSceneChanged(size_t index) const
{
    if (current_scene_changed_handler_)
    {
        current_scene_changed_handler_(index);
    }
}

void SceneCollectionImpl::OnSourceShow(const std::string& source_name) const
{
    // 构造函数中调用obs底层obs_load_sources的时候有可能触发StatusBar的消息开启Runloop，
    // 然后进一步触发Audio Channal source的source_show，此时SceneCollectionImpl由于这个
    // 意料之外的Runloop执行点还停在构造函数前部，current_scene_还未赋值，导致空指针引用
    if (!current_scene())
    {
        NOTREACHED() << "unexpected situation !";
        return;
    }

    // We care about only items that can be visualized in the current scene.
    // 场景切换时触发此回调，包括场景本身以及场景中的source，旧的活动场景和当前活动场景都触发
    auto scene_item = current_scene()->GetItem(source_name);
    if (scene_item && NeedItemVisualized(scene_item->type()) && source_show_handler_.first)
    {
        source_show_handler_.first(source_name);
    }
}

void SceneCollectionImpl::OnAudioSourceActivated(VolumeController* volume_controller)
{
    if (audio_source_activate_handler_.first)
    {
        audio_source_activate_handler_.first(volume_controller);
    }
}

void SceneCollectionImpl::OnAudioSourceDeactivate(const std::string& source_name) const
{
    if (audio_source_deactivate_handler_.first)
    {
        audio_source_deactivate_handler_.first(source_name);
    }
}

}   // namespace obs_proxy