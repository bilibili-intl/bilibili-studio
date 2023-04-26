#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCE_PROPERTY_VALUES_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCE_PROPERTY_VALUES_IMPL_H_

#include "obs/obs-studio/libobs/obs-data.h"

#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"

namespace obs_proxy {

// Unfortunately, few implementations of source wrappers, like `SceneImpl`, demand access to underlying
// data of PropertyValue, which however, is not allowed for clients of `PropertyValue` on bililive layer.
// Thus, we have to do something dirty, to make this `impl` accessible to `SceneImpl`, while not being
// exposed on public.

class PropertyValue::Impl {
public:
    using value_type = obs_data_t;

    Impl();

    // Takes over the ownership of `value` itself.
    explicit Impl(value_type* value);

    ~Impl();

    DISABLE_COPY(Impl);

    void RevertToDefault(const std::string& name);

    void Set(const std::string& name, long long value);

    void Set(const std::string& name, double value);

    void Set(const std::string& name, bool value);

    void Set(const std::string& name, const std::string& value);

    void Set(const std::string& name, std::unique_ptr<Impl> value);

    void Set(const std::string& name, std::unique_ptr<PropertyListValue::Impl> value);

    void Get(const std::string& name, long long& value) const;

    void Get(const std::string& name, double& value) const;

    void Get(const std::string& name, bool& value) const;

    void Get(const std::string& name, std::string& value) const;

    std::unique_ptr<Impl> GetSubProperty(const std::string& name);

    std::unique_ptr<PropertyListValue::Impl> GetSubPropertyList(const std::string& name);

    void GetDefault(const std::string& name, long long& value) const;

    void GetDefault(const std::string& name, double& value) const;

    void GetDefault(const std::string& name, bool& value) const;

    void GetDefault(const std::string& name, std::string& value) const;

    std::unique_ptr<Impl> GetDefaultSubProperty(const std::string& name);

    std::unique_ptr<PropertyListValue::Impl> GetDefaultSubPropertyList(const std::string& name);

    bool HasKey(const std::string& name) const;

    bool HasDefaultKey(const std::string& name) const;

    void Erase(const std::string& name);

    std::unique_ptr<Impl> Spawn() const;

    void DuplicateValues(const Impl* other);

    // Avoid using friend on impl-level.
    value_type* LeakUnderlyingValue()
    {
        return value_;
    }

private:
    value_type* value_;
};

class PropertyListValue::Impl {
public:
    using value_type = obs_data_array_t;

    Impl();

    explicit Impl(value_type* value);

    ~Impl();

    size_t size() const;

    void Append(std::unique_ptr<PropertyValue::Impl> value);

    void Insert(size_t pos, std::unique_ptr<PropertyValue::Impl> value);

    std::unique_ptr<PropertyValue::Impl> GetSubProperty(size_t pos);

    void Erase(size_t pos);

    value_type* LeakUnderlyingValue()
    {
        return value_;
    }

private:
    value_type* value_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCE_PROPERTY_VALUES_IMPL_H_