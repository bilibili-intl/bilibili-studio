#ifndef BILILIVE_SECRET_UTILS_BILILIVE_KV_RESOURCE_H_
#define BILILIVE_SECRET_UTILS_BILILIVE_KV_RESOURCE_H_

#include "bililive/secret/public/live_streaming_service.h"

namespace secret
{
    // 根据KV业务标识busikey从本地缓存获取KV配置源数据的MD5以及配置源数据
    bool BililiveGetLocalKVCache(const std::string& busikey, std::string* hash, std::string* value);

    // 将从服务端接口收到的KV业务标识busikey的配置源数据缓存到本地磁盘
    void BililiveSaveLocalKVCache(const std::string& busikey, const std::string& value);
}

#endif
