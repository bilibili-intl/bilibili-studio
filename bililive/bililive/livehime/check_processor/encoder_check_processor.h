#ifndef BILILIVE_BILILIVE_LIVEHIME_CHECK_PROCESSOR_ENCODER_CHECK_PROCESSOR_H
#define BILILIVE_BILILIVE_LIVEHIME_CHECK_PROCESSOR_ENCODER_CHECK_PROCESSOR_H

#include <string>

namespace EncoderCheckProcessor
{
    //软编："obs_x264", intel硬编： "obs_qsv11" ,AMD硬编："amd_amf_h264",英伟达硬编："ffmpeg_nvenc",英伟达图灵:"jim_nvenc"
    bool EncoderIsSupported(const std::string& encoder);
}

#endif

