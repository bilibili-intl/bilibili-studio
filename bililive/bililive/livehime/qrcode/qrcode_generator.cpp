#include "third_party/libqrencode/qrencode.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image_skia.h"
#include "SkBitmap.h"
#include "SkCanvas.h"

namespace bililive
{
	gfx::ImageSkia GenerateURLQRCode(const std::string& url, int blockPx)
	{
		gfx::ImageSkia result;

		//生成二维码数据
		QRcode* qr = QRcode_encodeString(url.data(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
		if (qr)
		{
			//生成用于绘制二维码的位图
			SkBitmap qrcodeBitmap;
			int width = qr->width * blockPx;
			int height = width;
			qrcodeBitmap.setConfig(SkBitmap::Config::kARGB_8888_Config, width, height);
			qrcodeBitmap.allocPixels();

			//绘制二维码
			{
				SkCanvas canvas(qrcodeBitmap);
				SkPaint whitePaint, blackPaint;
				whitePaint.setColor(SkColorSetARGB(0xff, 0xff, 0xff, 0xff));
				whitePaint.setStyle(SkPaint::Style::kFill_Style);
				blackPaint.setColor(SkColorSetARGB(0xff, 0, 0, 0));
				blackPaint.setStyle(SkPaint::Style::kFill_Style);

				//背景
				SkRect bgRect;
				bgRect.set(0, 0, width, height);
				canvas.drawRect(bgRect, whitePaint);
				
				//方格
				for (int i = 0; i < qr->width; ++i)
					for (int j = 0; j < qr->width; ++j)
					{
						if (qr->data[j * qr->width + i] & 1)
						{
							SkRect blockRect;
							blockRect.set(i * blockPx, j * blockPx, (1 + i) * blockPx, (1 + j) * blockPx);
							canvas.drawRect(blockRect, blackPaint);
						}
					}
			}

			QRcode_free(qr);
			result = gfx::ImageSkia(gfx::ImageSkiaRep(qrcodeBitmap, ui::SCALE_FACTOR_100P));
		}

		return result;
	}
};
