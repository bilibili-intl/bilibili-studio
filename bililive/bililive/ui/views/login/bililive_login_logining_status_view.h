#ifndef BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_LOGINING_STATUS_VIEW_H
#define BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_LOGINING_STATUS_VIEW_H

#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"

#include "bililive/bililive/ui/views/login/bilibili_login_control.h"

class LivehimeCircleImageView;

class LoginingStatusDelegate {
public:
    virtual ~LoginingStatusDelegate() = default;

    virtual void OnLoginingStatusStop() = 0;
    virtual void OnLoginingStatusMsgChanged(const string16& msg) = 0;
};

class LoginingStatusView :
    public views::View,
    public views::ButtonListener {
public:
    explicit LoginingStatusView(LoginingStatusDelegate* delegate);

    virtual ~LoginingStatusView();

    void ShowBeLogining();

    void ShowErrorMessage(const string16& msg);

    void SetUserFace(int64 mid);

private:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void InitView();

    void StartReturnTimer();

    void OnTimerArrive();

private:
    LoginingStatusDelegate* delegate_ = nullptr;

    LivehimeCircleImageView* user_pic_ = nullptr;

    views::View* user_pic_view_ = nullptr;

    views::Label *info_label_ = nullptr;

    BilibiliLoginLabelButton *cancel_btn_ = nullptr;

    scoped_ptr<base::Timer> animation_timer;

    base::WeakPtrFactory<LoginingStatusView> weakptr_factory_;
};

#endif // BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_LOGINING_STATUS_VIEW_H