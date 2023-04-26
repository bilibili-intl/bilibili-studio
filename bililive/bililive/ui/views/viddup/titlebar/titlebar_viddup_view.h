#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TITLEBAR_TITLEBAR_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TITLEBAR_TITLEBAR_VIEW_H

#include "base/prefs/pref_change_registrar.h"
#include "bililive/bililive/ui/views/viddup/titlebar/bililive_user_info_viddup_frame.h"

#include "base/memory/weak_ptr.h"

#include "ui/views/widget/widget_observer.h"
#include "ui/views/controls/menu/menu_delegate.h"

namespace
{
    class ButtonArea;
    class HeadshotButton;
    class AnchorNoticeButton;
    class TitlebarImageButton;
}

namespace views
{
    class ImageButton;
}

class BililiveLabelButton;

// The title bar of the main screen
class TitleBarViddupView
    : public views::View
    , views::WidgetObserver
    , views::ButtonListener
    , views::MenuDelegate
    , base::NotificationObserver
{
public:
    TitleBarViddupView();
    virtual ~TitleBarViddupView();

    int GetHTComponentForFrame(const gfx::Point &point);
    void SetBusinessFunctionButtonVisible(bool visible);

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void PaintChildren(gfx::Canvas* canvas) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // MenuDelegate
    void ExecuteCommand(int command_id, int event_flags) override;

    // WidgetObserver
    void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override;

    // NotificationObserver
    void Observe(int type,
                 const base::NotificationSource& source,
                 const base::NotificationDetails& details) override;

private:
    void InitViews();
    void InitUserInfo();
    void UpdateRoomInfo();
    void UpdateAvatar();
    void DrawImage(const gfx::ImageSkia& img);
    void OnGlobalSettingRedPointShowChanged();
    void ShowWebWidget(std::string url);

private:
    views::ImageView* image_view_ = nullptr;
    HeadshotButton* headshot_button_ = nullptr;
    TitlebarImageButton* menu_button_ = nullptr;
    AnchorNoticeButton* notices_button_ = nullptr;
    views::View* animate_notice_view_ = nullptr;
   // TitlebarImageButton* share_button_ = nullptr;
    views::ImageButton* mini_button_ = nullptr;
    views::ImageButton* max_button_ = nullptr;
    views::ImageButton* close_button_ = nullptr;
    ButtonArea* button_area_ = nullptr;

    bool setting_red_point_ = false;

    PrefChangeRegistrar profile_pref_registrar_;
    base::NotificationRegistrar notifation_register_;

    base::WeakPtrFactory<TitleBarViddupView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TitleBarViddupView);
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TITLEBAR_TITLEBAR_VIEW_H