#pragma once

#include "base/files/file_path.h"

namespace livehime {

// 将声网日志目录中选中的文件进行zip压缩打包，返回压缩包的二进制数据
std::string GetTodayAgoraLogText(bool compress = false);
// 将自研连麦的日志目录中选中的文件进行zip压缩打包，返回压缩包的二进制数据
std::string GetTodayWebrtcLogText(bool compress = false);

}   // namespace bililive
