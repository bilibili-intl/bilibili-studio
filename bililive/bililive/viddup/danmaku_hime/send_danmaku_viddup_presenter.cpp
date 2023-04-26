#include "send_danmaku_viddup_presenter.h"

#include <chrono>

#include "base/ext/callable_callback.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/resource/resource_bundle.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "grit/generated_resources.h"

namespace contracts
{

    SendDanmakuViddupPresenter::SendDanmakuViddupPresenter(SendDanmakuViddupView* view)
        : view_(view),
        wpf_(this)
    {
    }

    void SendDanmakuViddupPresenter::SendDanmaku(const string16& msg)
    {
        auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
        auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        auto handler = base::MakeCallable(base::Bind(
            &SendDanmakuViddupPresenter::OnRequestSendDanmaku, wpf_.GetWeakPtr()));

        GetBililiveProcess()->secret_core()->danmaku_hime_service()->SendViddupDanmaku(
            room_id, UTF16ToUTF8(msg), ts, 25, 1, handler).Call();
    }

    void SendDanmakuViddupPresenter::OnRequestSendDanmaku(
        bool valid_response, int code, const std::string& msg)
    {
        if (view_)
        {
            if (!valid_response)
            {
                view_->OnDanmakuSent(valid_response, false, {});
                return;
            }

            string16 final_msg;
            bool sending_succeeded = false;
            if (code == 0)
            {
                sending_succeeded = true;
            }
            else
            {
                final_msg = UTF8ToUTF16(msg);
                auto room_id = GetBililiveProcess()->secret_core()->user_info().room_id();
            }
            view_->OnDanmakuSent(true, sending_succeeded, final_msg);
        }
    }

}
