#include "mp4_motion.h"

#include "mp4_resource.h"

#include "d3dcompiler.h"

#include "media/iso/iso_constants.h"
#include "media/iso/iso_media_parser.h"
#include "media/iso/box/blpc_box.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "utils/unicode.h"

#include <future>
#include <thread>
#include <chrono>
#include <sstream>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

namespace livehime {
namespace motion {

    DirectX::XMFLOAT3 vertices[4] =
    {
        {-1.0f, 1.0f, 0.5f },
        {1.0f, 1.0f, 0.5f  },
        {1.0f, -1.0f, 0.5f },
        {-1.0f, -1.0f, 0.5f},
    };

    int indice[6] =
    {
        0, 1, 2,
        0, 2, 3
    };

    static int open_codec_context(int *stream_idx,
        AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
    {
        int ret, stream_index;
        AVStream *st;
        AVCodec *dec = NULL;
        AVDictionary *opts = NULL;

        ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
        if (ret < 0)
        {
            return ret;
        }
        else
        {
            stream_index = ret;
            st = fmt_ctx->streams[stream_index];

            dec = avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec)
            {
                return AVERROR(EINVAL);
            }

            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx) {
                return AVERROR(ENOMEM);
            }

            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
            {
                return ret;
            }

            av_dict_set(&opts, "refcounted_frames", "0", 0);
            if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
            {
                return ret;
            }
            *stream_idx = stream_index;
        }

        return 0;
    }

    MP4MotionImpl::MP4MotionImpl(Mp4PlayerCallback* callback) :
        callback_(callback)
    {
    }

    MP4MotionImpl::~MP4MotionImpl()
    {
        if (th_.native_handle() != nullptr)
        {
            th_.join();
        }

        Destroy();
    }

    bool MP4MotionImpl::InitD3D()
    {
        if (!is_initialize_called_)
        {
            is_initialize_called_ = true;
        }
        else
        {
            return d3d_device_.Get();
        }

        HRESULT hr = S_OK;

        // 驱动类型数组
        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        D3D_FEATURE_LEVEL featureLevel{};
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, d3d_device_.GetAddressOf(), &featureLevel, d3d_immediate_context_.GetAddressOf());

        if (hr == E_INVALIDARG)
        {
            // Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
            hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, d3d_device_.GetAddressOf(), &featureLevel, d3d_immediate_context_.GetAddressOf());
        }

        if (FAILED(hr))
        {
            d3d_device_.Reset();
            return false;
        }

        return true;
    }

    bool MP4MotionImpl::SetMp4File(const std::wstring& file_path)
    {
        if (!IsSupportMp4())
        {
            return false;
        }

        if (!ParseBLPCJson(file_path, vap_info_))
        {
            return false;
        }

        if (!InitResource())
        {
            return false;
        }

        Destroy();

        mp4_file_path_ = file_path;
        if (InitFfmpegFormFile())
        {
            interval_time_ = 1.0 / vap_info_.fps;
            mp4_file_type_ = kFile;

            return true;
        }
        else
        {
            mp4_file_path_ = L"";
            return false;
        }
    }

    bool MP4MotionImpl::SetMp4Buffer(const void* data, uint64_t data_len)
    {
        if (!IsSupportMp4())
        {
            return false;
        }

        if (!ParseBLPCJson(data, data_len, vap_info_))
        {
            return false;
        }

        if (!InitResource())
        {
            return false;
        }

        const uint8_t* u8_data = reinterpret_cast<const uint8_t*>(data);
        mp4_data_.insert(mp4_data_.end(), u8_data, u8_data + data_len);

        if (InitFfmpegFormBuffer())
        {
            interval_time_ = 1.0 / vap_info_.fps;
            mp4_file_type_ = kBuffer;
            return true;
        }
        else
        {
            mp4_data_.clear();
            return false;
        }
    }

    bool MP4MotionImpl::SetOutputSize(uint32_t width, uint32_t height)
    {
        output_width_ = width;
        output_height = height;
        return true;
    }

    bool MP4MotionImpl::GetOutputSize(uint32_t* width, uint32_t* height)
    {
        if (output_width_ == 0 ||
            output_height == 0)
        {
            *width = vap_info_.real_width;
            *height = vap_info_.real_height;
        }
        else
        {
            *width = output_width_;
            *height = output_height;
        }

        return true;
    }

    bool MP4MotionImpl::IsSupportMp4()
    {
        if (!is_initialize_called_)
        {
            return InitD3D();
        }

        return d3d_device_.Get();
    }

    bool MP4MotionImpl::Play()
    {
        if (!IsSupportMp4())
        {
            return false;
        }

        if (IsPlaying())
        {
            return false;
        }

        if (mp4_file_type_ == kNoMp4)
        {
            return false;
        }

        if (th_.native_handle() != nullptr)
        {
            std::async(std::launch::async, [this]()
                {
                    th_.join();
                    quit_play_ = false;
                    th_ = std::thread(&MP4MotionImpl::Mp4PlayThread, this);
                });
        }
        else
        {
            quit_play_ = false;
            th_ = std::thread(&MP4MotionImpl::Mp4PlayThread, this);
        }

        return true;
    }

    bool MP4MotionImpl::IsPlaying()
    {
        return !quit_play_;
    }

    bool MP4MotionImpl::PlayReady()
    {
        return context_ && video_decoder_context_;
    }

    void MP4MotionImpl::GetVideoSize(int* width, int* height)
    {
        *width = vap_info_.real_width;
        *height = vap_info_.real_height;
    }

    void MP4MotionImpl::Destroy()
    {
        if (!IsSupportMp4())
        {
            return;
        }

        if (video_decoder_context_)
        {
            avcodec_free_context(&video_decoder_context_);
            video_decoder_context_ = nullptr;
        }

        //io_buffer avcodec_free_context 会释放？
        io_buffer_ = nullptr;
        //if (io_buffer_)
        //{
        //    av_free(io_buffer_);
        //    io_buffer_ = nullptr;
        //}

        if (io_context_)
        {
            avio_context_free(&io_context_);
            io_context_ = nullptr;
        }

        if (context_)
        {
            avformat_free_context(context_);
            context_ = nullptr;
        }
    }

    bool MP4MotionImpl::parseBLPC(const std::wstring& file_name, BLPCInfo* blpc) {
        BLPCInfo blpc_info;
        if (!ParseBLPCJson(file_name, blpc_info)) {
            return false;
        }

        *blpc = blpc_info;
        return true;
    }

    HRESULT MP4MotionImpl::CreateShaderFromFile(const WCHAR* csoFileNameInOut, const WCHAR* hlslFileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlobOut)
    {
        HRESULT hr = S_OK;

        // 寻找是否有已经编译好的着色器
        if (csoFileNameInOut && D3DReadFileToBlob(csoFileNameInOut, ppBlobOut) == S_OK)
        {
            return hr;
        }
        else
        {
            DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
            //设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
            //但仍然允许着色器进行优化操作
            dwShaderFlags |= D3DCOMPILE_DEBUG;

            // 在Debug环境下禁用优化以避免出现一些不合理的情况
            dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ComPtr<ID3DBlob> errorBlob = nullptr;
            hr = D3DCompileFromFile(hlslFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel,
                dwShaderFlags, 0, ppBlobOut, &errorBlob);
            if (FAILED(hr))
            {
                if (errorBlob != nullptr)
                {
                    OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
                }
                return hr;
            }

            // 若指定了输出文件名，则将着色器二进制信息输出
            if (csoFileNameInOut)
            {
                return D3DWriteBlobToFile(*ppBlobOut, csoFileNameInOut, FALSE);
            }
        }

        return hr;
    }

    void MP4MotionImpl::Mp4PlayThread()
    {
        callback_->OnMp4PlayStart(this);

        while(!quit_play_)
        {
            AVPacket pkg;
            av_init_packet(&pkg);

            auto ret = av_read_frame(context_, &pkg);
            if (ret < 0)
            {
                av_packet_unref(&pkg);
                break;
            }

            ProcessPacket(pkg, vap_info_);

            av_packet_unref(&pkg);
        }

        ProcessRemainFrame();

        quit_play_ = true;

        Destroy();

        callback_->OnMp4PlayEnd(this);
    }

    bool MP4MotionImpl::InitResource()
    {
        uint32_t output_width{}, output_height{};
        GetOutputSize(&output_width, &output_height);
        if (output_width == 0 || output_height == 0)
        {
            return false;
        }

        TextureInfo text_info;
        text_info.rgb_text_rect = {
            (uint32_t)vap_info_.real_frame[0],
            (uint32_t)vap_info_.real_frame[1],
            (uint32_t)vap_info_.real_frame[0] + vap_info_.real_frame[2],
            (uint32_t)vap_info_.real_frame[1] + vap_info_.real_frame[3]
        };

        text_info.alpha_text_rect = {
            (uint32_t)vap_info_.alpha_frame[0],
            (uint32_t)vap_info_.alpha_frame[1],
            (uint32_t)vap_info_.alpha_frame[0] + vap_info_.alpha_frame[2],
            (uint32_t)vap_info_.alpha_frame[1] + vap_info_.alpha_frame[3]
        };

        text_info.yuv_text_size = { (uint32_t)vap_info_.video_width, (uint32_t)vap_info_.video_height };
        text_info.text_size = { (uint32_t)vap_info_.real_width, (uint32_t)vap_info_.real_height };

        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        buffer_desc.ByteWidth = sizeof(text_info);
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        D3D11_SUBRESOURCE_DATA init_data{};
        init_data.pSysMem = &text_info;
        if (FAILED(d3d_device_->CreateBuffer(&buffer_desc, &init_data, texture_info_const_buffer_.GetAddressOf())))
        {
            return false;
        }

        D3D11_TEXTURE2D_DESC texDesc;
        texDesc.Width = vap_info_.video_width;
        texDesc.Height = vap_info_.video_height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        if (FAILED(d3d_device_->CreateTexture2D(&texDesc, nullptr, input_texture_y_.GetAddressOf())))
        {
            return false;
        }

        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        if (FAILED(d3d_device_->CreateTexture2D(&texDesc, nullptr, output_texture_rgb_.GetAddressOf())))
        {
            return false;
        }
        texDesc.BindFlags &= ~D3D11_BIND_RENDER_TARGET;

        texDesc.Usage = D3D11_USAGE_STAGING;
        texDesc.Width = output_width;
        texDesc.Height = output_height;
        texDesc.BindFlags = 0;
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        if (FAILED(d3d_device_->CreateTexture2D(&texDesc, nullptr, stage_texture_.GetAddressOf())))
        {
            return false;
        }

        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.Format = DXGI_FORMAT_R8_UNORM;
        texDesc.Width = vap_info_.video_width / 2;
        texDesc.Height = vap_info_.video_height / 2;
        if (FAILED(d3d_device_->CreateTexture2D(&texDesc, nullptr, input_texture_u_.GetAddressOf())))
        {
            return false;
        }

        if (FAILED(d3d_device_->CreateTexture2D(&texDesc, nullptr, input_texture_v_.GetAddressOf())))
        {
            return false;
        }

        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        texDesc.Width = output_width;
        texDesc.Height = output_height;
        if (FAILED(d3d_device_->CreateTexture2D(&texDesc, nullptr, output_texture_rgba_.GetAddressOf())))
        {
            return false;
        }
        texDesc.BindFlags &= ~D3D11_BIND_RENDER_TARGET;

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = DXGI_FORMAT_R8_UNORM;
        srv_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        srv_desc.Texture2D.MostDetailedMip = 0;
        if (FAILED(d3d_device_->CreateShaderResourceView(input_texture_y_.Get(), &srv_desc, input_texture_view_y_.GetAddressOf())))
        {
            return false;
        }
        if (FAILED(d3d_device_->CreateShaderResourceView(input_texture_u_.Get(), &srv_desc, input_texture_view_u_.GetAddressOf())))
        {
            return false;
        }
        if (FAILED(d3d_device_->CreateShaderResourceView(input_texture_v_.Get(), &srv_desc, input_texture_view_v_.GetAddressOf())))
        {
            return false;
        }

        D3D11_RENDER_TARGET_VIEW_DESC rt_view_desc{};
        rt_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rt_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rt_view_desc.Texture2D.MipSlice = 0;
        if (FAILED(d3d_device_->CreateRenderTargetView(output_texture_rgb_.Get(), &rt_view_desc, yuv_to_rgb_rt_.GetAddressOf())))
        {
            return false;
        }

        if (FAILED(d3d_device_->CreateRenderTargetView(output_texture_rgba_.Get(), &rt_view_desc, alpha_mix_rt_.GetAddressOf())))
        {
            return false;
        }

        srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srv_desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        srv_desc.Texture2D.MostDetailedMip = 0;
        if (FAILED(d3d_device_->CreateShaderResourceView(output_texture_rgb_.Get(), &srv_desc, intput_texture_view_rgb_.GetAddressOf())))
        {
            return false;
        }

        D3D11_SAMPLER_DESC sample_state{};
        sample_state.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sample_state.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_state.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_state.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_state.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sample_state.MinLOD = 0;
        sample_state.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(d3d_device_->CreateSamplerState(&sample_state, sample_state_.GetAddressOf())))
        {
            return false;
        }

        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc.ByteWidth = sizeof(vertices) * 4;
        buffer_desc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA subresource_data;
        subresource_data.pSysMem = vertices;
        if (FAILED(d3d_device_->CreateBuffer(&buffer_desc, &subresource_data, &vertex_buffer_)))
        {
            return false;
        }

        buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        buffer_desc.ByteWidth = sizeof(int) * 6;
        subresource_data.pSysMem = indice;
        if (FAILED(d3d_device_->CreateBuffer(&buffer_desc, &subresource_data, &index_buffer_)))
        {
            return false;
        }

        ComPtr<ID3DBlob> blob = LoadShaderResource(IDR_COMMON_VS);
        if (blob->GetBufferSize() == 0)
        {
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        UINT num_elements = ARRAYSIZE(layout);
        if (FAILED(d3d_device_->CreateInputLayout(layout,
            num_elements,
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            &input_layout_)))
        {
            return false;
        }

        if (FAILED(d3d_device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, common_vs_.GetAddressOf())))
        {
            return false;
        }

        blob = LoadShaderResource(IDR_YUV_TO_RGB_PS);
        if (blob->GetBufferSize() == 0)
        {
            return false;
        }

        if (FAILED(d3d_device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, yuv_to_rgb_ps_.GetAddressOf())))
        {
            return false;
        }

        blob = LoadShaderResource(IDR_ALPHA_MIX_PS);
        if (blob->GetBufferSize() == 0)
        {
            return false;
        }

        if (FAILED(d3d_device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, alpha_mix_ps_.GetAddressOf())))
        {
            return false;
        }

        return true;
    }

    void MP4MotionImpl::UpdateD3DResource(AVFrame* yuv_frame)
    {
        d3d_immediate_context_->UpdateSubresource(input_texture_y_.Get(), 0, nullptr, yuv_frame->data[0], yuv_frame->linesize[0], 0);
        d3d_immediate_context_->UpdateSubresource(input_texture_u_.Get(), 0, nullptr, yuv_frame->data[1], yuv_frame->linesize[1], 0);
        d3d_immediate_context_->UpdateSubresource(input_texture_v_.Get(), 0, nullptr, yuv_frame->data[2], yuv_frame->linesize[2], 0);

        YuvToRgb();
    }

    void MP4MotionImpl::YuvToRgb()
    {
        uint32_t output_width{}, output_height{};
        GetOutputSize(&output_width, &output_height);
        if (output_width == 0 || output_height == 0)
        {
            return;
        }

        UINT stride = sizeof(DirectX::XMFLOAT3);
        UINT offset = 0;

        ID3D11Buffer* in[] = { vertex_buffer_.Get() };
        d3d_immediate_context_->IASetVertexBuffers(0, 1, in, &stride, &offset);
        d3d_immediate_context_->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
        d3d_immediate_context_->VSSetShader(common_vs_.Get(), nullptr, 0);
        d3d_immediate_context_->IASetInputLayout(input_layout_.Get());
        d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D11_VIEWPORT vp;
        vp.Height = (FLOAT)vap_info_.video_height;
        vp.MaxDepth = 1.0f;
        vp.MinDepth = 0.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = (FLOAT)vap_info_.video_width;
        d3d_immediate_context_->RSSetViewports(1, &vp);

        d3d_immediate_context_->PSSetShader(yuv_to_rgb_ps_.Get(), nullptr, 0);
        d3d_immediate_context_->OMSetRenderTargets(1, yuv_to_rgb_rt_.GetAddressOf(), nullptr);

        d3d_immediate_context_->PSSetConstantBuffers(0, 1, texture_info_const_buffer_.GetAddressOf());

        d3d_immediate_context_->PSSetShaderResources(0, 1, input_texture_view_y_.GetAddressOf());
        d3d_immediate_context_->PSSetShaderResources(1, 1, input_texture_view_u_.GetAddressOf());
        d3d_immediate_context_->PSSetShaderResources(2, 1, input_texture_view_v_.GetAddressOf());

        d3d_immediate_context_->PSSetSamplers(0, 1, sample_state_.GetAddressOf());
        d3d_immediate_context_->DrawIndexed(6, 0, 0);


        vp.Height = (FLOAT)vap_info_.real_height;
        vp.MaxDepth = 1.0f;
        vp.MinDepth = 0.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = (FLOAT)vap_info_.real_width;
        d3d_immediate_context_->RSSetViewports(1, &vp);

        d3d_immediate_context_->PSSetShader(alpha_mix_ps_.Get(), nullptr, 0);
        d3d_immediate_context_->OMSetRenderTargets(1, alpha_mix_rt_.GetAddressOf(), nullptr);
        d3d_immediate_context_->PSSetShaderResources(0, 1, intput_texture_view_rgb_.GetAddressOf());

        d3d_immediate_context_->DrawIndexed(6, 0, 0);

        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        d3d_immediate_context_->PSSetShaderResources(0, 1, nullSRV);

        d3d_immediate_context_->CopyResource(stage_texture_.Get(), output_texture_rgba_.Get());
    }

    void MP4MotionImpl::CallbackBgraFrame()
    {
        if (!callback_)
        {
            return;
        }

        uint32_t output_width{}, output_height{};
        GetOutputSize(&output_width, &output_height);
        if (output_width == 0 || output_height == 0)
        {
            return;
        }

        D3D11_MAPPED_SUBRESOURCE map_resource{};
        d3d_immediate_context_->Map(stage_texture_.Get(), 0, D3D11_MAP_READ, 0, &map_resource);

        timer_.Tick();
        double delta_time = timer_.DeltaTime();
        if (delta_time < interval_time_)
        {
            int sleep_time_ms = (interval_time_ - delta_time) * 1000;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));
            timer_.Tick();
        }

        callback_->OnMp4PlayFrameCallback(this, map_resource.pData, output_width, output_height, map_resource.RowPitch);

        d3d_immediate_context_->Unmap(stage_texture_.Get(), 0);
    }

    void MP4MotionImpl::ProcessRemainFrame()
    {
        int  ret = avcodec_send_packet(video_decoder_context_, nullptr);
        if (ret < 0)
        {
            return;
        }

        do
        {
            auto frame = av_frame_alloc();
            int ret = avcodec_receive_frame(video_decoder_context_, frame);
            if (ret < 0)
            {
                av_frame_free(&frame);
                break;
            }

            UpdateD3DResource(frame);

            CallbackBgraFrame();

            av_frame_free(&frame);
        }
        while (ret == 0);

    }

    bool MP4MotionImpl::InitFfmpegFormFile()
    {
        auto u8_file_name = utl::UTF16ToUTF8(mp4_file_path_);

        context_ = avformat_alloc_context();
        int ret = avformat_open_input(&context_, u8_file_name.c_str(), nullptr, nullptr);
        if (ret != 0)
        {
            goto error;
        }

        ret = open_codec_context(&stream_video_index_, &video_decoder_context_, context_, AVMEDIA_TYPE_VIDEO);
        if (ret != 0)
        {
            goto error;
        }

        return true;

    error:
        Destroy();
        return false;
    }

    int MP4MotionImpl::ReadPacket(void* opaque, uint8_t* buf, int buf_size)
    {
        MP4MotionImpl* mp4_impl = reinterpret_cast<MP4MotionImpl*>(opaque);

        int64_t remain_len = mp4_impl->mp4_data_.size() - mp4_impl->mp4_data_curpos_;
        int64_t copy_len = (std::min)(remain_len, static_cast<int64_t>(buf_size));
        if (copy_len <= 0)
        {
            return 0;
        }

        ::memcpy(buf, mp4_impl->mp4_data_.data() + mp4_impl->mp4_data_curpos_, copy_len);
        mp4_impl->mp4_data_curpos_ += copy_len;

        return copy_len;
    }

    Microsoft::WRL::ComPtr<ID3DBlob> MP4MotionImpl::LoadShaderResource(int resource_id)
    {
        ComPtr<ID3DBlob> blob;
        auto module = GetModuleHandle(L"bililive_motion.dll");
        if (!module)
        {
            return {};
        }

        auto hres = ::FindResource(module, MAKEINTRESOURCE(resource_id), L"SHADER");
        if (hres)
        {
            auto res_size = ::SizeofResource(module, hres);
            if (SUCCEEDED(::D3DCreateBlob(res_size, &blob)))
            {
                void* res_data = ::LockResource(::LoadResource(module, hres));
                ::memcpy(blob->GetBufferPointer(), res_data, res_size);
                return blob;
            }
        }
        else
        {
            return {};
        }

        return {};
    }

    bool MP4MotionImpl::InitFfmpegFormBuffer()
    {
        context_ = avformat_alloc_context();

        io_buffer_ = reinterpret_cast<uint8_t*>(av_malloc(1920 * 1080 * 4));
        io_context_ = avio_alloc_context(io_buffer_, 1920 * 1080 * 4, 0, this, &MP4MotionImpl::ReadPacket, nullptr, nullptr);
        context_ = avformat_alloc_context();
        context_->pb = io_context_;
        context_->flags = AVFMT_FLAG_CUSTOM_IO;

        int ret = avformat_open_input(&context_, nullptr, nullptr, nullptr);
        if (ret != 0)
        {
            goto error;
        }

        ret = open_codec_context(&stream_video_index_, &video_decoder_context_, context_, AVMEDIA_TYPE_VIDEO);
        if (ret != 0)
        {
            goto error;
        }

        mp4_data_curpos_ = 0;

        return true;

    error:
        Destroy();
        return false;
    }

    // https://docs.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
    bool MP4MotionImpl::ProcessPacket(const AVPacket& packet, const BLPCInfo& info)
    {
        int ret = avcodec_send_packet(video_decoder_context_, &packet);
        if (ret != 0)
        {
            return false;
        }

        for (;;)
        {
            auto frame = av_frame_alloc();
            int rf_ret = avcodec_receive_frame(video_decoder_context_, frame);
            if (rf_ret == AVERROR(EAGAIN))
            {
                av_frame_free(&frame);
                break;
            }

            if (rf_ret < 0)
            {
                av_frame_free(&frame);
                break;
            }

            UpdateD3DResource(frame);

            CallbackBgraFrame();

            av_frame_free(&frame);
        }

        return true;
    }

    bool MP4MotionImpl::ParseBLPCJson(const std::wstring& file_name, BLPCInfo& vap_info) const
    {

        std::ifstream file(file_name, std::ios::binary);
        if (!!file)
        {
            return ParseBLPCJson(file, vap_info);
        }

        return false;
    }

    bool MP4MotionImpl::ParseBLPCJson(const void* data, uint64_t data_len, BLPCInfo& vap_info) const
    {
        std::string blpc_json;
        std::string ss_input{ reinterpret_cast<const char*>(data), static_cast<uint32_t>(data_len) };
        std::stringstream ss{ ss_input };

        return ParseBLPCJson(ss, vap_info);
    }

    bool MP4MotionImpl::ParseBLPCJson(std::istream& s, BLPCInfo& vap_info) const
    {
        std::string blpc_json;
        media::iso::ISOMediaParser parser(s);
        parser.parse();

        for (const auto& box : parser.getBoxes()) {
            if (box->type == media::iso::kBoxType_blpc) {
                auto blpc_box = static_cast<media::iso::BLPCBox*>(box.get());
                blpc_json.append(
                    reinterpret_cast<const char*>(blpc_box->data.get()),
                    blpc_box->data_count);
            }
        }

        if (blpc_json.empty()) {
            return false;
        }

        std::string json = std::move(blpc_json);

        rapidjson::Document doc;
        doc.Parse(json.c_str());
        if (!doc.HasParseError())
        {
            if (!doc.ObjectEmpty())
            {
                rapidjson::Value& info = doc["info"];
                if (!info.ObjectEmpty())
                {
                    rapidjson::Value& value = info["v"];
                    if (!value.IsNull())
                    {
                        vap_info.version = value.GetInt();
                    }

                    value = info["f"];
                    if (!value.IsNull())
                    {
                        vap_info.total_fps = value.GetInt();
                    }

                    value = info["w"];
                    if (!value.IsNull())
                    {
                        vap_info.real_width = value.GetInt();
                    }

                    value = info["h"];
                    if (!value.IsNull())
                    {
                        vap_info.real_height = value.GetInt();
                    }

                    value = info["fps"];
                    if (!value.IsNull())
                    {
                        vap_info.fps = value.GetInt();
                    }

                    value = info["videoW"];
                    if (!value.IsNull())
                    {
                        vap_info.video_width = value.GetInt();
                    }

                    value = info["videoH"];
                    if (!value.IsNull())
                    {
                        vap_info.video_height = value.GetInt();
                    }

                    value = info["aFrame"];
                    if (!value.Empty())
                    {
                        auto a_frame_array = value.GetArray();
                        if (a_frame_array.Size() == 4)
                        {
                            int i = 0;
                            for (auto iter = a_frame_array.begin(); iter != a_frame_array.end(); ++iter)
                            {
                                vap_info.alpha_frame[i] = iter->GetInt();
                                i++;
                            }
                        }
                    }

                    value = info["rgbFrame"];
                    if (!value.Empty())
                    {
                        auto frame_array = value.GetArray();
                        if (frame_array.Size() == 4)
                        {
                            int i = 0;
                            for (auto iter = frame_array.begin(); iter != frame_array.end(); ++iter)
                            {
                                vap_info.real_frame[i] = iter->GetInt();
                                i++;
                            }
                        }
                    }
                }
            }
        }

        return true;
    }
}
}
