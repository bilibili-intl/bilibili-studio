#pragma once

#include "ui/gfx/image/image_skia.h"
#include <string>

namespace bililive
{
	//blockPx��ÿ������������ر߳�
	gfx::ImageSkia GenerateURLQRCode(const std::string& url, int blockPx);
};
