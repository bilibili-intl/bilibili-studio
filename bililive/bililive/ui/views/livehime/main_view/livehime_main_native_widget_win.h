#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LIVEHIME_MAIN_VIEW_NATIVEWIDGETWIN_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LIVEHIME_MAIN_VIEW_NATIVEWIDGETWIN_H

#include "ui/views/widget/native_widget_win.h"


class BilibiliLivehimeNativeWidgetWin
    : public views::NativeWidgetWin
{
public:
    explicit BilibiliLivehimeNativeWidgetWin(views::internal::NativeWidgetDelegate* delegate);
    virtual ~BilibiliLivehimeNativeWidgetWin();

protected:
    // Overridden from HWNDMessageHandlerDelegate:
    bool PreHandleMSG(UINT message,
        WPARAM w_param,
        LPARAM l_param,
        LRESULT *result) override;
    void PostHandleMSG(UINT message,
        WPARAM w_param,
        LPARAM l_param) OVERRIDE;
    bool GetClientAreaInsets(gfx::Insets *insets) const override;
    void HandleFrameChanged() override;

private:
    void OnMiniMainWidget(const string16& btn, void* data);
    void MiniMainWidget();
private:
    views::internal::NativeWidgetDelegate* delegate_;
    bool mini_dlg_sure_ = false;

    base::WeakPtrFactory<BilibiliLivehimeNativeWidgetWin> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(BilibiliLivehimeNativeWidgetWin);
};

#endif