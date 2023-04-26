#include "obs/obs_proxy/core_proxy/scene_collection/obs_audio_devices_impl.h"

#include "base/logging.h"

#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_property_values_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"

#include <unordered_map>

// TODO(KC): AudioDeviceImpl shares a lot of implementation code with SceneItemImpl. Try to abstract them.

namespace {

using obs_proxy::AudioDeviceImpl;
using obs_proxy::FilterImpl;

void OnBoundFilterFound(obs_source_t*, obs_source_t* raw_filter, void* param)
{
    std::string filter_name = obs_source_get_name(raw_filter);
    auto filter_type = obs_proxy::FilterIDToType(obs_source_get_id(raw_filter));
    auto filter = std::make_unique<FilterImpl>(raw_filter, filter_type, filter_name);

    auto audio_device = static_cast<AudioDeviceImpl*>(param);
    audio_device->AddExistingFilter(std::move(filter));
}

}   // namespace

namespace obs_proxy {

const char* kDefaultInputAudio = kAuxAudio1;
const char* kDefaultOutputAudio = kDesktopAudio1;

AudioDeviceImpl::AudioDeviceImpl(obs_source_t* channel_source, uint32_t channel, const std::string& name,
                                 bool brand_new)
    : channel_audio_source_(channel_source),
      channel_(channel),
      name_(name),
      properties_(std::make_unique<PropertiesImpl>(channel_source))
{
    obs_data_t* audio_device_property_values = obs_source_get_settings(channel_source);
    property_values_.impl_ = std::make_unique<PropertyValue::Impl>(audio_device_property_values);

    if (!brand_new)
    {
        // Incorporates all filters that have already bound with the scene item.
        obs_source_enum_filters(channel_source, OnBoundFilterFound, this);
    }
}

AudioDeviceImpl::~AudioDeviceImpl()
{
    //obs_source_release(channel_audio_source_);
}

const std::string& AudioDeviceImpl::name() const
{
    return name_;
}

uint32_t AudioDeviceImpl::bound_channel_number() const
{
    return channel_;
}

Filter* AudioDeviceImpl::AddNewFilter(FilterType type, const std::string& name)
{
    Filter* filter_view = nullptr;
    auto* existing_filter = obs_source_get_filter_by_name(channel_audio_source_, name.c_str());
    if (existing_filter)
    {
        LOG(WARNING) << "Filter with name " << name << " already exists!";
        obs_source_release(existing_filter);
        return filter_view;
    }

    try
    {
        auto filter = std::make_unique<FilterImpl>(type, name);
        obs_source_filter_add(channel_audio_source_, filter->LeakUnderlyingSource());
        filter_view = filter.get();
        filters_.push_back(std::move(filter));
    }
    catch (const CreatingFilterError& ex)
    {
        LOG(WARNING) << ex.what();
        filter_view = nullptr;
    }

    return filter_view;
}

void AudioDeviceImpl::AddExistingFilter(std::unique_ptr<FilterImpl> filter)
{
    filters_.push_back(std::move(filter));
}

void AudioDeviceImpl::RemoveFilter(const std::string& name)
{
    // Dirty workaround that being compatible with the dark case where there are multiple
    // filters with the same name.
    auto it = std::partition(filters_.begin(), filters_.end(),
                             [&name](const std::unique_ptr<FilterImpl>& filter) {
                                 return filter->name() != name;
                             });

    DCHECK(std::distance(it, filters_.end()) <= 1);
    if (it == filters_.end())
    {
        return;
    }

    std::for_each(it, filters_.end(), [this](const std::unique_ptr<FilterImpl>& filter) {
                      obs_source_filter_remove(channel_audio_source_, filter->LeakUnderlyingSource());
                  });

    filters_.erase(it, filters_.end());
}

Filter* AudioDeviceImpl::GetFilter(const std::string& name) const
{
    auto it = std::find_if(filters_.cbegin(), filters_.cend(),
                           [&name](const std::unique_ptr<FilterImpl>& filter) {
                               return filter->name() == name;
                           });
    return it == filters_.cend() ? nullptr : it->get();
}

std::vector<Filter*> AudioDeviceImpl::GetFilters() const
{
    return AcquireSourceViews<FilterImpl, Filter>(filters_);
}

bool AudioDeviceImpl::ReorderFilters(const std::vector<Filter*>& filters)
{
    // copied from SceneItemImpl::ReorderFilters
    std::unordered_map<Filter*, size_t> new_filter_indices;
    for (size_t i = 0; i < filters.size(); ++i) {
        new_filter_indices.insert({ filters[i], i });
    }

    bool filters_matched = filters_.size() == new_filter_indices.size() &&
        std::all_of(filters_.cbegin(), filters_.cend(),
        [&new_filter_indices](const std::unique_ptr<FilterImpl>& filter) {
        return new_filter_indices.count(filter.get()) != 0;
    });
    if (!filters_matched) {
        NOTREACHED();
        return false;
    }

    // Using bubble-sort to reorder filters.
    bool swapped = false;
    for (size_t i = 0; i < filters_.size(); ++i) {
        for (size_t j = 0; j < filters_.size() - i - 1; ++j) {
            if (new_filter_indices[filters_[j].get()] > new_filter_indices[filters_[j + 1].get()]) {
                obs_source_filter_set_order(channel_audio_source_, filters_[j]->LeakUnderlyingSource(),
                    obs_order_movement::OBS_ORDER_MOVE_DOWN);
                using std::swap;
                swap(filters_[j], filters_[j + 1]);
                swapped = true;
            }
        }

        if (!swapped) {
            break;
        }

        swapped = false;
    }

    return true;
}

const Properties& AudioDeviceImpl::GetProperties() const
{
    return *properties_;
}

PropertyValue& AudioDeviceImpl::GetPropertyValues()
{
    return property_values_;
}

void AudioDeviceImpl::UpdatePropertyValues()
{
    auto new_property_values = property_values_.impl_->LeakUnderlyingValue();
    obs_source_update(channel_audio_source_, new_property_values);
}

void AudioDeviceImpl::StartPreview(bool disable_output)
{
    if (disable_output)
        obs_source_set_monitoring_type(channel_audio_source_, OBS_MONITORING_TYPE_MONITOR_ONLY);
    else
        obs_source_set_monitoring_type(channel_audio_source_, OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT);
}

void AudioDeviceImpl::StopPreview()
{
    obs_source_set_monitoring_type(channel_audio_source_, OBS_MONITORING_TYPE_NONE);
}

}   // namespace obs_proxy