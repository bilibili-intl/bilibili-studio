#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_properties_impl.h"

#include "base/logging.h"

#include "bilibase/basic_types.h"

#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

namespace {

using obs_proxy::PropertyType;

}   // namespace

namespace obs_proxy {

PropertyImpl::PropertyImpl(obs_property_t* prop)
    : property_(prop)
{}

// obs_property_t is owned by its associated obs_properties_t, there is nothing we need to do.
PropertyImpl::~PropertyImpl()
{}

std::string PropertyImpl::name() const
{
    return bilibase::safe_c_str_cast(obs_property_name(property_));
}

PropertyType PropertyImpl::type() const
{
    auto type_value = obs_property_get_type(property_);
    auto value_min = bilibase::enum_cast(PropertyType::Invalid);
    auto value_max = bilibase::enum_cast(PropertyType::FrameRate);
    DCHECK(value_min <= type_value && type_value <= value_max);
    return PropertyType(type_value);
}

std::string PropertyImpl::description() const
{
    return bilibase::safe_c_str_cast(obs_property_description(property_));
}

obs_property_t* PropertyImpl::LeakUnderlying() const
{
    return property_;
}

void* PropertyImpl::EvilLeakUnderlying() const
{
    return property_;
}

PropertyImpl PropertyImpl::NextProperty() const
{
    obs_property_t* next_prop = property_;
    obs_property_next(&next_prop);
    return PropertyImpl(next_prop);
}

PropertyImpl::operator bool() const
{
    return property_ != nullptr;
}

bool operator==(const PropertyImpl& lhs, const PropertyImpl& rhs)
{
    return lhs.LeakUnderlying() == rhs.LeakUnderlying();
}

bool operator!=(const PropertyImpl& lhs, const PropertyImpl& rhs)
{
    return !(lhs == rhs);
}

// -*- PropertiesImpl implementations -*-

PropertiesImpl::PropertiesImpl(const obs_source_t* source)
    : bound_source_view_(source)
{
    properties_ = obs_source_properties(source);
}

PropertiesImpl::~PropertiesImpl()
{
    obs_properties_destroy(properties_);
}

Properties::iterator PropertiesImpl::begin() const
{
    auto first_property = obs_properties_first(properties_);
    return iterator(PropertyImpl(first_property));
}

Properties::iterator PropertiesImpl::end() const
{
    return iterator();
}

Properties::iterator PropertiesImpl::find(const std::string& name) const
{
    return std::find_if(begin(), end(), [&name](const Property& prop) {
                            return name == prop.name();
                        });
}

void PropertiesImpl::Refresh()
{
    obs_properties_destroy(properties_);
    properties_ = obs_source_properties(bound_source_view_);
}

// -*- BoolPropertyImpl implementations -*-

BoolPropertyImpl::BoolPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

BoolPropertyImpl::~BoolPropertyImpl()
{}

// -*- ButtonPropertyImpl implementations -*-

ButtonPropertyImpl::ButtonPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

ButtonPropertyImpl::~ButtonPropertyImpl()
{}

bool ButtonPropertyImpl::Click(SceneItem* item)
{
    auto context = impl_cast(item)->AsSource();
    return obs_property_button_clicked(LeakUnderlying(), context);
}

// -*- ColorPropertyImpl implementations -*-

ColorPropertyImpl::ColorPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

ColorPropertyImpl::~ColorPropertyImpl()
{}

// -*- EditableListPropertyImpl implementations -*-

EditableListPropertyImpl::EditableListPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

EditableListPropertyImpl::~EditableListPropertyImpl()
{}

// -*- IntPropertyImpl implementations -*-

IntPropertyImpl::IntPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

IntPropertyImpl::~IntPropertyImpl()
{}

IntProperty::NumberType IntPropertyImpl::int_type() const
{
    return NumberType(obs_property_int_type(LeakUnderlying()));
}

int IntPropertyImpl::int_min() const
{
    return obs_property_int_min(LeakUnderlying());
}

int IntPropertyImpl::int_max() const
{
    return obs_property_int_max(LeakUnderlying());
}

int IntPropertyImpl::int_step() const
{
    return obs_property_int_step(LeakUnderlying());
}

// -*- FloatPropertyImpl implementations -*-

FloatPropertyImpl::FloatPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

FloatPropertyImpl::~FloatPropertyImpl()
{}

FloatProperty::NumberType FloatPropertyImpl::float_type() const
{
    return NumberType(obs_property_float_type(LeakUnderlying()));
}

double FloatPropertyImpl::float_min() const
{
    return obs_property_float_min(LeakUnderlying());
}

double FloatPropertyImpl::float_max() const
{
    return obs_property_float_max(LeakUnderlying());
}

double FloatPropertyImpl::float_step() const
{
    return obs_property_float_step(LeakUnderlying());
}

// -*- TextPropertyImpl implementations -*_

TextPropertyImpl::TextPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

TextPropertyImpl::~TextPropertyImpl()
{}

TextProperty::TextType TextPropertyImpl::text_type() const
{
    return TextType(obs_property_text_type(LeakUnderlying()));
}

// -*- PathPropertyImpl implementations -*-

PathPropertyImpl::PathPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

PathPropertyImpl::~PathPropertyImpl()
{}

PathProperty::PathType PathPropertyImpl::path_type() const
{
    return PathType(obs_property_path_type(LeakUnderlying()));
}

// -*- ListPropertyImpl implementations -*-

ListPropertyImpl::ListPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

ListPropertyImpl::~ListPropertyImpl()
{}

ListProperty::ComboFormat ListPropertyImpl::list_format() const
{
    auto format_value = obs_property_list_format(LeakUnderlying());
    auto value_min = bilibase::enum_cast(ComboFormat::INVALID);
    auto value_max = bilibase::enum_cast(ComboFormat::STRING);
    DCHECK(value_min <= format_value && format_value <= value_max);
    return ComboFormat(format_value);
}

ListProperty::ComboType ListPropertyImpl::list_type() const
{
    auto type_value = obs_property_list_type(LeakUnderlying());
    auto value_min = bilibase::enum_cast(ComboType::INVALID);
    auto value_max = bilibase::enum_cast(ComboType::LIST);
    DCHECK(value_min <= type_value && type_value <= value_max);
    return ComboType(type_value);
}

size_t ListPropertyImpl::GetItemCount() const
{
    return obs_property_list_item_count(LeakUnderlying());
}

std::string ListPropertyImpl::GetItemName(size_t index) const
{
    DCHECK(index < GetItemCount());
    return bilibase::safe_c_str_cast(obs_property_list_item_name(LeakUnderlying(), index));
}

long long ListPropertyImpl::GetItemValueAsInt(size_t index) const
{
    DCHECK(index < GetItemCount());
    return obs_property_list_item_int(LeakUnderlying(), index);
}

double ListPropertyImpl::GetItemValueAsFloat(size_t index) const
{
    DCHECK(index < GetItemCount());
    return obs_property_list_item_float(LeakUnderlying(), index);
}

std::string ListPropertyImpl::GetItemValueAsString(size_t index) const
{
    DCHECK(index < GetItemCount());
    return bilibase::safe_c_str_cast(obs_property_list_item_string(LeakUnderlying(), index));
}

bool ListPropertyImpl::GetItemEnableStatus(size_t index) const
{
	DCHECK(index < GetItemCount());
	return !obs_property_list_item_disabled(LeakUnderlying(), index);
}

// -*- FontPropertyImpl implementations -*-

FontPropertyImpl::FontPropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

FontPropertyImpl::~FontPropertyImpl()
{}

// -*- FrameRatePropertyImpl implementations -*-

FrameRatePropertyImpl::FrameRatePropertyImpl(obs_property_t* prop)
    : PropertyImpl(prop)
{}

FrameRatePropertyImpl::~FrameRatePropertyImpl()
{}

size_t FrameRatePropertyImpl::fps_ranges_count() const
{
    return obs_property_frame_rate_fps_ranges_count(LeakUnderlying());
}

FrameRateProperty::MediaFramesPerSecond FrameRatePropertyImpl::fps_range_min(size_t index) const
{
    DCHECK(index < fps_ranges_count());
    auto min_value = obs_property_frame_rate_fps_range_min(LeakUnderlying(), index);
    return { min_value.numerator, min_value.denominator };
}

FrameRateProperty::MediaFramesPerSecond FrameRatePropertyImpl::fps_range_max(size_t index) const
{
    DCHECK(index < fps_ranges_count());
    auto max_value = obs_property_frame_rate_fps_range_max(LeakUnderlying(), index);
    return { max_value.numerator, max_value.denominator };
}

size_t FrameRatePropertyImpl::options_count() const
{
    return obs_property_frame_rate_options_count(LeakUnderlying());
}

std::string FrameRatePropertyImpl::GetOptionName(size_t index) const
{
    DCHECK(index < options_count());
    return bilibase::safe_c_str_cast(obs_property_frame_rate_option_name(LeakUnderlying(), index));
}

std::string FrameRatePropertyImpl::GetOptionDescription(size_t index) const
{
    DCHECK(index < options_count());
    return bilibase::safe_c_str_cast(obs_property_frame_rate_option_description(LeakUnderlying(), index));
}

}   // namespace obs_proxy