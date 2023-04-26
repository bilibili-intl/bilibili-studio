#include <obs-module.h>
#include <util/base.h>
#include "colive-service.h"

using namespace bililive::colive;

class colive_output {
public:
    obs_output_t *output;

    bool rtmp_publish_;
    ColiveMediaWPtr service_;
};

static const char *colive_output_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return "ColiveOutput";
}

static void *colive_output_create(obs_data_t *settings, obs_output_t *output)
{
    ColiveMediaPtr p = GetColiveServiceFactory()->AquireMedia();
    if (!p)
    {
        blog(LOG_WARNING, "%s", "[colive output] fail to create colive output before colive service created");
        return nullptr;
    }

    colive_output *context = new colive_output;
    context->output = output;
    context->rtmp_publish_ = !obs_data_get_bool(settings, "self_publish");
    context->service_ = p;

    blog(LOG_WARNING, "%s", "[colive output] created.");

    return context;
}

static void colive_output_destroy(void *data)
{
    if (data)
    {
        colive_output *context = (colive_output*)data;
        delete context;

        blog(LOG_WARNING, "%s", "[colive output] destroyed.");
    }
}

static bool colive_output_start(void *data)
{
    colive_output *context = (colive_output*)data;

    if (!obs_output_can_begin_data_capture(context->output, 0))
    {
        blog(LOG_WARNING, "%s", "[colive output] start: fail to start capture.");
        return false;
    }

    if (!obs_output_initialize_encoders(context->output, 0))
    {
        blog(LOG_WARNING, "%s", "[colive output] start: fail to initialize encoders.");
        return false;
    }

    auto ptr = context->service_.lock();
    if (ptr)
    {
        if (context->rtmp_publish_)
        {
            obs_service_t* service = obs_output_get_service(context->output);
            if (!service)
            {
                blog(LOG_WARNING, "%s", "[colive output] start: no service object.");
                return false;
            }

            const char* path = obs_service_get_url(service);
            const char* key = obs_service_get_key(service);
            if (path == nullptr)
                path = "";
            if (key == nullptr || std::string("None") == key)
                key = "";

            bool ret = ptr->SetRTMPPublish(std::string(path) + key);
            if (!ret)
            {
                return false;
            }
        }

        bool ret = ptr->Start();
        if (!ret)
        {
            return false;
        }
    }
    else
    {
        blog(LOG_WARNING, "%s", "[colive output] start: no colive service object.");
    }

    obs_output_begin_data_capture(context->output, 0);
    return true;
}

static void colive_output_stop(void *data, uint64_t ts)
{
    colive_output *context = (colive_output*)data;

    auto ptr = context->service_.lock();
    if (ptr)
    {
        ptr->Stop();
    }
    else
    {
        blog(LOG_WARNING, "%s", "[colive output] stop: colive service object is released.");
    }

    obs_output_end_data_capture(context->output);
    UNUSED_PARAMETER(ts);
}

static void colive_output_data(void *data, struct encoder_packet *packet)
{
    UNUSED_PARAMETER(data);
    UNUSED_PARAMETER(packet);
}

static const char *voice_link_output_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return "VoiceLinkOutput";
}

static bool voice_link_output_start(void *data)
{
    colive_output *context = (colive_output*)data;

    if (!obs_output_can_begin_data_capture(context->output, 0))
    {
        blog(LOG_WARNING, "%s", "[voice link output] start: fail to start capture.");
        return false;
    }

    if (!obs_output_initialize_encoders(context->output, 0))
    {
        blog(LOG_WARNING, "%s", "[voice link output] start: fail to initialize encoders.");
        return false;
    }

    obs_output_begin_data_capture(context->output, 0);
    return true;
}

static void voice_link_output_stop(void *data, uint64_t ts)
{
    colive_output *context = (colive_output*)data;

    obs_output_end_data_capture(context->output);
    UNUSED_PARAMETER(ts);
}

void register_colive_output()
{
    struct obs_output_info colive_output_info = {};
    colive_output_info.id = "colive_output";
    colive_output_info.flags = OBS_OUTPUT_AV | OBS_OUTPUT_ENCODED;
    colive_output_info.get_name = colive_output_getname;
    colive_output_info.create = colive_output_create;
    colive_output_info.destroy = colive_output_destroy;
    colive_output_info.start = colive_output_start;
    colive_output_info.stop = colive_output_stop;
    colive_output_info.encoded_packet = colive_output_data;
    obs_register_output(&colive_output_info);

    /**
     * 给旁路自推流用的。
     * 旁路自推流时，视频采集走别的逻辑，这里只需要音频。
     * 重要的是这里设置的 flags 必须要和上面 colive_output_start 里指定的 flags 相同。
     * 否则会出现编码器释放不掉的问题。
     */
    colive_output_info.id = "sp_colive_output";
    colive_output_info.flags = OBS_OUTPUT_AUDIO | OBS_OUTPUT_ENCODED;
    obs_register_output(&colive_output_info);

    colive_output_info.id = "voice_link_output";
    colive_output_info.flags = OBS_OUTPUT_AUDIO | OBS_OUTPUT_ENCODED;
    colive_output_info.get_name = voice_link_output_getname;
    colive_output_info.start = voice_link_output_start;
    colive_output_info.stop = voice_link_output_stop;
    obs_register_output(&colive_output_info);
}

