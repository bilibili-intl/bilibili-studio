#include <obs-module.h>
#include <util/base.h>

#include <memory>

#include "texture_service_impl.h"


class TextureSourceImpl : public livehime::ITextureDispatcher
{
public:
    using Ptr = std::shared_ptr<TextureSourceImpl>;

    TextureSourceImpl()
        : source_(nullptr),
        filter_(nullptr) {
    }

    void RegisterCallback() {
        livehime::TextureServiceImpl::Fetch()->AddDispatcher(this);
        if (obs_source_active(source_)) {
            livehime::TextureServiceImpl::Fetch()->AddActiveRef();
        }
    }

    void UnregisterCallback() {
        if (obs_source_active(source_)) {
            livehime::TextureServiceImpl::Fetch()->SubActiveRef();
        }
        livehime::TextureServiceImpl::Fetch()->RemoveDispatcher(this);
    }

    void NotifySourceActive(bool active) {
        if (active) {
            livehime::TextureServiceImpl::Fetch()->AddActiveRef();
        } else {
            livehime::TextureServiceImpl::Fetch()->SubActiveRef();
        }
    }

    void OnTexture(const obs_source_frame* frame) override {
        obs_source_output_video(source_, frame);
    }

    obs_source_t *source_;
    obs_source_t *filter_;
};

class TextureSource {
public:
    TextureSourceImpl::Ptr impl;
};


static void texture_source_defaults(obs_data_t* settings) {
}

static obs_properties_t* texture_source_getproperties(void* data) {
    obs_properties_t *props = obs_properties_create();
    return props;
}

static void texture_source_update(void* data, obs_data_t* settings) {
}

static const char* texture_source_getname(void* unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("TextureRendererSource");
}

static void* texture_source_create(obs_data_t* settings, obs_source_t* source) {
    TextureSourceImpl::Ptr impl(std::make_shared<TextureSourceImpl>());
    impl->source_ = source;

    obs_source_set_async_unbuffered(source, true);

    auto s = new TextureSource();
    s->impl = impl;
    impl->RegisterCallback();

    return s;
}

static void texture_source_destroy(void* data) {
    auto s = static_cast<TextureSource*>(data);
    s->impl->UnregisterCallback();
}

static void texture_source_activate(void* data) {
    auto s = static_cast<TextureSource*>(data);
    s->impl->NotifySourceActive(true);
}

static void texture_source_deactivate(void* data) {
    auto s = static_cast<TextureSource*>(data);
    s->impl->NotifySourceActive(false);
}

static void texture_source_show(void* data) {
}

static void texture_source_hide(void* data) {
}

static void texture_source_tick(void* data, float seconds) {

}

void register_texture_source()
{
    struct obs_source_info texture_source;
    memset(&texture_source, 0, sizeof(texture_source));

    texture_source.id = "texture_renderer_source";
    texture_source.type = OBS_SOURCE_TYPE_INPUT;
    texture_source.output_flags = OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_DO_NOT_DUPLICATE;
    texture_source.get_name = texture_source_getname;
    texture_source.create = texture_source_create;
    texture_source.destroy = texture_source_destroy;
    texture_source.get_defaults = texture_source_defaults;
    texture_source.get_properties = texture_source_getproperties;
    texture_source.activate = texture_source_activate;
    texture_source.deactivate = texture_source_deactivate;
    texture_source.show = texture_source_show;
    texture_source.hide = texture_source_hide;
    texture_source.video_tick = texture_source_tick;
    texture_source.update = texture_source_update;
    obs_register_source_s(&texture_source, sizeof(texture_source));
}