#ifndef LIVEHIME_SUB_EDIT_H
#define LIVEHIME_SUB_EDIT_H

#include "bililive/bililive/ui/views/controls/bilibili_native_edit_view.h"


class LivehimeSubEditView : public BilibiliNativeEditView

{
public:
    explicit LivehimeSubEditView(views::View *view, CooperateDirection dir = BD_CORNER, StyleFlags style = STYLE_DEFAULT);
    LivehimeSubEditView(StyleFlags style = STYLE_DEFAULT);
    virtual ~LivehimeSubEditView();

protected:
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual int GetHeightForWidth(int w) OVERRIDE;

private:
    void SetCommonStyle();
};

#endif
