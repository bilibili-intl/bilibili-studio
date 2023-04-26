#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"

#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_property_values_impl.h"

namespace obs_proxy {

// -*- PropertyValue implementation -*-

PropertyValue::PropertyValue()
    : impl_(std::make_unique<Impl>())
{}

PropertyValue::PropertyValue(PropertyValue&& other)
    : impl_(std::move(other.impl_))
{}

PropertyValue::~PropertyValue()
{}

void PropertyValue::RevertToDefault(const std::string& name)
{
    impl_->RevertToDefault(name);
}

void PropertyValue::Set(const std::string& name, bool value)
{
    impl_->Set(name, value);
}

void PropertyValue::Set(const std::string& name, double value)
{
    impl_->Set(name, value);
}

void PropertyValue::Set(const std::string& name, long long value)
{
    impl_->Set(name, value);
}

void PropertyValue::Set(const std::string& name, const char* value)
{
    impl_->Set(name, std::string(value));
}

void PropertyValue::Set(const std::string& name, const std::string& value)
{
    impl_->Set(name, value);
}

void PropertyValue::Set(const std::string& name, PropertyValue&& value)
{
    impl_->Set(name, std::move(value.impl_));
}

void PropertyValue::Set(const std::string& name, PropertyListValue&& value)
{
    impl_->Set(name, std::move(value.impl_));
}

bool PropertyValue::Get(const std::string& name, long long& value) const
{
    return GetWithFallback(name, value);
}

bool PropertyValue::Get(const std::string& name, bool& value) const
{
    return GetWithFallback(name, value);
}

bool PropertyValue::Get(const std::string& name, double& value) const
{
    return GetWithFallback(name, value);
}

bool PropertyValue::Get(const std::string& name, std::string& value) const
{
    return GetWithFallback(name, value);
}

bool PropertyValue::Get(const std::string& name, PropertyValue& value)
{
    if (impl_->HasKey(name))
    {
        value.impl_ = impl_->GetSubProperty(name);
    }
    else if (impl_->HasDefaultKey(name))
    {
        value.impl_ = impl_->GetDefaultSubProperty(name);
    }
    else
    {
        return false;
    }

    return true;
}

bool PropertyValue::Get(const std::string& name, PropertyListValue& value)
{
    if (impl_->HasKey(name))
    {
        value.impl_ = impl_->GetSubPropertyList(name);
    }
    else if (impl_->HasDefaultKey(name))
    {
        value.impl_ = impl_->GetDefaultSubPropertyList(name);
    }
    else
    {
        return false;
    }

    return true;
}

bool PropertyValue::HasKey(const std::string& name) const
{
    return impl_->HasKey(name) || impl_->HasDefaultKey(name);
}

void PropertyValue::Erase(const std::string& name)
{
    impl_->Erase(name);
}

PropertyValue PropertyValue::Spawn() const
{
    PropertyValue spawned;
    spawned.impl_ = impl_->Spawn();
    return spawned;
}

void PropertyValue::DupliateValues(const PropertyValue& other)
{
    impl_->DuplicateValues(other.impl_.get());
}

template<typename T>
bool PropertyValue::GetWithFallback(const std::string& name, T& value) const
{
    if (impl_->HasKey(name))
    {
        impl_->Get(name, value);
    }
    else if (impl_->HasDefaultKey(name))
    {
        impl_->GetDefault(name, value);
    }
    else
    {
        return false;
    }

    return true;
}

// -*- PropertyListValue implementation -*-

PropertyListValue::PropertyListValue()
    : impl_(std::make_unique<Impl>())
{}

PropertyListValue::PropertyListValue(PropertyListValue&& other)
    : impl_(std::move(other.impl_))
{}

PropertyListValue::~PropertyListValue()
{}

size_t PropertyListValue::size() const
{
    return impl_->size();
}

void PropertyListValue::Append(PropertyValue&& property_value)
{
    impl_->Append(std::move(property_value.impl_));
}

void PropertyListValue::Insert(size_t pos, PropertyValue&& property_value)
{
    impl_->Insert(pos, std::move(property_value.impl_));
}

bool PropertyListValue::Get(size_t pos, PropertyValue& property_value)
{
    auto&& value_impl = impl_->GetSubProperty(pos);
    if (!value_impl)
    {
        return false;
    }

    property_value.impl_ = std::move(value_impl);
    return true;
}

void PropertyListValue::Erase(size_t pos)
{
    impl_->Erase(pos);
}

}   // namespace obs_proxy