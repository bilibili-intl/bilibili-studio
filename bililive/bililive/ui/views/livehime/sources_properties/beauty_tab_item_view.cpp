#include "beauty_tab_item_view.h"


BeautyTabItemView::BeautyTabItemView(TabBarView* parent, const base::string16& text) :
    TabBarItemView(parent),
    text_(text)
{
}

BeautyTabItemView::~BeautyTabItemView()
{

}

const base::string16& BeautyTabItemView::GetTagText()
{
    return text_;
}

void BeautyTabItemView::SetPreferredSize(const gfx::Size& size)
{
    perferred_size_ = size;
}

gfx::Size BeautyTabItemView::GetPreferredSize()
{
    return perferred_size_;
}

void BeautyTabItemView::OnPaint(gfx::Canvas* canvas)
{
    auto rt = GetContentsBounds();

    int text_width = ftSixteen.GetStringWidth(text_);
    int text_x = (rt.width() - text_width) / 2.0f;
    SkColor text_color = select_ ? SkColorSetRGB(14, 190, 255) : SkColorSetRGB(97, 102, 109);
    canvas->DrawStringInt(text_, ftSixteen, text_color, text_x, 0, text_width, ftSixteen.GetHeight());

    if (select_)
    {
        auto rt = GetContentsBounds();

        gfx::Rect select_rt;
        select_rt.set_width(text_width * 0.67857f);
        select_rt.set_height(GetLengthByDPIScale(4));
        select_rt.set_x((rt.width() - select_rt.width()) / 2.0);
        select_rt.set_y(ftSixteen.GetHeight() + GetLengthByDPIScale(4));

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SkColorSetRGB(14, 190, 255));
        paint.setAntiAlias(true);
        canvas->DrawRoundRect(select_rt, GetLengthByDPIScale(4), paint);
    }
}

bool BeautyTabItemView::OnSelect()
{
    select_ = true;
    return true;
}

void BeautyTabItemView::OnSelectChanged(int select_index)
{
    select_ = false;
}

