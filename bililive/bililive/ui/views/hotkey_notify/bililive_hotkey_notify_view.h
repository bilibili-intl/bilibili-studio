#ifndef BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_HOTKEY_NOTIFY_BILILIVE_HOTKEY_NOTIFY_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_HOTKEY_NOTIFY_BILILIVE_HOTKEY_NOTIFY_VIEW_H


#include "ui/gfx/image/image_skia.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/gfx/font.h"

#include "base/timer/timer.h"


class HotkeyNotifyView : public views::WidgetDelegateView
{
public:
    enum HotkeyNotify
    {
        HN_NOSUPPORT = -1,
        HN_LiveStart,
        HN_LiveStop,
        HN_RecordStart,
        HN_RecordStop,
        HN_MicMute,
        HN_MicRestore,
        HN_SystemVolumeMute,
        HN_SystemVolumeRestore,

        HN_SceneSwitch1,
        HN_SceneSwitch2,
        HN_SceneSwitch3,
        HN_SceneSwitch4,
        HN_SceneSwitch5,
        HN_SceneSwitch6,

        HN_ClearDanmakuTTSQueue,

        HN_NUM_COUNT,
    };

    static void ShowForm(HotkeyNotify hotkey_notify, gfx::Rect show_area);
    static void ShowForm(const base::string16 &text, gfx::Rect show_area,
        gfx::ImageSkia *image = NULL, const gfx::Font *font = NULL);

protected:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    
    // View
    void OnPaintBackground(gfx::Canvas *canvas) override;

    explicit HotkeyNotifyView(HotkeyNotify hotkey_notify);
    explicit HotkeyNotifyView(const base::string16 &text, gfx::ImageSkia *image, const gfx::Font *font);
    virtual ~HotkeyNotifyView();

private:
    void OnDeltaFadeOut();
    void OnFadeOutTimer();
    void ShowWidget();
    void AdjustPosition(gfx::Rect show_area);

private:
    HotkeyNotify hotkey_notify_;
    gfx::ImageSkia image_;
    base::string16 text_;
    base::RepeatingTimer<HotkeyNotifyView> timer_;
    unsigned char alpha_;
    gfx::Font font_;

    base::WeakPtrFactory<HotkeyNotifyView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(HotkeyNotifyView);
};

#endif