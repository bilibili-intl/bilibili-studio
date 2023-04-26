/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_SERVICES_SERVICE_UTILS_H_
#define BILILIVE_SECRET_SERVICES_SERVICE_UTILS_H_

#include <chrono>
#include <string>

#include "bililive/secret/net/request_connection_common_types.h"

namespace secret {

extern const char kArgAppKey[];
extern const char kAppKey[];
extern const char kArgTimestamp[];
extern const char kArgSign[];
extern const char kArgPlatform[];
extern const char kArgAppVersion[];

extern const char kPlatform[];

extern const char kArgBFSUploadBucket[];
extern const char kBFSUploadBucket[];

template<typename Precision = std::chrono::seconds>
std::string GetUnixTimestamp()
{
    auto timestamp = std::chrono::duration_cast<Precision>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    return std::to_string(timestamp);
}

std::string GenerateSignForRequestData(const RequestData& params);
std::string GenerateSignForRequestParams(const RequestParams& params);

std::string EncryptPassword(const std::string& password, const std::string& hash, const std::string& key);

std::string HttpDateFromTime(base::Time time);

// Convenient approach to get access token from account info.
const std::string& QueryAccessToken();

int64_t QueryMid();

uint64_t FileTimeToUnixEpoch(const FILETIME& ft);

std::string GetOSVerName();

std::string GenerateBFSUploadToken(const std::string& time_point);

std::map<std::string, std::string> GetParamsFromUrl(std::string& url);

void AppendCSRF(RequestParams& body);
void AppendCSRF(RequestParams2& body);
void AppendCSRF(RequestUpload& body);

}   // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_SERVICE_UTILS_H_
