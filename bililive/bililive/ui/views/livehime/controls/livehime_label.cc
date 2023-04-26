#include "livehime_label.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

// TitleLabel
const gfx::Font& LivehimeTitleLabel::GetFont()
{
    return ftFourteen;
}

void LivehimeTitleLabel::InitThema()
{
    SetFont(GetFont());
	SetTextColor(GetColor(LabelTitle));
}

void LivehimeSmallTitleLabel::InitThema()
{
    SetFont(ftTwelve);
	SetTextColor(GetColor(LabelTitle));
}


// ContentLabel
const gfx::Font& LivehimeContentLabel::GetFont()
{
    return ftPrimary;
}

void LivehimeContentLabel::InitThema()
{
    SetFont(GetFont());
	SetTextColor(GetColor(LabelTitle));
}

void LivehimeSmallContentLabel::InitThema()
{
    SetFont(ftTwelve);
	SetTextColor(GetColor(LabelTitle));
}


// TipLabel
const gfx::Font& LivehimeTipLabel::GetFont()
{
    return ftTwelve;
}

LivehimeTipLabel::LivehimeTipLabel()
{
    InitParam();
}

LivehimeTipLabel::LivehimeTipLabel(const string16& text)
    : BililiveLabel(text)
{
    InitParam();
}

LivehimeTipLabel::LivehimeTipLabel(const string16& text, SkColor clr)
    : BililiveLabel(text)
{
    InitParam();
    SetTextColor(clr);
}

void LivehimeTipLabel::InitParam()
{
    SetFont(GetFont());
    SetTextColor(clrLabelDescribe);
}


SplitLineHorizontalLabel::SplitLineHorizontalLabel()
{
    bg_color_ = SkColorSetARGB(0x33, 0x53, 0x67, 0x77);
}

void SplitLineHorizontalLabel::OnPaintBackground(gfx::Canvas* canvas)
{
	auto bound = GetContentsBounds();
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(bg_color_);
	paint.setStyle(SkPaint::kFill_Style);
	canvas->DrawRoundRect(bound, 0, paint);
}

gfx::Size SplitLineHorizontalLabel::GetPreferredSize()
{
	gfx::Size size = __super::GetPreferredSize();
	size.set_height(1);
	return size;
}


LivehimeStyleLabel::LivehimeStyleLabel(LabelStyle style, const string16& text) : BililiveLabel(text)
{
    switch (style)
    {
    case LivehimeStyleLabel::LabelStyle::Style_Title:
    {
		SetFont(ftFourteenBold);
		SetTextColor(GetColor(LabelTitle));
    } break;
	case LivehimeStyleLabel::LabelStyle::Style_Content:
	{
		SetFont(ftFourteen);
		SetTextColor(GetColor(LabelTitle));
	} break;
    case LivehimeStyleLabel::LabelStyle::Style_Content12:
	{
		SetFont(ftTwelve);
		SetTextColor(GetColor(LabelTitle));
	} break;
	case LivehimeStyleLabel::LabelStyle::Style_Notes:
	{
		SetFont(ftTwelve);


		SetTextColor(SkColorSetA(GetColor(LabelTitle), 0x99));
	} break;
	case LivehimeStyleLabel::LabelStyle::Style_Warning:
	{
		SetFont(ftTwelve);
		SetTextColor(SkColorSetRGB(0xff, 0x66, 0x66));
	} break;
    default:
        break;
    }
}