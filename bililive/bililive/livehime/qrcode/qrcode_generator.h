#pragma once

#include "ui/gfx/image/image_skia.h"
#include <string>

namespace bililive
{
	//blockPx：每个方块多少像素边长
	gfx::ImageSkia GenerateURLQRCode(const std::string& url, int blockPx);
};
