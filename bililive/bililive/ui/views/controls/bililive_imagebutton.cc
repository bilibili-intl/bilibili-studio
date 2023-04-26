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

    // ��ǰ���ǵ�ͼƬ��Դ�����gif��ʱ������� 1x, 1.5x �� 2x���������ǵ� gif ��Դͳһ��� 2x �ģ�
    // Ȼ���� GetPreferredSize() ʱ���ݵ�ǰ��ͼƬ dpi ����ʵ�ʴ�С
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
    // ���ﲻ����hovered̬�µ�ֱ�ӹرգ���Ȼ�û�û�л��Ὣ��껮������bubble��bubble�͹ر���
    if (enable_help() && state() == Button::STATE_HOVERED)
    {
        ShowHelp(this);
    }
}

void BililiveImageButton::NotifyClick(const ui::Event& event)
{
    // crash���ָ���������Щview��û���Ϣ��壩�����ť��תϵͳ�����ʱ��
    // ���ܻ���Ϊ���ڶ�ʧ�����ֱ�ӹرգ�UI�̲߳��Ŷӵȴ�mouserelease�¼�������ϣ���
    // �Ӷ������������ReportBehaviorEventʹ�õ��ѱ��ͷŵ���Դ����������
    // ��������ĳ�ReportBehaviorEvent����__super::NotifyClick֮ǰ������Ч����
    // ע�⣡���������������ڰ�ť�¼�����֮ǰ������������ڵ��ʱ�ϱ���̬�����ĳ�����
    // �Ͳ�Ҫ���ð�ť�Զ��ϱ��ķ�ʽ��Ӧ�ø����ڰ�ťlistener��������
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
