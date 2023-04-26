#include "bililive/bililive/livehime/svga_player/utils/svga_scale_info.h"

#include <algorithm>


namespace svga {

    SVGAScaleInfo::SVGAScaleInfo() {}

    void SVGAScaleInfo::performScaleType(
        float canvas_width, float canvas_height,
        float video_width, float video_height, ScaleType scale_type)
    {
        if (canvas_width == 0 ||
            canvas_height == 0 ||
            video_width == 0 ||
            video_height == 0)
        {
            return;
        }

        resetVars();

        auto cw_vw_f = (canvas_width - video_width) / 2.f;
        auto ch_vh_f = (canvas_height - video_height) / 2.f;

        auto video_ratio = video_width / video_height;
        auto canvas_ratio = canvas_width / canvas_height;

        auto ch_d_vh = canvas_height / video_height;
        auto cw_d_vw = canvas_width / video_width;

        switch (scale_type) {
        case ScaleType::CENTER:
            tran_fx_ = cw_vw_f;
            tran_fy_ = ch_vh_f;
            break;

        case ScaleType::CENTER_CROP:
            if (video_ratio > canvas_ratio) {
                ratio_ = ch_d_vh;
                ratio_x_ = false;
                scale_fx_ = ch_d_vh;
                scale_fy_ = ch_d_vh;
                tran_fx_ = (canvas_width - video_width * ch_d_vh) / 2.f;
            } else {
                ratio_ = cw_d_vw;
                ratio_ = true;
                scale_fx_ = cw_d_vw;
                scale_fy_ = cw_d_vw;
                tran_fy_ = (canvas_height - video_height * cw_d_vw) / 2.f;
            }
            break;

        case ScaleType::CENTER_INSIDE:
            if (video_width < canvas_width && video_height < canvas_height) {
                tran_fx_ = cw_vw_f;
                tran_fy_ = ch_vh_f;
            } else {
                if (video_ratio > canvas_ratio) {
                    ratio_ = cw_d_vw;
                    ratio_x_ = true;
                    scale_fx_ = cw_d_vw;
                    scale_fy_ = cw_d_vw;
                    tran_fy_ = (canvas_height - video_height * cw_d_vw) / 2.f;
                } else {
                    ratio_ = ch_d_vh;
                    ratio_x_ = false;
                    scale_fx_ = ch_d_vh;
                    scale_fy_ = ch_d_vh;
                    tran_fx_ = (canvas_width - video_width * ch_d_vh) / 2.f;
                }
            }
            break;

        case ScaleType::FIT_CENTER:
            if (video_ratio > canvas_ratio) {
                ratio_ = cw_d_vw;
                ratio_x_ = true;
                scale_fx_ = cw_d_vw;
                scale_fy_ = cw_d_vw;
                tran_fy_ = (canvas_height - video_height * cw_d_vw) / 2.f;
            } else {
                ratio_ = ch_d_vh;
                ratio_x_ = false;
                scale_fx_ = ch_d_vh;
                scale_fy_ = ch_d_vh;
                tran_fx_ = (canvas_width - video_width * ch_d_vh) / 2.f;
            }
            break;

        case ScaleType::FIT_START:
            if (video_ratio > canvas_ratio) {
                ratio_ = cw_d_vw;
                ratio_x_ = true;
                scale_fx_ = cw_d_vw;
                scale_fy_ = cw_d_vw;
            } else {
                ratio_ = ch_d_vh;
                ratio_x_ = false;
                scale_fx_ = ch_d_vh;
                scale_fy_ = ch_d_vh;
            }
            break;

        case ScaleType::FIT_END:
            if (video_ratio > canvas_ratio) {
                ratio_ = cw_d_vw;
                ratio_x_ = true;
                scale_fx_ = cw_d_vw;
                scale_fy_ = cw_d_vw;
                tran_fy_ = canvas_height - video_height * cw_d_vw;
            } else {
                ratio_ = ch_d_vh;
                ratio_x_ = false;
                scale_fx_ = ch_d_vh;
                scale_fy_ = ch_d_vh;
                tran_fx_ = canvas_width - video_width * ch_d_vh;
            }
            break;

        case ScaleType::FIT_XY:
            ratio_ = std::max(cw_d_vw, ch_d_vh);
            ratio_x_ = cw_d_vw > ch_d_vh;
            scale_fx_ = cw_d_vw;
            scale_fy_ = ch_d_vh;
            break;

        default:
            ratio_ = cw_d_vw;
            ratio_x_ = true;
            scale_fx_ = cw_d_vw;
            scale_fy_ = cw_d_vw;
            break;
        }
    }

    void SVGAScaleInfo::resetVars() {
        tran_fx_ = 0;
        tran_fy_ = 0;
        scale_fx_ = 1;
        scale_fy_ = 1;
        ratio_ = 1;
        ratio_x_ = false;
    }

}