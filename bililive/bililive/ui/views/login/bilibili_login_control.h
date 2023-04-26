#ifndef BILILIVE_SECRET_UI_VIEWS_HIDE_AWARE_VIEW_H
#define BILILIVE_SECRET_UI_VIEWS_HIDE_AWARE_VIEW_H

#include "ui/views/bubble/bubble_border.h"
#include "ui/views/bubble/bubble_delegate.h"
#include "ui/views/controls/button/checkbox.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/view.h"

#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/ui/views/controls/bililive_textfield.h"

#include "bililive/bililive/ui/views/controls/bilibili_native_edit_view.h"

class TipBubble : public views::BubbleDelegateView {
public:
    explicit TipBubble(View* anchor, const string16& tip)
        : BubbleDelegateView(anchor, views::BubbleBorder::TOP_LEFT),
        tip_(tip){}

protected:
    void Init() override {
        SetLayoutManager(new views::FillLayout());
        AddChildView(new views::Label(tip_));
    }
private:
    string16 tip_;
};

class HideAwareView : public views::View
{
protected:
    gfx::Size GetPreferredSize() override
    {
        gfx::Size size = visible() ? View::GetPreferredSize() : gfx::Size();
        return size;
    }

    int GetHeightForWidth(int w) override
    {
        return visible() ? View::GetHeightForWidth(w) : 0;
    }
};

// BilibiliLoginLabelButton
class BilibiliLoginLabelButton : public BililiveLabelButton
{
public:
    explicit BilibiliLoginLabelButton(views::ButtonListener* listener, const string16& text);
};

class LoginTextfield : public BilibiliNativeEditView
{
public:
    LoginTextfield(StyleFlags style = STYLE_DEFAULT);
    LoginTextfield(views::View *view, CooperateDirection dir = BD_RIGHT, StyleFlags style = STYLE_DEFAULT);
    virtual ~LoginTextfield();

protected:
    // View
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

private:
    void SetCommonStyle();
};

#endif