#pragma once

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

class BililiveLabelButton;

class JoinSlideTipsView
    : public BililiveWidgetDelegate,
    public views::ButtonListener
{
public:
    explicit JoinSlideTipsView();
    virtual ~JoinSlideTipsView();
    static void ShowForm(views::Widget *parent,
        const EndDialogSignalHandler *handler = nullptr);

protected:
    // view
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    // views::ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    gfx::Size GetPreferredSize() override;

    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    void WindowClosing() override;

private:
    BililiveLabelButton* btn_ok_ = nullptr;
    BililiveLabelButton* btn_cancel_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(JoinSlideTipsView);
};
