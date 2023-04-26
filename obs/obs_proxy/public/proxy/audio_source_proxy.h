#ifndef OBS_OBS_PROXY_PUBLIC_AUDIO_SOURCE_PROXY_H_
#define OBS_OBS_PROXY_PUBLIC_AUDIO_SOURCE_PROXY_H_

#include "bilibase/basic_macros.h"

namespace obs_proxy {

class AudioDevice;
class SceneItem;

namespace internal {

enum class SourceType {
    AudioDevice,
    SceneItem,
    Empty
};

}   // namespace internal

// An audio source can either be an `AudioDevice` or a `SceneItem`.
// We use this proxy-delegate mainly to circumvent the restriction where return type does not participate in
// overload resolution.

class AudioSourceProxy {
public:
    AudioSourceProxy(AudioDevice* audio_device_source)
        : decayed_source_(audio_device_source),
          source_type_(internal::SourceType::AudioDevice)
    {}

    AudioSourceProxy(SceneItem* scene_item_source)
        : decayed_source_(scene_item_source),
          source_type_(internal::SourceType::SceneItem)
    {}

    AudioSourceProxy(nullptr_t)
        : decayed_source_(nullptr),
          source_type_(internal::SourceType::Empty)
    {}

    ~AudioSourceProxy() = default;

    DEFAULT_COPY(AudioSourceProxy);

    operator AudioDevice*() const
    {
        bool type_matched = source_type_ == internal::SourceType::AudioDevice ||
                            decayed_source_ == nullptr;
        return type_matched ? static_cast<AudioDevice*>(decayed_source_) : nullptr;
    }

    operator SceneItem*() const
    {
        bool type_matched = source_type_ == internal::SourceType::SceneItem ||
                            decayed_source_ == nullptr;
        return type_matched ? static_cast<SceneItem*>(decayed_source_) : nullptr;
    }

    operator bool() const
    {
        return decayed_source_ != nullptr;
    }

    internal::SourceType source_type() const
    {
        return source_type_;
    }

private:
    void* decayed_source_;
    internal::SourceType source_type_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_AUDIO_SOURCE_PROXY_H_