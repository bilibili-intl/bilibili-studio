#include <util/base.h>
#include <obs-module.h>
#include <mutex>
#include "colive-service.h"
#include "avdata-util.h"

#define do_log(level, format, ...) \
    blog(level, "[agora video encoder: '%s'] " format, \
            obs_encoder_get_name(enc->encoder), ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG,   format, ##__VA_ARGS__)


using namespace bililive::colive;


class colive_venc_encoder {
public:
    obs_encoder_t* encoder;
    ColiveMediaWPtr agoraService;

    size_t width;
    size_t height;
    size_t fps;
    size_t colive_bitrate;
    size_t rtmp_bitrate;

    std::mutex selectedRangeMutex;
    size_t selectedX;
    size_t selectedY;
    size_t selectedWidth;
    size_t selectedHeight;

    double outputProgressStep_;
    int nOutputProgress_;
};

static const char *colive_venc_getname(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("colive video encoder");
}

static void colive_venc_video_info(void *data, struct video_scale_info *info)
{
    colive_venc_encoder *enc = (colive_venc_encoder*)data;

    info->format = VIDEO_FORMAT_I420;
    info->width = enc->width;
    info->height = enc->height;

    // 下面两个是试出来的数据。暂时写死
    info->colorspace = VIDEO_CS_601;
    info->range = VIDEO_RANGE_PARTIAL;
}

static bool colive_venc_update(void *data, obs_data_t *settings)
{
    colive_venc_encoder *enc = (colive_venc_encoder*)data;
    std::lock_guard<std::mutex> lg(enc->selectedRangeMutex);

    enc->selectedX = (size_t)obs_data_get_int(settings, "selected_x");
    enc->selectedY = (size_t)obs_data_get_int(settings, "selected_y");
    enc->selectedWidth = (size_t)obs_data_get_int(settings, "selected_width");
    enc->selectedHeight = (size_t)obs_data_get_int(settings, "selected_height");

    if (enc->selectedWidth > enc->width)
        enc->selectedWidth = enc->width;
    if (enc->selectedHeight > enc->height)
        enc->selectedHeight = enc->height;

    if (enc->selectedX + enc->selectedWidth > enc->width)
        enc->selectedX = enc->width - enc->selectedWidth;
    if (enc->selectedY + enc->selectedHeight > enc->height)
        enc->selectedY = enc->height - enc->selectedHeight;

    return true;
}

static void colive_venc_destroy(void *data)
{
    if (data)
    {
        colive_venc_encoder *enc = (colive_venc_encoder*)data;
        delete enc;

        blog(LOG_INFO, "%s", "[colive venc] agora-venc destroyed.");
    }
}

static void *colive_venc_create(obs_data_t *settings, obs_encoder_t *encoder)
{
    ColiveMediaPtr p = GetColiveServiceFactory()->AquireMedia();
    if (!p)
    {
        blog(LOG_WARNING, "%s", "[colive venc] fail to create venc before colive service created");
        return nullptr;
    }

    if (!settings)
    {
        blog(LOG_WARNING, "%s", "[colive venc] fail to create venc without settings");
        return nullptr;
    }

    size_t width, height, fps, colive_bitrate, rtmp_bitrate, capture_width, capture_height;
    width = (size_t)obs_data_get_int(settings, "width");
    height = (size_t)obs_data_get_int(settings, "height");
    fps = (size_t)obs_data_get_int(settings, "fps");
    colive_bitrate = (size_t)obs_data_get_int(settings, "colive_bitrate");
    rtmp_bitrate = (size_t)obs_data_get_int(settings, "rtmp_bitrate");
    capture_width = (size_t)obs_data_get_int(settings, "capture_width");
    capture_height = (size_t)obs_data_get_int(settings, "capture_height");

    if (width == 0 || height == 0 ||
        capture_width == 0 || capture_height == 0 ||
        fps == 0 || colive_bitrate == 0 || rtmp_bitrate == 0)
    {
        blog(LOG_WARNING, "%s", "[colive venc] invalid settings");
        return nullptr;
    }

    colive_venc_encoder *result = new colive_venc_encoder();
    result->agoraService = p;
    result->encoder = encoder;
    result->width = capture_width;
    result->height = capture_height;
    result->fps = fps;
    result->colive_bitrate = colive_bitrate;
    result->rtmp_bitrate = rtmp_bitrate;

    double activeFps = video_output_get_frame_rate(obs_get_video());
    if (activeFps > 0.0)
        result->outputProgressStep_ = fps / activeFps;
    else
        result->outputProgressStep_ = 1.0;

    result->nOutputProgress_ = -1;

    colive_venc_update(result, settings);

    VideoParamInfo info;
    info.colive_width = width;
    info.colive_height = height;
    info.colive_fps = result->fps;
    info.colive_start_bitrate = result->colive_bitrate;
    info.bypass_bitrate = result->rtmp_bitrate;
    p->SetVideoParam(info);

    blog(LOG_INFO, "%s", "[colive venc] agora-venc created.");

    return result;
}


static bool colive_venc_encode(void *data, struct encoder_frame *frame,
struct encoder_packet *packet, bool *received_packet)
{
    colive_venc_encoder *enc = (colive_venc_encoder*)data;
    ColiveMediaPtr p = enc->agoraService.lock();
    if (!p)
        return false;

    int currentProgress = static_cast<int>(enc->outputProgressStep_ * frame->pts);
    if (currentProgress > enc->nOutputProgress_)
    {
        enc->nOutputProgress_ = currentProgress;

        int width = obs_encoder_get_width(enc->encoder);
        int height = obs_encoder_get_height(enc->encoder);

        std::unique_ptr<IColiveMedia::VFrame> vframe;

        int selectedX, selectedWidth;
        int selectedY, selectedHeight;
        {
            std::lock_guard<std::mutex> lg(enc->selectedRangeMutex);
            selectedX = enc->selectedX;
            selectedWidth = enc->selectedWidth;
            selectedY = enc->selectedY;
            selectedHeight = enc->selectedHeight;
        }

        if (!FromOBSFrame(
            vframe, frame, width, height,
            selectedX, selectedWidth, selectedY, selectedHeight))
            return false;

        vframe->presentationTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(ColiveClock::now().time_since_epoch()).count();

        p->PushLocalVideo(std::move(vframe));
    }

    return true;
}

static void colive_venc_defaults(obs_data_t *settings)
{

}


static obs_properties_t *colive_venc_properties(void *unused)
{
    UNUSED_PARAMETER(unused);

    obs_properties_t *props = obs_properties_create();
    obs_properties_add_int(props, "colive_bitrate", "colive_bitrate", 100, 3000, 100);
    obs_properties_add_int(props, "rtmp_bitrate", "rtmp_bitrate", 100, 3000, 100);
    obs_properties_add_int(props, "width", "width", 320, 1920, 8);
    obs_properties_add_int(props, "height", "height", 240, 1080, 4);
    obs_properties_add_int(props, "capture_width", "capture_width", 240, 1920, 4);
    obs_properties_add_int(props, "capture_height", "capture_height", 240, 1920, 4);
    obs_properties_add_int(props, "selected_x", "selected_x", 0, 1920, 8);
    obs_properties_add_int(props, "selected_y", "selected_y", 0, 1920, 8);
    obs_properties_add_int(props, "selected_width", "selected_width", 320, 1920, 8);
    obs_properties_add_int(props, "selected_height", "selected_height", 240, 1920, 8);
    obs_properties_add_int(props, "fps", "fps", 15, 60, 1);
    return props;
}

static bool colive_venc_extra_data(void *data, uint8_t **extra_data, size_t *size)
{
    colive_venc_encoder *enc = (colive_venc_encoder*)data;
    return true;
}

static bool colive_venc_sei_data(void *data, uint8_t **extra_data, size_t *size)
{
    colive_venc_encoder *enc = (colive_venc_encoder*)data;
    return true;
}

void register_colive_video_encoder()
{
    struct obs_encoder_info colive_venc_info = {};
    colive_venc_info.id = "colive_venc";
    colive_venc_info.type = OBS_ENCODER_VIDEO;
    colive_venc_info.codec = "h264";
    colive_venc_info.get_name = colive_venc_getname;
    colive_venc_info.create = colive_venc_create;
    colive_venc_info.update = colive_venc_update;
    colive_venc_info.destroy = colive_venc_destroy;
    colive_venc_info.encode = colive_venc_encode;
    colive_venc_info.get_defaults = colive_venc_defaults;
    colive_venc_info.get_properties = colive_venc_properties;
    colive_venc_info.get_extra_data = colive_venc_extra_data;
    colive_venc_info.get_sei_data = colive_venc_sei_data;
    colive_venc_info.get_video_info = colive_venc_video_info;

    obs_register_encoder(&colive_venc_info);
}
