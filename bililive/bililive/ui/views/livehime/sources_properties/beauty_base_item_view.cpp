#include "beauty_base_item_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "ui/base/resource/resource_bundle.h"


BeautyBaseItemView::BeautyBaseItemView(TabBarView* parent) :
    TabBarItemView(parent),
    img_size_(GetSizeByDPIScale({ 44, 44 }))
{
    //auto& rb = ResourceBundle::GetSharedInstance();
    //new_img_ = *rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_NEW);
}

BeautyBaseItemView::~BeautyBaseItemView()
{

}

gfx::Size BeautyBaseItemView::GetPreferredSize()
{
    return GetSizeByDPIScale({ 64, 72 });
}

void BeautyBaseItemView::SetNew(bool new_effect)
{
    is_new_ = new_effect;
}

void BeautyBaseItemView::SetSideRound(bool is_round)
{
    is_round_ = is_round;
}

void BeautyBaseItemView::SetImage(const gfx::ImageSkia& image)
{
    img_ = image;
}

void BeautyBaseItemView::SetSelImage(const gfx::ImageSkia& image)
{
    sel_img_ = image;
}

void BeautyBaseItemView::SetText(const base::string16& text)
{
    text_ = text;
}

void BeautyBaseItemView::SetPercent(float percent)
{
    percent_ = percent;
}

void BeautyBaseItemView::OnPaint(gfx::Canvas* canvas)
{
    auto rt = GetContentsBounds();

    int img_x = (rt.width() - img_size_.width()) / 2.0f;
    int img_y = GetLengthByDPIScale(7);

    if (is_select_)
    {
        if (!sel_img_.isNull())
        {
            canvas->DrawImageInt(sel_img_, 0, 0, sel_img_.width(), sel_img_.height(), img_x, img_y, img_size_.width(), img_size_.height(), true);
        }
    }
    else
    {
        if (!img_.isNull())
        {
            canvas->DrawImageInt(img_, 0, 0, img_.width(), img_.height(), img_x, img_y, img_size_.width(), img_size_.height(), true);
        }
    }

    if (!text_.empty())
    {
        int str_width = ftTwelve.GetStringWidth(text_);
        int str_x = (rt.width() - str_width) / 2.0f;
        int str_y = img_y + img_size_.height() + GetLengthByDPIScale(4);

        canvas->DrawStringInt(text_, ftTwelve, SkColorSetRGB(24, 25, 28), str_x, str_y, str_width, ftTwelve.GetHeight());
    }

    SkRect oval{ (float)img_x, (float)img_y,  (float)(img_x + img_size_.width()), (float)(img_y + img_size_.height()) };
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(GetLengthByDPIScale(2.0f));
    paint.setColor(is_select_ ? SkColorSetRGB(14, 190, 255) : SkColorSetRGB(195, 208, 223));
    paint.setAntiAlias(true);

    //if (BeautyTag().id != 0)
    //{
    //    int start_angle = -90;
    //    int degree = percent_ * 360;
    //    if (is_round_)
    //    {
    //        degree -= 180;
    //    }

    //    canvas->sk_canvas()->drawArc(oval, start_angle, degree, false, paint);
    //}

    if (is_new_)
    {
        int new_x = rt.width() - new_img_.width();
        canvas->DrawImageInt(new_img_, new_x, 0);
    }
}

bool BeautyBaseItemView::OnSelect()
{
    is_select_ = true;

    return true;
}

void BeautyBaseItemView::OnSelectChanged(int select_index)
{
    is_select_ = false;
}

