#ifndef BILILIVE_PUBLIC_SECRET_BILILIVE_SECRET_H_
#define BILILIVE_PUBLIC_SECRET_BILILIVE_SECRET_H_

#include "base/memory/ref_counted_memory.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop_proxy.h"

#include "bililive/secret/public/account_info.h"
#include "bililive/secret/public/anchor_info.h"
#include "bililive/secret/public/app_service.h"
#include "bililive/secret/public/network_info.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/secret/public/live_streaming_service.h"
#include "bililive/secret/public/login_info.h"
#include "bililive/secret/public/user_info.h"
#include "bililive/secret/public/user_account_service.h"
#include "bililive/secret/public/danmaku_broadcast_service.h"
#include "bililive/secret/public/danmaku_broadcast_viddup_service.h"
#include "bililive/secret/public/danmaku_hime_service.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

// TODO: Original implementation of the entire secret module is a pile of shit.
// TODO: Rewrite it once convenient. No more shit on legacy shit, PLEASE!

typedef scoped_refptr<base::RefCountedString> RefString;

class Profile;
class BililiveProcess;

// Class BililiveSecret should not be ref-counted/shareable, because it is owned only by
// BililiveProcess. However, technical debt won't pay off in a day, we have to keep it as is
// until we rectify the original implementation.

class BililiveSecret : public base::RefCountedThreadSafe<BililiveSecret> {
protected:
    virtual ~BililiveSecret() {}

    friend base::RefCountedThreadSafe<BililiveSecret>;

public:
    virtual bool Init(base::MessageLoopProxy* message_loop_proxy,
                      const base::FilePath& global_profile_dir,
                      BililiveProcess* bililive_process) = 0;

    virtual void UnInit() = 0;

    virtual void SetLogout() = 0;

    virtual void SetTokenInvalid() = 0;

    virtual void SaveAccountInfo(
        bool auto_login, int64_t mid, const std::string& account,
        const std::string& token, const std::string& refresh_token, int64_t expires,
        const std::string& cookies, const std::string& domains, bool save_to_profile,
        bool webcookie = false) = 0;

    virtual void SaveSecretProfile() = 0;

    virtual std::shared_ptr<std::vector<secret::LoginInfo>> GetHistoryLoginInfo() = 0;
    virtual std::shared_ptr<secret::LoginInfo> GetLastValidLoginInfo() = 0;
    virtual void RemoveHistoryLoginInfo(const std::string& dst_account, bool delete_account_directory) = 0;

    virtual void SetAuthCookie(const cef_proxy::cookies& cookies) = 0;

    virtual const char* get_csrf_token() const = 0;

    virtual int64_t get_last_uid() = 0;

    virtual secret::AccountInfo& account_info() const = 0;

    virtual secret::UserInfo& user_info() = 0;

    virtual secret::AnchorInfo& anchor_info() = 0;

    virtual secret::NetworkInfo& network_info() = 0;

    virtual secret::UserAccountService* user_account_service() const = 0;

    virtual secret::LiveStreamingService* live_streaming_service() const = 0;

    virtual secret::DanmakuHimeService* danmaku_hime_service() const = 0;

    //virtual secret::DanmakuBroadcastService* danmaku_broadcast_service() const = 0;

    virtual secret::DanmakuBroadcastViddupService* danmaku_broadcast_viddup_service() const = 0;

    virtual secret::EventTrackingService* event_tracking_service() const = 0;
};

#endif  // BILILIVE_PUBLIC_SECRET_BILILIVE_SECRET_H_
