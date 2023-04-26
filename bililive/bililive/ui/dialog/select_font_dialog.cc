#include "shell_dialogs.h"
#include "ui/views/widget/widget.h"

#include <commdlg.h>

namespace bililive
{
	SelectFontDialog::SelectFontDialog()
		: isBold_(false)
		, isItalic_(false)
	{
	}

	SelectFontDialog::~SelectFontDialog()
	{
	}

	void SelectFontDialog::SetCurrentFont(const std::wstring& fontName, int height, bool isBold, bool isItalic)
	{
		fontName_ = fontName;
		height_ = height;
		isBold_ = isBold;
		isItalic_ = isItalic;
	}

	SelectFontDialog::Result SelectFontDialog::DoModel(views::Widget* parent)
	{
		std::wstring fontNameCopy = fontName_;

		//从字体名字找到对应字体，去除字形
		{
			HDC desktopDC = GetDC(0);
			LOGFONTW lf = { 0 };
			lf.lfCharSet = DEFAULT_CHARSET;

			struct EnumParam
			{
				std::wstring fontName;
				bool found;
				std::vector<std::wstring> candidates;
			} param;

			param.fontName = fontName_;
			param.found = false;

			//遍历所有字体，找出所有完全匹配和前方匹配的
			EnumFontFamiliesExW(desktopDC, &lf, 
			[](const LOGFONTW* lf, const TEXTMETRIC* metric, DWORD fontType, LPARAM lp)->int{
				EnumParam& param = *(EnumParam*)lp;

				//不支持奇怪字形
				if (lf->lfWeight != FW_REGULAR)
					return 1;
				if (lf->lfItalic != 0)
					return 1;

				if (param.fontName == lf->lfFaceName)
				{
					param.found = true;
					return 0;
				}

				size_t faceNameLen = wcsnlen(lf->lfFaceName, sizeof(lf->lfFaceName) / sizeof(*lf->lfFaceName));
				faceNameLen = std::min(faceNameLen, param.fontName.length());

				if (std::equal(lf->lfFaceName, lf->lfFaceName + faceNameLen, param.fontName.begin()))
				{
					param.candidates.emplace_back(lf->lfFaceName);
				}
				return 1;
			}, (LPARAM)&param, 0
			);

			ReleaseDC(0, desktopDC);

			//如果没有完全匹配但是有前方匹配的，找出匹配长度最长的一个
			if (!param.found && !param.candidates.empty())
			{
				const wchar_t* str = 0;
				int len = -1;
				for (auto& x : param.candidates)
				{
					if ((int)x.length() > len)
					{
						str = x.data();
						len = x.length();
					}
				}

				fontNameCopy = str;
			}
		}

		LOGFONTW logFont = { 0 };
		logFont.lfHeight = height_;
		logFont.lfItalic = isItalic_ ? TRUE : FALSE;
		fontNameCopy.copy(logFont.lfFaceName, sizeof(logFont.lfFaceName) / sizeof(*logFont.lfFaceName) - 1);
		if (isBold_) logFont.lfWeight = FW_BOLD;

		CHOOSEFONTW chooseFont = { 0 };
		chooseFont.lStructSize = sizeof(chooseFont);

		if (parent)
			chooseFont.hwndOwner = parent->GetNativeWindow();

		chooseFont.lpLogFont = &logFont;
		chooseFont.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOSCRIPTSEL | CF_NOVERTFONTS;

		BOOL r = ChooseFontW(&chooseFont);

		if (r == TRUE)
		{
			fontName_ = logFont.lfFaceName;
			height_ = logFont.lfHeight;
			isBold_ = logFont.lfWeight >= FW_BOLD;
			isItalic_ = logFont.lfItalic != FALSE;

			return RESULT_OK;
		}
		else
			return RESULT_CANCEL;
	}
};
