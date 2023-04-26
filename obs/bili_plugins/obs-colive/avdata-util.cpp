#include "colive-service.h"
#include "avdata-util.h"

namespace bililive
{
    namespace colive
    {
        bool ToAgoraVideoFrame(agora::media::IVideoFrameObserver::VideoFrame& videoFrame, IColiveMedia::VFrame* src)
        {
            if (videoFrame.width == 0 || videoFrame.height == 0
                || src == 0 || src->width == 0 || src->height == 0)
                return false;

            bool isSameWidth = videoFrame.width == src->width;
            bool isSameHeight = videoFrame.height == src->height;
            assert(isSameWidth);
            assert(isSameHeight);

            if (!isSameWidth || !isSameHeight)
                return false;

            for (int y = 0; y < src->height; ++y)
            {
                memcpy((uint8_t*)videoFrame.yBuffer + videoFrame.yStride * y, src->yPlanar.data() + src->yStride * y, src->width);
            }

            for (int y = 0; y < src->height / 2; ++y)
            {
                memcpy((uint8_t*)videoFrame.uBuffer + videoFrame.uStride * y, src->uPlanar.data() + src->uStride * y, src->width / 2);
                memcpy((uint8_t*)videoFrame.vBuffer + videoFrame.vStride * y, src->vPlanar.data() + src->vStride * y, src->width / 2);
            }

            return true;
        }


        bool FromAgoraVideoFrame(std::unique_ptr<IColiveMedia::VFrame>& vframe, agora::media::IVideoFrameObserver::VideoFrame& src)
        {
            if (!vframe)
                vframe.reset(new IColiveMedia::VFrame());

            if (vframe == 0 || src.width == 0 || src.height == 0)
                return false;

            vframe->width = src.width;
            vframe->height = src.height;

            vframe->yStride = src.yStride;
            vframe->yPlanar.resize(src.yStride * src.height);
            memcpy(vframe->yPlanar.data(), src.yBuffer, vframe->yPlanar.size());

            vframe->uStride = src.uStride;
            vframe->uPlanar.resize(src.uStride * src.height / 2);
            memcpy(vframe->uPlanar.data(), src.uBuffer, vframe->uPlanar.size());

            vframe->vStride = src.vStride;
            vframe->vPlanar.resize(src.vStride * src.height / 2);
            memcpy(vframe->vPlanar.data(), src.vBuffer, vframe->vPlanar.size());

            return true;
        }


        bool ToOBSVideoFrame(obs_source_frame** obsFrame, IColiveMedia::VFrame* src)
        {
            if (!obsFrame || *obsFrame != 0)
                return false;

            obs_source_frame* result = obs_source_frame_create(VIDEO_FORMAT_I420, src->width, src->height);
            if (!result)
                return false;

            for (int y = 0; y < src->height; ++y)
            {
                uint8_t* pSrc = (uint8_t*)src->yPlanar.data() + src->yStride * y;
                uint8_t* pDst = (uint8_t*)result->data[0] + result->linesize[0] * y;
                memcpy(pDst, pSrc, src->width);
            }

            for (int y = 0; y < src->height / 2; ++y)
            {
                uint8_t* pSrc = (uint8_t*)src->uPlanar.data() + src->uStride * y;
                uint8_t* pDst = (uint8_t*)result->data[1] + result->linesize[1] * y;
                memcpy(pDst, pSrc, src->width / 2);

                pSrc = (uint8_t*)src->vPlanar.data() + src->vStride * y;
                pDst = (uint8_t*)result->data[2] + result->linesize[2] * y;
                memcpy(pDst, pSrc, src->width / 2);
            }

            video_format_get_parameters(VIDEO_CS_601, VIDEO_RANGE_PARTIAL,
                result->color_matrix,
                result->color_range_min,
                result->color_range_max);

            result->timestamp = src->presentationTimeMs * 1000000;

            *obsFrame = result;
            return true;
        }


        bool ToOBSAudioFrame(obs_source_audio* obsAudioFrame, IColiveMedia::AFrame* src)
        {
            if (!obsAudioFrame)
                return false;

            obsAudioFrame->data[0] = (uint8_t*)src->data.data();
            obsAudioFrame->format = AUDIO_FORMAT_16BIT;

            obsAudioFrame->samples_per_sec = src->samplerate;
            if (src->channels == 2)
                obsAudioFrame->speakers = SPEAKERS_STEREO;
            else if (src->channels == 1)
                obsAudioFrame->speakers = SPEAKERS_MONO;
            else
                assert(false);

            obsAudioFrame->timestamp = src->presentationTimeMs * 1000000;
            obsAudioFrame->frames = src->data.size() / src->channels;
            return true;
        }


        bool FromOBSFrame(
            std::unique_ptr<IColiveMedia::VFrame>& vframe,
            encoder_frame* src, int width, int height,
            int startX, int selectedWidth, int startY, int selectedHeight)
        {
            //for yuv420p, startX and selectedwidth must be even
            if (startX % 2 != 0 || selectedWidth % 2 != 0)
                return false;
            if (startY % 2 != 0 || selectedHeight % 2 != 0)
                return false;
            if (width <= 0 || height <= 0)
                return false;
            if (startX < 0 || startX >= width || selectedWidth > width - startX)
                return false;
            if (startY < 0 || startY >= height || selectedHeight > height - startY)
                return false;

            if (!vframe)
                vframe.reset(new IColiveMedia::VFrame());

            vframe->width = selectedWidth;
            vframe->height = selectedHeight;

            int halfHeight = selectedHeight / 2;
            int halfStartX = startX / 2;
            int halfStartY = startY / 2;

            //only support yuv420p (not nv12)
            if (!src->data[0] || !src->data[1] || !src->data[2])
                return false;

            vframe->yStride = src->linesize[0];
            vframe->yPlanar.resize(vframe->yStride * selectedHeight);
            std::copy_n(
                src->data[0] + startX + startY * vframe->yStride,
                vframe->yPlanar.size() - startX,
                vframe->yPlanar.begin());

            vframe->uStride = src->linesize[1];
            vframe->uPlanar.resize(vframe->uStride * halfHeight);
            std::copy_n(
                src->data[1] + halfStartX + halfStartY * vframe->uStride,
                vframe->uPlanar.size() - halfStartX,
                vframe->uPlanar.begin());

            vframe->vStride = src->linesize[2];
            vframe->vPlanar.resize(vframe->vStride * halfHeight);
            std::copy_n(
                src->data[2] + halfStartX + halfStartY * vframe->vStride,
                vframe->vPlanar.size() - halfStartX,
                vframe->vPlanar.begin());

            return true;
        }


        bool ToAgoraAudioFrame(agora::media::IAudioFrameObserver::AudioFrame& audioFrame, IColiveMedia::AFrame* src)
        {
            if (!src)
                return false;
            if (audioFrame.samples < (int)src->data.size() / src->channels)
                return false;

            audioFrame.bytesPerSample = 2;
            audioFrame.type = agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16;

            audioFrame.channels = src->channels;
            audioFrame.samplesPerSec = src->samplerate;

            audioFrame.samples = src->data.size() / audioFrame.channels;
            std::copy_n(src->data.data(), src->data.size(), (short*)audioFrame.buffer);

            return true;
        }


        bool FromAgoraAudioFrame(std::unique_ptr<IColiveMedia::AFrame>& aframe, agora::media::IAudioFrameObserver::AudioFrame& src)
        {
            if (!aframe)
                aframe.reset(new IColiveMedia::AFrame());

            if (src.type != agora::media::IAudioFrameObserver::FRAME_TYPE_PCM16)
                return false;
            if (src.bytesPerSample != 2)
                return false;

            aframe->channels = src.channels;
            aframe->samplerate = src.samplesPerSec;
            aframe->presentationTimeMs = src.renderTimeMs;
            aframe->data.resize(src.samples * src.channels);
            std::copy_n((short*)src.buffer, aframe->data.size(), aframe->data.data());

            return true;
        }


        bool FromOBSFrame(std::unique_ptr<IColiveMedia::AFrame>& aframe, encoder_frame* src, int samplerate, int channels)
        {
            if (!src)
                return false;
            if (samplerate == 0 || channels <= 0 || channels > 2)
                return false;

            if (!aframe)
                aframe.reset(new IColiveMedia::AFrame());

            aframe->samplerate = samplerate;
            aframe->channels = channels;
            aframe->data.resize(src->frames * channels);
            std::copy_n((short*)src->data[0], src->frames * channels, aframe->data.data());
            return true;
        }

    };
};

