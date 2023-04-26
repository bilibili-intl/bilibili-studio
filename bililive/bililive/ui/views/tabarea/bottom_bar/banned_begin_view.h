#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_BANNED_BEGIN_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_BANNED_BEGIN_VIEW_H

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/settings/fixed_slider.h"

#include "ui/views/view.h"

namespace {

enum BannedType {
    TYPE_ALL = 0,
    TYPE_LEVEL,
};

const char kOff[] = "off";
const int kDefaultLevel = 1;
const int kDefaultMinute = 0;

}

class BannedBeginDelegate {
public:
    virtual void BannedOn(int minute, const std::string& type, int level ) = 0;
};

class BannedBeginView
    : public views::View,
    public views::ButtonListener,
    public views::SliderListener,
    public BililiveComboboxExListener {
public:
    explicit BannedBeginView(BannedBeginDelegate *delegate);

    virtual ~BannedBeginView();

    void ResetCombobox();

private:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // SliderListener
    void SliderValueChanged(views::Slider* sender,
        float value,
        float old_value,
        views::SliderChangeReason reason) override;

    // BililiveComboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    void InitView();

private:
    BililiveImageButton* switch_icon_ = nullptr;

    LivehimeContentLabel* banned_label_ = nullptr;

    LivehimeContentLabel* banned_time_label_ = nullptr;

    LivehimeContentLabel* banned_type_label_ = nullptr;

    LivehimeContentLabel* banned_level_ = nullptr;

    LivehimeTipLabel* banned_tips_ = nullptr;

    LivehimeCombobox* banned_combobox_ = nullptr;

    LivehimeCombobox* banned_type_combobox_ = nullptr;

    LivehimeSlider* level_slider_ = nullptr;

    views::View* slider_view_ = nullptr;

    BannedBeginDelegate *delegate_ = nullptr;
};

#endif //BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_BANNED_BEGIN_VIEW_H