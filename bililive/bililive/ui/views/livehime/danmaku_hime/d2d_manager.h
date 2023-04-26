#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_D2D_MANAGER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_D2D_MANAGER_H_

#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <wincodec.h>

#include <memory>
#include <map>
#include "base/memory/singleton.h"
#include "base/strings/string16.h"
#include "base/win/scoped_comptr.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"

class D2DManager {
public:
    static D2DManager* GetInstance();

    bool ProcessDeviceError(HRESULT hr);

    float GetDpiX() const;
    float GetDpiY() const;

    void EnumSystemFonts();

    base::win::ScopedComPtr<IWICImagingFactory> GetWicFactory();
    base::win::ScopedComPtr<ID2D1Factory> GetD2DFactory();

    base::win::ScopedComPtr<ID2D1PathGeometry> CreatePathGeometry();

    base::win::ScopedComPtr<IWICBitmap> CreateWICBitmap(int width, int height);
    base::win::ScopedComPtr<IWICBitmap> CreateWICBitmap(int width, int height, void* data);
    base::win::ScopedComPtr<ID2D1RenderTarget> CreateWICRenderTarget(
        IWICBitmap* wic_bitmap, bool dpi_awareness);
    base::win::ScopedComPtr<ID2D1DCRenderTarget> CreateDCRenderTarget();

    base::win::ScopedComPtr<IDXGISurface> CreateDXGISurface(int width, int height);
    base::win::ScopedComPtr<ID2D1RenderTarget> CreateHardwareRenderTarget(
        IDXGISurface* surface, bool dpi_awareness);

    base::win::ScopedComPtr<IDWriteTextLayout> CreateTextLayout(
        const base::string16& text, int max_width, int max_height);
    base::win::ScopedComPtr<IDWriteInlineObject> CreateEllipsisTrimmingSign();

    base::win::ScopedComPtr<IWICBitmapSource> DecodeImage(const string16& file_name);
    base::win::ScopedComPtr<IWICBitmapSource> DecodeImage(const BYTE* buffer, size_t size);

    //is_cache: 是否开启解码缓存,适用对象生命周期长并且有大量重复对象存在(eg：弹幕姬普通消息)
    //          开启缓存的时候,渲染的缓存对象最好也同步打开,降低内存->ImageInlineObject
    //          其他情况还是不走缓存的情况好
    base::win::ScopedComPtr<IWICBitmapSource> DecodeImageFromData(const std::string& data,bool is_cache = false);
    base::win::ScopedComPtr<IWICBitmapSource> DecodePngImage(const std::string& buffer);
    base::win::ScopedComPtr<IWICBitmapSource> DecodePngImageFromData(const std::string& buffer,bool is_cache = false);
    WICBitmaps DecodeGifFromData(const std::string& data, bool is_cache = false);
    int ScaleImage(
        const string16& src_file_name, const string16& dst_file_name, int width, int height);

    bool LookImage(const string16& file_name, uint32_t x, uint32_t y, uint32_t* color);

private:
    D2DManager();
    ~D2DManager();

    HRESULT CreateD3DDevice();

    base::win::ScopedComPtr<IWICBitmapSource> ProcessDecoder(
        base::win::ScopedComPtr<IWICBitmapDecoder> decoder);

    bool ScaleFrame(
        const base::win::ScopedComPtr<IWICBitmapFrameDecode>& frame_decode,
        const base::win::ScopedComPtr<IWICBitmapFrameEncode>& frame_encode,
        UINT de_width, UINT de_height);

    float dpi_x_;
    float dpi_y_;

    base::win::ScopedComPtr<ID2D1Factory> d2d_factory_;
    base::win::ScopedComPtr<IDWriteFactory> dwrite_factory_;
    base::win::ScopedComPtr<IDWriteTextFormat> text_format_;
    base::win::ScopedComPtr<IWICImagingFactory> wic_factory_;

    base::win::ScopedComPtr<ID3D11Device> d3d_device_;
    base::win::ScopedComPtr<ID3D11DeviceContext> d3d_devicecontext_;

    std::map<std::string, base::win::ScopedComPtr<IWICBitmapSource>> local_cache_;
    std::map<std::string, WICBitmaps> local_gif_cache_;
    friend struct DefaultSingletonTraits<D2DManager>;
    DISALLOW_COPY_AND_ASSIGN(D2DManager);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_D2D_MANAGER_H_