#include <obs-module.h>
#include <util/platform.h>
#include <util/base.h>
#include <atomic>
#include "colive-service.h"
#include "avdata-util.h"

using namespace bililive::colive;

class ColiveSourceImpl : public IColiveMedia::OnRemoteMedia
{
public:
    using Ptr = std::shared_ptr<ColiveSourceImpl>;

    void RegisterCallback();

    void OnVideo(uint64_t uid, IColiveMedia::VFrame* vf, bool is_little_video) override;
    void OnAudio(uint64_t uid, IColiveMedia::AFrame* af) override;

    obs_source_t *source_;
    uint64_t uid_;
    ColiveMediaWPtr colive_service_;

    const int64_t TIMESTAMP_INVALID;
    std::atomic_int64_t timestamp_ns_offset_;
    ColiveSourceImpl()
        : source_(nullptr)
        , uid_(0)
        , TIMESTAMP_INVALID(std::numeric_limits<int64_t>::min())
        , timestamp_ns_offset_(TIMESTAMP_INVALID)
    {
    }
};

class ColiveSource
{
public:
    ColiveSourceImpl::Ptr impl;
};

static void colive_source_defaults(obs_data_t *settings)
{

}

static obs_properties_t *colive_source_getproperties(void *data)
{
    ColiveSource* s = (ColiveSource*)data;
    obs_properties_t *props = obs_properties_create();

    return props;
}

static void colive_source_update(void *data, obs_data_t *settings)
{
    ColiveSource* s = (ColiveSource*)data;
}

static const char *colive_source_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("ColiveSource");
}

static void *colive_source_create(obs_data_t *settings, obs_source_t *source)
{
    ColiveMediaWPtr p = GetColiveServiceFactory()->AquireMedia();
    if (p.expired())
    {
        blog(LOG_WARNING, "%s", "[colive source] fail to create before colive service.");
        return nullptr;
    }

    if (!settings)
    {
        blog(LOG_WARNING, "%s", "[colive source] fail create with settings of nullptr.");
        return nullptr;
    }

    if (!obs_data_has_user_value(settings, "uid"))
    {
        blog(LOG_WARNING, "%s", "[colive source] fail create with no uid in settings .");
        return nullptr;
    }

    ColiveSourceImpl::Ptr impl(new ColiveSourceImpl());
    impl->source_ = source;
    impl->colive_service_ = p;
    impl->uid_ = (uint64_t)obs_data_get_int(settings, "uid");
    obs_source_set_async_unbuffered(source, true);

    ColiveSource* s = new ColiveSource();
    s->impl = impl;

    impl->RegisterCallback();

    return s;
}

static void colive_source_destroy(void *data)
{
    if (data)
    {
        ColiveSource* s = (ColiveSource*)data;
        delete s;
    }
}

static void colive_source_activate(void *data)
{
    ColiveSource* s = (ColiveSource*)data;
}

static void colive_source_deactivate(void *data)
{
    ColiveSource* s = (ColiveSource*)data;
}


static void colive_source_tick(void *data, float seconds)
{
    ColiveSource* s = (ColiveSource*)data;
}

static const char *voice_link_source_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("VoiceLinkSource");
}

void register_colive_source()
{
    struct obs_source_info ColiveSource = { 0 };
    ColiveSource.id = "colive_source";
    ColiveSource.type = OBS_SOURCE_TYPE_INPUT;
    ColiveSource.output_flags = OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_AUDIO |
        OBS_SOURCE_DO_NOT_DUPLICATE;
    ColiveSource.get_name = colive_source_getname;
    ColiveSource.create = colive_source_create;
    ColiveSource.destroy = colive_source_destroy;
    ColiveSource.get_defaults = colive_source_defaults;
    ColiveSource.get_properties = colive_source_getproperties;
    ColiveSource.activate = colive_source_activate;
    ColiveSource.deactivate = colive_source_deactivate;
    ColiveSource.video_tick = colive_source_tick;
    ColiveSource.update = colive_source_update;
    obs_register_source_s(&ColiveSource, sizeof(ColiveSource));

    ColiveSource.id = "voice_link_source";
    ColiveSource.type = OBS_SOURCE_TYPE_INPUT;
    ColiveSource.output_flags = OBS_SOURCE_AUDIO |
        OBS_SOURCE_DO_NOT_DUPLICATE;
    ColiveSource.get_name = voice_link_source_getname;

    obs_register_source_s(&ColiveSource, sizeof(ColiveSource));
}

void ColiveSourceImpl::RegisterCallback()
{
    auto p1 = colive_service_.lock();
    if (p1)
    {
        p1->AddOnRemoteMedia(shared_from_this());
    }
}

void ColiveSourceImpl::OnVideo(uint64_t uid, IColiveMedia::VFrame* vf, bool is_little_video)
{
    if (uid != uid_)
        return;

    obs_source_frame* p = 0;

    if (ToOBSVideoFrame(&p, vf))
    {
        int64_t timestamp_ns_offset = -((int64_t)p->timestamp);
        int64_t timestamp_invalid = TIMESTAMP_INVALID;
        timestamp_ns_offset_.compare_exchange_strong(timestamp_invalid, timestamp_ns_offset);

        p->timestamp += timestamp_ns_offset_;
        obs_source_output_video(source_, p);
        obs_source_frame_destroy(p);
    }
}

void ColiveSourceImpl::OnAudio(uint64_t uid, IColiveMedia::AFrame* af)
{
    if (uid != uid_)
        return;

    obs_source_audio audio_frame = {};

    if (ToOBSAudioFrame(&audio_frame, af))
    {
        int64_t timestamp_ns_offset = -((int64_t)audio_frame.timestamp);

        int64_t timestamp_invalid = TIMESTAMP_INVALID;
        timestamp_ns_offset_.compare_exchange_strong(timestamp_invalid, timestamp_ns_offset);

        audio_frame.timestamp += timestamp_ns_offset_;

        obs_source_output_audio(source_, &audio_frame);
    }
}
