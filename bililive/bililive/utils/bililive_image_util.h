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

//��·���м���ͼƬ
gfx::ImageSkia decodeImgFromFile(const base::FilePath& file_name);

// �� {img} ����һ���µ�ͼƬ�����أ�����ͼƬ���ܾ������ţ��Ծ������ {bounds} ��ȷ����ʾ������
// ��� {target_rect} ��Ϊ nullptr���������غ� {target_rect} ��������ͼƬ�� {bounds} �еĴ�С��λ�ã����У���
// ��� {img} �� {bounds} �Ĵ�СΪ 0�����������ؿյ� ImageSkia ����
// bounds �� target_rect �ĵ�λ�� img �ĳߴ絥λһ��
gfx::ImageSkia CreateScaledImageEx(
    const gfx::ImageSkia& img, const gfx::Rect& bounds, gfx::Rect* target_rect, bool fill = false);

// �򻯰汾��ֱ��ָ����С��size �ĵ�λӦ�� img �ĳߴ絥λһ�¡�
gfx::ImageSkia CreateScaledImageEx(
    const gfx::ImageSkia& img, const gfx::Size& size, bool fill = false);

// �� {img} ����һ���µ�Բ��ͼƬ�����أ�Բ�εİ뾶ȡ {img} �Ŀ�͸ߵ���Сֵ��һ�룬Բ��λ��ͼƬ���ġ�
// ��� {img} �Ĵ�СΪ 0�����������ؿյ� ImageSkia ����
gfx::ImageSkia CreateCircleImageEx(const gfx::ImageSkia& img);

// �򻯰汾��������ͼ��size �ĵ�λӦ�� img �ĳߴ絥λһ�¡�
gfx::ImageSkia CreateCircleImageEx(const gfx::ImageSkia& img, const gfx::Size& size);

// ��img���ŵ�ָ����Сsize�������Ϊ�뾶Ϊradius��Բ��img
gfx::ImageSkia CreateRoundRectImage(const gfx::ImageSkia& img, const gfx::Size& size, int radius);

// �򻯰汾�����img��Բ�ǰ汾
gfx::ImageSkia CreateRoundRectImage(const gfx::ImageSkia& img, int radius);

// ������ǰͼƬ�ĻҶ�ͼ
gfx::ImageSkia CreateGrayscaleImage(const gfx::ImageSkia& img);

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_UTILS_BILILIVE_IMAGE_UTIL_H_
