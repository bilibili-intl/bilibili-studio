#ifndef BILILIVE_BILILIVE_VIDDUP_DANMAKU_HIME_SEND_DANMAKU_PRESENTER_H_
#define BILILIVE_BILILIVE_VIDDUP_DANMAKU_HIME_SEND_DANMAKU_PRESENTER_H_

#include "base/memory/weak_ptr.h"
#include "base/strings/string16.h"


namespace contracts
{
    class SendDanmakuViddupView
    {
    public:
        virtual ~SendDanmakuViddupView() = default;

        virtual void OnDanmakuSent(
            bool valid_response, bool sending_succeeded, const string16& msg) = 0;
    };

    class SendDanmakuViddupPresenter
    {
    public:
        explicit SendDanmakuViddupPresenter(SendDanmakuViddupView* view);

        void SendDanmaku(const string16& msg);

    private:
        void OnRequestSendDanmaku(bool valid_response, int code, const std::string& msg);

        SendDanmakuViddupView* view_;
        base::WeakPtrFactory<SendDanmakuViddupPresenter> wpf_;
    };

}

#endif  // BILILIVE_BILILIVE_VIDDUP_DANMAKU_HIME_SEND_DANMAKU_PRESENTER_H_