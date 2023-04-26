/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/net/request_connection_manager.h"
#include "base/strings/string_split.h"
#include "net/cookies/cookie_monster.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/public/common/pref_names.h"

namespace {

const char kNetworkIOThreadName[] = "RequestConnectionNetworkIO";
const char kRelayThreadName[] = "RequestConnectionRelayThread";

//重要！！！
//cookie注入新的域名，此处需要添加到list
//外部调用此接口,传递对应的域名, string GetCookieString(const std::string& doMain);
//eg:https://core.bilivideo.com/live-delay/measure/time
std::vector<std::string> g_netDomainList = {
    "core.bilivideo.com"
};
}   // namespace

namespace secret {

RequestConnectionManager::RequestConnectionManager()
    : network_io_thread_(kNetworkIOThreadName),
      relay_thread_(kRelayThreadName),
    weak_factory_(this)
{
    if (GetSecretCore()->bililive_process()->global_profile()) {
        auto prefs = GetSecretCore()->bililive_process()->global_profile()->GetPrefs();
        if (prefs) {
            ff_switch_ = prefs->GetBoolean(prefs::kFastForwardSwitch);
            proxy_id_ = prefs->GetString(prefs::kFastForwardHeadersProxyId);
            proxy_env_ = prefs->GetString(prefs::kFastForwardHeadersProxyEnv);
        }
    }
    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_IO;

    CHECK(network_io_thread_.StartWithOptions(options));
    LOG(INFO) << "Network IO Thread ID: " << network_io_thread_.thread_id();

    CHECK(relay_thread_.StartWithOptions(options));
    LOG(INFO) << "Relay Thread ID: " << relay_thread_.thread_id();

    request_context_getter_ = make_scoped_refptr(
        new RequestConnectionContextGetter(network_io_thread_.message_loop_proxy(),
                                           relay_thread_.message_loop_proxy()));
}

RequestConnectionManager::~RequestConnectionManager()
{
    // context-getter may be owned by request connections, and which could be owned by active
    // threads; thus they may not be destroyed immediately in here.

    {
        base::AutoLock lock(conn_access_mutex_);
        request_connections_.clear();
    }

    request_context_getter_->MarkAsShuttingDown();
    request_context_getter_ = nullptr;
}

std::string CefCookiesToString(const cef_proxy::cookies& cookies) {
	base::ListValue root;
	for (auto& cookie : cookies)
	{
		base::DictionaryValue* item = new base::DictionaryValue();

		item->SetString("name", cookie.name);
		item->SetString("value", cookie.value);
		item->SetString("domain", cookie.domain);
		item->SetString("path", cookie.path);
		item->SetInteger("secure", cookie.secure);
		item->SetInteger("httponly", cookie.httponly);
		item->SetInteger64("creation", cookie.creation);
		item->SetInteger64("last_access", cookie.last_access);
		item->SetInteger("has_expires", cookie.has_expires);
		item->SetInteger64("expires", cookie.expires);

		root.Append(item);
	}
	std::string json_str;
	base::JSONWriter::Write(&root, &json_str);
    return json_str;
}
std::string CefCookiesToString(const cef_proxy::cookies& cookies, const std::string& domain)
{
    base::ListValue root;
    for (auto& cookie : cookies)
    {
        base::DictionaryValue* item = new base::DictionaryValue();

        item->SetString("name", cookie.name);
        item->SetString("value", cookie.value);
        item->SetString("domain", domain);
        item->SetString("path", cookie.path);
        item->SetInteger("secure", cookie.secure);
        item->SetInteger("httponly", cookie.httponly);
        item->SetInteger64("creation", cookie.creation);
        item->SetInteger64("last_access", cookie.last_access);
        item->SetInteger("has_expires", cookie.has_expires);
        item->SetInteger64("expires", cookie.expires);

        root.Append(item);
    }
    std::string json_str;
    base::JSONWriter::Write(&root, &json_str);
    return json_str;
}
void RequestConnectionManager::SetAuthCookie(const cef_proxy::cookies& cookie)
{
    cookies_ = CefCookiesToString(cookie);
    net::CookieMonster* cookie_monster = request_context_getter_->GetURLRequestContext()->cookie_store()->GetCookieMonster();

    net::CookieStore::SetCookiesCallback callback(
        base::Bind(&RequestConnectionManager::OnCookieSet,
            weak_factory_.GetWeakPtr()));

    for (auto& iter : cookie)
    {
        std::string domain = ".bilibili.tv";

        cookie_monster->SetCookieWithDetailsAsync(GURL(domain),
            iter.name,
            iter.value,
            iter.domain,
            iter.path,
            base::Time::FromTimeT(iter.expires),
            iter.secure,
            iter.httponly,
            net::CookiePriority::COOKIE_PRIORITY_DEFAULT,
            callback);
    }
    for (const auto& itr : g_netDomainList) {
        cookies_list_.push_back(CefCookiesToString(cookie, itr));
        for (auto& iter : cookie)
        {
            cookie_monster->SetCookieWithDetailsAsync(GURL(itr),
                iter.name,
                iter.value,
                itr,
                iter.path,
                base::Time::FromTimeT(iter.expires),
                iter.secure,
                iter.httponly,
                net::CookiePriority::COOKIE_PRIORITY_DEFAULT,
                callback);
        }
    }
}

void RequestConnectionManager::OnCookieSet(bool success)
{
    DCHECK(success);
}

void RequestConnectionManager::FastForwardChangeEnv(const GURL& url,const RequestHeaders& extra_headers,
    GURL& modify_url, RequestHeaders& modify_headers)
{
    modify_headers = extra_headers;
    modify_url = url;
    std::string raw_url = url.spec();
    std::string result_url;
    if (ff_switch_) {
        std::string::size_type pos;
        pos = raw_url.find("://");
        if (pos != std::string::npos) {
            std::string protocol = raw_url.substr(0, pos + 3);
            std::string url_value = raw_url.substr(pos + 3, raw_url.length() - pos - 3);
            std::string::size_type host_name_pos = url_value.find("/");
            std::string host_name = url_value.substr(0, host_name_pos);
            std::string path_name = url_value.substr(host_name_pos);

            result_url.append(protocol);
            result_url.append("ff-proxy.bilibili.com");
            result_url.append(path_name);
            modify_url = GURL(result_url);
            modify_headers["x-ff-proxy-target-host"] = host_name;
            modify_headers["x-ff-proxy-id"] = proxy_id_;
            modify_headers["x-ff-proxy-env"] = proxy_env_;
        }
    }
    modify_headers["buvid"] = GetSecretCore()->network_info().buvid();
    //LOG(INFO) << "ReHttpsFFChange: " << "url spec: " << url.spec();
    //LOG(INFO) << "ReHttpsFFChange: " << "url raw: " << url.raw_string();

    //LOG(INFO) << "ReHttpsFFChange: " << "modify_url spec: " << modify_url.spec();
    //LOG(INFO) << "ReHttpsFFChange: " << "modify_url raw: " << modify_url.raw_string();
}

string RequestConnectionManager::GetCookieString(const std::string& doMain)
{
    auto itr = std::find(cookies_list_.begin(), cookies_list_.end(), doMain);
    if (itr != cookies_list_.end()) {
        return *itr;
    }
    return string();
}

RequestConnectionBase*
    RequestConnectionManager::AddRequestConnection(const scoped_refptr<RequestConnectionBase>& req_conn)
{
    auto view_ptr = req_conn.get();

    {
        base::AutoLock lock(conn_access_mutex_);
        request_connections_.insert(req_conn);
    }

    return view_ptr;
}

void RequestConnectionManager::RemoveRequestConnection(RequestConnectionBase* req_conn)
{
    base::AutoLock lock(conn_access_mutex_);
    auto it = std::find_if(request_connections_.cbegin(), request_connections_.cend(),
                           [req_conn](const scoped_refptr<RequestConnectionBase>& req) {
                               return req_conn == req.get();
                           });
    if (it != request_connections_.cend()) {
        request_connections_.erase(it);
    }
}

void RequestConnectionManager::OnRequestEnd(RequestConnectionBase* req_conn)
{
    RemoveRequestConnection(req_conn);
}

}   // namespace secret