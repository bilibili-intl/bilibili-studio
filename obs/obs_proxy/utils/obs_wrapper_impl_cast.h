#ifndef OBS_OBS_PROXY_UTILS_OBS_WRAPPER_IMPL_CAST_H_
#define OBS_OBS_PROXY_UTILS_OBS_WRAPPER_IMPL_CAST_H_

#include "obs/obs_proxy/core_proxy/scene_collection/obs_audio_devices_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_filter_wrapper_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_scene_collection_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_volume_controller_impl.h"

namespace obs_proxy {

namespace internal {

template<typename T>
class TypeKeeper {
private:
    // ReSharper disable CppFunctionIsNotImplemented
    static SceneCollectionImpl Map(SceneCollection*);
    static SceneImpl Map(Scene*);
    static SceneItemImpl Map(SceneItem*);
    static FilterImpl Map(Filter*);
    static VolumeControllerImpl Map(VolumeController*);
    static AudioDeviceImpl Map(AudioDevice*);
    // ReSharper restore CppFunctionIsNotImplemented

    using item_type = decltype(Map(static_cast<std::remove_const_t<T>*>(nullptr)));

public:
    using impl_type = std::conditional_t<std::is_const<T>::value,
                                         std::add_const_t<item_type>, item_type>;
};

}   // namespace internal

// Some components in proxy layer (e.g. preview part) may need to access elements of a scene
// collection in terms of their real implementation type, to gain more controls; and yet
// with only interface type available. This auxiliary function provides a way to do this cast
// while avoiding leaking type details.
// It is also a compile-time cast, thus no runtime overhead is incurred.
template<typename I>
typename internal::TypeKeeper<I>::impl_type* impl_cast(I* interface_ptr)
{
    return static_cast<typename internal::TypeKeeper<I>::impl_type*>(interface_ptr);
}

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_UTILS_OBS_WRAPPER_IMPL_CAST_H_