#include "obs/obs_proxy/public/proxy/obs_source_properties.h"

#include "base/logging.h"

#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_properties_impl.h"

namespace obs_proxy {

class PropertyIterator::Impl {
public:
    explicit Impl(const PropertyImpl& prop);

    ~Impl();

    DEFAULT_COPY(Impl);

    const PropertyImpl& GetProperty() const;

    void SeekNextPropety();

private:
    PropertyImpl current_property_;
};

PropertyIterator::Impl::Impl(const PropertyImpl& prop)
    : current_property_(prop)
{}

PropertyIterator::Impl::~Impl()
{}

const PropertyImpl& PropertyIterator::Impl::GetProperty() const
{
    return current_property_;
}

void PropertyIterator::Impl::SeekNextPropety()
{
    current_property_ = current_property_.NextProperty();
}

// -*- PropertyIterator implementation -*-

PropertyIterator::PropertyIterator()
{}

template<typename P>
PropertyIterator::PropertyIterator(const P& prop)
    : impl_(std::make_shared<Impl>(prop))
{}

PropertyIterator::~PropertyIterator()
{}

PropertyIterator::reference PropertyIterator::operator*() const
{
    DCHECK(!IsEnd()) << "End iterator can not be dereferenced";
    return impl_->GetProperty();
}

PropertyIterator::pointer PropertyIterator::operator->() const
{
    DCHECK(!IsEnd()) << "End iterator can not be dereferenced";
    return &impl_->GetProperty();
}

PropertyIterator& PropertyIterator::operator++()
{
    DCHECK(!IsEnd()) << "End iterator is not incrementable";
    impl_->SeekNextPropety();
    return *this;
}

PropertyIterator PropertyIterator::operator++(int)
{
    PropertyIterator cpy = *this;
    ++(*this);
    return cpy;
}

bool PropertyIterator::IsEnd() const
{
    return !impl_ || !impl_->GetProperty();
}

bool operator==(const PropertyIterator& lhs, const PropertyIterator& rhs)
{
    return (lhs.impl_ == rhs.impl_) ||
           (lhs.IsEnd() && rhs.IsEnd()) ||
           (!lhs.IsEnd() && !rhs.IsEnd() && (lhs.impl_->GetProperty() == rhs.impl_->GetProperty()));
}

bool operator!=(const PropertyIterator& lhs, const PropertyIterator& rhs)
{
    return !(lhs == rhs);
}

// Tricks to circumvent no-RTTI-available problem.

template PropertyIterator::PropertyIterator(const PropertyImpl&);

template<>
std::unique_ptr<BoolProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<BoolPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<ButtonProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<ButtonPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<ColorProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<ColorPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<IntProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<IntPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<FloatProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<FloatPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<TextProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<TextPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<PathProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<PathPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<ListProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<ListPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<FontProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<FontPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<EditableListProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<EditableListPropertyImpl>(raw_prop);
}

template<>
std::unique_ptr<FrameRateProperty> property_cast(const Property& base_prop)
{
    auto raw_prop = static_cast<obs_property_t*>(base_prop.EvilLeakUnderlying());
    return std::make_unique<FrameRatePropertyImpl>(raw_prop);
}

}   // namespace obs_proxy