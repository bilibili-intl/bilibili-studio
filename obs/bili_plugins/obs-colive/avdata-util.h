#ifndef OBS_BILI_PLUGINS_OBS_COLIVE_AVDATA_UTIL_H_
#define OBS_BILI_PLUGINS_OBS_COLIVE_AVDATA_UTIL_H_

#include "colive-service.h"

#include <obs-module.h>
#include <IAgoraMediaEngine.h>


namespace bililive
{
    namespace colive
    {
        bool ToAgoraVideoFrame(agora::media::IVideoFrameObserver::VideoFrame& videoFrame, IColiveMedia::VFrame* src);
        bool FromAgoraVideoFrame(std::unique_ptr<IColiveMedia::VFrame>& vframe, agora::media::IVideoFrameObserver::VideoFrame& src);

        bool ToOBSVideoFrame(obs_source_frame** obsFrame, IColiveMedia::VFrame* src);
        bool FromOBSFrame(
            std::unique_ptr<IColiveMedia::VFrame>& vframe,
            encoder_frame* src, int width, int height,
            int startX, int selectedWidth, int startY, int selectedHeight);

        bool ToAgoraAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& audioFrame, IColiveMedia::AFrame* src);
        bool FromAgoraAudioFrame(std::unique_ptr<IColiveMedia::AFrame>& aframe, agora::media::IAudioFrameObserver::AudioFrame& src);

        bool ToOBSAudioFrame(obs_source_audio* obsAudioFrame, IColiveMedia::AFrame* src);
        bool FromOBSFrame(std::unique_ptr<IColiveMedia::AFrame>& aframe, encoder_frame* src, int samplerate, int channels);
    };
};

#endif  // OBS_BILI_PLUGINS_OBS_COLIVE_AVDATA_UTIL_H_