#include "livehime_menu.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

LivehimeMenuItemView::LivehimeMenuItemView(views::MenuDelegate* delegate)
    : BililiveMenuItemView(delegate)
{
    SetFont(ftPrimary);
    SetBorderColor(clrWindowsBorder);
    SetTextColor(views::Button::ButtonState::STATE_NORMAL, clrTextPrimary);
    SetTextColor(views::Button::ButtonState::STATE_HOVERED, clrLivehime);
    SetTextColor(views::Button::ButtonState::STATE_PRESSED, clrLivehime);
    SetTextColor(views::Button::ButtonState::STATE_DISABLED, SkColorSetA(clrTextPrimary, disable_alpha));
    //SetBackgroundColor(views::Button::ButtonState::STATE_NORMAL, clrWindowsContent);
    //SetBackgroundColor(views::Button::ButtonState::STATE_HOVERED, clrWindowsContent);
    //SetBackgroundColor(views::Button::ButtonState::STATE_PRESSED, clrWindowsContent);
    //SetBackgroundColor(views::Button::ButtonState::STATE_DISABLED, SkColorSetA(clrWindowsContent, disable_alpha));
    // 设置菜单栏按钮的颜色
    auto color = GetColor(WindowClient);
    SetBackgroundColor(views::Button::ButtonState::STATE_NORMAL, color);
    SetBackgroundColor(views::Button::ButtonState::STATE_HOVERED, color);
    SetBackgroundColor(views::Button::ButtonState::STATE_PRESSED, color);
    SetBackgroundColor(views::Button::ButtonState::STATE_DISABLED, SkColorSetA(color, disable_alpha));
    SetArrowImage(ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_SUBMENU_ARROW), 
        ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_TITLEBAR_SUBMENU_ARROW_HV));
}

LivehimeMenuItemView::LivehimeMenuItemView(views::MenuItemView* parent, int command, views::MenuItemView::Type type)
    : BililiveMenuItemView(parent, command, type)
{
}

LivehimeMenuItemView::~LivehimeMenuItemView()
{
}

views::MenuItemView* LivehimeMenuItemView::CreateCurrentDevrivedInstance(
    BililiveMenuItemView* parent, int command, views::MenuItemView::Type type)
{
    return new LivehimeMenuItemView(parent, command, type);
}

gfx::Size LivehimeMenuItemView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(kBorderCtrlHeight);
    return size;
}

int LivehimeMenuItemView::GetHeightForWidth(int w)
{
    return kBorderCtrlHeight;
}

const views::MenuConfig& LivehimeMenuItemView::GetMenuConfig() const
{
    const views::MenuConfig& cfg = __super::GetMenuConfig();
    views::MenuConfig& menu_config = const_cast<views::MenuConfig&>(cfg);
    menu_config.submenu_horizontal_inset = -3;
    menu_config.menu_vertical_border_size = 1;
    return menu_config;
}