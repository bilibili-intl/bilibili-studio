#include <Windows.h>
#include <Shlwapi.h>

#include "base/file_util.h"

#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/image_util.h"

#include <atlbase.h>
#include <memory>

#include "SkBitmap.h"
#include "SkShader.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/image/image_util.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/Image_skia.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/skia_util.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "third_party/skia/src/core/SkBitmapScaler.h"
#include "third_party/skia/src/core/SkConvolver.h"

#include "base/logging.h"

#include "bilibase/basic_types.h"

#include "third_party/libnsgif/libnsgif.h"

namespace {

void* bitmap_create(int width, int height)
{
    return malloc(width * height * 4);
}

void bitmap_destroy(void *bitmap)
{
    free(bitmap);
}

unsigned char* bitmap_get_buffer(void *bitmap)
{
    return reinterpret_cast<unsigned char*>(bitmap);
}

gfx::ImageSkia MakeSkiaImageForJPG(const unsigned char* data, size_t size)
{
    if (!data || size == 0)
    {
        return gfx::ImageSkia();
    }

    gfx::Image image = gfx::ImageFromJPEGEncodedData(data, size);
    return image.IsEmpty() ? gfx::ImageSkia() : *image.ToImageSkia();
}

gfx::ImageSkia MakeSkiaImageForPNG(const unsigned char* data, size_t size)
{
    if (!data || size == 0)
    {
        return gfx::ImageSkia();
    }

    gfx::ImageSkia image_skia;

    SkBitmap bitmap;
    if (gfx::PNGCodec::Decode(data, size, &bitmap)) {
        image_skia = gfx::ImageSkia(gfx::ImageSkiaRep(bitmap, ui::SCALE_FACTOR_100P));
    }

    return image_skia;
}

gfx::ImageSkia MakeSkiaImageForGIF(const unsigned char* data, size_t size, gif_animation *out_gif = nullptr)
{
    if (!data || size == 0)
    {
        return gfx::ImageSkia();
    }

    gif_bitmap_callback_vt gif_cb;
    gif_cb.bitmap_create = bitmap_create;
    gif_cb.bitmap_destroy = bitmap_destroy;
    gif_cb.bitmap_get_buffer = bitmap_get_buffer;
    gif_cb.bitmap_set_opaque = nullptr;
    gif_cb.bitmap_test_opaque = nullptr;
    gif_cb.bitmap_modified = nullptr;

    gif_animation local_gif;

    gif_animation *gif = out_gif ? out_gif : &local_gif;

    gif_create(gif, &gif_cb);

    if (GIF_OK != gif_initialise(gif, size, const_cast<unsigned char*>(data))) {
        return gfx::ImageSkia();
    }

    if (GIF_OK != gif_decode_frame(gif, 0)) {
        gif_finalise(gif);
        return gfx::ImageSkia();
    }

    DCHECK(gif->frame_count > 0) << "why ?";

    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, gif->width, gif->height, 0);
    bitmap.allocPixels();
    bitmap.lockPixels();
    memcpy_s(bitmap.pixelRef()->pixels(),
             gif->width * gif->height * 4,
             reinterpret_cast<void*>(gif->frame_image),
             gif->width * gif->height * 4);
    bitmap.unlockPixels();

    gfx::ImageSkia image_skia = bililive::MakeSkiaImageFromRawPixels(
        reinterpret_cast<const unsigned char*>(gif->frame_image),
        gif->width * gif->height * 4,
        gfx::Size(gif->width, gif->height));

    if (!out_gif)
    {
        gif_finalise(gif);
    }

    return image_skia;
}

}   // namespace

namespace bililive {

gfx::ImageSkia MakeSkiaImage(const unsigned char* data, size_t size, gif_animation *gif/* = nullptr*/)
{
    ImageType format = GetImageTypeFromBinary(data, size);
    switch (format) {
        case ImageType::IT_JPG:
            return MakeSkiaImageForJPG(data, size);

        case ImageType::IT_PNG:
            return MakeSkiaImageForPNG(data, size);

        case ImageType::IT_GIF:
            return MakeSkiaImageForGIF(data, size, gif);

        default:
            LOG(WARNING) << "Unsupported image format: " << bilibase::enum_cast(format);
            break;
    }
    return gfx::ImageSkia();
}

gfx::ImageSkia MakeSkiaImageFromRawPixels(const unsigned char* data, size_t size,
    const gfx::Size& size_in_pixel)
{
    if (!data || size == 0)
    {
        return gfx::ImageSkia();
    }

    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, size_in_pixel.width(), size_in_pixel.height(), 0);
    bitmap.allocPixels();
    bitmap.lockPixels();
    memcpy_s(bitmap.pixelRef()->pixels(), size, data, size);
    bitmap.unlockPixels();

    return gfx::ImageSkia(gfx::ImageSkiaRep(bitmap, ui::SCALE_FACTOR_100P));
}

gfx::ImageSkia decodeImgFromFile(const base::FilePath& file_name)
{
    if (base::PathExists(file_name) && !base::DirectoryExists(file_name))
    {
        auto in_bytes = bililive::GetBinaryFromFile(file_name);
        if (!in_bytes.empty())
        {
            return bililive::MakeSkiaImage(
                reinterpret_cast<const uint8_t*>(in_bytes.data()), in_bytes.size());
        }
    }
    return {};
}

gfx::ImageSkia CreateScaledImageEx(
    const gfx::ImageSkia& img, const gfx::Rect& bounds, gfx::Rect* target_rect, bool fill)
{
    if (img.size().IsEmpty() || bounds.IsEmpty()) {
        return {};
    }

    int img_width = img.width();
    int img_height = img.height();

    int new_img_width;
    int new_img_height;
    double width_scale = img_width * 1.0 / bounds.width();
    double height_scale = img_height * 1.0 / bounds.height();
    if (fill ? (width_scale < height_scale) : (width_scale > height_scale)) {
        new_img_width = bounds.width();
        new_img_height = std::ceil(img_height / width_scale);
    } else if (fill ? (width_scale > height_scale) : (width_scale < height_scale)) {
        new_img_height = bounds.height();
        new_img_width = std::ceil(img_width / height_scale);
    } else {
        new_img_width = bounds.width();
        new_img_height = bounds.height();
    }

    SkBitmap result;
    SkConvolutionProcs convolutionProcs{};
    if (!SkBitmapScaler::Resize(
        &result, *img.bitmap(),
        SkBitmapScaler::ResizeMethod::RESIZE_LANCZOS3,
        new_img_width, new_img_height, &convolutionProcs))
    {
        return {};
    }

    if (target_rect) {
        int new_x = bounds.x() + (bounds.width() - new_img_width) / 2;
        int new_y = bounds.y() + (bounds.height() - new_img_height) / 2;
        target_rect->SetRect(new_x, new_y, new_img_width, new_img_height);
    }

    return gfx::ImageSkia::CreateFrom1xBitmap(result);
}

gfx::ImageSkia CreateScaledImageEx(const gfx::ImageSkia& img, const gfx::Size& size, bool fill) {
    return CreateScaledImageEx(img, gfx::Rect(size), nullptr, fill);
}

gfx::ImageSkia CreateCircleImageEx(const gfx::ImageSkia& img) {
    if (img.size().IsEmpty()) {
        return {};
    }

    int diameter = std::min(img.width(), img.height());
    int radius = diameter / 2;

    int dx = (img.width() - diameter) / 2;
    int dy = (img.height() - diameter) / 2;

    gfx::ImageSkiaRep img_rep = img.GetRepresentation(ui::SCALE_FACTOR_100P);
    if (img_rep.is_null()) {
        return {};
    }

    SkMatrix matrix;
    matrix.setTranslate(-dx, -dy);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(
        gfx::CreateImageRepShader(
            img_rep, SkShader::kClamp_TileMode, matrix).get());

    gfx::Canvas canvas(gfx::Size(diameter, diameter), img_rep.scale_factor(), false);
    canvas.DrawCircle(gfx::Point(radius, radius), radius, paint);

    return gfx::ImageSkia(canvas.ExtractImageRep());
}

gfx::ImageSkia CreateCircleImageEx(const gfx::ImageSkia& img, const gfx::Size& size) {
    return CreateCircleImageEx(CreateScaledImageEx(img, size));
}

gfx::ImageSkia CreateRoundRectImage(const gfx::ImageSkia& img, int radius) {
    if (img.size().IsEmpty()) {
        return {};
    }

    gfx::ImageSkiaRep img_rep = img.GetRepresentation(
        ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()));
    if (img_rep.is_null()) {
        return {};
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(
        gfx::CreateImageRepShader(
            img_rep, SkShader::kClamp_TileMode, SkMatrix::I()).get());

    gfx::Canvas canvas(img.size(), img_rep.scale_factor(), false);
    canvas.DrawRoundRect(gfx::Rect(0, 0, img.width(), img.height()), radius, paint);

    return gfx::ImageSkia(canvas.ExtractImageRep());
}

gfx::ImageSkia CreateRoundRectImage(const gfx::ImageSkia& img, const gfx::Size& size, int radius) {
    return CreateRoundRectImage(CreateScaledImageEx(img, size), radius);
}

gfx::ImageSkia CreateGrayscaleImage(const gfx::ImageSkia& img) {
    const gfx::ImageSkiaRep& img_rep = img.GetRepresentation(
        ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()));
    const SkBitmap& img_bmp = img_rep.sk_bitmap();
    SkBitmap dst_bmp;
    img_bmp.deepCopyTo(&dst_bmp, img_bmp.getConfig());

    SkBitmap::Config config = dst_bmp.getConfig();
    // 当前只处理kARGB_8888，其他的先不管了
    if (config == SkBitmap::kARGB_8888_Config)
    {
        unsigned char* pixels = (unsigned char*)dst_bmp.pixelRef()->pixels();
        int bytesPerPixel = dst_bmp.bytesPerPixel();
        size_t size = dst_bmp.getSize();
        for (int i = 0; i < (int)size; i += bytesPerPixel)
        {
            //unsigned char a = pixels[i + 3];
            unsigned char r = pixels[i + 2];
            unsigned char g = pixels[i + 1];
            unsigned char b = pixels[i];
            // 采用加权平均值算法
            unsigned char gray = r * 0.3 + g * 0.59 + b * 0.11;
            pixels[i] = gray;
            pixels[i + 1] = gray;
            pixels[i + 2] = gray;
            //pixels[i + 3] = 255;
        }
    }
    else
    {
        NOTREACHED();
    }

    return gfx::ImageSkia(gfx::ImageSkiaRep(dst_bmp, ui::SCALE_FACTOR_100P));
}


}   // namespace bililive
