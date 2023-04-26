#include "bililive_sequence_image_button.h"

#include "ui/gfx/canvas.h"

#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/src/core/SkConvolver.h"
#include "third_party/skia/src/core/SkBitmapScaler.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


BililiveSequenceImageButton::BililiveSequenceImageButton(views::ButtonListener* listener)
    : listener_(listener)
    , cursor_(NULL)
    , sequence_wrapper_(new BililiveSequenceImageWrapper(this))
    , owned_(true)
    , h_align_(gfx::ALIGN_CENTER)
    , v_align_(gfx::ALIGN_VCENTER)
{
    set_focus_border(nullptr);
    set_focusable(true);
}

BililiveSequenceImageButton::~BililiveSequenceImageButton()
{
    CleanUp();
}

void BililiveSequenceImageButton::SetStretchPaintSeq(bool stretch)
{
    is_stretch_seq_ = stretch;
    SchedulePaint();
}

void BililiveSequenceImageButton::SetStretchPaintBase(bool stretch)
{
    is_stretch_base_ = stretch;
    SchedulePaint();
}

gfx::Size BililiveSequenceImageButton::GetPreferredSize()
{
    if (!preferred_size_.IsEmpty())
    {
        return preferred_size_;
    }
    gfx::Size size;
    if (sequence_wrapper_)
    {
        size = sequence_wrapper_->image_size();
    }
    if (!base_image_.isNull())
    {
        size.SetToMax(base_image_.size());
    }
    return size;
}

void BililiveSequenceImageButton::OnPaint(gfx::Canvas* canvas)
{
    views::View::OnPaint(canvas);

    PaintImage(canvas, image_, true, is_stretch_seq_);
    PaintImage(canvas, base_image_, false, is_stretch_base_);
}

void BililiveSequenceImageButton::PaintImage(
    gfx::Canvas* canvas, const gfx::ImageSkia &image, bool ignore_align, bool is_stretch)
{
    if (!image.isNull())
    {
        SkPaint paint;
        paint.setFilterBitmap(true);
        //paint.setFilterLevel(SkPaint::kHigh_FilterLevel);
        if (!is_stretch)
        {
            int x = std::round((width() - image.width()) / 2);
            int y = std::round((height() - image.height()) / 2);
            if (!ignore_align)
            {
                switch (h_align_)
                {
                case gfx::ALIGN_LEFT:
                    x = 0;
                    break;
                case gfx::ALIGN_RIGHT:
                    x = width() - image.width();
                    break;
                default:
                    break;
                }
                switch (v_align_)
                {
                case gfx::ALIGN_TOP:
                    y = 0;
                    break;
                case gfx::ALIGN_BOTTOM:
                    y = height() - image.height();
                    break;
                default:
                    break;
                }
            }
            canvas->DrawImageInt(image, x, y, paint);
        }
        else
        {
            float scale = image.width() * 1.0f / image.height();
            int cx = width();
            int cy = std::round(cx / scale);
            if (cy > height())
            {
                cy = height();
                cx = std::round(cy * scale);
            }
            canvas->DrawImageInt(image, 0, 0, image.width(), image.height(),
                std::round((width() - cx) / 2), std::round((height() - cy) / 2), cx, cy, true, paint);
        }
    }
}

gfx::NativeCursor BililiveSequenceImageButton::GetCursor(const ui::MouseEvent& event)
{
    if (!enabled())
        return gfx::kNullCursor;

    if (!cursor_)
    {
        /*static HCURSOR g_hand_cursor = LoadCursor(NULL, IDC_HAND);
        return g_hand_cursor;*/
        return gfx::kNullCursor;
    }
    else
    {
        return cursor_;
    }
}

void BililiveSequenceImageButton::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add == false)
        {
            CleanUp();
        }
    }
}

void BililiveSequenceImageButton::SetImage(const gfx::ImageSkia* image)
{
    base_image_ = *image;
    SchedulePaint();
}

void BililiveSequenceImageButton::SetRepeating(bool repeating)
{
    if (repeating != repeating_)
    {
        if (sequence_wrapper_)
        {
            pre_sequence_wrapper_repeating_ = sequence_wrapper_->repeating();
            sequence_wrapper_->SetRepeating(repeating);
        }
        repeating_ = repeating;
    }
}

void BililiveSequenceImageButton::EnableAnimation(bool enable, bool clear)
{
    enable_animation_ = enable;
    if (!enable_animation_)
    {
        if (clear)
        {
            image_ = gfx::ImageSkia();
            SchedulePaint();
        }
    }
}

void BililiveSequenceImageButton::CleanUp()
{
    if (sequence_wrapper_)
    {
        sequence_wrapper_->SetDelegate(nullptr);

        if (owned_)
        {
            sequence_wrapper_->CleanUp();
        }
        else
        {
            sequence_wrapper_->Stop();
        }

        sequence_wrapper_ = NULL;
    }
}

void BililiveSequenceImageButton::SetPreferredSize(const gfx::Size& preferred_size)
{
    preferred_size_ = preferred_size;
}

void BililiveSequenceImageButton::OnSequenceImageFrameChanged(
    const gfx::ImageSkia* image, size_t curent_frame, size_t frame_counts)
{
    if (enable_animation_)
    {
        if (image)
        {
            image_ = *image;
        }
        else
        {
            image_ = gfx::ImageSkia();
        }
        SchedulePaint();

        if (delegate_)
        {
            delegate_->OnSequenceImageFrameChanged(this, curent_frame, frame_counts);
        }
    }
}

void BililiveSequenceImageButton::OnSequenceMotionStart()
{
    if (hide_when_stop_)
    {
        SetVisible(true);
    }
    PreferredSizeChanged();
}

void BililiveSequenceImageButton::OnSequenceMotionStop()
{
    SetVisible(!hide_when_stop_);
    image_ = gfx::ImageSkia();
}

void BililiveSequenceImageButton::AttachSequenceImageWrapper(RefSequenceImageWrapper wrapper, bool owned)
{
    if (sequence_wrapper_ != wrapper)
    {
        sequence_wrapper_->SetDelegate(nullptr);
        // 还原wrapper_原先的重复属性
        sequence_wrapper_->SetRepeating(pre_sequence_wrapper_repeating_);

        sequence_wrapper_ = wrapper;
        owned_ = owned;
        sequence_wrapper_->SetDelegate(this);
        PreferredSizeChanged();

        pre_sequence_wrapper_repeating_ = sequence_wrapper_->repeating();
        sequence_wrapper_->SetRepeating(repeating_);
    }
    else
    {
        owned_ = owned;
        sequence_wrapper_->SetDelegate(this);
    }
}

void BililiveSequenceImageButton::SetImageAlignment(gfx::HorizontalAlignment h_align,
    gfx::VerticalAlignment v_align)
{
    h_align_ = h_align;
    v_align_ = v_align;
}
