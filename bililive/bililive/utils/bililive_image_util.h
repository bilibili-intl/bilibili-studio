#ifndef BILILIVE_BILILIVE_UTILS_BILILIVE_IMAGE_UTIL_H_
#define BILILIVE_BILILIVE_UTILS_BILILIVE_IMAGE_UTIL_H_

#include "ui/base/layout.h"

struct gif_animation;

namespace gfx {

class Rect;
class Size;
class ImageSkia;

}   // namespace gfx

namespace bililive {

// If `data` is null-pointer, the function simply returns an empty object.
gfx::ImageSkia MakeSkiaImage(const unsigned char* data, size_t size, gif_animation *gif = nullptr);
gfx::ImageSkia MakeSkiaImageFromRawPixels(const unsigned char* data, size_t size, const gfx::Size& size_in_pixel);

//从路径中加载图片
gfx::ImageSkia decodeImgFromFile(const base::FilePath& file_name);

// 从 {img} 创建一张新的图片并返回，这张图片可能经过缩放，以尽量填充 {bounds} 并确保显示完整。
// 如果 {target_rect} 不为 nullptr，则函数返回后 {target_rect} 将包含新图片在 {bounds} 中的大小和位置（居中）。
// 如果 {img} 或 {bounds} 的大小为 0，函数将返回空的 ImageSkia 对象。
// bounds 和 target_rect 的单位与 img 的尺寸单位一致
gfx::ImageSkia CreateScaledImageEx(
    const gfx::ImageSkia& img, const gfx::Rect& bounds, gfx::Rect* target_rect, bool fill = false);

// 简化版本，直接指定大小。size 的单位应与 img 的尺寸单位一致。
gfx::ImageSkia CreateScaledImageEx(
    const gfx::ImageSkia& img, const gfx::Size& size, bool fill = false);

// 从 {img} 创建一张新的圆形图片并返回，圆形的半径取 {img} 的宽和高的最小值的一半，圆心位于图片中心。
// 如果 {img} 的大小为 0，函数将返回空的 ImageSkia 对象。
gfx::ImageSkia CreateCircleImageEx(const gfx::ImageSkia& img);

// 简化版本，可缩放图像。size 的单位应与 img 的尺寸单位一致。
gfx::ImageSkia CreateCircleImageEx(const gfx::ImageSkia& img, const gfx::Size& size);

// 将img缩放到指定大小size，并输出为半径为radius的圆角img
gfx::ImageSkia CreateRoundRectImage(const gfx::ImageSkia& img, const gfx::Size& size, int radius);

// 简化版本，输出img的圆角版本
gfx::ImageSkia CreateRoundRectImage(const gfx::ImageSkia& img, int radius);

// 创建当前图片的灰度图
gfx::ImageSkia CreateGrayscaleImage(const gfx::ImageSkia& img);

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UTILS_BILILIVE_IMAGE_UTIL_H_
