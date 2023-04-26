#ifndef BILILIVE_SECRET_UTILS_BILILIVE_KV_RESOURCE_H_
#define BILILIVE_SECRET_UTILS_BILILIVE_KV_RESOURCE_H_

#include "bililive/secret/public/live_streaming_service.h"

namespace secret
{
    // ����KVҵ���ʶbusikey�ӱ��ػ����ȡKV����Դ���ݵ�MD5�Լ�����Դ����
    bool BililiveGetLocalKVCache(const std::string& busikey, std::string* hash, std::string* value);

    // ���ӷ���˽ӿ��յ���KVҵ���ʶbusikey������Դ���ݻ��浽���ش���
    void BililiveSaveLocalKVCache(const std::string& busikey, const std::string& value);
}

#endif
