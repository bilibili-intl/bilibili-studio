/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_PUBLIC_NETWORK_INFO_H_
#define BILILIVE_SECRET_PUBLIC_NETWORK_INFO_H_

#include <string>

#include "bilibase/basic_macros.h"

namespace secret {

class EventTrackingServiceImpl;
class EventTrackingServiceIntlImpl;

class NetworkInfo
{
public:
    NetworkInfo() = default;
    ~NetworkInfo() = default;

    int network_type() const { return net_type_; }
    std::string ip() const { return ip_; }
    std::wstring region() const { return region_; }
    std::wstring communications_operator() const { return co_; }
    std::string buvid() const { return buvid_; }

    void set_os_ver(const std::string& os_ver) {
        os_ver_ = os_ver;
    }
    const std::string& os_ver() const {
        return os_ver_;
    }

private:
    friend class EventTrackingServiceImpl;
    friend class EventTrackingServiceIntlImpl;
    int net_type_ = 0;
    std::string ip_;
    std::wstring region_;
    std::wstring co_;
    std::string buvid_;
    std::string os_ver_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_ACCOUNT_INFO_H_
