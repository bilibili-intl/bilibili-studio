#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_WIDGET_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_WIDGET_H_

#include "bililive/bililive/livehime/main_view/livehime_main_close_contract.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"

class LivehimeMainCloseView
    : public BililiveWidgetDelegate,
      public views::ButtonListener
{
public:
    static void DoModal(gfx::NativeView par, const EndDialogSignalHandler* handler);

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
    LivehimeMainCloseView();

    virtual ~LivehimeMainCloseView();

    void InitView();

    void InitData();

    void SaveData();

private:
    LivehimeRadioButton* min_radio_button_;
    LivehimeRadioButton* exit_radio_botton_;
    LivehimeCheckbox* remember_choice_;

    LivehimeActionLabelButton* ok_button_;
    LivehimeActionLabelButton* cancel_button_;

    int radio_button_groupid_;

    std::unique_ptr<contracts::LivehimeClosePresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeMainCloseView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_CLOSE_WIDGET_H_
