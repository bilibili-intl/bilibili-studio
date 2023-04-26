#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/win/windows_version.h"

#include "ui/gfx/image/image_skia.h"

#include "bililive/bililive/utils/image_util.h"
#include "bililive/bililive/utils/bililive_image_util.h"
//#include "bililive/bililive/livehime/vtuber/ffmpeg_scaler.h"
#include "ui/base/win/dpi.h"
#include "SkPixelRef.h"

#include <wincodecsdk.h>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")


D2DManager::D2DManager()
    : dpi_x_(96.f),
      dpi_y_(96.f)
{
    local_cache_.clear();
    local_gif_cache_.clear();
    LOG(INFO) << "== Initializing Direct2D and WIC... ==";

    HRESULT hr = ::D2D1CreateFactory(
        D2D1_FACTORY_TYPE_MULTI_THREADED,
        d2d_factory_.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create Direct2D factory: " << hr;
        return;
    }

    d2d_factory_->GetDesktopDpi(&dpi_x_, &dpi_y_);

    hr = ::DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(dwrite_factory_.Receive()));
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create DWrite factory: " << hr;
        return;
    }

    hr = dwrite_factory_->CreateTextFormat(
        L"微软雅黑", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        15, L"zh-CN", text_format_.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create TextFormat: " << hr;
        return;
    }

    hr = CreateD3DDevice();
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << L"Failed to create d3d device.";
    }

    if (base::win::GetVersion() >= base::win::VERSION_WIN8) {
        hr = ::CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(wic_factory_.Receive()));
        if (FAILED(hr)) {
            LOG(INFO) << "Failed to create WICImageFactory2 on Win8 or above system: "
                << hr << ". Fallback to WICImageFactory1";

            hr = ::CoCreateInstance(
                CLSID_WICImagingFactory1,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(wic_factory_.Receive()));
            if (FAILED(hr)) {
                DCHECK(false);
                LOG(WARNING) << "Failed to create WICImageFactory: " << hr;
                return;
            }
        }
    } else {
        hr = ::CoCreateInstance(
            CLSID_WICImagingFactory1,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(wic_factory_.Receive()));
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create WICImageFactory: " << hr;
            return;
        }
    }

    LOG(INFO) << "== Finish initializing Direct2D and WIC. ==";
}

D2DManager::~D2DManager()
{
}

HRESULT D2DManager::CreateD3DDevice() {
    D3D_FEATURE_LEVEL featureLevel[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    d3d_device_.Release();
    d3d_devicecontext_.Release();

    return ::D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevel, 3, D3D11_SDK_VERSION,
        d3d_device_.Receive(), nullptr, d3d_devicecontext_.Receive());
}


D2DManager* D2DManager::GetInstance() {
    return Singleton<D2DManager>::get();
}

float D2DManager::GetDpiX() const {
    return dpi_x_;
}

float D2DManager::GetDpiY() const {
    return dpi_y_;
}

bool D2DManager::ProcessDeviceError(HRESULT hr) {
    /**
     * 在设备丢失时，每一个 DanmakuRenderView 保有的设备相关资源都失效了，
     * 所以在它们下次 EndDraw() 时每个 View 都会报告错误。我们只需要在第一次报告错误时
     * 重新创建设备就行了，之后的错误应该直接返回 true 让它们重试即可。这里看下当前设备的
     * 状态，防止重复创建设备。
     */
    hr = d3d_device_->GetDeviceRemovedReason();
    if (SUCCEEDED(hr)) {
        return true;
    }

    switch (hr) {
    case DXGI_ERROR_DEVICE_REMOVED:
    case DXGI_ERROR_DEVICE_RESET:
    {
        // 重新创建设备
        HRESULT result = CreateD3DDevice();
        if (SUCCEEDED(result)) {
            return true;
        }
        break;
    }

    default:
        break;
    }

    return false;
}

void D2DManager::EnumSystemFonts() {
    base::win::ScopedComPtr<IDWriteFontCollection> collection;
    HRESULT hr = dwrite_factory_->GetSystemFontCollection(collection.Receive());
    if (SUCCEEDED(hr)) {
        auto count = collection->GetFontFamilyCount();
        for (UINT32 i = 0; i < count; ++i) {
            base::win::ScopedComPtr<IDWriteFontFamily> family;
            hr = collection->GetFontFamily(i, family.Receive());
            if (SUCCEEDED(hr)) {
                base::win::ScopedComPtr<IDWriteLocalizedStrings> strings;
                hr = family->GetFamilyNames(strings.Receive());
                if (SUCCEEDED(hr)) {
                    auto font_count = strings->GetCount();
                    for (UINT32 j = 0; j < font_count; ++j) {
                        UINT32 length;
                        hr = strings->GetStringLength(j, &length);
                        if (SUCCEEDED(hr)) {
                            WCHAR* buffer = new WCHAR[length + 1];
                            hr = strings->GetString(j, buffer, length + 1);
                            if (SUCCEEDED(hr)) {
                                ::OutputDebugStringW(buffer);
                                ::OutputDebugStringW(L"\n");
                            }

                            delete[] buffer;
                        }
                    }
                }
            }
        }
    }
}

base::win::ScopedComPtr<ID2D1PathGeometry> D2DManager::CreatePathGeometry() {
    base::win::ScopedComPtr<ID2D1PathGeometry> geo;
    if (d2d_factory_) {
        HRESULT hr = d2d_factory_->CreatePathGeometry(geo.Receive());
        if(FAILED(hr)) {
            DCHECK(false);
            return {};
        }
    }

    return geo;
}

base::win::ScopedComPtr<IWICBitmap> D2DManager::CreateWICBitmap(int width, int height) {
    base::win::ScopedComPtr<IWICBitmap> bitmap;
    if (wic_factory_) {
        HRESULT hr = wic_factory_->CreateBitmap(
            width, height,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnLoad,
            bitmap.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create WICBitmap: " << hr
                << ", Width: " << width << ", Height: " << height;
            return {};
        }
    }

    return bitmap;
}

base::win::ScopedComPtr<IWICBitmap> D2DManager::CreateWICBitmap(int width, int height, void* data) {
    base::win::ScopedComPtr<IWICBitmap> bitmap;
    if (wic_factory_) {
        HRESULT hr = wic_factory_->CreateBitmapFromMemory(
            width, height,
            GUID_WICPixelFormat32bppPBGRA,
            width * 4, width * height * 4,
            static_cast<BYTE*>(data),
            bitmap.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create WICBitmap: " << hr
                << ", Width: " << width << ", Height: " << height;
            return {};
        }
    }

    return bitmap;
}

base::win::ScopedComPtr<ID2D1RenderTarget> D2DManager::CreateWICRenderTarget(
    IWICBitmap* wic_bitmap, bool dpi_awareness) {

    base::win::ScopedComPtr<ID2D1RenderTarget> render_target;

    if (d2d_factory_) {
        float dpi_x = dpi_awareness ? dpi_x_ : 96.f;
        float dpi_y = dpi_awareness ? dpi_y_ : 96.f;

        const D2D1_PIXEL_FORMAT format =
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

        const D2D1_RENDER_TARGET_PROPERTIES properties
            = D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                format, dpi_x, dpi_y,
                D2D1_RENDER_TARGET_USAGE_NONE);

        HRESULT hr = d2d_factory_->CreateWicBitmapRenderTarget(
            wic_bitmap, properties, render_target.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create WICBitmap RenderTarget: " << hr
                << ", DpiX: " << dpi_x << ", DpiY: " << dpi_y;
            return {};
        }
    }

    return render_target;
}

base::win::ScopedComPtr<ID2D1DCRenderTarget> D2DManager::CreateDCRenderTarget() {
    base::win::ScopedComPtr<ID2D1DCRenderTarget> render_target;

    if (d2d_factory_) {
        const D2D1_PIXEL_FORMAT format =
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED);

        // DPI 固定为 96
        const D2D1_RENDER_TARGET_PROPERTIES properties =
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                format);

        HRESULT hr = d2d_factory_->CreateDCRenderTarget(
            &properties,
            render_target.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create DC RenderTarget: " << hr;
            return {};
        }
    }

    return render_target;
}

base::win::ScopedComPtr<IDXGISurface> D2DManager::CreateDXGISurface(int width, int height) {
    if (!d3d_device_) {
        return {};
    }

    D3D11_TEXTURE2D_DESC tex_desc = { 0 };
    tex_desc.ArraySize = 1;
    tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    tex_desc.Width = width;
    tex_desc.Height = height;
    tex_desc.MipLevels = 1;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

    base::win::ScopedComPtr<ID3D11Texture2D> d3d_texture;
    HRESULT hr = d3d_device_->CreateTexture2D(&tex_desc, nullptr, d3d_texture.Receive());
    if (FAILED(hr)) {
        if (hr != DXGI_ERROR_DEVICE_REMOVED
            && hr != DXGI_ERROR_DEVICE_RESET) {
            DCHECK(false);
        }
        return {};
    }

    base::win::ScopedComPtr<IDXGISurface> dxgi_surface;
    hr = d3d_texture.QueryInterface(dxgi_surface.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        return {};
    }

    return dxgi_surface;
}

base::win::ScopedComPtr<ID2D1RenderTarget> D2DManager::CreateHardwareRenderTarget(
    IDXGISurface* surface, bool dpi_awareness)
{
    float dpi_x = dpi_awareness ? dpi_x_ : 96.f;
    float dpi_y = dpi_awareness ? dpi_y_ : 96.f;

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_HARDWARE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpi_x, dpi_y, D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

    base::win::ScopedComPtr<ID2D1RenderTarget> render_target;
    HRESULT hr = d2d_factory_->CreateDxgiSurfaceRenderTarget(surface, props, render_target.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        return {};
    }

    return render_target;
}

base::win::ScopedComPtr<IDWriteTextLayout> D2DManager::CreateTextLayout(
    const base::string16& text, int max_width, int max_height)
{
    base::win::ScopedComPtr<IDWriteTextLayout> text_layout;
    if (dwrite_factory_) {
        HRESULT hr = dwrite_factory_->CreateTextLayout(
            text.c_str(), text.length(), text_format_.get(), max_width, max_height, text_layout.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create TextLayout: " << hr
                << ", Text: " << text << ", MaxWidth: " << max_width << ", MaxHeight: " << max_height;
            return {};
        }
    }

    return text_layout;
}

base::win::ScopedComPtr<IDWriteInlineObject> D2DManager::CreateEllipsisTrimmingSign() {
    base::win::ScopedComPtr<IDWriteInlineObject> inline_object;
    if (dwrite_factory_) {
        HRESULT hr = dwrite_factory_->CreateEllipsisTrimmingSign(
            text_format_.get(), inline_object.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create EllipsisTrimmingSign: " << hr;
            return {};
        }
    }

    return inline_object;
}


base::win::ScopedComPtr<IWICBitmapSource> D2DManager::DecodeImage(const string16& file_name) {
    if (!wic_factory_) {
        return {};
    }

    base::win::ScopedComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = wic_factory_->CreateDecoderFromFilename(
        file_name.c_str(),                     // Image to be decoded
        nullptr,                               // Do not prefer a particular vendor
        GENERIC_READ,                          // Desired read access to the file
        WICDecodeMetadataCacheOnDemand,        // Cache metadata when needed
        decoder.Receive());                    // Pointer to the decoder
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create decoder from file: " << hr
            << ", File name: " << file_name;
        return {};
    }

    return ProcessDecoder(decoder);
}

base::win::ScopedComPtr<IWICBitmapSource> D2DManager::DecodeImage(const BYTE* buffer, size_t size) {
    if (!wic_factory_) {
        return {};
    }

    base::win::ScopedComPtr<IWICStream> stream;
    HRESULT hr = wic_factory_->CreateStream(stream.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create image stream: " << hr
            << ", File size: " << size;
        return {};
    }

    hr = stream->InitializeFromMemory(const_cast<BYTE*>(buffer), size);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to init from memory: " << hr
            << ", File size: " << size;
        return {};
    }

    base::win::ScopedComPtr<IWICBitmapDecoder> decoder;
    hr = wic_factory_->CreateDecoderFromStream(
        stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create decoder from stream: " << hr
            << ", File size: " << size;
        return {};
    }

    return ProcessDecoder(decoder);
}

base::win::ScopedComPtr<IWICBitmapSource> D2DManager::DecodeImageFromData(const std::string& data, bool is_cache) {
    if (is_cache) {
        auto itr = local_cache_.find(data);
        if (itr != local_cache_.end()) {
            //LOG(INFO) << "## DecodeImageFromData get cache image";
            return itr->second;
        }
        //LOG(INFO) << "## DecodeImageFromData add new image to cache";
        auto decode_image = DecodeImage(reinterpret_cast<const BYTE*>(data.data()), data.size());
        local_cache_[data] = decode_image;
        return decode_image;
    }
    else {
        return DecodeImage(reinterpret_cast<const BYTE*>(data.data()), data.size());
    }
}

base::win::ScopedComPtr<IWICBitmapSource> D2DManager::DecodePngImage(const std::string& buffer)
{
    auto img_skia = bililive::MakeSkiaImage(
        reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size());

    if (img_skia.isNull()) {
        return {};
    }

    auto& rep = img_skia.GetRepresentation(ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()));
    if (rep.is_null()) {
        return {};
    }

    auto bitmap = CreateWICBitmap(
        rep.sk_bitmap().width(), rep.sk_bitmap().height(), rep.sk_bitmap().pixelRef()->pixels());
    if (!bitmap) {
        return {};
    }

    base::win::ScopedComPtr<IWICBitmapSource> source;
    HRESULT hr = bitmap.QueryInterface(source.Receive());
    if (FAILED(hr)) {
        return {};
    }
    return source;
}

base::win::ScopedComPtr<IWICBitmapSource> D2DManager::DecodePngImageFromData(const std::string& buffer, bool is_cache)
{
    if (is_cache) {
        auto itr = local_cache_.find(buffer);
        if (itr != local_cache_.end()) {
            //LOG(INFO) << "## DecodePngImageFromData get cache image";
            return itr->second;
        }
        //LOG(INFO) << "## DecodePngImageFromData  add new image to cache";
        auto decode_image = DecodePngImage(buffer);
        local_cache_[buffer] = decode_image;
        return decode_image;
    }
    else {
        return DecodePngImage(buffer);
    }
}

WICBitmaps D2DManager::DecodeGifFromData(const std::string& data, bool is_cache)
{
    if (is_cache) {
        auto itr = local_gif_cache_.find(data);
        if (itr != local_gif_cache_.end()) {
            //LOG(INFO) << "## DecodeGifFromData get cache image";
            return itr->second;
        }

        //LOG(INFO) << "## DecodeGifFromData add new image to cache";
        auto decode_image = WICBitmapsRead::GetInstance()->DecodeImage(reinterpret_cast<const BYTE*>(data.data()), data.size());
        local_gif_cache_[data] = decode_image;
        return decode_image;
    }
    else {
        return WICBitmapsRead::GetInstance()->DecodeImage(reinterpret_cast<const BYTE*>(data.data()), data.size());
    }
}

int D2DManager::ScaleImage(
    const string16& src_file_name, const string16& dst_file_name, int width, int height)
{
    if (!wic_factory_) {
        return -1;
    }

    base::win::ScopedComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = wic_factory_->CreateDecoderFromFilename(
        src_file_name.c_str(),                     // Image to be decoded
        nullptr,                               // Do not prefer a particular vendor
        GENERIC_READ,                          // Desired read access to the file
        WICDecodeMetadataCacheOnDemand,        // Cache metadata when needed
        decoder.Receive());                    // Pointer to the decoder
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create decoder from file: " << hr
            << ", File name: " << src_file_name;
        return -1;
    }

    UINT count;
    hr = decoder->GetFrameCount(&count);
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to get frame count: " << hr;
        return -1;
    }

    // 图片小于指定大小的话，不用压缩
    {
        if (count == 0) {
            return 0;
        }

        base::win::ScopedComPtr<IWICBitmapFrameDecode> frame_decode;
        hr = decoder->GetFrame(0, frame_decode.Receive());
        if (SUCCEEDED(hr)) {
            UINT frame_width, frame_height;
            hr = frame_decode->GetSize(&frame_width, &frame_height);
            if (SUCCEEDED(hr)) {
                if (frame_width <= UINT(width) && frame_height <= UINT(height)) {
                    return 0;
                }
            }
        }
    }

    GUID container_format;
    hr = decoder->GetContainerFormat(&container_format);
    if (FAILED(hr)) {
        return -1;
    }

    base::win::ScopedComPtr<IWICStream> stream;
    hr = wic_factory_->CreateStream(stream.Receive());
    if (FAILED(hr)) {
        return -1;
    }

    hr = stream->InitializeFromFilename(dst_file_name.c_str(), GENERIC_WRITE);
    if (FAILED(hr)) {
        return -1;
    }

    base::win::ScopedComPtr<IWICBitmapEncoder> encoder;
    hr = wic_factory_->CreateEncoder(container_format, nullptr, encoder.Receive());
    if (FAILED(hr)) {
        return -1;
    }

    hr = encoder->Initialize(stream.get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) {
        return -1;
    }

    for (UINT i = 0; i < count; ++i) {
        base::win::ScopedComPtr<IWICBitmapFrameDecode> frame_decode;
        hr = decoder->GetFrame(i, frame_decode.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to get frame " << i << ": " << hr;
            continue;
        }

        // 创建帧编码器
        base::win::ScopedComPtr<IWICBitmapFrameEncode> frame_encode;
        hr = encoder->CreateNewFrame(frame_encode.Receive(), nullptr);
        if (FAILED(hr)) {
            continue;
        }

        hr = frame_encode->Initialize(nullptr);
        if (FAILED(hr)) {
            continue;
        }

        // 压缩
        if (!ScaleFrame(frame_decode, frame_encode, width, height)) {
            hr = frame_encode->WriteSource(frame_decode.get(), nullptr);
            if (FAILED(hr)) {
                continue;
            }
        }

        hr = frame_encode->Commit();
        if (FAILED(hr)) {
            continue;
        }
    }

    hr = encoder->Commit();
    if (FAILED(hr)) {
        return -1;
    }

    return 1;
}

bool D2DManager::LookImage(const string16& file_name, uint32_t x, uint32_t y, uint32_t* color) {
    if (!wic_factory_) {
        return false;
    }

    base::win::ScopedComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = wic_factory_->CreateDecoderFromFilename(
        file_name.c_str(),                     // Image to be decoded
        nullptr,                               // Do not prefer a particular vendor
        GENERIC_READ,                          // Desired read access to the file
        WICDecodeMetadataCacheOnDemand,        // Cache metadata when needed
        decoder.Receive());                    // Pointer to the decoder
    if (FAILED(hr)) {
        return false;
    }

    // Retrieve the first frame of the image from the decoder
    base::win::ScopedComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, frame.Receive());
    if (FAILED(hr)) {
        return false;
    }

    base::win::ScopedComPtr<IWICBitmapSource> source;
    hr = frame.QueryInterface(source.Receive());
    if (FAILED(hr)) {
        return false;
    }

    UINT frame_width, frame_height;
    hr = frame->GetSize(&frame_width, &frame_height);

    // 假定是 8:8:8:8
    std::unique_ptr<uint32_t> buf(new uint32_t[frame_width * frame_height]);
    hr = source->CopyPixels(
        nullptr, frame_width * 4, frame_width * frame_height * 4, reinterpret_cast<BYTE*>(buf.get()));
    if (FAILED(hr)) {
        return false;
    }

    *color = buf.get()[y * frame_width + x];
    return true;
}

base::win::ScopedComPtr<IWICBitmapSource> D2DManager::ProcessDecoder(
    base::win::ScopedComPtr<IWICBitmapDecoder> decoder) {

    // Retrieve the first frame of the image from the decoder
    base::win::ScopedComPtr<IWICBitmapFrameDecode> frame;
    HRESULT hr = decoder->GetFrame(0, frame.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to get frame: " << hr;
        return {};
    }

    base::win::ScopedComPtr<IWICFormatConverter> converter;
    // Format convert the frame to 32bppPBGRA
    hr = wic_factory_->CreateFormatConverter(converter.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create converter: " << hr;
        return {};
    }

    base::win::ScopedComPtr<IWICBitmapSource> source;
    hr = frame.QueryInterface(source.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to query bitmap source from frame: " << hr;
        return {};
    }

    hr = converter->Initialize(
        source.get(),                          // Input bitmap to convert
        GUID_WICPixelFormat32bppPBGRA,         // Destination pixel format
        WICBitmapDitherTypeNone,               // Specified dither pattern
        nullptr,                               // Specify a particular palette
        0.f,                                   // Alpha threshold
        WICBitmapPaletteTypeCustom             // Palette translation type
    );
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to init converter: " << hr;
        return {};
    }

    base::win::ScopedComPtr<IWICBitmapSource> wic_bitmap;
    hr = converter.QueryInterface(wic_bitmap.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to get bitmap source: " << hr;
        return {};
    }

    return wic_bitmap;
}

bool D2DManager::ScaleFrame(
    const base::win::ScopedComPtr<IWICBitmapFrameDecode>& frame_decode,
    const base::win::ScopedComPtr<IWICBitmapFrameEncode>& frame_encode,
    UINT de_width, UINT de_height)
{
    UINT frame_width, frame_height;
    HRESULT hr = frame_decode->GetSize(&frame_width, &frame_height);
    if (FAILED(hr)) {
        return false;
    }

    WICPixelFormatGUID src_format;
    hr = frame_decode->GetPixelFormat(&src_format);
    if (FAILED(hr)) {
        return false;
    }

    if (frame_width <= de_width && frame_height <= de_height) {
        return false;
    }

    base::win::ScopedComPtr<IWICFormatConverter> converter;
    {
        hr = wic_factory_->CreateFormatConverter(converter.Receive());
        if (FAILED(hr)) {
            return false;
        }

        hr = converter->Initialize(
            frame_decode,
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr, 0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) {
            return false;
        }
    }

    // 压缩
    UINT out_width, out_height;
    {
        double sw = frame_width / double(de_width);
        double sh = frame_height / double(de_height);
        double scale = std::max(sw, sh);

        out_width = UINT(frame_width / scale);
        out_height = UINT(frame_height / scale);
    }

    base::win::ScopedComPtr<IWICBitmap> bmp;
    hr = wic_factory_->CreateBitmapFromSource(converter, WICBitmapCacheOnDemand, bmp.Receive());
    if (FAILED(hr)) {
        return false;
    }

    WICRect lock_rect = { 0, 0, frame_width, frame_height };
    base::win::ScopedComPtr<IWICBitmapLock> bmp_lock;
    hr = bmp->Lock(&lock_rect, WICBitmapLockRead, bmp_lock.Receive());
    if (FAILED(hr)) {
        return false;
    }

    UINT data_size;
    WICInProcPointer pixels;
    hr = bmp_lock->GetDataPointer(&data_size, &pixels);
    if (FAILED(hr)) {
        return false;
    }

    UINT data_stride;
    hr = bmp_lock->GetStride(&data_stride);
    if (FAILED(hr)) {
        return false;
    }

    src_format = GUID_WICPixelFormat32bppBGRA;
    hr = frame_encode->SetPixelFormat(&src_format);
    if (FAILED(hr)) {
        return false;
    }

    hr = frame_encode->SetSize(out_width, out_height);
    if (FAILED(hr)) {
        return false;
    }

    UINT buf_size = out_width * out_height * 4;

    std::unique_ptr<BYTE> buf(new BYTE[buf_size]);
    auto buf_ptr = buf.get();
    //bool scale_ret = ffsc::FFmpegScale(
    //    frame_width, frame_height, AV_PIX_FMT_BGRA, pixels, data_stride,
    //    out_width, out_height, reinterpret_cast<void**>(&buf_ptr));

    //if (!scale_ret) {
    //    return false;
    //}

    hr = frame_encode->WritePixels(
        out_height, out_width * 4, buf_size, buf.get());
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

base::win::ScopedComPtr<IWICImagingFactory> D2DManager::GetWicFactory() {
    return wic_factory_;
}

base::win::ScopedComPtr<ID2D1Factory> D2DManager::GetD2DFactory() {
    return d2d_factory_;
}