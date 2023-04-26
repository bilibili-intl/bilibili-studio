#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"

#include "base/strings/utf_string_conversions.h"

#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/utils/bililive_image_util.h"

#include "third_party/skia/include/core/SkPaint.h"


BililiveImageButton::BililiveImageButton(views::ButtonListener* listener)
    : views::ImageButton(listener)
    , stretch_(false)
    , had_set_preferred_(false)
    , enable_gif_(false)
    , cursor_(nullptr)
{
    SetImageAlignment(ImageButton::ALIGN_CENTER, ImageButton::ALIGN_MIDDLE);
    set_focus_border(nullptr);
    set_focusable(true);
}

BililiveImageButton::~BililiveImageButton()
{
    ResetImageRender();
}

void BililiveImageButton::SetStretchPaint(bool stretch)
{
    stretch_ = stretch;
    SchedulePaint();
}

gfx::Size BililiveImageButton::GetRawImageSize() const
{
    if (gif_)
    {
        return gif_->image_size();
    }
    return gfx::Size();
}

gfx::Size BililiveImageButton::GetPreferredSize()
{
    if (had_set_preferred_)
    {
        return preferred_size_;
    }
    gfx::Size size = views::ImageButton::GetPreferredSize();

    // 当前我们的图片资源在添加gif的时候不能添加 1x, 1.5x 和 2x，所以我们的 gif 资源统一添加 2x 的，
    // 然后在 GetPreferredSize() 时根据当前的图片 dpi 决定实际大小
    if (gif_ && gif_->image_type() == bililive::ImageType::IT_GIF)
    {
        static float scale = ui::win::GetDeviceScaleFactor();
        static ui::ScaleFactor scale_factor_to_load = ui::GetScaleFactorFromScale(scale);
        if (scale_factor_to_load == ui::SCALE_FACTOR_100P)
        {
            size.set_width(size.width() / 2);
            size.set_height(size.height() / 2);
        }
        else if (scale_factor_to_load == ui::SCALE_FACTOR_150P)
        {
            size.set_width(size.width() * 0.75f);
            size.set_height(size.height() * 0.75f);
        }
    }
    return size;
}

void BililiveImageButton::OnPaint(gfx::Canvas* canvas)
{
    if (!stretch_)
    {
        views::ImageButton::OnPaint(canvas);
    }
    else
    {
        View::OnPaint(canvas);

        gfx::Rect rt = GetContentsBounds();
        gfx::ImageSkia img = GetImageToPaint();

        SkPaint paint;
        paint.setFilterBitmap(true);

        if (!img.isNull())
        {
            if (!background_image_.isNull())
            {
                canvas->DrawImageInt(background_image_, 0, 0, background_image_.width(), background_image_.height(),
                    rt.x(), rt.y(), rt.width(), rt.height(), true, paint);
            }

            {
                gfx::Rect rrt;
                auto result = bililive::CreateScaledImageEx(img, rt, &rrt);
                canvas->DrawImageInt(
                    result, 0, 0, result.width(), result.height(),
                    rrt.x(), rrt.y(), rrt.width(), rrt.height(), true, paint);
            }

            if (!overlay_image_.isNull())
            {
                canvas->DrawImageInt(overlay_image_, 0, 0, overlay_image_.width(), overlay_image_.height(),
                    rt.x(), rt.y(), rt.width(), rt.height(), true, paint);
            }
        }

        OnPaintFocusBorder(canvas);
    }
}

gfx::NativeCursor BililiveImageButton::GetCursor(const ui::MouseEvent& event)
{
    if (!enabled())
        return gfx::kNullCursor;

    if (!cursor_)
    {
        static HCURSOR g_hand_cursor = LoadCursor(nullptr, IDC_HAND);
        return g_hand_cursor;
    }
    else
    {
        return cursor_;
    }
}

void BililiveImageButton::SetAllStateImage(const gfx::ImageSkia* image)
{
    ResetImageRender();
    OnImageFrameChanged(image);
}

void BililiveImageButton::OnImageFrameChanged(const gfx::ImageSkia* image)
{
    SetImage(views::Button::STATE_NORMAL, image);
    SetImage(views::Button::STATE_HOVERED, image);
    SetImage(views::Button::STATE_PRESSED, image);
    SetImage(views::Button::STATE_DISABLED, image);
    SchedulePaint();
}

bool BililiveImageButton::SetImageFromFile(const base::FilePath &file_path)
{
    ResetImageRender();
    gif_ = new BililiveImageRender(this);
    gif_->SetEnableGif(enable_gif_);
    return (gif_->InitFromFile(file_path) != bililive::ImageType::IT_UNKNOW);
}

bool BililiveImageButton::SetImageFromBinary(const unsigned char * const data, unsigned int len, bool attach_gif_data/* = false*/)
{
    ResetImageRender();
    gif_ = new BililiveImageRender(this);
    gif_->SetEnableGif(enable_gif_);
    return (gif_->InitFromBinary(data, len, attach_gif_data) != bililive::ImageType::IT_UNKNOW);
}

void BililiveImageButton::SetEnableGif(bool enable)
{
    enable_gif_ = enable;
    if (gif_.get())
    {
        gif_->SetEnableGif(enable);
    }
}

void BililiveImageButton::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add == false)
        {
            ResetImageRender();
        }
    }
}

void BililiveImageButton::StateChanged()
{
    // 这里不做非hovered态下的直接关闭，不然用户没有机会将鼠标划到帮助bubble上bubble就关闭了
    if (enable_help() && state() == Button::STATE_HOVERED)
    {
        ShowHelp(this);
    }
}

void BililiveImageButton::NotifyClick(const ui::Event& event)
{
    // crash发现个现象：在有些view里（用户信息面板）点击按钮跳转系统浏览器时，
    // 可能会因为窗口丢失焦点而直接关闭（UI线程不排队等待mouserelease事件处理完毕？）
    // 从而引起接下来的ReportBehaviorEvent使用到已被释放的资源引发崩溃。
    // 所以这里改成ReportBehaviorEvent置于__super::NotifyClick之前，看看效果。
    // 注意！由于这里把埋点置于按钮事件触发之前，所以如果是在点击时上报动态参数的场景，
    // 就不要采用按钮自动上报的方式，应该改用在按钮listener里进行埋点
    ReportBehaviorEvent();

    __super::NotifyClick(event);
}

void BililiveImageButton::ResetImageRender()
{
    if (gif_)
    {
        gif_->CleanUp();
        gif_ = nullptr;
    }
}

void BililiveImageButton::Clone(BililiveImageButton &dst)
{
    if (gif_.get() && gif_->data() && gif_->data_len())
    {
        dst.SetImageFromBinary(gif_->data(), gif_->data_len());
    }
    else
    {
        dst.SetAllStateImage(&GetImage(views::Button::STATE_NORMAL));
    }
}

void BililiveImageButton::SetPreferredSize(const gfx::Size& preferred_size)
{
    had_set_preferred_ = true;
    preferred_size_ = preferred_size;
}
