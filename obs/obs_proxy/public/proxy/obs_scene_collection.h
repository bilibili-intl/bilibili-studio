#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SCENE_COLLECTION_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SCENE_COLLECTION_H_

#include "obs/obs_proxy/public/proxy/audio_source_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

namespace obs_proxy {

class SceneCollection {
public:
    virtual ~SceneCollection() {}

    virtual const std::string& name() const = 0;

    virtual void set_name(const std::string& new_name) = 0;

    // Adds a new scene with the given name, and returns a non-owning pointer to it.
    // Returns the null pointer if not succeed.
    virtual Scene* AddNewScene(const std::string& name, bool custom_size, int cx, int cy) = 0;

    virtual bool RemoveScene(const std::string& name) = 0;

    virtual Scene* current_scene() const = 0;

    // Returns a non-owning pointer to the specific scene;
    // Returns the null pointer if no such scene was found.
    virtual Scene* GetScene(const std::string& name) const = 0;

    virtual std::vector<Scene*> GetScenes() const = 0;

    virtual AudioDevice* GetAudioDevice(const std::string& name) const = 0;

    virtual std::vector<AudioDevice*> GetAudioDevices() const = 0;

    virtual std::vector<VolumeController*> GetVolumeControllers() const = 0;

    virtual VolumeController* GetVolumeControllerForAudioSource(AudioSourceProxy audio_source) const = 0;

    virtual AudioSourceProxy GetAudioSourceForVolumeController(VolumeController* volume_controller) const = 0;

    // Transits to a given scene, and target scene will become the current scene.
    // Please ensure there is a scene that matches the given name.
    virtual Scene* TransitToScene(const std::string& name) = 0;

    virtual uint32_t transition_duration() const = 0;

    virtual void set_transition_duration(uint32_t duration) = 0;

    // Writes data of current scene collection onto the disk.
    virtual void Save() const = 0;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SCENE_COLLECTION_H_