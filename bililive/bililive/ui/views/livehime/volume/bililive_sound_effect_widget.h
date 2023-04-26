#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_VOLUME_BILILIVE_SOUND_EFFECT_WIDGET_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_VOLUME_BILILIVE_SOUND_EFFECT_WIDGET_H

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"

class BililiveSoundEffectView;

class BililiveSoundEffectWidget
    : public BililiveWidgetDelegate,
      public views::ButtonListener
{
public:
    static void ShowForm(views::Widget *parent);

protected:
    BililiveSoundEffectWidget();

    virtual ~BililiveSoundEffectWidget();

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;

    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // WidgetDelegate
    views::View *GetContentsView() override{ return this; }

    void WindowClosing() override;

    ui::ModalType GetModalType() const override{ return ui::MODAL_TYPE_WINDOW; }

private:
    void InitViews();

private:
    static BililiveSoundEffectWidget* instance_;

    BililiveSoundEffectView* sound_effect_view_;

    LivehimeActionLabelButton* ok_button_;
    LivehimeActionLabelButton* cancel_button_;

    DISALLOW_COPY_AND_ASSIGN(BililiveSoundEffectWidget);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_VOLUME_BILILIVE_SOUND_EFFECT_WIDGET_H