#include "danmaku_broadcast_viddup_service_impl.h"

#include "base/ext/bind_lambda.h"
#include "base/ext/callable_callback.h"
#include "base/location.h"
#include "base/message_loop/message_loop_proxy.h"

#include "bililive/secret/server_broadcast/internal_danmaku_viddup_service_client_context.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"


namespace secret {

DanmakuBroadcastViddupServiceImpl::DanmakuBroadcastViddupServiceImpl(base::MessageLoopProxy *ui_proxy)
    : callback_(nullptr),
      is_running_(false),
      ui_proxy_(ui_proxy) {
}

DanmakuBroadcastViddupServiceImpl::~DanmakuBroadcastViddupServiceImpl() {
    DCHECK(context_ == nullptr);
}

void DanmakuBroadcastViddupServiceImpl::StartListening(
    int64_t uid, int64_t room_id, const ServerList& servers, const std::string& token,
    DanmakuBroadcastViddupCallback* c)
{
    DCHECK(ui_proxy_->BelongsToCurrentThread());

    callback_ = c;

    if (is_running_) {
        return;
    }

    context_ = new internal::DanmakuViddupServiceClientContext(
        room_id, uid, token,
        servers,
        base::Bind(&DanmakuBroadcastViddupServiceImpl::OnDanmakuMsgFromContext,
            base::Unretained(this)),
        base::Bind(&DanmakuBroadcastViddupServiceImpl::OnAudienceNumFromContext,
            base::Unretained(this)),
        std::bind(&DanmakuBroadcastViddupServiceImpl::OnTokenExpiredFromContext, this),
        std::bind(&DanmakuBroadcastViddupServiceImpl::OnDanmakuACKFromContext, this, std::placeholders::_1));

    is_running_ = context_->Start();
}

void DanmakuBroadcastViddupServiceImpl::StopListening() {
    DCHECK(ui_proxy_->BelongsToCurrentThread());

    callback_ = nullptr;

    StopContext();

    is_running_ = false;
}

void DanmakuBroadcastViddupServiceImpl::StopContext() {
    if (context_) {
        context_->Invalid();
        context_->Stop();
        context_ = nullptr;
    }
}

void DanmakuBroadcastViddupServiceImpl::OnDanmakuMsgFromContext(RefDictionary json, const std::string& json_content) {
    ui_proxy_->PostTask(
        FROM_HERE,
        base::BindLambda([this, json, json_content] {
        if (this->callback_) {
            this->callback_->OnDanmakuMessage(json,json_content);
        }
    }));
}

void DanmakuBroadcastViddupServiceImpl::OnAudienceNumFromContext(int num) {
    ui_proxy_->PostTask(
        FROM_HERE,
        base::BindLambda([this, num] {
        if (this->callback_) {
            this->callback_->OnAudienceNumber(num);
        }
    }));
}

void DanmakuBroadcastViddupServiceImpl::OnTokenExpiredFromContext() {
    ui_proxy_->PostTask(
        FROM_HERE,
        base::BindLambda([this] {
        if (this->callback_) {
            this->callback_->OnTokenExpired();
        }
    }));
}

void DanmakuBroadcastViddupServiceImpl::OnDanmakuACKFromContext(const std::set<uint32_t>& seqs) {
    ui_proxy_->PostTask(
        FROM_HERE,
        base::BindLambda([this, seqs] {
        if (this->callback_) {
            this->callback_->OnDanmakuACK(seqs);
        }
    }));
}

}