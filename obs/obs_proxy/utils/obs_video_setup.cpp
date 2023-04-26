#include "obs/obs_proxy/utils/obs_video_setup.h"

#include "base/prefs/pref_service.h"
#include "base/strings/string_util.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

#include "obs/obs-studio/libobs/obs.h"

namespace {

using obs_proxy::FPSValue;

struct integer_type {};
struct fraction_type {};
struct common_type {};

FPSValue AcquireFPSValue(PrefService* pref_service, integer_type)
{
    uint32_t num = pref_service->GetInteger(prefs::kVideoFPSInt);
    return std::make_tuple(num, 1);
}

FPSValue AcquireFPSValue(PrefService* pref_service, fraction_type)
{
    uint32_t num = pref_service->GetInteger(prefs::kVideoFPSNum);
    uint32_t den = pref_service->GetInteger(prefs::kVideoFPSDen);

    // Shall we really need to do this fixup?
    if (den == 0) {
        num = 30;
        den = 1;
    }

    return std::make_tuple(num, den);
}

FPSValue AcquireFPSValue(PrefService* pref_service, common_type)
{
    std::string value = pref_service->GetString(prefs::kVideoFPSCommon);
    uint32_t num = 30;
    uint32_t den = 1;
    if (value == "10")
    {
        num = 10;
        den = 1;
    }
    else if (value == "20")
    {
        num = 20;
        den = 1;
    }
    else if (value == "24")
    {
        num = 24;
        den = 1;
    }
    else if (value == "25")
    {
        num = 25;
        den = 1;
    }
    else if (value == "29.97")
    {
        num = 30000;
        den = 1001;
    }
    else if (value == "48")
    {
        num = 48;
        den = 1;
    }
    else if (value == "59.94")
    {
        num = 60000;
        den = 1001;
    }
    else if (value == "60")
    {
        num = 60;
        den = 1;
    }

    return std::make_tuple(num, den);
}

}   // namespace

namespace obs_proxy {

const char* GetConfiguredRendererModule(PrefService* pref_service)
{
    std::string renderer = pref_service->GetString(prefs::kVideoRenderer);
    bool use_d3d = base::EqualsCaseInsensitiveASCII(renderer, prefs::kVideoRendererD3D);
    return use_d3d ? prefs::kRendererD3DModuleName : prefs::kRendererOpenGLModuleName;
}

FPSValue GetConfiguredFPS(PrefService* pref_service)
{
    uint32_t type = pref_service->GetInteger(prefs::kVideoFPSType);
    switch (type) {
        case 0:
            return AcquireFPSValue(pref_service, common_type());
        case 1:
            return AcquireFPSValue(pref_service, integer_type());
        case 2:
            return AcquireFPSValue(pref_service, fraction_type());
        default:
            LOG(WARNING) << "Unknown video fps type " << type;
            return FPSValue(30, 1);
    }
}

VideoMetrics GetConfiguredVideoOutputMetrics(PrefService* pref_service)
{
    uint32_t output_width = pref_service->GetInteger(prefs::kVideoOutputCX);
    uint32_t output_height = pref_service->GetInteger(prefs::kVideoOutputCY);

    // 横竖屏分辨率采用同一份配置，根据横竖屏模式互换宽高
    int type = pref_service->GetInteger(prefs::kLivehimeLiveModelType);
    if (type == 1)
    {
        return std::make_tuple(output_height, output_width);
    }
    else if(type != 0)
    {
        NOTREACHED();
    }

    return std::make_tuple(output_width, output_height);
}

video_colorspace GetConfiguredVideoColorspace(PrefService* pref_service)
{
    std::string colorspace = pref_service->GetString(prefs::kVideoColorSpace);
    bool cs_601 = base::EqualsCaseInsensitiveASCII(colorspace, prefs::kVideoColorSpace601);
    return cs_601 ? VIDEO_CS_601 : VIDEO_CS_709;
}

video_range_type GetConfiguredVideoRange(PrefService* pref_service)
{
    std::string range = pref_service->GetString(prefs::kVideoColorRange);
    bool full_range = base::EqualsCaseInsensitiveASCII(range, prefs::kVideoRangeFull);
    return full_range ? VIDEO_RANGE_FULL : VIDEO_RANGE_PARTIAL;
}

video_format GetConfiguredVideoFormat(PrefService* pref_service)
{
    video_format chosen_format = VIDEO_FORMAT_RGBA;
    std::string color_format = pref_service->GetString(prefs::kVideoColorFormat);
    if (base::EqualsCaseInsensitiveASCII(color_format, prefs::kVideoFormatI420))
    {
        chosen_format = VIDEO_FORMAT_I420;
    }
    else if (base::EqualsCaseInsensitiveASCII(color_format, prefs::kVideoFormatI444))
    {
        chosen_format = VIDEO_FORMAT_I444;
    }
    else if (base::EqualsCaseInsensitiveASCII(color_format, prefs::kVideoFormatNV12))
    {
        chosen_format = VIDEO_FORMAT_NV12;
    }

    return chosen_format;
}

obs_scale_type GetConfiguredScaleType(PrefService* pref_service)
{
    obs_scale_type scale_type = OBS_SCALE_BICUBIC;
    std::string scale_type_value = pref_service->GetString(prefs::kVideoScaleType);
    if (base::EqualsCaseInsensitiveASCII(scale_type_value, prefs::kVideoScaleBicubic))
    {
        scale_type = OBS_SCALE_BILINEAR;
    }
    else if (base::EqualsCaseInsensitiveASCII(scale_type_value, prefs::kVideoScaleLanczos))
    {
        scale_type = OBS_SCALE_LANCZOS;
    }

    return scale_type;
}

}   // namespace obs_proxy