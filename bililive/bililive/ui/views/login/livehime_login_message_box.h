#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LOGIN_MESSAGEBOX_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LOGIN_MESSAGEBOX_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/secret/public/login_info.h"

class LivehimeLoginDeleteAccountView
    : public BililiveWidgetDelegate,
      public views::ButtonListener
{
public:
    enum OptionType
    {
        Cancel,
        DelRecord,
        DelDirectory,
    };

    static void DoModal(const secret::LoginInfo& account_info, 
        gfx::NativeView par, const EndDialogSignalHandler* handler);

protected:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;

    // views::ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    explicit LivehimeLoginDeleteAccountView(const secret::LoginInfo& account_info);
    virtual ~LivehimeLoginDeleteAccountView();

    void InitView();

private:
    secret::LoginInfo account_info_;
    LivehimeRadioButton* just_record_radio_button_ = nullptr;
    LivehimeRadioButton* all_info_radio_botton_ = nullptr;

    LivehimeActionLabelButton* ok_button_ = nullptr;
    LivehimeActionLabelButton* cancel_button_ = nullptr;

    int radio_button_groupid_ = -1;

    DISALLOW_COPY_AND_ASSIGN(LivehimeLoginDeleteAccountView);
};

#endif
