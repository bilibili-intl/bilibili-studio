#include "bililive/secret/core/bililive_secret_core_impl.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/path_service.h"
#include "base/prefs/pref_service.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/bililive/profiles/profile.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_features.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/secret/common/secret_pref_names.h"
#include "bililive/secret/utils/bililive_crypto.h"

BililiveSecretCoreImpl* g_secret_core = nullptr;

BililiveSecretCoreImpl* GetSecretCore()
{
    DCHECK(g_secret_core);
    return g_secret_core;
}

namespace {

const wchar_t kSecretPrefFileName[] = L"Secret Preference";

const char kCsrfName[] = "csrf";

void RegisterLoginProfileprefs(PrefRegistrySimple* registry)
{
    registry->RegisterStringPref(prefs::kLastLogin, "default");
    registry->RegisterIntegerPref(prefs::kLastLoginType, 0);
    registry->RegisterInt64Pref(prefs::kLastLoginUser, 0);

    base::ListValue *list_history = new base::ListValue();
    registry->RegisterListPref(prefs::kHistory, list_history);
}

void RegisterSecretPrefs(PrefRegistrySimple* registry)
{
    RegisterLoginProfileprefs(registry);
}

}   // namespace

BililiveSecretCoreImpl::BililiveSecretCoreImpl()
    : exit_manager_(new base::AtExitManager()),
      message_loop_proxy_(nullptr),
      logout_(false),
      token_invalid_(false)
{
    g_secret_core = this;
}

BililiveSecretCoreImpl::~BililiveSecretCoreImpl()
{
    //ensure io_thread_ is release before danmaku_transactions_
    //、streaming_transactions_ and getroominfo_transactions_.
    io_thread_.reset();

    g_secret_core = nullptr;
}

bool BililiveSecretCoreImpl::Init(base::MessageLoopProxy* message_loop_proxy,
                                  const base::FilePath& global_profile_dir,
                                  BililiveProcess* bililive_process)
{
    int argc = 0;
    char **argv = nullptr;
    CommandLine::Init(argc, argv);

#if defined(NDEBUG)
    bool enable_debug_logging = false;
    if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchDebugConsole)) {
        enable_debug_logging = true;
    }
#else   // NDEBUG
    bool enable_debug_logging = true;
#endif  // NDEBUG


    bililive::InitBililiveLogging(enable_debug_logging);
    BililiveContext::Current()->Init();

    bililive::RegisterPathProvider();

    BililiveFeatures::Init();

    global_profile_dir_ = global_profile_dir;
    bililive_process_ = bililive_process;
    message_loop_proxy_ = message_loop_proxy;
    thread_task_runner_handle_ = std::make_unique<base::ThreadTaskRunnerHandle>(message_loop_proxy);

    base::Thread::Options options;
    options.message_loop_type = base::MessageLoop::TYPE_IO;

    io_thread_.reset(new base::Thread("Secret_IO_Thread"));
    if (!io_thread_->StartWithOptions(options))
    {
        NOTREACHED();
        return false;
    }

    auto secret_pref_path = global_profile_dir.Append(kSecretPrefFileName);
    pref_service_ = std::make_unique<secret::SecretPrefService>(
        secret_pref_path, io_thread_->message_loop_proxy(), RegisterSecretPrefs);

    request_connection_manager_ = std::make_unique<secret::RequestConnectionManager>();

    user_account_service_ = std::make_unique<UserAccountServiceImpl>(request_connection_manager_.get());
    live_streaming_servece_ = std::make_unique<LiveStreamingServiceImpl>(request_connection_manager_.get());
    danmaku_hime_service_ = std::make_unique<DanmakuHimeServiceImpl>(request_connection_manager_.get());

    event_tracking_service_ = std::make_unique<secret::EventTrackingServiceIntlImpl>(request_connection_manager_.get());
    danmaku_broadcast_viddup_service_ = std::make_unique<DanmakuBroadcastViddupServiceImpl>(message_loop_proxy_);

    return true;
}

void BililiveSecretCoreImpl::UnInit()
{
    DCHECK(message_loop_proxy_->BelongsToCurrentThread());

    if (logout_)
    {
        token_invalid_ = false;
        int64_t mid = account_info_ ? account_info_->mid() : 0;
        UpdateAccountHistory(mid, token_invalid_);
        RevokeToken();
    }

    danmaku_hime_service_ = nullptr;
    live_streaming_servece_ = nullptr;
    user_account_service_ = nullptr;
    event_tracking_service_ = nullptr;

    request_connection_manager_ = nullptr;

    pref_service_ = nullptr;
}

BililiveProcess* BililiveSecretCoreImpl::bililive_process() const
{
    return bililive_process_;
}

SecretPrefService* BililiveSecretCoreImpl::pref_service() const
{
    return pref_service_.get();
}

const base::FilePath& BililiveSecretCoreImpl::global_profile_path() const
{
    return global_profile_dir_;
}

UserAccountServiceImpl* BililiveSecretCoreImpl::user_account_service() const
{
    return user_account_service_.get();
}

LiveStreamingServiceImpl* BililiveSecretCoreImpl::live_streaming_service() const
{
    return live_streaming_servece_.get();
}

DanmakuHimeServiceImpl* BililiveSecretCoreImpl::danmaku_hime_service() const
{
    return danmaku_hime_service_.get();
}

DanmakuBroadcastViddupServiceImpl* BililiveSecretCoreImpl::danmaku_broadcast_viddup_service() const
{
    return danmaku_broadcast_viddup_service_.get();
}

EventTrackingService* BililiveSecretCoreImpl::event_tracking_service() const
{
    return event_tracking_service_.get();
}

AccountInfo& BililiveSecretCoreImpl::account_info() const {
    if (account_info_)
    {
        return *account_info_.get();
    }

    static AccountInfo dumy_info;
    return dumy_info;
}

secret::UserInfoImpl& BililiveSecretCoreImpl::user_info()
{
    return user_info_;
}

secret::AnchorInfoImpl& BililiveSecretCoreImpl::anchor_info()
{
    return anchor_info_;
}

secret::NetworkInfo& BililiveSecretCoreImpl::network_info()
{
    return network_info_;
}

void BililiveSecretCoreImpl::SaveAccountInfo(
    bool auto_login, int64_t mid, const std::string& account,
    const std::string& token, const std::string& refresh_token, int64_t expires,
    const std::string& cookies, const std::string& domains, bool save_to_profile,
    bool webcookie/* = false*/)
{
    account_info_ = std::make_unique<secret::AccountInfo>(mid, account, token, refresh_token, expires,
        auto_login, cookies, domains, save_to_profile, webcookie);
}

void BililiveSecretCoreImpl::SaveSecretProfile() {
    if (!account_info_.get()) {
        return;
    }

    if (!account_info_->mini_login())
    {
        pref_service()->GetPrefs()->SetString(prefs::kLastLogin, account_info_->account());
    }

    pref_service()->GetPrefs()->SetInteger(prefs::kLastLoginType, account_info_->mini_login() ? 1 : 0);
    pref_service()->GetPrefs()->SetInt64(prefs::kLastLoginUser, account_info_->mid());

    bool token_persisted = account_info_->persisted();
    auto list_value = pref_service()->GetPrefs()->GetList(prefs::kHistory);

    if (!list_value) {
        return;
    }

    std::unique_ptr<base::ListValue> list_history(list_value->DeepCopy());
    bool exist = false;

    auto encrypt_token = secret::BililiveEncrypt(account_info_->access_token());
    auto encrypt_refresh_token = secret::BililiveEncrypt(account_info_->refresh_token());
    auto encrypt_cookies = secret::BililiveEncrypt(account_info_->cookies());
    auto encrypt_domains = secret::BililiveEncrypt(account_info_->domains());

    for (size_t i = 0; i < list_history->GetSize(); i++) {
        base::DictionaryValue* data_node = nullptr;
        list_history->GetDictionary(i, &data_node);

        if (!data_node) {
            break;
        }

        int64_t mid;
        data_node->GetInteger64("mid", &mid);

        if (mid == account_info_->mid()) {
            if (!account_info_->mini_login())
            {
                data_node->SetString("account", account_info_->account());
            }
            data_node->SetBoolean("auto_login", account_info_->auto_login());
            data_node->SetBoolean("mini_login", account_info_->mini_login());
            if (token_persisted) {
                data_node->SetString("token", encrypt_token);
                data_node->SetString("refresh_token", encrypt_refresh_token);
                data_node->SetInteger64("expires", account_info_->token_expires());
                data_node->SetString("cookies", encrypt_cookies);
                data_node->SetString("domains", encrypt_domains);
            } else {
                data_node->SetString("token", "");
                data_node->SetString("refresh_token", "");
                data_node->SetInteger("expires", 0);
                data_node->SetString("cookies", "");
                data_node->SetString("domains", "");
            }

            exist = true;

            break;
        }
    }

    if (!exist) {
        base::DictionaryValue* dict_value = new base::DictionaryValue();

        if (dict_value) {
            if (!account_info_->mini_login())
            {
                dict_value->SetString("account", account_info_->account());
            }
            dict_value->SetInteger64("mid", account_info_->mid());
            dict_value->SetBoolean("auto_login", account_info_->auto_login());
            dict_value->SetBoolean("mini_login", account_info_->mini_login());
            if (token_persisted) {
                dict_value->SetString("token", encrypt_token);
                dict_value->SetString("refresh_token", encrypt_refresh_token);
                dict_value->SetInteger64("expires", account_info_->token_expires());
                dict_value->SetString("cookies", encrypt_cookies);
                dict_value->SetString("domains", encrypt_domains);
            } else {
                dict_value->SetString("token", "");
                dict_value->SetString("refresh_token", "");
                dict_value->SetInteger("expires", 0);
                dict_value->SetString("cookies", "");
                dict_value->SetString("domains", "");
            }

            list_history->Append(dict_value);
        }
    }

    pref_service()->GetPrefs()->Set(prefs::kHistory, *list_history.get());
    pref_service()->GetPrefs()->CommitPendingWrite();
}

std::shared_ptr<std::vector<secret::LoginInfo>> BililiveSecretCoreImpl::GetHistoryLoginInfo()
{
    auto infos = std::make_shared<std::vector<secret::LoginInfo>>();
    auto profile = pref_service();

    if (!profile)
    {
        return infos;
    }

    auto list_value = profile->GetPrefs()->GetList(prefs::kHistory);

    if (!list_value)
    {
        return infos;
    }

    for (size_t i = 0; i < list_value->GetSize(); i++)
    {
        const base::DictionaryValue* data_node = nullptr;
        ignore_result(list_value->GetDictionary(i, &data_node));

        if (!data_node)
        {
            return infos;
        }

        secret::LoginInfo info;

        data_node->GetString("account", &info.name);
        data_node->GetInteger64("mid", &info.mid);
        data_node->GetInteger64("expires", &info.expires);
        data_node->GetBoolean("auto_login", &info.auto_login);
        data_node->GetBoolean("mini_login", &info.mini_login);
        data_node->GetString("token", &info.token);
        data_node->GetString("refresh_token", &info.refresh_token);
        data_node->GetString("cookies", &info.cookies);
        data_node->GetString("domains", &info.domains);

        info.token = secret::BililiveDecrypt(info.token);
        info.refresh_token = secret::BililiveDecrypt(info.refresh_token);
        info.cookies = secret::BililiveDecrypt(info.cookies);
        info.domains = secret::BililiveDecrypt(info.domains);

        infos->push_back(info);
    }

    return infos;
}

std::shared_ptr<secret::LoginInfo> BililiveSecretCoreImpl::GetLastValidLoginInfo()
{
    auto profile = pref_service();
    if (!profile)
    {
        return nullptr;
    }

    auto list_value = profile->GetPrefs()->GetList(prefs::kHistory);
    if (!list_value || list_value->GetSize() == 0)
    {
        return nullptr;
    }

    auto info = std::make_shared<secret::LoginInfo>();

    int64_t last_login_mid = pref_service()->GetPrefs()->GetInt64(prefs::kLastLoginUser);

    for (size_t i = 0; i < list_value->GetSize(); i++) {
        const base::DictionaryValue* data_node = nullptr;
        ignore_result(list_value->GetDictionary(i, &data_node));

        if (!data_node)
        {
            return info;
        }

        int64_t mid = 0;
        data_node->GetInteger64("mid", &mid);

        if (last_login_mid == mid) {
            info->mid = mid;
            data_node->GetString("account", &info->name);
            data_node->GetString("token", &info->token);
            data_node->GetString("refresh_token", &info->refresh_token);
            data_node->GetInteger64("expires", &info->expires);
            data_node->GetBoolean("auto_login", &info->auto_login);
            data_node->GetBoolean("mini_login", &info->mini_login);
            data_node->GetString("cookies", &info->cookies);
            data_node->GetString("domains", &info->domains);

            // 最后成功登录的用户信息要和本次登录类型匹配
            // (BililiveFeatures::current()->Enabled(BililiveFeatures::NativeLogin) && !info->mini_login)
            // or
            // (!BililiveFeatures::current()->Enabled(BililiveFeatures::NativeLogin) && info->mini_login)
            bool valid_record = BililiveFeatures::current()->Enabled(BililiveFeatures::NativeLogin) != info->mini_login;
            DCHECK(valid_record);
            if (!valid_record)
            {
                return nullptr;
            }

            break;
        }
    }

    info->token = secret::BililiveDecrypt(info->token);
    info->refresh_token = secret::BililiveDecrypt(info->refresh_token);
    info->cookies = secret::BililiveDecrypt(info->cookies);
    info->domains = secret::BililiveDecrypt(info->domains);

    return info;
}

void BililiveSecretCoreImpl::RemoveHistoryLoginInfo(const std::string& dst_account, bool delete_account_directory)
{
    UpdateAccountHistory(dst_account, false, delete_account_directory);
}

void BililiveSecretCoreImpl::SetLogout() {
    logout_ = true;
}

void BililiveSecretCoreImpl::SetTokenInvalid()
{
    token_invalid_ = true;
    int64_t mid = account_info_ ? account_info_->mid() : 0;
    UpdateAccountHistory(mid, token_invalid_);
}

void BililiveSecretCoreImpl::UpdateAccountHistory(int64_t dst_mid, bool just_invalid_or_delete,
    bool delete_account_directory/* = false*/)
{
    UpdateAccountHistoryInternal(dst_mid, "", just_invalid_or_delete, delete_account_directory);
}

void BililiveSecretCoreImpl::UpdateAccountHistory(const std::string& account,
    bool just_invalid_or_delete, bool delete_account_directory /*= false*/)
{
    UpdateAccountHistoryInternal(0, account, just_invalid_or_delete, delete_account_directory);
}

void BililiveSecretCoreImpl::UpdateAccountHistoryInternal(int64_t dst_mid, const std::string& dst_account,
    bool just_invalid_or_delete,
    bool delete_account_directory /*= false*/)
{
    if (dst_mid <= 0 && dst_account.empty())
    {
        return;
    }

    auto list_value = pref_service()->GetPrefs()->GetList(prefs::kHistory);
    if (!list_value)
    {
        return;
    }

    std::unique_ptr<base::ListValue> list_history(list_value->DeepCopy());

    for (size_t index = 0; index < list_history->GetSize(); index++)
    {
        base::DictionaryValue* data_node = nullptr;
        list_history->GetDictionary(index, &data_node);
        if (!data_node)
        {
            break;
        }

        std::string account;
        data_node->GetString("account", &account);

        int64 mid = 0;
        data_node->GetInteger64("mid", &mid);

        if ((dst_mid == mid && dst_mid != 0) || (dst_account == account && !account.empty()))
        {
            if (just_invalid_or_delete)
            {
                if (data_node)
                {
                    data_node->SetBoolean("auto_login", false);
                    data_node->SetString("token", "");
                    data_node->SetString("refresh_token", "");
                    data_node->SetInteger("expires", 0);
                    data_node->SetString("cookies", "");
                    data_node->SetString("domains", "");
                }
            }
            else
            {
                scoped_ptr<base::Value> out_value;
                list_history->Erase(list_history->begin() + index, &out_value);
            }

            if (mid != 0 && delete_account_directory)
            {
                base::FilePath account_directory_path;
                if (PathService::Get(bililive::DIR_USER_DATA, &account_directory_path))
                {
                    account_directory_path = account_directory_path.Append(std::to_wstring(mid));
                    base::ThreadRestrictions::ScopedAllowIO allow_io;
                    base::DeleteFile(account_directory_path, true);
                }
            }

            break;
        }
    }

    pref_service()->GetPrefs()->Set(prefs::kHistory, *list_history.get());
}

void BililiveSecretCoreImpl::RevokeToken() {
    auto token = account_info().access_token();

    if (token.empty()) {
        return;
    }
}

void BililiveSecretCoreImpl::SetAuthCookie(const cef_proxy::cookies& cookies)
{
    // 把csrf token单独记录下来，POST请求的body需要添加这个token
    auto iter = std::find_if(cookies.begin(), cookies.end(), [](const cef_proxy::cookie_t& cookie)->bool {
            return cookie.name.compare("bili_jct") == 0;
        });
    if (iter != cookies.end())
    {
        tokens_[kCsrfName] = iter->value;
    }
    else
    {
        NOTREACHED();
        LOG(WARNING) << "[secret] can't find csrf token.";
    }
    user_account_service()->SetAuthCookie(cookies);
}

const char* BililiveSecretCoreImpl::get_csrf_token() const
{
    if (tokens_.find(kCsrfName) != tokens_.end())
    {
        const char* scrf = tokens_.at(kCsrfName).c_str();
        return scrf;
    }
    return "";
}

int64_t BililiveSecretCoreImpl::get_last_uid() 
{
    int64_t last_login_mid = pref_service()->GetPrefs()->GetInt64(prefs::kLastLoginUser);
    return last_login_mid;

}