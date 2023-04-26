#include "obs/obs_proxy/core_proxy/scene_collection/obs_filter_wrapper_impl.h"

#include "bilibase/error_exception_util.h"

#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/common/id_table_utils.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_property_values_impl.h"

namespace {

const char* const filter_type_id_table[] {
    obs_proxy::kFilterMaskID,
    obs_proxy::kFilterCropID,
    obs_proxy::kFilterGainID,
    obs_proxy::kFilterColorID,
    obs_proxy::kFilterScrollID,
    obs_proxy::kFilterColorKeyID,
    obs_proxy::kFilterSharpnessID,
    obs_proxy::kFilterChromaKeyID,
    obs_proxy::kFilterAsyncDelayID,
    obs_proxy::kFilterNoiseSuppressID,
    obs_proxy::kFilterBeautyID,
    obs_proxy::kFilterForceMonoID,
    obs_proxy::kFilterReverbID,
    obs_proxy::kFilterUnpremultiplyID,
    obs_proxy::kFilterFlipSettingID,
    obs_proxy::kFilterVtuberSettingsID,
    obs_proxy::kFilterMP4MotionID,
    obs_proxy::kFilterSingIdentifyID,
	obs_proxy::kFilterMicRecordID
};

}   // namespace

namespace obs_proxy {

FilterImpl::FilterImpl(FilterType type, const std::string& name)
    : type_(type), name_(name)
{
    filter_ = obs_source_create(FilterTypeToID(type), name.c_str(), nullptr, nullptr);
    ENSURE(filter_ != nullptr).Require<CreatingFilterError>();

    obs_data_t* filter_properties = obs_source_get_settings(filter_);
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(filter_properties);
}

FilterImpl::FilterImpl(obs_source_t* raw_filter, FilterType type, const std::string& name)
    : type_(type), name_(name), filter_(raw_filter)
{
    // Since, in our design, a `FilterImpl` instance shares ownership of underlying filter source
    // with its host, we have to increment the reference here. Because OBS doesn't employ this share
    // semantics, which would end up with a sole owner, i.e. the filter's host.
    obs_source_addref(raw_filter);
    obs_data_t* filter_properties = obs_source_get_settings(filter_);
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(filter_properties);
}

FilterImpl::~FilterImpl()
{
    // We don't care if this filter was attached on a source,
    // just leaving the work to the host.
    obs_source_release(filter_);
}

const std::string& FilterImpl::name() const
{
    return name_;
}

FilterType FilterImpl::type() const
{
    return type_;
}

PropertyValue& FilterImpl::GetPropertyValues()
{
    return property_values_;
}

void FilterImpl::UpdatePropertyValues()
{
    obs_source_update(filter_, property_values_.impl_->LeakUnderlyingValue());
}

const char* FilterTypeToID(FilterType type)
{
    return EnumValueToTableEntry(type, filter_type_id_table);
}

FilterType FilterIDToType(const char* id)
{
    return TableEntryToEnumValue<FilterType>(id, filter_type_id_table);
}

}   // namespace obs_proxy