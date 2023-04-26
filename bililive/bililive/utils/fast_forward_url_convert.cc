#include "fast_forward_url_convert.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/bililive/bililive_process.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"
#include "net_util.h"
namespace bililive
{
    std::string FastForwardChangeEnv(const std::string& url)
    {
        std::string  env_url = url;
        if (GetBililiveProcess()->global_profile()) {
            auto prefs = GetBililiveProcess()->global_profile()->GetPrefs();
            if (prefs) {
                bool  ff_switch = prefs->GetBoolean(prefs::kFastForwardSwitch);
                if (ff_switch) {
                    std::string webview_proxy_id = prefs->GetString(prefs::kFastForwardWebviewQueryArgsProxyId);
                    std::string host_prefix = prefs->GetString(prefs::kFastForwardWebviewHostPrefix);

                    std::string protocol;
                    std::string host_name;
                    std::string::size_type pos = url.find("://");
                    if (pos != std::string::npos) {
                        std::string url_value;
                        protocol = url.substr(0, pos + 3);
                        url_value = url.substr(pos + 3, url.length() - pos - 3);
                        std::string::size_type host_name_pos = url_value.find("/");
                        if (host_name_pos != std::string::npos) {
                            host_name = url_value.substr(0, host_name_pos);
                            if (host_name.find("ff-") != std::string::npos ||
                                host_name.find("bilibili.com") == std::string::npos ||
                                host_name.find("bilibili.co") == std::string::npos) {
                                return url;
                            }
                            host_name = host_prefix.append(host_name);
                            env_url.clear();
                            env_url.append(protocol);
                            env_url.append(host_name);
                            env_url.append(url_value.substr(host_name_pos, url_value.length() - host_name_pos));

                            std::string web_proxy_id = "ff-proxy-id=";
                            web_proxy_id.append(webview_proxy_id);
                            env_url = bililive::AppendURLQueryParams(env_url, web_proxy_id);
                        }
                    }
                }
            }
        }
       // LOG(INFO) << __FUNCTION__ << " raw_url : " << url;
       // LOG(INFO) << __FUNCTION__ << " env_url : " << env_url;
        return env_url;
    }
    std::wstring FastForwardChangeEnv(const std::wstring& url)
    {
        std::wstring  env_url = url;
        if (GetBililiveProcess()->global_profile()) {
            auto prefs = GetBililiveProcess()->global_profile()->GetPrefs();
            if (prefs) {
                bool  ff_switch = prefs->GetBoolean(prefs::kFastForwardSwitch);
                if (ff_switch) {
                    std::string webview_proxy_id = prefs->GetString(prefs::kFastForwardWebviewQueryArgsProxyId);
                    std::wstring host_prefix = base::UTF8ToUTF16(prefs->GetString(prefs::kFastForwardWebviewHostPrefix));

                    std::wstring protocol;
                    std::wstring host_name;
                    std::wstring::size_type pos = url.find(L"://");
                    if (pos != std::wstring::npos) {
                        std::wstring url_value;
                        protocol = url.substr(0, pos + 3);
                        url_value = url.substr(pos + 3, url.length() - pos - 3);
                        std::wstring::size_type host_name_pos = url_value.find(L"/");
                        if (host_name_pos != std::wstring::npos) {
                            host_name = url_value.substr(0, host_name_pos);
                            if (host_name.find(L"ff-") != std::wstring::npos||
                                host_name.find(L"bilibili.com") == std::string::npos ||
                                host_name.find(L"bilibili.co") == std::string::npos) {
                                return url;
                            }
                            host_name = host_prefix.append(host_name);
                            env_url.clear();
                            env_url.append(protocol);
                            env_url.append(host_name);
                            env_url.append(url_value.substr(host_name_pos, url_value.length() - host_name_pos));

                            std::string web_proxy_id = "ff-proxy-id=";
                            web_proxy_id.append(webview_proxy_id);
                            env_url = base::UTF8ToUTF16(bililive::AppendURLQueryParams(base::UTF16ToUTF8(env_url), web_proxy_id));
                        }
                    }
                }
            }
        }
        //LOG(INFO) << __FUNCTION__ << " raw_url : " << url;
        //LOG(INFO) << __FUNCTION__ << " env_url : " << env_url;
        return env_url;
    }
}