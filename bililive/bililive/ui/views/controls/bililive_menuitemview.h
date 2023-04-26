#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_MENUITEMVIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_MENUITEMVIEW_H

#include "ui/views/view.h"
#include "ui/views/controls/menu/menu_item_view.h"
#include "ui/views/controls/menu/submenu_view.h"
#include "ui/views/controls/button/button.h"

namespace
{
    class BililiveSubmenuView;
}

class BililiveMenuItemView : public views::MenuItemView
{
public:
    // The menu item view's class name.
    static const char kViewClassName[];

    explicit BililiveMenuItemView(views::MenuDelegate* delegate);
    virtual ~BililiveMenuItemView(){}

    // ÷ÿ‘ÿMenuItemView
    views::MenuItemView* AppendMenuItem(int item_id, const base::string16& label, Type type = views::MenuItemView::NORMAL);
    views::MenuItemView* AppendSubMenu(int item_id, const string16& label);

    views::MenuItemView* AppendCheckMenu(int item_id, const string16& label);

    void SetFont(const gfx::Font &font) { font_ = font; };
    void SetTextColor(views::Button::ButtonState for_state, SkColor color);
    void SetBackgroundColor(views::Button::ButtonState for_state, SkColor color);
    void SetBorderColor(SkColor color);
    void SetArrowImage(gfx::ImageSkia *image, gfx::ImageSkia *sel_image);
    void SetCheckedImage(gfx::ImageSkia *image) { checked_image_ = image; };
    void SetRadioImage(gfx::ImageSkia *image, bool selected);

    //void set_minimum_preferred_width(int minimum_preferred_width) { minimum_preferred_width_ = minimum_preferred_width; };
    void set_maximize_preferred_width(int maximize_preferred_width) { maximize_preferred_width_ = maximize_preferred_width; };

    void RemoveMenuItemAt(int index);

    int GetMenuItemCount();

protected:
    // View
    virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual const char* GetClassName() const OVERRIDE{ return kViewClassName; };

    // MenuItemView
    virtual views::SubmenuView* CreateSubmenu() OVERRIDE;
    virtual const gfx::Font& GetFont() OVERRIDE;

    // ÷ÿ‘ÿMenuItemView
    views::MenuItemView* AppendMenuItemImpl(int item_id, const base::string16& label, const base::string16& sublabel, const gfx::ImageSkia& icon,
        views::MenuItemView::Type type, ui::MenuSeparatorType separator_style);
    views::MenuItemView* AddMenuItemAt(int index, int item_id, const base::string16& label, const base::string16& sublabel, const gfx::ImageSkia& icon,
        views::MenuItemView::Type type, ui::MenuSeparatorType separator_style);

protected:
    // Creates a MenuItemView. This is used by the various AddXXX methods.
    explicit BililiveMenuItemView(views::MenuItemView* parent, int command, views::MenuItemView::Type type);

    virtual views::MenuItemView* CreateCurrentDevrivedInstance(BililiveMenuItemView* parent, int command, views::MenuItemView::Type type);

    const BililiveMenuItemView* GetRootBililiveMenuItem() const;
    const gfx::Font& font();
    SkColor text_color(views::Button::ButtonState state);
    SkColor border_color();
    SkColor background_color(views::Button::ButtonState state);
    const gfx::ImageSkia* arrow_image(bool render_selection);
    const gfx::ImageSkia* checked_image();
    const gfx::ImageSkia* radio_image(bool selected);
    int minimum_preferred_width();
    int maximize_preferred_width();

private:
    void InitParam();
    void PaintMinorText(gfx::Canvas* canvas, SkColor color);

private:
    friend class BililiveSubmenuView;
    views::MenuItemView::Type menu_type_;
    int minimum_preferred_width_;
    int maximize_preferred_width_;
    gfx::Font font_;
    SkColor border_color_;
    SkColor text_colors_[views::Button::ButtonState::STATE_COUNT];
    SkColor bk_colors_[views::Button::ButtonState::STATE_COUNT];
    gfx::ImageSkia *arrow_image_;
    gfx::ImageSkia *arrow_selection_image_;
    gfx::ImageSkia *checked_image_;
    gfx::ImageSkia *radio_checked_image_;
    gfx::ImageSkia *radio_unchecked_image_;
    int menu_items_ = 0;

    DISALLOW_COPY_AND_ASSIGN(BililiveMenuItemView);
};


#endif