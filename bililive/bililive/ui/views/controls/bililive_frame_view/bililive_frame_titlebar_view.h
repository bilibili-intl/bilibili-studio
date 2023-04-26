#ifndef BILILIVE_BILILIVE_UI_VIEWS_TITLEBAR_TITLE_BAR_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TITLEBAR_TITLE_BAR_VIEW_H

#include "bililive_frame_titlebar.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"


class BililiveLabel;

class BililiveFrameTitleBarView
    : public BililiveFrameTitleBar
    , public views::ButtonListener
{
public:
    BililiveFrameTitleBarView(gfx::ImageSkia *icon, base::string16 caption,
        int titlebar_button_combination,bool title_center =false);
    virtual ~BililiveFrameTitleBarView();

    void SetTitle(const base::string16& title) override;

protected:
    // View
    gfx::Size GetPreferredSize() override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // BililiveFrameTitleBar
    void OnActiveChanged(bool prev_state) override;

    virtual void ChangeMaxButtonStateImage();

private:
    void InitViews(int button_combination);

private:
    bool title_center_ = false;
    views::ImageView *icon_image_view_;
    BililiveLabel *caption_label_;
    std::map<TitleBarButton, views::ImageButton*> button_map_;

    DISALLOW_COPY_AND_ASSIGN(BililiveFrameTitleBarView);
};

namespace {
    int kGeneralMsgBoxTitleBarPadding = GetLengthByDPIScale(16);
}

class GeneralMsgBoxTitleBarView
    : public BililiveFrameTitleBar
    , public views::ButtonListener
{
public:
    GeneralMsgBoxTitleBarView( base::string16 caption );
    virtual ~GeneralMsgBoxTitleBarView();

    void SetTitle(const base::string16& title) override;
    void SetCloseBtnVisible(bool flag);
protected:
    // View
    gfx::Size GetPreferredSize() override;
    bool OnMousePressed(const ui::MouseEvent& event) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // BililiveFrameTitleBar
    void OnActiveChanged(bool prev_state) override;

private:
    void InitViews();

private:
    BililiveLabel* caption_label_;
    views::ImageButton* button_;
    DISALLOW_COPY_AND_ASSIGN(GeneralMsgBoxTitleBarView);
};

#endif