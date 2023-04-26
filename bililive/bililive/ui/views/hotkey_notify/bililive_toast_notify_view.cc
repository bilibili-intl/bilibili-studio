#include "bililive_toast_notify_view.h"

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
    ToastNotifyView* last_notify_toast = nullptr;
    const int kStartEndSpace = GetLengthByDPIScale(8);
    const int KMaxToastWidth = GetLengthByDPIScale(524);
}


void ToastNotifyView::ShowForm(const base::string16 &text, gfx::Rect show_area,
    gfx::ImageSkia *image/* = NULL*/, const gfx::Font *font/* = NULL*/,
    int delay_time) {

    ToastNotifyView* hnv = new ToastNotifyView(text, image, font, delay_time);
    hnv->AdjustPosition(show_area);
}

ToastNotifyView::ToastNotifyView(const base::string16 &text,
    gfx::ImageSkia *image, const gfx::Font *font, int delay_time)
    : delay_time_(delay_time)
    , text_(text)
    , alpha_(250)
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
        font_ = ftFourteen;
    }
    ShowWidget();
}


ToastNotifyView::~ToastNotifyView()
{
    if (this == last_notify_toast)
    {
        last_notify_toast = nullptr;
    }
}

void ToastNotifyView::ShowWidget()
{
    // 需要显示新的toast就把旧的toast关了，省得同一个窗口短时间内提示多次导致重叠显示
    if (last_notify_toast)
    {
        last_notify_toast->GetWidget()->Close();
    }
    last_notify_toast = this;

    const int max_width = KMaxToastWidth - kStartEndSpace * 2;
    const int line_max_num = max_width / LivehimePaddingCharWidth(font_);
    views::BoxLayout *layout = new views::BoxLayout(views::BoxLayout::kHorizontal, kStartEndSpace, kStartEndSpace, kStartEndSpace);
    SetLayoutManager(layout);

    views::ImageView *image = new views::ImageView();
    image->SetImage(image_);

    views::Label* label = nullptr;

    const int len = text_.length();
    if (len > line_max_num) {
        base::string16 text;
        for (int i = 0, count = 0; i < len; ++i) {
            text.push_back(text_.at(i));
            if (++count > line_max_num) {
                text.push_back('\n');
                count = 0;
            }
            if (text_.at(i) == '\n') {
                count = 0;
            }
        }
        label = new views::Label(text, font_);
    }
    else {
        label = new views::Label(text_, font_);
    }
    label->SetAutoColorReadabilityEnabled(false);
    label->SetEnabledColor(SK_ColorWHITE);
    label->SetBackgroundColor(0);
    label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    //AddChildView(image);
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
    int millsecond = delay_time_ * 500 - 1000;
    millsecond = millsecond >= 1500 ? millsecond : 1500;
    BililiveThread::PostDelayedTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&ToastNotifyView::OnDeltaFadeOut, weakptr_factory_.GetWeakPtr()), base::TimeDelta::FromMilliseconds(millsecond));
}

void ToastNotifyView::AdjustPosition(gfx::Rect show_area)
{
    gfx::Rect hnv_s = GetWidget()->GetWindowBoundsInScreen();
    GetWidget()->SetBounds(gfx::Rect(
        show_area.x() + (show_area.width() - hnv_s.width()) / 2,
        show_area.y() + (show_area.height() - hnv_s.height()) / 2,
        hnv_s.width(), hnv_s.height()));
}

void ToastNotifyView::OnPaintBackground(gfx::Canvas *canvas)
{
    __super::OnPaintBackground(canvas);

    const gfx::Rect& rect = GetContentsBounds();
    // 圆角背景
    int radius = GetLengthByDPIScale(4);
    U8CPU alpha = 0.6f * 255;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SkColorSetARGB(alpha, 0, 0, 0));
    paint.setStyle(SkPaint::kFill_Style);
    canvas->DrawRoundRect(GetContentsBounds(), radius, paint);
}

void ToastNotifyView::OnDeltaFadeOut()
{
    timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(100),
        base::Bind(&ToastNotifyView::OnFadeOutTimer, weakptr_factory_.GetWeakPtr()));
}

void ToastNotifyView::OnFadeOutTimer()
{
    alpha_ -= 25;
    GetWidget()->SetOpacity(alpha_);

    if (alpha_<=0)
    {
        timer_.Stop();
        GetWidget()->Close();
    }
}