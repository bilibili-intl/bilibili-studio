#include "bililive_hotkey_notify_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

#include "ui/views/widget/widget.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/platform_font.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


namespace
{
    HotkeyNotifyView* last_notify_toast = nullptr;
}

void HotkeyNotifyView::ShowForm(HotkeyNotify hotkey_notify, gfx::Rect show_area)
{
    HotkeyNotifyView* hnv = new HotkeyNotifyView(hotkey_notify);
    hnv->AdjustPosition(show_area);
}

void HotkeyNotifyView::ShowForm(const base::string16 &text, gfx::Rect show_area,
    gfx::ImageSkia *image/* = NULL*/, const gfx::Font *font/* = NULL*/) {

    HotkeyNotifyView* hnv = new HotkeyNotifyView(text, image, font);
    hnv->AdjustPosition(show_area);
}

HotkeyNotifyView::HotkeyNotifyView(HotkeyNotify hotkey_notify)
    : hotkey_notify_(hotkey_notify)
    , alpha_(250)
    , weakptr_factory_(this)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    switch (hotkey_notify_)
    {
    case HN_LiveStart:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_LIVE_START);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_LIVE_START);
        break;
    case HN_LiveStop:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_LIVE_STOP);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_LIVE_STOP);
        break;
    case HN_RecordStart:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_RECORD_START);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_RECORD_START);
        break;
    case HN_RecordStop:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_RECORD_STOP);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_RECORD_STOP);
        break;
    case HN_MicMute:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_MIC_MUTE);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_MIC_MUTE);
        break;
    case HN_MicRestore:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_MIC_UNMUTE);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_MIC_UNMUTE);
        break;
    case HN_SystemVolumeMute:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_SYSTEM_MUTE);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_SYSTEM_MUTE);
        break;
    case HN_SystemVolumeRestore:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_SYSTEM_UNMUTE);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_SYSTEM_UNMUTE);
        break;
    case HN_SceneSwitch1:
    case HN_SceneSwitch2:
    case HN_SceneSwitch3:
    case HN_SceneSwitch4:
    case HN_SceneSwitch5:
    case HN_SceneSwitch6:
        text_ = rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + 
            std::to_wstring(hotkey_notify_ - HN_SceneSwitch1 + 1);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEMAIN_HOTKEY_SCENE_SWITCH);
        break;
    case HN_ClearDanmakuTTSQueue:
        text_ = rb.GetLocalizedString(IDS_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE);
        image_ = *rb.GetImageSkiaNamed(IDR_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE);
        break;
    default:
        break;
    }
    font_ = ftTwenty;
    ShowWidget();
}

HotkeyNotifyView::HotkeyNotifyView(const base::string16 &text, gfx::ImageSkia *image, const gfx::Font *font)
    : hotkey_notify_(HN_NOSUPPORT)
    , alpha_(250)
    , text_(text)
    , weakptr_factory_(this)
{
    if (image)
    {
        image_ = *image;
    }
    if (font)
    {
        font_ = *font;
    }
    else
    {
        font_ = gfx::Font((new gfx::Font(gfx::PlatformFont::CreateDefault()))->DeriveFont(10, gfx::Font::NORMAL/*BOLD*/));
    }
    ShowWidget();
}

HotkeyNotifyView::~HotkeyNotifyView()
{
    if (this == last_notify_toast)
    {
        last_notify_toast = nullptr;
    }
}

void HotkeyNotifyView::ShowWidget()
{
    // 需要显示新的toast就把旧的toast关了，省得同一个窗口短时间内提示多次导致重叠显示
    if (last_notify_toast)
    {
        last_notify_toast->GetWidget()->Close();
    }
    last_notify_toast = this;

    views::BoxLayout *layout = new views::BoxLayout(views::BoxLayout::kHorizontal, 12, 12, 12);
    SetLayoutManager(layout);

    views::ImageView *image = new views::ImageView();
    image->SetImage(image_);
    views::Label *label = new views::Label(text_, font_);
    label->SetAutoColorReadabilityEnabled(false);
    label->SetEnabledColor(SK_ColorWHITE);
    label->SetBackgroundColor(0);
    AddChildView(image);
    AddChildView(label);

    views::Widget *widget = new views::Widget();
    widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    params.remove_standard_frame = true;
    params.delegate = this;
    params.remove_taskbar_icon = true;
    params.double_buffer = true;
    params.keep_on_top = true;
    params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
    params.accept_events = false;
    widget->Init(params);
    widget->ShowInactive();

    //widget->CenterWindow(widget->GetContentsView()->GetPreferredSize());

    BililiveThread::PostDelayedTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&HotkeyNotifyView::OnDeltaFadeOut, weakptr_factory_.GetWeakPtr()), base::TimeDelta::FromSeconds(1));
}

void HotkeyNotifyView::AdjustPosition(gfx::Rect show_area)
{
    gfx::Rect hnv_s = GetWidget()->GetWindowBoundsInScreen();
    GetWidget()->SetBounds(gfx::Rect(
        show_area.x() + (show_area.width() - hnv_s.width()) / 2, 
        show_area.y() + (show_area.height() - hnv_s.height()) / 2,
        hnv_s.width(), hnv_s.height()));
}

void HotkeyNotifyView::OnPaintBackground(gfx::Canvas *canvas)
{
    U8CPU alpha = 128;
    gfx::Rect rtTrans(0, 0, bounds().width(), bounds().height());
    SkPaint paint;
    paint.setAlpha(alpha);
    canvas->DrawRoundRect(rtTrans, 5, paint);
    paint.setColor(SkColorSetARGB(alpha, 100, 100, 100));
    bililive::DrawRoundRect(canvas, rtTrans.x(), rtTrans.y(), rtTrans.width(), rtTrans.height(), 5, paint);
}

void HotkeyNotifyView::OnDeltaFadeOut()
{
    timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(100),
        base::Bind(&HotkeyNotifyView::OnFadeOutTimer, weakptr_factory_.GetWeakPtr()));
}

void HotkeyNotifyView::OnFadeOutTimer()
{
    alpha_ -= 25;
    GetWidget()->SetOpacity(alpha_);

    if (alpha_ <= 0)
    {
        timer_.Stop();
        GetWidget()->Close();
    }
}