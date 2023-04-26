#ifndef BILILIVE_MOTION_MP4_MOTION_H_
#define BILILIVE_MOTION_MP4_MOTION_H_

#include "bililive_motion.h"
#include <condition_variable>
#include <windows.h>
#include <wrl.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <atomic>
#include <thread>
#include <vector>
#include "utils/Timer.h"


extern "C" {
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


namespace livehime {
namespace motion {

    class MP4MotionImpl :
        public MP4Motion
    {
    public:
        explicit MP4MotionImpl(Mp4PlayerCallback* callback);
        ~MP4MotionImpl();

        bool IsSupportMp4() override;
        bool InitD3D() override;

        bool SetMp4File(const std::wstring& file_path) override;
        bool SetMp4Buffer(const void* data, uint64_t data_len) override;

        bool SetOutputSize(uint32_t width, uint32_t height) override;
        bool GetOutputSize(uint32_t* width, uint32_t* height) override;

        bool Play() override;

        bool IsPlaying() override;

        bool PlayReady() override;

        void GetVideoSize(int* width, int* height) override;

        void Destroy() override;

        bool parseBLPC(const std::wstring& file_name, BLPCInfo* blpc) override;

    private:
        enum MP4DataType
        {
            kNoMp4,
            kFile,
            kBuffer
        };

        struct TextureInfo
        {
            DirectX::XMUINT4 rgb_text_rect = {};
            DirectX::XMUINT4 alpha_text_rect = {};
            DirectX::XMUINT2 yuv_text_size = {};
            DirectX::XMUINT2 text_size = {};
        };

        static HRESULT CreateShaderFromFile(
            const WCHAR* csoFileNameInOut,
            const WCHAR* hlslFileName,
            LPCSTR entryPoint,
            LPCSTR shaderModel,
            ID3DBlob** ppBlobOut);

        void Mp4PlayThread();

        bool InitResource();
        void UpdateD3DResource(AVFrame* yuv_frame);
        void YuvToRgb();
        void CallbackBgraFrame();
        void ProcessRemainFrame();

        bool InitFfmpegFormFile();
        bool InitFfmpegFormBuffer();
        bool ProcessPacket(const AVPacket& packet, const BLPCInfo& info);
        bool ParseBLPCJson(const std::wstring& file_name, BLPCInfo& info) const;
        bool ParseBLPCJson(const void* data, uint64_t data_len, BLPCInfo& info) const;
        bool ParseBLPCJson(std::istream& s, BLPCInfo& info) const;

        static int ReadPacket(void* opaque, uint8_t* buf, int buf_size);
        static Microsoft::WRL::ComPtr<ID3DBlob> LoadShaderResource(int resource_id);

        bool                            is_initialize_called_ = false;
        std::atomic_bool                quit_play_ = true;
        MP4DataType                     mp4_file_type_ = kNoMp4;
        std::wstring                    mp4_file_path_;
        std::vector<uint8_t>            mp4_data_;
        int64_t                         mp4_data_curpos_ = 0;
        std::thread                     th_;

        Mp4PlayerCallback*              callback_ = nullptr;

        BLPCInfo                        vap_info_;
        AVFormatContext*                context_ = nullptr;
        AVCodecContext*                 video_decoder_context_ = nullptr;
        AVIOContext*                    io_context_ = nullptr;
        uint8_t*                        io_buffer_ = nullptr;
        int                             stream_video_index_ = -1;

        uint32_t                        output_width_ = 0;
        uint32_t                        output_height = 0;
        Timer                           timer_;
        double                          interval_time_ = 0.0;

        template <class T>
        using ComPtr = Microsoft::WRL::ComPtr<T>;

        ComPtr<ID3D11Device>                d3d_device_;
        ComPtr<ID3D11DeviceContext>         d3d_immediate_context_;

        ComPtr<ID3D11Buffer>                texture_info_const_buffer_;

        ComPtr<ID3D11Texture2D>             input_texture_y_;
        ComPtr<ID3D11Texture2D>             input_texture_u_;
        ComPtr<ID3D11Texture2D>             input_texture_v_;

        ComPtr<ID3D11ShaderResourceView>    input_texture_view_y_;
        ComPtr<ID3D11ShaderResourceView>    input_texture_view_u_;
        ComPtr<ID3D11ShaderResourceView>    input_texture_view_v_;

        ComPtr<ID3D11Texture2D>             output_texture_rgb_;
        ComPtr<ID3D11Texture2D>             output_texture_rgba_;

        ComPtr<ID3D11Texture2D>             stage_texture_;

        ComPtr<ID3D11ShaderResourceView>    intput_texture_view_rgb_;

        ComPtr<ID3D11SamplerState>          sample_state_;

        ComPtr<ID3D11InputLayout>           input_layout_;
        ComPtr<ID3D11Buffer>                vertex_buffer_;
        ComPtr<ID3D11Buffer>                index_buffer_;

        ComPtr<ID3D11RenderTargetView>      yuv_to_rgb_rt_;
        ComPtr<ID3D11RenderTargetView>      alpha_mix_rt_;

        ComPtr<ID3D11VertexShader>          common_vs_;
        ComPtr<ID3D11PixelShader>           yuv_to_rgb_ps_;
        ComPtr<ID3D11PixelShader>           alpha_mix_ps_;
    };
}
}

#endif  // BILILIVE_MOTION_MP4_MOTION_H_