#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCE_PROPERTY_VALUES_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCE_PROPERTY_VALUES_H_

#include <string>
#include <memory>

#include "bilibase/basic_macros.h"

#include "obs/obs_proxy/app/obs_proxy_export.h"

namespace obs_proxy {

// `PropertyValue` and `PropertyListValue` are proxy level wrapper for `obs_data_t` and
// `obs_data_array_t`, but unlike those obs data types, they are specific and are used as
// properties of sources only.

class PropertyListValue;

class OBS_PROXY_EXPORT PropertyValue {
public:
    PropertyValue();

    PropertyValue(PropertyValue&& other);

    ~PropertyValue();

    PropertyValue& operator=(PropertyValue&&) = delete;

    DISABLE_COPY(PropertyValue);

    void RevertToDefault(const std::string& name);

    void Set(const std::string& name, long long value);

    void Set(const std::string& name, double value);

    void Set(const std::string& name, bool value);

    // Added to avoid overload resolution caveat where bool is preferred over std::string.
    void Set(const std::string& name, const char* value);

    void Set(const std::string& name, const std::string& value);

    // Takes over the ownership of `value`.
    void Set(const std::string& name, PropertyValue&& value);

    // Takes over the ownership of `value`.
    void Set(const std::string& name, PropertyListValue&& value);

    // All `Get*` functions will return user value if user value was present; and they will
    // return default value, if default value was present, and yet user value was not present.
    // All `Get*` functions won't touch `value` if the retrieve was failed.

    bool Get(const std::string& name, long long& value) const;

    bool Get(const std::string& name, bool& value) const;

    bool Get(const std::string& name, double& value) const;

    bool Get(const std::string& name, std::string& value) const;

    // Functions that retrieve sub-property or sub-property-list are not const, because the caller
    // may need an approach to modify an item deeply nested inside, without cloning the entire one.
    // Therefore, please make sure these special sub values have shorter lifetime than their parent.

    bool Get(const std::string& name, PropertyValue& value);

    bool Get(const std::string& name, PropertyListValue& value);

    // Returns false if neither user value nor default value is present;
    // Returns true, otherwise.
    bool HasKey(const std::string& name) const;

    void Erase(const std::string& name);

    // Returns a copy of the current property value.
    // The copy duplicates almost all values, except predefined default values.
    PropertyValue Spawn() const;

    // Duplicates values of another PropertyValue.
    // Be noted that, only user-defined values are duplicated.
    void DupliateValues(const PropertyValue& other);

private:
    template<typename T>
    bool GetWithFallback(const std::string& name, T& value) const;

private:
    friend class PropertyListValue;
    // Be careful of these evil friends...
    friend class AudioDeviceImpl;
    friend class FilterImpl;
    friend class SceneItemImpl;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class OBS_PROXY_EXPORT PropertyListValue {
public:
    PropertyListValue();

    PropertyListValue(PropertyListValue&& other);

    ~PropertyListValue();

    PropertyListValue& operator=(PropertyListValue&&) = delete;

    DISABLE_COPY(PropertyListValue);

    size_t size() const;

    void Append(PropertyValue&& property_value);

    void Insert(size_t pos, PropertyValue&& property_value);

    bool Get(size_t pos, PropertyValue& property_value);

    void Erase(size_t pos);

private:
    friend class PropertyValue;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCE_PROPERTY_VALUES_H_