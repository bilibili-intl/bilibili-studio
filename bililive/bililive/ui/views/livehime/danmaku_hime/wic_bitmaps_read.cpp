#include "wic_bitmaps_read.h"
#include "src/webp/demux.h"
#include "src/utils/utils.h"

WICBitmapsRead::WICBitmapsRead() {
    HRESULT hr = ::CoCreateInstance(
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

WICBitmapsRead* WICBitmapsRead::GetInstance() {
    return Singleton<WICBitmapsRead>::get();
}


base::win::ScopedComPtr<IWICBitmap> WICBitmapsRead::CreateWICBitmap(int width, int height) {
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

base::win::ScopedComPtr<IWICBitmap> WICBitmapsRead::CreateWICBitmap(unsigned int width, unsigned int height, unsigned int stride, unsigned int bufferSize, BYTE* data)
{
    base::win::ScopedComPtr<IWICBitmap> bitmap;
    if (wic_factory_) {
        HRESULT hr = wic_factory_->CreateBitmapFromMemory(
            width, height,
            GUID_WICPixelFormat32bppPBGRA,
            stride,
            bufferSize,
            data,
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

WICBitmaps WICBitmapsRead::DecodeImage(const string16& file_name) {
    if (!wic_factory_) {
        return {};
    }

    base::win::ScopedComPtr<IWICBitmapDecoder> decoder;
    HRESULT hr = wic_factory_->CreateDecoderFromFilename(
        file_name.c_str(),                     // Image to be decoded
        nullptr,                               // Do not prefer a particular vendor
        GENERIC_READ,                          // Desired read access to the file
        WICDecodeMetadataCacheOnLoad,          // Cache metadata on load
        decoder.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create decoder from file: " << hr
            << ", File name: " << file_name;
        return {};
    }

    return ProcessDecoder(decoder);
}

WICBitmaps WICBitmapsRead::DecodeImage(const BYTE* buffer, size_t size) {
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
        stream.get(), nullptr, WICDecodeMetadataCacheOnLoad, decoder.Receive());
    if (FAILED(hr)) {
        DCHECK(false);
        LOG(WARNING) << "Failed to create decoder from stream: " << hr
            << ", File size: " << size;
        return {};
    }

    return ProcessDecoder(decoder);
}

WICBitmaps WICBitmapsRead::DecodeImageWebp(const BYTE* buffer, size_t size)
{
    WICBitmaps bmps;
    WebPDecoderConfig config;
    WebPDecBuffer* const output_buffer = &config.output;
    WebPBitstreamFeatures* const bitstream = &config.input;
    if (!WebPInitDecoderConfig(&config)) {
        fprintf(stderr, "Library version mismatch!\n");
    }
    VP8StatusCode status;
    status = WebPGetFeatures(buffer, size, bitstream);
    if (status != VP8_STATUS_OK) {
    }
    // ´øalpha Ô¤³Ë
    // RGB-premultiplied transparent modes (alpha value is preserved)
    output_buffer->colorspace = bitstream->has_alpha ? MODE_bgrA : MODE_BGR;

    //if (config.input.has_animation) {

    WebPData wd = { buffer, size };
    struct WebPDemuxer* demux = WebPDemux(&wd);
    bmps.width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    bmps.height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
    bmps.loop_count = WebPDemuxGetI(demux, WEBP_FF_LOOP_COUNT);
    uint32_t bgcolor = WebPDemuxGetI(demux, WEBP_FF_BACKGROUND_COLOR);
    uint8_t data[4];
    // a r g b
    PutLE32(data, bgcolor);
    float a = data[0] / 255.f;
    float r = data[1] / 255.f;
    float g = data[2] / 255.f;
    float b = data[3] / 255.f;
    bmps.bg_color = D2D1::ColorF(r, g, b, 0.0);

    //equal to 0 will return the last frame of the image
    int frame_idx = 1;
    WebPIterator iter;
    int result = WebPDemuxGetFrame(demux, frame_idx, &iter);
    if (result) {
        do {
            // ... (Consume 'iter'; e.g. Decode 'iter.fragment' with WebPDecode(),
            // ... and get other frame properties like width, height, offsets etc.
            // ... see 'struct WebPIterator' below for more info).
            status = WebPDecode(iter.fragment.bytes, iter.fragment.size, &config);
            if (status == VP8_STATUS_OK) {
                base::win::ScopedComPtr<IWICBitmap> bitmap = CreateWICBitmap(
                    config.output.width,
                    config.output.height,
                    config.output.u.RGBA.stride,
                    config.output.u.RGBA.size,
                    config.output.u.RGBA.rgba);
                base::win::ScopedComPtr<IWICBitmapSource> bitmap_source;
                HRESULT hr = bitmap.QueryInterface(bitmap_source.Receive());
                if (FAILED(hr)) {
                    return WICBitmaps();
                }
                WICFrame wic_frame;
                wic_frame.left = iter.x_offset;
                wic_frame.top = iter.y_offset;
                wic_frame.width = config.output.width;
                wic_frame.height = config.output.height;
                wic_frame.frame_interval = iter.duration;
                wic_frame.disposal = iter.dispose_method + 1;
                wic_frame.bitmap = bitmap_source;
                bmps.frames.push_back(wic_frame);
            }
        } while (WebPDemuxNextFrame(&iter));
        WebPDemuxReleaseIterator(&iter);
        WebPFreeDecBuffer(output_buffer);
    }
    //}
    return bmps;
}


void WICBitmapsRead::GetGlobalMetadata(
    base::win::ScopedComPtr<IWICBitmapDecoder> decoder,
    WICBitmaps& bmps) {

    base::win::ScopedComPtr<IWICMetadataQueryReader> reader;
    HRESULT hr = decoder->GetMetadataQueryReader(reader.Receive());
    if (SUCCEEDED(hr)) {
        PROPVARIANT prop_var;
        PropVariantInit(&prop_var);
        hr = reader->GetMetadataByName(L"/logscrdesc/Width", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            bmps.width = prop_var.uiVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/logscrdesc/Height", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            bmps.height = prop_var.uiVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/logscrdesc/GlobalColorTableFlag", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_BOOL && prop_var.boolVal) {
            PropVariantClear(&prop_var);
            hr = reader->GetMetadataByName(L"/logscrdesc/BackgroundColorIndex", &prop_var);
            if (SUCCEEDED(hr) && prop_var.vt == VT_UI1) {
                UINT bg_index = prop_var.bVal;
                base::win::ScopedComPtr<IWICPalette> palette;
                hr = wic_factory_->CreatePalette(palette.Receive());
                if (SUCCEEDED(hr)) {
                    hr = decoder->CopyPalette(palette.get());
                }

                if (SUCCEEDED(hr)) {
                    UINT color_count = 0;
                    hr = palette->GetColorCount(&color_count);
                    if (SUCCEEDED(hr) && color_count > 0) {
                        UINT actual_count = 0;
                        WICColor* color_table = new WICColor[color_count];
                        hr = palette->GetColors(color_count, color_table, &actual_count);
                        if (SUCCEEDED(hr) && actual_count > 0 && bg_index < actual_count) {
                            auto bg_color = color_table[bg_index];
                            float alpha = (bg_color >> 24) / 255.f;
                            bmps.bg_color = D2D1::ColorF(bg_color, alpha);
                        }
                        delete[] color_table;
                    }
                }
            }
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/logscrdesc/PixelAspectRatio", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI1) {
            UINT ratio = prop_var.bVal;
            if (ratio != 0) {
                float pixel_ratio = (ratio + 15.f) / 64.f;
                if (pixel_ratio > 1.f) {
                    bmps.height = static_cast<int>(bmps.height / pixel_ratio);
                } else {
                    bmps.width = static_cast<int>(bmps.width * pixel_ratio);
                }
            }
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/appext/application", &prop_var);
        if (SUCCEEDED(hr)
            && prop_var.vt == (VT_UI1 | VT_VECTOR)
            && prop_var.caub.cElems == 11
            && (!memcmp(prop_var.caub.pElems, "NETSCAPE2.0", prop_var.caub.cElems) ||
                !memcmp(prop_var.caub.pElems, "ANIMEXTS1.0", prop_var.caub.cElems))) {

            PropVariantClear(&prop_var);
            hr = reader->GetMetadataByName(L"/appext/data", &prop_var);
            if (SUCCEEDED(hr)
                && (prop_var.vt == (VT_UI1 | VT_VECTOR)
                    && prop_var.caub.cElems >= 4
                    && prop_var.caub.pElems[0] > 0
                    && prop_var.caub.pElems[1] == 1)) {
                bmps.loop_count = MAKEWORD(prop_var.caub.pElems[2],
                    prop_var.caub.pElems[3]);
            }
        }
        PropVariantClear(&prop_var);
    }
}

void WICBitmapsRead::GetFrameMetadata(
    base::win::ScopedComPtr<IWICBitmapFrameDecode> decoder,
    WICFrame& frame) {

    base::win::ScopedComPtr<IWICMetadataQueryReader> reader;
    HRESULT hr = decoder->GetMetadataQueryReader(reader.Receive());
    if (SUCCEEDED(hr)) {
        PROPVARIANT prop_var;
        PropVariantInit(&prop_var);
        hr = reader->GetMetadataByName(L"/grctlext/Disposal", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI1) {
            frame.disposal = prop_var.bVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/grctlext/Delay", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            frame.frame_interval = prop_var.uiVal * 10;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/imgdesc/Left", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            frame.left = prop_var.uiVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/imgdesc/Top", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            frame.top = prop_var.uiVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/imgdesc/Width", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            frame.width = prop_var.uiVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/imgdesc/Height", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_UI2) {
            frame.height = prop_var.uiVal;
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/imgdesc/InterlaceFlag", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_BOOL) {
            auto v_interlace = prop_var.boolVal;
            if (v_interlace) {
                frame.interlace = true;
            } else {
                frame.interlace = false;
            }
        }

        PropVariantClear(&prop_var);
        hr = reader->GetMetadataByName(L"/imgdesc/SortFlag", &prop_var);
        if (SUCCEEDED(hr) && prop_var.vt == VT_BOOL) {
            auto v_sort = prop_var.boolVal;
            if (v_sort) {
                frame.sort = true;
            } else {
                frame.sort = false;
            }
        }
        PropVariantClear(&prop_var);
    }
}

WICBitmaps WICBitmapsRead::ProcessDecoder(
    base::win::ScopedComPtr<IWICBitmapDecoder> decoder) {

    UINT frame_count = 0;
    HRESULT hr = decoder->GetFrameCount(&frame_count);
    if (FAILED(hr) || frame_count < 1) {
        DCHECK(false);
        return {};
    }

    WICBitmaps bmps;
    GetGlobalMetadata(decoder, bmps);

    for (UINT i = 0; i < frame_count; ++i) {
        base::win::ScopedComPtr<IWICBitmapFrameDecode> frame_decoder;
        HRESULT hr = decoder->GetFrame(i, frame_decoder.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to get frame: " << hr;
            continue;
        }

        base::win::ScopedComPtr<IWICFormatConverter> converter;
        // Format convert the frame to 32bppPBGRA
        hr = wic_factory_->CreateFormatConverter(converter.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create converter: " << hr;
            continue;
        }

        base::win::ScopedComPtr<IWICBitmapSource> source;
        hr = frame_decoder.QueryInterface(source.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to query bitmap source from frame: " << hr;
            continue;
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
            continue;
        }

        base::win::ScopedComPtr<IWICBitmapSource> wic_bitmap;
        hr = converter.QueryInterface(wic_bitmap.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to get bitmap source: " << hr;
            continue;
        }

        WICFrame wic_frame;
        GetFrameMetadata(frame_decoder, wic_frame);
        wic_frame.bitmap = wic_bitmap;

        bmps.frames.push_back(wic_frame);
    }

    if (bmps.width == 0 || bmps.height == 0) {
        if (!bmps.frames.empty()) {
            UINT width = 0;
            UINT height = 0;
            HRESULT hr = bmps.front()->GetSize(&width, &height);
            if (SUCCEEDED(hr)) {
                bmps.width = width;
                bmps.height = height;
            }
        }
    }

    if (bmps.width == 0 || bmps.height == 0) {
        DCHECK(false);
        return {};
    }

    return bmps;
}