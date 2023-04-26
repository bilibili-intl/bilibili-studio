#pragma once

#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_view_theme.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"


class DanmakuInteractionTitleViddupView :
    public views::View,
    public DanmakuThemeInterface
{
public:
    static const int kViewHeight;

public:
    DanmakuInteractionTitleViddupView();
    virtual ~DanmakuInteractionTitleViddupView();

    //DanmakuThemeInterface
    void SwitchThemeImpl() override;

    void EnableEffect(bool enable);
    void Lock(bool lock);
    void Pin(bool pin);
    void SetNoSubpixelRendering(bool no_subpixel_rendering);
    void SetFunctionButtonBright(bool bright);
    void UpdateFunctionAreaApplyCount(int apply_count);
    void UpdateFunctionButtonVisible();
    void UpdateSettingBtRedPointStatus();
    void ShowCoreUserDisturbBubble();

private:
    void InitView();

    BililiveLabel* title_label_ = nullptr;
};