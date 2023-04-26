#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SCENE_COLLECTION_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SCENE_COLLECTION_IMPL_H_

#include <functional>
#include <memory>

#include "base/files/file_path.h"

#include "bilibase/basic_macros.h"

#include "obs/obs_proxy/core_proxy/common/basic_types.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_audio_devices_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_volume_controller_impl.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

namespace obs_proxy {

class SceneCollectionImpl : public SceneCollection {
public:
    using CurrentSceneChangedHandler = std::function<void(size_t index)>;
    using SourceShowHandler = std::function<void(const std::string& item_name)>;
    using AudioSourceActivateHandler = std::function<void(VolumeController* audio_device)>;
    using AudioSourceDeactivateHandler = std::function<void(const std::string& name)>;
    using SourceRenameHandler = std::function<void(const std::string& prev_name, const std::string& new_name)>;

    ~SceneCollectionImpl();

    DISABLE_COPY(SceneCollectionImpl);

    static std::unique_ptr<SceneCollectionImpl> FromCollectionFile(const base::FilePath& collection_path);

    static std::unique_ptr<SceneCollectionImpl> ForDefault(const base::FilePath& collection_save_path);

    void set_name(const std::string& new_name) override;

    const std::string& name() const override;

    Scene* AddNewScene(const std::string& name, bool custom_size, int cx, int cy) override;

    void AddExistingScene(std::unique_ptr<SceneImpl> scene);

    bool RemoveScene(const std::string& name) override;

    Scene* current_scene() const override;

    // This function may be used frequently internally, make it covariant return type.
    SceneImpl* GetScene(const std::string& name) const override;

    std::vector<Scene*> GetScenes() const override;

    std::vector<VolumeController*> GetVolumeControllers() const override;

    void AddAudioDevice(std::unique_ptr<AudioDeviceImpl> audio_device);

    AudioDeviceImpl* GetAudioDevice(const std::string& name) const override;

    std::vector<AudioDevice*> GetAudioDevices() const override;

    VolumeController* GetVolumeControllerForAudioSource(AudioSourceProxy audio_source) const override;

    AudioSourceProxy GetAudioSourceForVolumeController(VolumeController* volume_controller) const override;

    Scene* TransitToScene(const std::string& name) override;

    uint32_t transition_duration() const override;

    void set_transition_duration(uint32_t duration) override;

    void Save() const override;

    void Cleanup();

    void RegisterCurrentSceneChangedHandler(CurrentSceneChangedHandler handler);

    void RegisterSourceShowHandler(SourceShowHandler handler);

    void RegisterAudioSourceActivateHandler(AudioSourceActivateHandler handler);

    void RegisterAudioSourceDeactivateHandler(AudioSourceDeactivateHandler handler);

    void RegisterSourceRenameHandler(SourceRenameHandler handler);

    void SceneItemRemove(obs_source_t* underlying_source);

private:
    static void OnSourceShowHandlerRedirect(void* data, calldata_t* params);

    static void OnSourceActivateHandlerRedirect(void* data, calldata_t* params);

    static void OnSourceDeactivateHandlerRedirect(void* data, calldata_t* params);

    static void OnSourceRenameHandlerRedirect(void* data, calldata_t* params);

    static void OnSourceLoadHandlerRedirect(void* data, calldata_t* params);

    static void OnAudioSourceActivateHandle(obs_weak_source_t* raw_source,
                                          SceneCollectionImpl* scene_collection);

    static void OnTransitionStop(void* data, calldata_t* cb_data);

    void OnCurrentSceneChanged(size_t index) const;
    void OnAudioSourceActivated(VolumeController* volume_controller);
    void OnAudioSourceDeactivate(const std::string& device_name) const;
    void OnSourceShow(const std::string& source_name) const;

    void AddGameSource(obs_proxy::SceneImpl* joinmic_scene);
    bool AddGameDlgSource();

    // Creates a scene collection from data.
    SceneCollectionImpl(const base::FilePath& path, const std::string& name, obs_data_t* collection_data);

    // Creates a brand new scene collection instance.
    SceneCollectionImpl(const base::FilePath& path, const std::string& name);

    void PostInitialize(bool from_file);

    void InitDefaultTransitions();

    void ResetAudioDevice(const std::string& audio_source_id, const std::string& device_id,
                          const std::string& name, uint32_t channel);

    void SetupDefaultAudioDevices();

    // Try to load audio source from collection data.
    // Each time before loading, we verify the unique device id of the attached device and the one
    // recorded in the collection data; and we load from source only if these two device ids match.
    // Also be noted that, one same device which is attached on different USB port will be granted
    // different device id.
    void TryLoadAudioDevice(const std::string& device_name, const std::string& audio_source_id, int channel,
                            obs_data_t* collection_data);

    VolumeController* GetVolumeController(obs_source_t* bound_audio_source) const;

    void SetCurrentScene(SceneImpl* scene, bool force);

    // We need source activity signals to control life-time of audio source devices.
    // OBS core has a speical thread for rendering visualizable sources, and before rendering completes,
    // there is no way to obtain their visual properties(width and height for example).
    // Therefore, we need a signal/callback when obs has finished rendering sources; and that is why
    // this function matters.
    void BindObsSignals();

    void BindTransitionSignalHandler(Transition* transition);

    std::vector<Transition*> GetTransitions() const;

    // The owner of the data will be transferred to the caller.
    obs_data_t* GenerateCollectionData() const;

    obs_proxy::VolumeController* AddVolumeController(obs_source_t* raw_source);
    void AddVolumeController(std::unique_ptr<VolumeControllerImpl> volume_controller);

    void RemoveVolumeController(obs_source_t* bound_audio_source);

    void DismissSignalHandlers();

private:
    base::FilePath collection_path_;
    std::string name_;
    std::vector<std::unique_ptr<AudioDeviceImpl>> audio_devices_;
    std::vector<std::unique_ptr<SceneImpl>> scenes_;
    std::vector<std::unique_ptr<VolumeControllerImpl>> volume_controllers_;
    std::vector<std::unique_ptr<Transition>> transitions_;
    SceneImpl* current_scene_;
    Transition* fade_transition_;
    uint32_t transition_duration_;

    CurrentSceneChangedHandler current_scene_changed_handler_;
    OBSSignal source_load_handler_;
    SignalHandler<AudioSourceActivateHandler> audio_source_activate_handler_;
    SignalHandler<AudioSourceDeactivateHandler> audio_source_deactivate_handler_;
    SignalHandler<SourceShowHandler> source_show_handler_;
    SignalHandler<SourceRenameHandler> source_rename_handler_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SCENE_COLLECTION_IMPL_H_