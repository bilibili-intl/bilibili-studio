#include "livehime_search_list_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"



LivehimeSearchListView::LivehimeSearchListView(BilibiliSearchListListener *listener, views::View *relate_view/* = nullptr*/,
    BilibiliNativeEditView::CooperateDirection dir/* = BilibiliNativeEditView::BD_RIGHT*/)
    : BilibiliSearchListView(listener, relate_view, dir)
{
    SetBackgroundColor(clrWindowsContent);
    SetBorderColor(clrCtrlBorderNor, clrCtrlBorderHov, clrDroplistBorder);
    SetTextColor(clrLabelContent);
    SetFont(ftPrimary);
    SetPlaceHolderTextColor(clrPlaceholderText);
    SetHorizontalMargins(kCtrlLeftInset, kCtrlRightInset);
}

LivehimeSearchListView::~LivehimeSearchListView()
{
}

gfx::Size LivehimeSearchListView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(kBorderCtrlHeight);
    return size;
}

int LivehimeSearchListView::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}
