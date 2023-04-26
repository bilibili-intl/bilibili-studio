#ifndef LIVEHIME_SEARCH_LIST_VIEW_H
#define LIVEHIME_SEARCH_LIST_VIEW_H

#include "bililive/bililive/ui/views/controls/bilibili_search_list_view.h"


class LivehimeSearchListView : public BilibiliSearchListView
{
public:
    LivehimeSearchListView(BilibiliSearchListListener *listener, views::View *relate_view = nullptr,
        BilibiliNativeEditView::CooperateDirection dir = BilibiliNativeEditView::BD_RIGHT);
    virtual ~LivehimeSearchListView();

protected:
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual int GetHeightForWidth(int w) OVERRIDE;

};

#endif
