#ifndef BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_HOTKEY_NOTIFY_BILILIVE_TOAST_NOTIFY_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_HOTKEY_NOTIFY_BILILIVE_TOAST_NOTIFY_VIEW_H


#include "ui/gfx/image/image_skia.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/gfx/font.h"

#include "base/timer/timer.h"

class ToastNotifyView : public views::WidgetDelegateView
{
public:

    static void ShowForm(const base::string16 &text, gfx::Rect show_area,
        gfx::ImageSkia *image = NULL, const gfx::Font *font = NULL,
        int delay_time = 5 );   //定时器500ms，默认展示为2.5s，业务层可控制展示时间

protected:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }

    // View
    void OnPaintBackground(gfx::Canvas *canvas) override;

    explicit ToastNotifyView(const base::string16 &text, gfx::ImageSkia *image, const gfx::Font *font,int delay_time);
    virtual ~ToastNotifyView();

private:
    void OnDeltaFadeOut();
    void OnFadeOutTimer();
    void ShowWidget();
    void AdjustPosition(gfx::Rect show_area);

private:

    gfx::ImageSkia image_;
    base::string16 text_;
    base::RepeatingTimer<ToastNotifyView> timer_;
    int  delay_time_;
    gfx::Font font_;
    unsigned char alpha_;

    base::WeakPtrFactory<ToastNotifyView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ToastNotifyView);
};

#endif