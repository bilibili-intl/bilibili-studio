#include "shell_dialogs.h"

#include <algorithm>
#include <commdlg.h>
#include "ui/views/widget/widget.h"

static COLORREF CustClrs[16] = 
{
	RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff),
	RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff),
	RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff),
	RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff), RGB(0xff, 0xff, 0xff)
};

static COLORREF SkColorToCOLORREF(SkColor c)
{
	return RGB(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
}

static SkColor COLORREFToSkColor(COLORREF c)
{
	return SkColorSetARGB(0xff, GetRValue(c), GetGValue(c), GetBValue(c));
}

namespace bililive
{
	SelectColorDialog::SelectColorDialog()
	{
	}

	SelectColorDialog::~SelectColorDialog()
	{
	}

	void SelectColorDialog::SetCurrentColor(SkColor color)
	{
		color_ = color;
	}

	SkColor SelectColorDialog::GetCurrentColor()
	{
		return color_;
	}

	SelectColorDialog::Result SelectColorDialog::DoModel(views::Widget* parent)
	{
		CHOOSECOLORW chooseColor = { sizeof(chooseColor) };

		if (parent)
			chooseColor.hwndOwner = parent->GetNativeWindow();

		chooseColor.lpCustColors = CustClrs;

		chooseColor.Flags |= CC_RGBINIT;
		chooseColor.rgbResult = SkColorToCOLORREF(color_);
		
		if (ChooseColorW(&chooseColor) != FALSE)
		{
			color_ = COLORREFToSkColor(chooseColor.rgbResult);
			return RESULT_OK;
		}
		else
			return RESULT_CANCEL;
	}
}