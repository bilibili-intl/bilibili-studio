#ifndef BILILIVE_SECRET_CORE_BILILIVE_SECRET_CORE_IMPL_H_
#define BILILIVE_SECRET_CORE_BILILIVE_SECRET_CORE_IMPL_H_

#include "bililive/public/secret/bililive_secret.h"

#include "base/at_exit.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"

#include "bililive/bililive/profiles/profile.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/secret/core/bililive_secret_pref_service.h"
#include "bililive/secret/core/anchor_info_impl.h"
#include "bililive/secret/core/user_info_impl.h"
#include "bililive/secret/net/request_connection_manager.h"
#include "bililive/secret/public/account_info.h"
#include "bililive/secret/public/user_info.h"
#include "bililive/secret/services/event_tracking_service_impl.h"
#include "bililive/secret/services/event_tracking_service_intl_impl.h"
#include "bililive/secret/services/live_streaming_service_impl.h"
#include "bililive/secret/services/user_account_service_impl.h"
#include "bililive/secret/services/danmaku_broadcast_viddup_service_impl.h"
#include "bililive/secret/services/danmaku_hime_service_impl.h"

class BililiveDanmakuTransactions;

using secret::AccountInfo;
using secret::RequestConnectionManager;
using secret::SecretPrefService;
using secret::EventTrackingService;
using secret::LiveStreamingServiceImpl;
using secret::DanmakuHimeServiceImpl;
using secret::DanmakuBroadcastViddupServiceImpl;
using secret::UserAccountServiceImpl;
using secret::UserInfo;


class BililiveSecretCoreImpl : public BililiveSecret
{
    typedef std::map<std::string, std::string> tokens;
public:
    BililiveSecretCoreImpl();

    ~BililiveSecretCoreImpl();

    bool Init(base::MessageLoopProxy* message_loop_proxy,
              const base::FilePath& global_profile_dir,
              BililiveProcess* bililive_process) override;

    void UnInit() override;

    void SetLogout() override;

    void SetTokenInvalid() override;

    void SaveAccountInfo(
        bool auto_login, int64_t mid, const std::string& account,
        const std::string& token, const std::string& refresh_token, int64_t expires,
        const std::string& cookies, const std::string& domains, bool save_to_profile,
        bool webcookie = false) override;

    void SaveSecretProfile() override;

    std::shared_ptr<std::vector<secret::LoginInfo>> GetHistoryLoginInfo() override;
    std::shared_ptr<secret::LoginInfo> GetLastValidLoginInfo() override;
    void RemoveHistoryLoginInfo(const std::string& dst_account, bool delete_account_directory) override;

    void RevokeToken();

    int64_t get_last_uid() override;

    void SetAuthCookie(const cef_proxy::cookies& cookies) override;

    const char* get_csrf_token()const override;

    BililiveProcess* bililive_process() const;

    SecretPrefService* pref_service() const;

    const base::FilePath& global_profile_path() const;

    AccountInfo& account_info() const override;

    secret::UserInfoImpl& user_info() override;

    secret::AnchorInfoImpl& anchor_info() override;

    secret::NetworkInfo& network_info() override;

    UserAccountServiceImpl* user_account_service() const override;

    LiveStreamingServiceImpl* live_streaming_service() const override;

    DanmakuHimeServiceImpl* danmaku_hime_service() const override;

    DanmakuBroadcastViddupServiceImpl* danmaku_broadcast_viddup_service() const override;

    EventTrackingService* event_tracking_service() const override;

private:
    void UpdateAccountHistory(int64_t mid, bool just_invalid_or_delete,
        bool delete_account_directory = false);
    void UpdateAccountHistory(const std::string& account, bool just_invalid_or_delete,
        bool delete_account_directory = false);
    void UpdateAccountHistoryInternal(int64_t mid, const std::string& account, bool just_invalid_or_delete,
        bool delete_account_directory = false);

private:
    scoped_ptr<base::AtExitManager> exit_manager_;

    BililiveProcess* bililive_process_;

    base::MessageLoopProxy* message_loop_proxy_;
    std::unique_ptr<base::ThreadTaskRunnerHandle> thread_task_runner_handle_;

    scoped_ptr<base::Thread> io_thread_;

    base::FilePath global_profile_dir_;

    std::unique_ptr<SecretPrefService> pref_service_;

    std::unique_ptr<RequestConnectionManager> request_connection_manager_;

    // User account service must be first service that being initialized, becaseu it
    // provides user authentication capabilities.
    std::unique_ptr<UserAccountServiceImpl> user_account_service_;
    std::unique_ptr<LiveStreamingServiceImpl> live_streaming_servece_;
    std::unique_ptr<DanmakuHimeServiceImpl> danmaku_hime_service_;
    std::unique_ptr<DanmakuBroadcastViddupServiceImpl> danmaku_broadcast_viddup_service_;
    std::unique_ptr<EventTrackingService> event_tracking_service_;

    std::unique_ptr<AccountInfo> account_info_;
    secret::UserInfoImpl user_info_;
    secret::AnchorInfoImpl anchor_info_;
    secret::NetworkInfo network_info_;
    tokens tokens_;

    bool logout_;

    bool token_invalid_;
};

BililiveSecretCoreImpl* GetSecretCore();

#endif  // BILILIVE_SECRET_CORE_BILILIVE_SECRET_CORE_IMPL_H_
