#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_DANMAKU_SEARCH_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_DANMAKU_SEARCH_VIEW_H

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"

#include "ui/views/view.h"

class DanmakuSearchView
    : public views::View,
    public BilibiliNativeEditController,
    public views::ButtonListener {
public:
    explicit DanmakuSearchView();

    virtual ~DanmakuSearchView();

private:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    bool OnKeyPressed(const ui::KeyEvent& event) override;

    void InitView();

    void Search();

private:
    LivehimeTextfield* search_edit_ = nullptr;

    views::ImageButton* search_btn_ = nullptr;

    LivehimeContentLabel* search_label_ = nullptr;

    LivehimeCombobox* search_combobox_ = nullptr;
};

#endif //BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_DANMAKU_SEARCH_VIEW_H