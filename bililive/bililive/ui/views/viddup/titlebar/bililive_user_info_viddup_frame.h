#ifndef BILILIVE_BILILIVE_UI_VIEWS_TITLEBAR_BILILIVE_USER_INFO_VIDDUP_FRAME_H
#define BILILIVE_BILILIVE_UI_VIEWS_TITLEBAR_BILILIVE_USER_INFO_VIDDUP_FRAME_H

#include "base/memory/weak_ptr.h"
#include "base/notification/notification_registrar.h"
#include "base/notification/notification_observer.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/painter.h"
#include "ui/views/widget/widget_delegate.h"

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_bubble.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_user_related_ctrls.h"

#include "grit/theme_resources.h"


namespace
{
    class MedalView;
    class CaptainWarnTipButton;
    class GuardHoverTipButton;
}

class LivehimeUserPhotoView;
class BililiveProgressBar;

// The user information dialog box is displayed
class TitleBarUserInfoViddupView :
    public views::View,
    public views::ButtonListener,
    public base::NotificationObserver
{
public:
    static void ShowForm(views::View* anchor_view, views::View* detect_view);

protected:
    explicit TitleBarUserInfoViddupView(views::View* relation_view);
    virtual ~TitleBarUserInfoViddupView();

    // view
    void ViewHierarchyChanged(const View::ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // NotificationObserver
    void Observe(int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    void StartDoCheckMouse();
    void StopDoCheckMouse();

private:
    void InitViews();

    std::wstring GetLiveRoomUrl();

    void OnCheckMouse();

    void UpdateAvatar();

    void UpdateRoomInfo();

    void UpdateSan();

    void UpdateMedalView();

    void UpdateCaptainLoss();

private:
    views::View* relation_view_ = nullptr;

    LivehimeUserPhotoExView* avatar_ = nullptr;
    BililiveLabelButton* user_name_button_ = nullptr;
    BililiveLabel* room_id_label_ = nullptr;
    BililiveLabel* uid_label_ = nullptr;
    BililiveLabel* user_level_label_ = nullptr;
    BililiveProgressBar* level_xp_progress_ = nullptr;
    BililiveLabel* level_xp_label_ = nullptr;

    BililiveLabelButton* san_button_ = nullptr;
    BililiveLabelButton* fans_button_ = nullptr;
    BililiveLabelButton* sailors_button_ = nullptr;
    BililiveLabelButton* medal_fans_button_ = nullptr;

    BililiveLabelButton* live_room_button_ = nullptr;
    BililiveLabelButton* live_data_button_ = nullptr;

    BililiveLabelButton* logout_button_ = nullptr;
    MedalView* medal_view_ = nullptr;

    gfx::ImageSkia* medal_img_ = nullptr;
    gfx::ImageSkia* default_medal_img_ = nullptr;

    CaptainWarnTipButton* warn_button_ = nullptr;
    GuardHoverTipButton* sailors_icon_ = nullptr;
    BililiveSingleChildShowContainerView* container_view_ = nullptr;

    base::RepeatingTimer<TitleBarUserInfoViddupView> timer_;
    base::WeakPtrFactory<TitleBarUserInfoViddupView> weakptr_factory_;
    base::NotificationRegistrar notifation_register_;

    DISALLOW_COPY_AND_ASSIGN(TitleBarUserInfoViddupView);
};

#endif