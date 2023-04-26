#include "danmaku_hime_presenter_impl.h"

#include <chrono>

#include "base/ext/callable_callback.h"
#include "base/ext/count_down_latch.h"

#include "bilibase/scope_guard.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"


namespace
{
    const int kRetryCount = 3;
    const int kRetryTimeout = 45;
}

DanmakuHimePresenterImpl::DanmakuHimePresenterImpl()
    : weak_ptr_factory_(this) {}

bool DanmakuHimePresenterImpl::IsInitialized()
{
    return is_initialized_;
}

std::string DanmakuHimePresenterImpl::RequestImageSync(const std::string& url)
{
    std::string result;
    base::CountdownLatch icon_ready(1);

    if (!url.empty())
    {
        GetBililiveProcess()->secret_core()->danmaku_hime_service()->DownloadImage(
            url,
            [&result, &icon_ready](bool valid_response, const std::string& data)
            {
                ON_SCOPE_EXIT{ icon_ready.Countdown(); };
                if (valid_response)
                {
                    result = data;
                }
            }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

            icon_ready.Wait();
    }

    return result;
}