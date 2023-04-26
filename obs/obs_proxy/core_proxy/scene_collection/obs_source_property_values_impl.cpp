#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_property_values_impl.h"

#include "base/logging.h"

namespace obs_proxy {

PropertyValue::Impl::Impl()
    : value_(obs_data_create())
{}

PropertyValue::Impl::Impl(value_type* value)
    : value_(value)
{}

PropertyValue::Impl::~Impl()
{
    obs_data_release(value_);
}

void PropertyValue::Impl::RevertToDefault(const std::string& name)
{
    obs_data_unset_user_value(value_, name.c_str());
}

void PropertyValue::Impl::Set(const std::string& name, long long value)
{
    obs_data_set_int(value_, name.c_str(), value);
}

void PropertyValue::Impl::Set(const std::string& name, double value)
{
    obs_data_set_double(value_, name.c_str(), value);
}

void PropertyValue::Impl::Set(const std::string& name, bool value)
{
    obs_data_set_bool(value_, name.c_str(), value);
}

void PropertyValue::Impl::Set(const std::string& name, const std::string& value)
{
    obs_data_set_string(value_, name.c_str(), value.c_str());
}

void PropertyValue::Impl::Set(const std::string& name, std::unique_ptr<Impl> value)
{
    obs_data_set_obj(value_, name.c_str(), value->LeakUnderlyingValue());
}

void PropertyValue::Impl::Set(const std::string& name,
                              std::unique_ptr<PropertyListValue::Impl> value)
{
    obs_data_set_array(value_, name.c_str(), value->LeakUnderlyingValue());
}

void PropertyValue::Impl::Get(const std::string& name, long long& value) const
{
    value = obs_data_get_int(value_, name.c_str());
}

void PropertyValue::Impl::Get(const std::string& name, double& value) const
{
    value = obs_data_get_double(value_, name.c_str());
}

void PropertyValue::Impl::Get(const std::string& name, bool& value) const
{
    value = obs_data_get_bool(value_, name.c_str());
}

void PropertyValue::Impl::Get(const std::string& name, std::string& value) const
{
    const char* str = obs_data_get_string(value_, name.c_str());
    value = str;
}

std::unique_ptr<PropertyValue::Impl> PropertyValue::Impl::GetSubProperty(const std::string& name)
{
    auto* sub_value = obs_data_get_obj(value_, name.c_str());
    DCHECK(sub_value != nullptr);
    return std::make_unique<Impl>(sub_value);
}

std::unique_ptr<PropertyListValue::Impl>
    PropertyValue::Impl::GetSubPropertyList(const std::string& name)
{
    auto* sub_list_value = obs_data_get_array(value_, name.c_str());
    DCHECK(sub_list_value != nullptr);
    return std::make_unique<PropertyListValue::Impl>(sub_list_value);
}

void PropertyValue::Impl::GetDefault(const std::string& name, long long& value) const
{
    value = obs_data_get_default_int(value_, name.c_str());
}

void PropertyValue::Impl::GetDefault(const std::string& name, double& value) const
{
    value = obs_data_get_default_double(value_, name.c_str());
}

void PropertyValue::Impl::GetDefault(const std::string& name, bool& value) const
{
    value = obs_data_get_default_bool(value_, name.c_str());
}

void PropertyValue::Impl::GetDefault(const std::string& name, std::string& value) const
{
    const char* str = obs_data_get_default_string(value_, name.c_str());
    value = str;
}

std::unique_ptr<PropertyValue::Impl>
    PropertyValue::Impl::GetDefaultSubProperty(const std::string& name)
{
    auto* sub_value = obs_data_get_default_obj(value_, name.c_str());
    DCHECK(sub_value != nullptr);
    return std::make_unique<Impl>(sub_value);
}

std::unique_ptr<PropertyListValue::Impl>
    PropertyValue::Impl::GetDefaultSubPropertyList(const std::string& name)
{
    auto* sub_list_value = obs_data_get_default_array(value_, name.c_str());
    DCHECK(sub_list_value != nullptr);
    return std::make_unique<PropertyListValue::Impl>(sub_list_value);
}

bool PropertyValue::Impl::HasKey(const std::string& name) const
{
    return obs_data_has_user_value(value_, name.c_str());
}

bool PropertyValue::Impl::HasDefaultKey(const std::string& name) const
{
    return obs_data_has_default_value(value_, name.c_str());
}

void PropertyValue::Impl::Erase(const std::string& name)
{
    obs_data_erase(value_, name.c_str());
}

std::unique_ptr<PropertyValue::Impl> PropertyValue::Impl::Spawn() const
{
    auto value = obs_data_create();
    obs_data_apply(value, value_);
    return std::make_unique<Impl>(value);
}

void PropertyValue::Impl::DuplicateValues(const Impl* other)
{
    obs_data_clear(value_);
    obs_data_apply(value_, other->value_);
}

// -*- PropertyListValue::Impl implementation -*-

PropertyListValue::Impl::Impl()
    : value_(obs_data_array_create())
{}

PropertyListValue::Impl::Impl(value_type* value)
    : value_(value)
{}

PropertyListValue::Impl::~Impl()
{
    obs_data_array_release(value_);
}

size_t PropertyListValue::Impl::size() const
{
    return obs_data_array_count(value_);
}

void PropertyListValue::Impl::Append(std::unique_ptr<PropertyValue::Impl> value)
{
    obs_data_array_push_back(value_, value->LeakUnderlyingValue());
}

void PropertyListValue::Impl::Insert(size_t pos, std::unique_ptr<PropertyValue::Impl> value)
{
    obs_data_array_insert(value_, pos, value->LeakUnderlyingValue());
}

std::unique_ptr<PropertyValue::Impl> PropertyListValue::Impl::GetSubProperty(size_t pos)
{
    auto sub_value = obs_data_array_item(value_, pos);
    if (!sub_value)
    {
        return nullptr;
    }

    return std::make_unique<PropertyValue::Impl>(sub_value);
}

void PropertyListValue::Impl::Erase(size_t pos)
{
    obs_data_array_erase(value_, pos);
}

}   // namespace obs_proxy