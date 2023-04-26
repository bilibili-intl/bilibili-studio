#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_BANNED_END_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_BANNED_END_VIEW_H

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "ui/views/view.h"



class BannedEndDelegate {
public:
    virtual void BannedOff() = 0;
};

class BannedEndView
    : public views::View,
    public views::ButtonListener {
public:
    explicit BannedEndView(BannedEndDelegate *delegate);

    virtual ~BannedEndView();

    void set_label_text(const string16& text);

private:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void InitView();

private:
    BililiveImageButton* switch_icon_ = nullptr;

    LivehimeContentLabel* banned_label_ = nullptr;

    LivehimeContentLabel* banned_time_label_ = nullptr;

    LivehimeTipLabel* banned_tips_ = nullptr;

    BannedEndDelegate *delegate_ = nullptr;
};

#endif //BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_BANNED_END_VIEW_H