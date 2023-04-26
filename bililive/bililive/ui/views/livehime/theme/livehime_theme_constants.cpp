#include "livehime_theme_constants.h"

const std::map<LivehimeColorType, SkColor> g_default_theme_colors {
    { Theme, clrLivehime },
    { ThemeLight, clrLivehimeFocus },

    { WindowTitle, clrLivehime },
    { WindowTitleText, SK_ColorWHITE },
    { WindowChildTitleText, clrTextTitle },
    { WindowClient, clrWindowsContent },
    { WindowStatus, clrWindowsContent },
    { WindowBorder, clrWindowsBorder },

    { TextTitle, clrTextTitle },
    { TextContent, clrTextPrimary },
    { TextDescribe, clrTextSecondary },
    { TextTip, clrTextTip },
    { TextWarning, clrTextTipWarn },
    { TextPlaceholder, clrPlaceholderText },

    { SeparatorLine, clrWindowsGrayBk },

    { CtrlBorderNor, clrControlBorder },
    { CtrlBorderHov, clrLivehime },
    { CtrlBorderPre, clrLivehime },

    { ButtonPositive, clrLivehime },
    { ButtonPositiveHov, clrLivehimeFocus },
    { ButtonPositivePre, clrLivehimeFocus },
    { ButtonNegative, clrControlBorder },
    { ButtonNegativeHov, clrLivehimeFocus },
    { ButtonNegativePre, clrLivehimeFocus },

    { OptionButtonNor, clrTextPrimary },
    { OptionButtonHov, clrTextPrimary },
    { OptionButtonPre, clrTextPrimary },

    { LinkButtonNor, clrTextPrimary },
    { LinkButtonHov, clrLivehime },
    { LinkButtonPre, clrLivehime },

    { TabbedTextNor, clrTextPrimary },
    { TabbedTextHov, clrTextPrimary },
    { TabbedTextPre, clrLivehime },
    { TabbedBkNor, clrWindowsContent },
    { TabbedBkHov, clrWindowsGrayBk },
    { TabbedBkPre, clrTabbedBkSelected },
    { TabbedBkSel, clrTabbedBkSelected },

    { ListHeaderBk, clrListHeaderBk },
    { ListItemBkNor, clrListItemBk },
    { ListItemBkHov, clrListItemBkHover },
    { ListItemBkPre, clrListHeaderBk },

    { ProgressEmpty, clrControlBorder },
    { ProgressFull, clrLivehime },

    { VolumeEmpty, clrVolumeBack },
    { VolumeFull, clrVolumeNormal },
    { VolumeLoud, clrVolumeLoud },

    { LabelTitle, clrLabelTitle },

    { DropBorder, clrDroplistBorder },
};