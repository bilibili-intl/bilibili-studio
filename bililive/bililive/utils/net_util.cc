#include "net_util.h"

#include <map>
#include <regex>
#include <sstream>

#include "base/logging.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/ip_endpoint.h"
#include "net/base/net_util.h"

#include <IPTypes.h>
#include <iphlpapi.h>
#include <Shlwapi.h>

namespace
{
    const wchar_t kCmdStr[] = L"route PRINT -4";
    const char kRegexPattern[] = R"__(^[ \t]*\d+\.\d+\.\d+\.\d+[ \t]+\d+\.\d+\.\d+\.\d+[ \t]+[^ \t]+[ \t]+(\d+\.\d+\.\d+\.\d+)[ \t]+(\d+)[ \r\n\t]*$)__";

    const std::map<bililive::ExecURLType, std::string> kExecURL{
        { bililive::ExecURLType::LAYOUT_MODEL_SUGGESTION, "" }
    };

    // Policy settings to include/exclude network interfaces.
    enum HostAddressSelectionPolicy
    {
        INCLUDE_HOST_SCOPE_VIRTUAL_INTERFACES = 0x0,
        EXCLUDE_HOST_SCOPE_VIRTUAL_INTERFACES = 0x1,
    };

    // A subset of IP address attributes which are actionable by the
    // application layer. Currently unimplemented for all hosts;
    // IP_ADDRESS_ATTRIBUTE_NONE is always returned.
    enum IPAddressAttributes
    {
        IP_ADDRESS_ATTRIBUTE_NONE = 0,

        // A temporary address is dynamic by nature and will not contain MAC
        // address. Presence of MAC address in IPv6 addresses can be used to
        // track an endpoint and cause privacy concern. Please refer to
        // RFC4941.
        IP_ADDRESS_ATTRIBUTE_TEMPORARY = 1 << 0,

        // A temporary address could become deprecated once the preferred
        // lifetime is reached. It is still valid but shouldn't be used to
        // create new connections.
        IP_ADDRESS_ATTRIBUTE_DEPRECATED = 1 << 1,

        // Anycast address.
        IP_ADDRESS_ATTRIBUTE_ANYCAST = 1 << 2,

        // Tentative address.
        IP_ADDRESS_ATTRIBUTE_TENTATIVE = 1 << 3,

        // DAD detected duplicate.
        IP_ADDRESS_ATTRIBUTE_DUPLICATED = 1 << 4,

        // May be detached from the link.
        IP_ADDRESS_ATTRIBUTE_DETACHED = 1 << 5,
    };

    struct NetworkInterface
    {
        NetworkInterface() = default;
        NetworkInterface(const std::string& name,
            const std::string& friendly_name,
            uint32_t interface_index,
            net::NetworkChangeNotifier::ConnectionType type,
            const net::IPAddressNumber& address,
            uint32_t prefix_length,
            int ip_address_attributes)
            : name(name),
            friendly_name(friendly_name),
            interface_index(interface_index),
            type(type),
            address(address),
            prefix_length(prefix_length),
            ip_address_attributes(ip_address_attributes)
        {
        }
        ~NetworkInterface() = default;

        std::string name;
        std::string friendly_name;  // Same as |name| on non-Windows.
        uint32_t interface_index;  // Always 0 on Android.
        net::NetworkChangeNotifier::ConnectionType type;
        net::IPAddressNumber address;
        uint32_t prefix_length;
        int ip_address_attributes;  // Combination of |IPAddressAttributes|.
    };

    typedef std::vector<NetworkInterface> NetworkInterfaceList;

    // Converts Windows defined types to NetworkInterfaceType.
    net::NetworkChangeNotifier::ConnectionType GetNetworkInterfaceType(DWORD ifType)
    {
        net::NetworkChangeNotifier::ConnectionType type =
            net::NetworkChangeNotifier::CONNECTION_UNKNOWN;
        if (ifType == IF_TYPE_ETHERNET_CSMACD)
        {
            type = net::NetworkChangeNotifier::CONNECTION_ETHERNET;
        }
        else if (ifType == IF_TYPE_IEEE80211)
        {
            type = net::NetworkChangeNotifier::CONNECTION_WIFI;
        }
        // TODO(mallinath) - Cellular?
        return type;
    }

    bool GetNetworkListImpl(NetworkInterfaceList* networks,
        int policy,
        const IP_ADAPTER_ADDRESSES* adapters)
    {
        for (const IP_ADAPTER_ADDRESSES* adapter = adapters; adapter != nullptr;
            adapter = adapter->Next)
        {
            // Ignore the loopback device.
            if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
            {
                continue;
            }

            if (adapter->OperStatus != IfOperStatusUp)
            {
                continue;
            }

            // Ignore any HOST side vmware adapters with a description like:
            // VMware Virtual Ethernet Adapter for VMnet1
            // but don't ignore any GUEST side adapters with a description like:
            // VMware Accelerated AMD PCNet Adapter #2
            if ((policy & EXCLUDE_HOST_SCOPE_VIRTUAL_INTERFACES) &&
                strstr(adapter->AdapterName, "VMnet") != nullptr)
            {
                continue;
            }

            for (IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress;
                address; address = address->Next)
            {
                int family = address->Address.lpSockaddr->sa_family;
                if (family == AF_INET || family == AF_INET6)
                {
                    net::IPEndPoint endpoint;
                    if (endpoint.FromSockAddr(address->Address.lpSockaddr,
                        address->Address.iSockaddrLength))
                    {
                        size_t prefix_length = address->OnLinkPrefixLength;

                        // If the duplicate address detection (DAD) state is not changed to
                        // Preferred, skip this address.
                        if (address->DadState != IpDadStatePreferred)
                        {
                            continue;
                        }

                        uint32_t index =
                            (family == AF_INET) ? adapter->IfIndex : adapter->Ipv6IfIndex;

                        // From http://technet.microsoft.com/en-us/ff568768(v=vs.60).aspx, the
                        // way to identify a temporary IPv6 Address is to check if
                        // PrefixOrigin is equal to IpPrefixOriginRouterAdvertisement and
                        // SuffixOrigin equal to IpSuffixOriginRandom.
                        int ip_address_attributes = IP_ADDRESS_ATTRIBUTE_NONE;
                        if (family == AF_INET6)
                        {
                            if (address->PrefixOrigin == IpPrefixOriginRouterAdvertisement &&
                                address->SuffixOrigin == IpSuffixOriginRandom)
                            {
                                ip_address_attributes |= IP_ADDRESS_ATTRIBUTE_TEMPORARY;
                            }
                            if (address->PreferredLifetime == 0)
                            {
                                ip_address_attributes |= IP_ADDRESS_ATTRIBUTE_DEPRECATED;
                            }
                        }
                        networks->push_back(NetworkInterface(
                            adapter->AdapterName,
                            base::SysWideToNativeMB(adapter->FriendlyName), index,
                            GetNetworkInterfaceType(adapter->IfType), endpoint.address(),
                            prefix_length, ip_address_attributes));
                    }
                }
            }
        }
        return true;
    }

    bool GetNetworkListA(NetworkInterfaceList* networks)
    {
        // Max number of times to retry GetAdaptersAddresses due to
        // ERROR_BUFFER_OVERFLOW. If GetAdaptersAddresses returns this indefinitely
        // due to an unforseen reason, we don't want to be stuck in an endless loop.
        static constexpr int MAX_GETADAPTERSADDRESSES_TRIES = 10;
        // Use an initial buffer size of 15KB, as recommended by MSDN. See:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx
        static constexpr int INITIAL_BUFFER_SIZE = 15000;

        ULONG len = INITIAL_BUFFER_SIZE;
        ULONG flags = 0;
        // Initial buffer allocated on stack.
        char initial_buf[INITIAL_BUFFER_SIZE];
        // Dynamic buffer in case initial buffer isn't large enough.
        std::unique_ptr<char[]> buf;

        IP_ADAPTER_ADDRESSES* adapters = nullptr;
        {
            adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(&initial_buf);
            ULONG result =
                GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, adapters, &len);

            // If we get ERROR_BUFFER_OVERFLOW, call GetAdaptersAddresses in a loop,
            // because the required size may increase between successive calls,
            // resulting in ERROR_BUFFER_OVERFLOW multiple times.
            for (int tries = 1; result == ERROR_BUFFER_OVERFLOW &&
                tries < MAX_GETADAPTERSADDRESSES_TRIES;
                ++tries)
            {
                buf.reset(new char[len]);
                adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(buf.get());
                result = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, adapters, &len);
            }

            if (result == ERROR_NO_DATA)
            {
                // There are 0 networks.
                return true;
            }
            else if (result != NO_ERROR)
            {
                LOG(ERROR) << "GetAdaptersAddresses failed: " << result;
                return false;
            }
        }

        return GetNetworkListImpl(networks, EXCLUDE_HOST_SCOPE_VIRTUAL_INTERFACES, adapters);
    }

    net::NetworkChangeNotifier::ConnectionType ConnectionTypeFromInterfaceList(
        const NetworkInterfaceList& interfaces)
    {
        bool first = true;
        net::NetworkChangeNotifier::ConnectionType result = net::NetworkChangeNotifier::CONNECTION_NONE;
        for (size_t i = 0; i < interfaces.size(); ++i)
        {
#if defined(OS_WIN)
            if (interfaces[i].friendly_name == "Teredo Tunneling Pseudo-Interface")
                continue;
#endif
#if defined(OS_MACOSX)
            // Ignore link-local addresses as they aren't globally routable.
            // Mac assigns these to disconnected interfaces like tunnel interfaces
            // ("utun"), airdrop interfaces ("awdl"), and ethernet ports ("en").
            if (interfaces[i].address.IsLinkLocal())
                continue;
#endif

            // Remove VMware network interfaces as they're internal and should not be
            // used to determine the network connection type.
//             if (base::ToLowerASCII(interfaces[i].friendly_name).find("vmnet") !=
//                 std::string::npos)
//             {
//                 continue;
//             }
            if (StrStrIA(interfaces[i].friendly_name.c_str(), "vmnet") != nullptr)
            {
                continue;
            }
            if (first)
            {
                first = false;
                result = interfaces[i].type;
            }
            else if (result != interfaces[i].type)
            {
                return net::NetworkChangeNotifier::CONNECTION_UNKNOWN;
            }
        }
        return result;
    }
}

namespace bililive
{
    std::vector<std::string> GetLocalValidIpList()
    {
        std::vector<std::string> ret;

        std::string result = "";

        std::map<std::string, int> interface_map;
        if (base::GetAppOutput(kCmdStr, &result, true))
        {
            std::stringstream stream(result);
            std::string line;
            std::regex re(kRegexPattern);
            while (std::getline(stream, line))
            {
                std::smatch mat;
                if (std::regex_search(line, mat, re))
                {
                    std::string interface_ip = mat[1].str();
                    if (!strcmp(interface_ip.c_str(), "127.0.0.1") ||
                        strstr(interface_ip.c_str(), "169.254."))
                    {
                        continue;
                    }
                    std::string str_matric = mat[2].str();
                    int matric;
                    char dummy;
                    if (sscanf(str_matric.c_str(), "%d%c", &matric, &dummy) != 1)
                    {
                        continue;
                    }
                    auto it = interface_map.find(interface_ip);
                    if (it != interface_map.end())
                    {
                        it->second = std::min<int>(it->second, matric);
                    }
                    else
                    {
                        interface_map.insert(std::make_pair(interface_ip, matric));
                    }
                }
            }
            std::vector<std::pair<std::string, int>> tmp_buf(interface_map.begin(), interface_map.end());
            std::sort(tmp_buf.begin(), tmp_buf.end(),
                [](std::pair<std::string, int>& lhs, std::pair<std::string, int>& rhs)
                {
                    return lhs.second < rhs.second;
                }
            );
            std::transform(tmp_buf.begin(), tmp_buf.end(), std::back_inserter(ret),
                [](std::pair<std::string, int>& x) { return x.first; }
            );
        }
        return ret;
    }

    std::string AppendURLQueryParams(const std::string& url, const std::string& params)
    {
        if (params.empty())
        {
            return url;
        }

        std::string location_flag;
        auto location_flag_pos = url.rfind('#');
        if (std::string::npos != location_flag_pos)
        {
            location_flag = url.substr(location_flag_pos);
        }
        std::string str(url.substr(0, location_flag_pos));
        if (str.find("?") != std::string::npos)
        {
            if (params[0] == '&')
            {
                str += params;
            }
            else
            {
                str += "&" + params;
            }
            DCHECK(params[0] != '?');
        }
        else
        {
            if (params[0] == '?')
            {
                str += params;
            }
            else
            {
                str += "?" + params;
            }
            DCHECK(params[0] != '&');
        }

        str += location_flag;

        return str;
    }

    net::NetworkChangeNotifier::ConnectionType GetCurrentConnectionType()
    {
        net::NetworkChangeNotifier::ConnectionType ct = net::NetworkChangeNotifier::CONNECTION_ETHERNET;
        NetworkInterfaceList networks;
        if (GetNetworkListA(&networks))
        {
            ct = ConnectionTypeFromInterfaceList(networks);
        }
        return ct;
    }

    std::string GetCurrentConnectionTypeName()
    {
        net::NetworkChangeNotifier::ConnectionType ct = GetCurrentConnectionType();
        switch (ct)
        {
        case net::NetworkChangeNotifier::CONNECTION_ETHERNET:
            return "Ethernet";
            break;
        case net::NetworkChangeNotifier::CONNECTION_WIFI:
            return "Wifi";
            break;
        case net::NetworkChangeNotifier::CONNECTION_2G:
        case net::NetworkChangeNotifier::CONNECTION_3G:
        case net::NetworkChangeNotifier::CONNECTION_4G:
            return "Cellular ";
            break;
        case net::NetworkChangeNotifier::CONNECTION_NONE:
            return "offline";
            break;
        }

        return "unknown";
    }

    bool ShellExecute(const std::string& target)
    {
        return (int)::ShellExecuteA(nullptr, "open", target.c_str(), nullptr, nullptr, SW_SHOW) >= 32;
    }

    bool ShellExecute(const std::wstring& target)
    {
        return (int)::ShellExecuteW(nullptr, L"open", target.c_str(), nullptr, nullptr, SW_SHOW) >= 32;
    }

    bool ShellExecute(ExecURLType type)
    {
        if (kExecURL.find(type) == kExecURL.end())
        {
            return false;
        }

        return ShellExecute(kExecURL.at(type));
    }

}