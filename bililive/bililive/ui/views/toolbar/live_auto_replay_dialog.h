#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TOOLBAR_LIVE_REPLAY_DIALOG_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TOOLBAR_LIVE_REPLAY_DIALOG_H_

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/livehime/feedback/feedback_presenter.h"

class LivehimeActionLabelButton;

class LiveAutoReplayDialog
    : public BililiveWidgetDelegate,
      public views::ButtonListener
{
public:
    explicit LiveAutoReplayDialog();
    ~LiveAutoReplayDialog();

    static void ShowWindow(views::Widget* parent);

protected:

    gfx::Size GetPreferredSize() override;

    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

    // views::ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
private:

    void InitViews();

private:
    static LiveAutoReplayDialog* instance_;

    LivehimeActionLabelButton* ok_button_ = nullptr;
    LivehimeActionLabelButton* cancel_button_ = nullptr;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TOOLBAR_LIVE_REPLAY_DIALOG_H_