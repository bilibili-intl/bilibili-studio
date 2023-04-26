#ifndef LIVEHIME_INSIDE_FRAME_TITLEBAR_H
#define LIVEHIME_INSIDE_FRAME_TITLEBAR_H

#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_titlebar.h"

#include "ui/views/controls/button/button.h"

class BililiveLabel;

class LivehimeInsideFrameTitleBar 
    : public BililiveFrameTitleBar
    , public views::ButtonListener
{
public:
    explicit LivehimeInsideFrameTitleBar(const base::string16 &title);

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void OnThemeChanged() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitViews();

private:
    BililiveLabel *caption_label_ = nullptr;
};

#endif
