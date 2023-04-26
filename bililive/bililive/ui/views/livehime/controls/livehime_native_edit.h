#ifndef LIVEHIME_NATIVE_EDIT_H
#define LIVEHIME_NATIVE_EDIT_H

#include "bililive/bililive/ui/views/controls/bilibili_native_edit_view.h"


class LivehimeNativeEditView : public BilibiliNativeEditView
{
public:
    explicit LivehimeNativeEditView(views::View *view, CooperateDirection dir = BD_RIGHT, StyleFlags style = STYLE_DEFAULT);
    LivehimeNativeEditView(StyleFlags style = STYLE_DEFAULT);
    virtual ~LivehimeNativeEditView();

    void SetMinHeight(int height);
protected:
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual int GetHeightForWidth(int w) OVERRIDE;

    // BilibiliNativeEditView
    virtual void OnInvalidInput() OVERRIDE;

private:
    void SetCommonStyle();
    int min_height_ = 0;
};

#endif
