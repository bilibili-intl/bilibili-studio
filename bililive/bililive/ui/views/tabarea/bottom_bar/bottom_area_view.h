#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_AREA_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_AREA_VIEW_H

#include "bottom_area_popup_view.h"

#include "base/notification/notification_observer.h"
#include "base/notification/notification_source.h"
#include "base/notification/notification_registrar.h"

#include "ui/views/controls/button/button.h"

namespace
{
    class BottomBarFunctionButton;
}

class BottomAreaDelegate
{
public:
    virtual void OnRefreshButtonPressed() = 0;
};


// Ñ¡Ïî¿¨±êÇ©
class BottomAreaView
    : public views::View
    , public views::ButtonListener
    , public base::NotificationObserver
{
public:
    explicit BottomAreaView(BottomAreaDelegate *delegate);

    bool IsBottomAreaFunctionViewPopupNow() const;

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details) override;

private:
    BottomAreaDelegate *delegate_;
    BottomBarFunctionButton *function_buttons_[TabareaBottomFunction_COUNT];
    base::NotificationRegistrar notifation_register_;

    DISALLOW_COPY_AND_ASSIGN(BottomAreaView);

};

#endif