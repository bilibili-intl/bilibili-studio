#ifndef BILILIVE_BILILIVE_UTILS_NET_UTIL_H
#define BILILIVE_BILILIVE_UTILS_NET_UTIL_H

#include <string>
#include <vector>

#include <shellapi.h>

#include "base/strings/stringprintf.h"
#include "net/base/network_change_notifier.h"

namespace bililive
{
    std::vector<std::string> GetLocalValidIpList();

    std::string AppendURLQueryParams(const std::string& url, const std::string& params);

    net::NetworkChangeNotifier::ConnectionType GetCurrentConnectionType();
    std::string GetCurrentConnectionTypeName();

    bool ShellExecute(const std::string& target);
    bool ShellExecute(const std::wstring& target);

    enum class ExecURLType
    {
        LAYOUT_MODEL_SUGGESTION,
    };
    bool ShellExecute(ExecURLType type);
}

#endif