#include "livehime_native_edit.h"
#include "livehime_hover_tip_button.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


namespace
{
    //class InvalidInputBubble : public LivehimeTipsBubble
    //{
    //public:
    //    explicit InvalidInputBubble(views::View *anchor, const base::string16 &title, const base::string16 &tips, views::BubbleBorder::Arrow arrow)
    //        : LivehimeTipsBubble(anchor, title, tips, arrow)
    //    {

    //    }
    //    virtual ~InvalidInputBubble() = default;

    //protected:
    //    // View
    //    gfx::Size GetPreferredSize() override
    //    {
    //        //static int cx = LivehimePaddingCharWidth(ftThirteen) * 13;
    //        gfx::Size size = BililiveBubble::GetPreferredSize();
    //        //size.set_width(cx);
    //        return size;
    //    }

    //private:
    //};
}


LivehimeNativeEditView::LivehimeNativeEditView(StyleFlags style/* = STYLE_DEFAULT*/)
    : BilibiliNativeEditView(style)
{
    SetCommonStyle();
}

LivehimeNativeEditView::LivehimeNativeEditView(views::View *view, 
    CooperateDirection dir /*= BD_RIGHT*/, StyleFlags style/* = STYLE_DEFAULT*/)
    : BilibiliNativeEditView(view, dir, style)
{
    SetCommonStyle();
}

LivehimeNativeEditView::~LivehimeNativeEditView()
{
}

gfx::Size LivehimeNativeEditView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();

    if (min_height_ > 0) {     //TODO
        size.set_height(min_height_);
    }
    else {
        size.set_height(kBorderCtrlHeight);
    }
    return size;
}

int LivehimeNativeEditView::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeNativeEditView::SetMinHeight(int height) {
    min_height_ = height;
}

void LivehimeNativeEditView::SetCommonStyle()
{
    SetBackgroundColor(SkColorSetRGB(56, 57, 70)); // rgb(56, 57, 70)
    SetTextColor(clrLabelContent);
    SetFont(ftPrimary);
    set_placeholder_text_color(clrPlaceholderText);
    RemoveBorder();

}

void LivehimeNativeEditView::OnInvalidInput()
{
    //LivehimeTipsBubble::Show(this, L"不能接受的字符", L"你只能在此键入数字。", views::BubbleBorder::TOP_CENTER);
}
