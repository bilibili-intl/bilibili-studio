/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/services/service_utils.h"

#include "base/base64.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "base/win/windows_version.h"

#include "bililive/common/bililive_features.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"

// Make sure this header file won't be included prior to our secret core file;
// Otherwise, it will result in WinSock2.h-related compilation errors.
#include "openssl/ssl.h"

namespace
{
    template<class Body>
    void AppendCSRFInternal(Body& body)
    {
        // cookie登录态方式才需要在POST的情况下添加CSRF token
        if (BililiveFeatures::current()->Enabled(BililiveFeatures::Feature::NativeLogin))
        {
            return;
        }

        auto core = GetSecretCore();
        if (core)
        {
            std::string csrf = core->get_csrf_token();

            body["csrf_token"] = csrf;
            body["csrf"] = csrf;
        }
    }

}

namespace secret {

const char kArgAppKey[] = "appkey";
const char kAppKey[] = "aae92bc66f3edfab";
const char kArgTimestamp[] = "ts";
const char kArgSign[] = "sign";
const char kArgPlatform[] = "platform";
const char kArgAppVersion[] = "version";

const char kPlatform[] = "pc_link";

const char kArgBFSUploadBucket[] = "bucket";
const char kBFSUploadBucket[] = "live";

const char* const kDays[] { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

const char* const kMonths[] {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

std::string GenerateSignForRequestData(const RequestData& params)
{
    /*static_assert(std::is_same<RequestParams::key_compare, std::less<std::string>>::value,
        "RequestParams must use std::less as its key comparator!");*/

    std::string content = params.ToRequestContent().second;
    base::MD5Digest result;
    //ComputeMD5(reinterpret_cast<uint8_t*>(&result), content.data(), content.length());

    return base::MD5DigestToBase16(result);
}

std::string GenerateSignForRequestParams(const RequestParams& params)
{
    static_assert(std::is_same<RequestParams::key_compare, std::less<std::string>>::value,
                  "RequestParams must use std::less as its key comparator!");

    std::string content = params.ToRequestContent().second;
    base::MD5Digest result;
    //ComputeMD5(reinterpret_cast<uint8_t*>(&result), content.data(), content.length());

    return base::MD5DigestToBase16(result);
}

// Don't try to modify following code unless you are an expert in cryptography.
// It is too hard to make it work, and yet no one in the team know how it exactly works.
std::string EncryptPassword(const std::string& password, const std::string& hash, const std::string& key)
{
    BIO *bio = BIO_new_mem_buf((void*)key.c_str(), key.size());

    if (nullptr == bio) {
        LOG(ERROR) << "Init the bio error!";
        return "";
    }

    RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);

    if (rsa == nullptr) {
        BIO_free(bio);
        LOG(ERROR) << "RSA pubkey is empty!";
        return "";
    }

    std::string data(hash + password);

    std::vector<unsigned char> cryptoBuffer(RSA_size(rsa));
    std::vector<unsigned char> cryptoResult;

    size_t srcIndex = 0;
    while (srcIndex < data.size()) {
        int leftSize = data.size() - srcIndex;
        int curLen = (cryptoBuffer.size() - RSA_PKCS1_PADDING_SIZE);
        if (curLen > leftSize) {
            curLen = leftSize;
        }

        RSA_public_encrypt(curLen, (unsigned char*)data.c_str() + srcIndex, &cryptoBuffer[0], rsa,
                           RSA_PKCS1_PADDING);
        srcIndex += curLen;
        cryptoResult.insert(cryptoResult.end(), cryptoBuffer.begin(), cryptoBuffer.end());
    }

    BIO_free(bio);
    RSA_free(rsa);

    std::string result;
    std::string cryptoStr(cryptoResult.begin(), cryptoResult.end());

    base::Base64Encode(cryptoStr, &result);

    return result;
}

std::string HttpDateFromTime(base::Time time)
{
    base::Time::Exploded http_time;
    time.UTCExplode(&http_time);

    return base::StringPrintf("%s, %02d %s %d %02d:%02d:%02d GMT",
                              kDays[http_time.day_of_week],
                              http_time.day_of_month,
                              kMonths[http_time.month - 1],
                              http_time.year,
                              http_time.hour,
                              http_time.minute,
                              http_time.second);
}

const std::string& QueryAccessToken()
{
    return GetSecretCore()->account_info().access_token();
}

uint64_t FileTimeToUnixEpoch(const FILETIME& ft)
{
    const uint64_t kFileTimeTickRate = 10000000;
    const uint64_t kEpochDifference = 11644473600;

    ULARGE_INTEGER filetime_epoch;
    filetime_epoch.LowPart = ft.dwLowDateTime;
    filetime_epoch.HighPart = ft.dwHighDateTime;

    return filetime_epoch.QuadPart / kFileTimeTickRate - kEpochDifference;
}

std::string GetOSVerName()
{
    using namespace base;
    auto version = win::GetVersion();

    switch (version) {
    case win::Version::VERSION_VISTA:
        return "Windows vista";
    case win::Version::VERSION_WIN7:
        return "Windows 7";
    case win::Version::VERSION_WIN8:
        return "Windows 8";
    case win::Version::VERSION_WIN8_1:
        return "Windows 8.1";
    case win::Version::VERSION_WIN10:
        return "Windows 10";
    default:
        return "unknown version";
    }
}

int64_t QueryMid()
{
    return GetSecretCore()->account_info().mid();
}

std::string GenerateBFSUploadToken(const std::string& time_point)
{
    unsigned int result[5] {0};
    //GenerateUploadToken(result, time_point.c_str(), time_point.size());
    auto token = base::StringPrintf("%08x%08x%08x%08x%08x",
                                    result[0], result[1], result[2], result[3], result[4]);
    token.append(":").append(time_point);

    return token;
}

std::map<std::string, std::string> GetParamsFromUrl(std::string& url)
{
    std::map<std::string, std::string> result;

    auto pos = url.find("?");
    if (pos == std::string::npos) {
        return result;
    }

    std::string param_url = url.substr(pos + 1);
    url = url.substr(0, pos);
    
    std::vector<std::string> param_vec;
    base::SplitStringUsingSubstr(param_url, "&", &param_vec);

    for (auto iter : param_vec) {
        pos = iter.find("=");
        if (pos != std::string::npos) {
            auto key = iter.substr(0, pos);
            auto value = iter.substr(pos + 1);
            result.insert(std::make_pair(key, value));
        }
    }

    return result;
}

void AppendCSRF(RequestParams& body)
{
    AppendCSRFInternal(body);
}

void AppendCSRF(RequestParams2& body)
{
    AppendCSRFInternal(body);
}

void AppendCSRF(RequestUpload& body)
{
    AppendCSRFInternal(body.extra_params);
}

}   // namespace secret
