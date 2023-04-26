#ifndef BILILIVE_MOTION_BILILIVE_MOTION_H_
#define BILILIVE_MOTION_BILILIVE_MOTION_H_

#include <memory>
#include <string>


namespace livehime {
namespace motion {

    struct BLPCInfo
    {
        //版本号
        int version = 0;
        //视频总共的FPS
        int total_fps = 0;
        //输出rgba的宽高
        int real_width = 0;
        int real_height = 0;
        int fps = 0;
        int video_width = 0;
        int video_height = 0;
        //真实视频的在视频里的位置{x,y,width,height}
        int real_frame[4] = { 0 };
        //alpha数据的位置{x,y,width,height}
        int alpha_frame[4] = { 0 };
    };

    class MP4Motion {
    public:
        virtual ~MP4Motion() = default;

        virtual bool IsSupportMp4() = 0;
        virtual bool InitD3D() = 0;
        virtual bool SetMp4File(const std::wstring& file_path) = 0;
        virtual bool SetMp4Buffer(const void* data, uint64_t data_len) = 0;
        virtual bool SetOutputSize(uint32_t width, uint32_t height) = 0;
        virtual bool GetOutputSize(uint32_t* width, uint32_t* height) = 0;
        virtual bool Play() = 0;
        virtual bool PlayReady() = 0;
        virtual bool IsPlaying() = 0;
        virtual void Destroy() = 0;
        virtual void GetVideoSize(int *width, int *height) = 0;
        virtual bool parseBLPC(const std::wstring& file_name, BLPCInfo* blpc) = 0;
    };

    class Mp4PlayerCallback
    {
    public:
        virtual void OnMp4PlayStart(MP4Motion* mp4) {};
        virtual void OnMp4PlayFrameCallback(MP4Motion* mp4, void* data, uint32_t width, uint32_t height, uint32_t rowptich) {};
        virtual void OnMp4PlayEnd(MP4Motion* mp4) {};

    protected:
        virtual ~Mp4PlayerCallback() {}
    };

}
}

__declspec(dllexport) livehime::motion::MP4Motion* CreateBililiveMotion(livehime::motion::Mp4PlayerCallback* callback);

#endif  // BILILIVE_MOTION_BILILIVE_MOTION_H_