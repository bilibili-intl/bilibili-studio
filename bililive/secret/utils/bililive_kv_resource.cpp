#include "bililive/secret/utils/bililive_kv_resource.h"

#include <array>
#include <cstdint>
#include <functional>

#include "base/base64.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/md5.h"
#include "base/path_service.h"
#include "base/files/file_path.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/public/common/bililive_paths.h"
#include "bililive/secret/utils/bililive_crypto.h"

namespace {

}   // namespace

namespace secret
{
    bool BililiveGetLocalKVCache(const std::string& busikey, std::string* hash, std::string* value)
    {
        base::ThreadRestrictions::ScopedAllowIO allow_io;
        base::FilePath cp;
        if (!PathService::Get(bililive::DIR_KV_CACHE, &cp))
        {
            return false;
        }

        // 业务标识要不要编码一波，省得用户自己找到了然后通过名称可拿到一些引起吐槽的信息
        std::string sec_busikey = base::MD5String(busikey);
        cp = cp.AppendASCII(sec_busikey);
        if (!base::PathExists(cp))
        {
            return false;
        }

        std::string value_contents;
        if (!file_util::ReadFileToString(cp, &value_contents))
        {
            return false;
        }

        // 解密一下内容
        value_contents = secret::BililiveDecrypt(value_contents);

        if (value)
        {
            *value = value_contents;
        }

        if (hash)
        {
            *hash = base::MD5String(value_contents);
        }

        return true;
    }

    void BililiveSaveLocalKVCache(const std::string& busikey, const std::string& value)
    {
        base::ThreadRestrictions::ScopedAllowIO allow_io;
        base::FilePath cp;
        if (!PathService::Get(bililive::DIR_KV_CACHE, &cp))
        {
            return;
        }

        std::string sec_busikey = base::MD5String(busikey);
        cp = cp.AppendASCII(sec_busikey);

        std::string sec_value = secret::BililiveEncrypt(value);
        file_util::WriteFile(cp, sec_value.data(), sec_value.length());
    }

}   // namespace secret
