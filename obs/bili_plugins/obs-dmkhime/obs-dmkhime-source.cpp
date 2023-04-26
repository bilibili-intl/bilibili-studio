#include <obs-module.h>
#include <util/base.h>

#include <memory>

#include "dmkhime-tex-service.h"


class DmkhimeSourceImpl : public bililive::IDmkhimeTextureDispatcher
{
public:
    using Ptr = std::shared_ptr<DmkhimeSourceImpl>;

    DmkhimeSourceImpl()
        : source_(nullptr),
          filter_(nullptr) {
    }

    void RegisterCallback() {
        auto service = bililive::DmkhimeTexService::Get();
        if (!service) {
            return;
        }

        service->AddDispatcher(this);
        if (obs_source_active(source_)) {
            service->AddActiveRef();
        }
    }

    void UnregisterCallback() {
        auto service = bililive::DmkhimeTexService::Get();
        if (!service) {
            return;
        }

        if (obs_source_active(source_)) {
            service->SubActiveRef();
        }
        service->RemoveDispatcher(this);
    }

    void NotifySourceActive(bool active) {
        auto service = bililive::DmkhimeTexService::Get();
        if (!service) {
            return;
        }

        if (active) {
            service->AddActiveRef();
        } else {
            service->SubActiveRef();
        }
    }

    void OnTexture(const obs_source_frame* frame) override {
        obs_source_output_video(source_, frame);
    }

    obs_source_t *source_;
    obs_source_t *filter_;
};

class DmkhimeSource {
public:
    DmkhimeSourceImpl::Ptr impl;
};


static void dmkhime_source_defaults(obs_data_t* settings) {
}

static obs_properties_t* dmkhime_source_getproperties(void* data) {
    obs_properties_t *props = obs_properties_create();
    return props;
}

static void dmkhime_source_update(void* data, obs_data_t* settings) {
}

static const char* dmkhime_source_getname(void* unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("DmkhimeSource");
}

static void* dmkhime_source_create(obs_data_t* settings, obs_source_t* source) {
    DmkhimeSourceImpl::Ptr impl(std::make_shared<DmkhimeSourceImpl>());
    impl->source_ = source;

    obs_source_set_async_unbuffered(source, true);

    auto s = new DmkhimeSource();
    s->impl = impl;
    impl->RegisterCallback();

    return s;
}

static void dmkhime_source_destroy(void* data) {
    auto s = static_cast<DmkhimeSource*>(data);
    s->impl->UnregisterCallback();
}

static void dmkhime_source_activate(void* data) {
    auto s = static_cast<DmkhimeSource*>(data);
    s->impl->NotifySourceActive(true);
}

static void dmkhime_source_deactivate(void* data) {
    auto s = static_cast<DmkhimeSource*>(data);
    s->impl->NotifySourceActive(false);
}

static void dmkhime_source_show(void* data) {
}

static void dmkhime_source_hide(void* data) {
}

static void dmkhime_source_tick(void* data, float seconds) {

}

void register_dmkhime_source()
{
    struct obs_source_info dmkhime_source;
    memset(&dmkhime_source, 0, sizeof(dmkhime_source));

    dmkhime_source.id = "dmkhime_source";
    dmkhime_source.type = OBS_SOURCE_TYPE_INPUT;
    dmkhime_source.output_flags = OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_DO_NOT_DUPLICATE;
    dmkhime_source.get_name = dmkhime_source_getname;
    dmkhime_source.create = dmkhime_source_create;
    dmkhime_source.destroy = dmkhime_source_destroy;
    dmkhime_source.get_defaults = dmkhime_source_defaults;
    dmkhime_source.get_properties = dmkhime_source_getproperties;
    dmkhime_source.activate = dmkhime_source_activate;
    dmkhime_source.deactivate = dmkhime_source_deactivate;
    dmkhime_source.show = dmkhime_source_show;
    dmkhime_source.hide = dmkhime_source_hide;
    dmkhime_source.video_tick = dmkhime_source_tick;
    dmkhime_source.update = dmkhime_source_update;
    obs_register_source_s(&dmkhime_source, sizeof(dmkhime_source));
}