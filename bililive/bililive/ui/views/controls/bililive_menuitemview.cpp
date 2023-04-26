#include "bililive_menuitemview.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/strings/stringprintf.h"

#include "ui/gfx/canvas.h"
#include "ui/views/controls/menu/menu_separator.h"
#include "ui/views/controls/menu/menu_image_util.h"
#include "ui/views/controls/menu/menu_scroll_view_container.h"


namespace
{
    class BililiveSubmenuView : public views::SubmenuView
    {
    public:
        explicit BililiveSubmenuView(views::MenuItemView* parent)
            : views::SubmenuView(parent)
            , minimum_preferred_width_(0)
        {
        }
        virtual ~BililiveSubmenuView() {};

        void set_minimum_preferred_width(int minimum_preferred_width)
        {
            minimum_preferred_width_ = minimum_preferred_width;
        };

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            //return __super::GetPreferredSize();

            if (!has_children())
                return gfx::Size();

            int max_minor_text_width_ = 0;
            // The maximum width of items which contain maybe a label and multiple views.
            int max_complex_width = 0;
            // The max. width of items which contain a label and maybe an accelerator.
            int max_simple_width = 0;
            int height = 0;
            for (int i = 0; i < child_count(); ++i)
            {
                View* child = child_at(i);
                if (!child->visible())
                    continue;

                gfx::Size child_pref_size =
                    child->visible() ? child->GetPreferredSize() : gfx::Size();
                max_complex_width = std::max(max_complex_width, child_pref_size.width());
                height += child_pref_size.height();
            }
            if (max_minor_text_width_ > 0)
            {
                max_minor_text_width_ +=
                    GetMenuItem()->GetMenuConfig().label_to_minor_text_padding;
            }
            gfx::Insets insets = GetInsets();
            return gfx::Size(
                std::max(max_complex_width,
                    std::max(max_simple_width + max_minor_text_width_ + insets.width(), minimum_preferred_width_ - 2 * insets.width())),
                height + insets.height());
        }

        // View method. Overridden to schedule a paint. We do this so that when
        // scrolling occurs, everything is repainted correctly.
        void OnBoundsChanged(const gfx::Rect& previous_bounds) override
        {
            SetBorderColor();

            __super::OnBoundsChanged(previous_bounds);
        }

    private:
        void SetBorderColor()
        {
            views::MenuScrollViewContainer* contaner = GetScrollViewContainer();
            if (contaner)
            {
                contaner->set_border(views::Border::CreateSolidBorder(1, ((BililiveMenuItemView*)GetMenuItem())->border_color()));
                //contaner->SchedulePaint();
            }
        }

    private:
        int minimum_preferred_width_;

        DISALLOW_COPY_AND_ASSIGN(BililiveSubmenuView);
    };
}


const char BililiveMenuItemView::kViewClassName[] = "BililiveMenuItemView";

BililiveMenuItemView::BililiveMenuItemView(views::MenuDelegate* delegate)
    : views::MenuItemView(delegate)
    , menu_type_(NORMAL)
{
    checked_image_ = const_cast<gfx::ImageSkia*>(views::GetMenuCheckImage());
    radio_unchecked_image_ = const_cast<gfx::ImageSkia*>(views::GetRadioButtonImage(false));
    radio_checked_image_ = const_cast<gfx::ImageSkia*>(views::GetRadioButtonImage(true));
    arrow_image_ = const_cast<gfx::ImageSkia*>(views::GetSubmenuArrowImage());
    arrow_selection_image_ = const_cast<gfx::ImageSkia*>(views::GetSubmenuArrowImage());
    InitParam();
}

BililiveMenuItemView::BililiveMenuItemView(views::MenuItemView* parent, int command, views::MenuItemView::Type type)
    : views::MenuItemView(parent, command, type)
    , menu_type_(type)
{
    checked_image_ = arrow_image_ = radio_checked_image_ = radio_unchecked_image_ = arrow_selection_image_ = nullptr;
    InitParam();

    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        for (int state = views::Button::ButtonState::STATE_NORMAL;
            state < views::Button::ButtonState::STATE_COUNT; state++)
        {
            text_colors_[state] = root_menu->text_colors_[state];
            bk_colors_[state] = root_menu->bk_colors_[state];
        }
    }
}

void BililiveMenuItemView::InitParam()
{
    ui::NativeTheme* native_theme = GetNativeTheme();

    minimum_preferred_width_ = 0;
    maximize_preferred_width_ = -1;
    border_color_ = native_theme->GetSystemColor(ui::NativeTheme::kColorId_UnfocusedBorderColor);

    text_colors_[views::Button::ButtonState::STATE_NORMAL] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_ButtonEnabledColor);
    text_colors_[views::Button::ButtonState::STATE_HOVERED] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_ButtonHoverColor);
    text_colors_[views::Button::ButtonState::STATE_PRESSED] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_ButtonHighlightColor);
    text_colors_[views::Button::ButtonState::STATE_DISABLED] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);

    bk_colors_[views::Button::ButtonState::STATE_NORMAL] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_MenuBackgroundColor);
    bk_colors_[views::Button::ButtonState::STATE_HOVERED] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_HoverMenuItemBackgroundColor);
    bk_colors_[views::Button::ButtonState::STATE_PRESSED] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_SelectedMenuItemForegroundColor);
    bk_colors_[views::Button::ButtonState::STATE_DISABLED] = native_theme->GetSystemColor(ui::NativeTheme::kColorId_MenuBackgroundColor);
}

views::MenuItemView* BililiveMenuItemView::AppendMenuItem(int item_id, const base::string16& label, Type type/* = views::MenuItemView::NORMAL*/)
{
    menu_items_++;
    return AppendMenuItemImpl(item_id, label, string16(), gfx::ImageSkia(), type,
        ui::NORMAL_SEPARATOR);
}

void BililiveMenuItemView::RemoveMenuItemAt(int index) {
    __super::RemoveMenuItemAt(index);
    menu_items_--;
}

int BililiveMenuItemView::GetMenuItemCount() {
    return menu_items_;
}

views::MenuItemView* BililiveMenuItemView::AppendSubMenu(int item_id, const string16& label)
{
    return AppendMenuItemImpl(item_id, label, string16(), gfx::ImageSkia(), SUBMENU, 
        ui::NORMAL_SEPARATOR);
}

views::MenuItemView* BililiveMenuItemView::AppendCheckMenu(int item_id, const string16& label)
{
    return AppendMenuItemImpl(item_id, label, string16(), gfx::ImageSkia(), CHECKBOX,
        ui::NORMAL_SEPARATOR);
}

views::MenuItemView* BililiveMenuItemView::AppendMenuItemImpl(
    int item_id,
    const string16& label,
    const string16& sublabel,
    const gfx::ImageSkia& icon,
    Type type,
    ui::MenuSeparatorType separator_style)
{
    const int index = GetSubmenu() ? GetSubmenu()->child_count() : 0;
    return AddMenuItemAt(index, item_id, label, sublabel, icon, type, separator_style);
}

views::MenuItemView* BililiveMenuItemView::AddMenuItemAt(
    int index,
    int item_id,
    const string16& label,
    const string16& sublabel,
    const gfx::ImageSkia& icon,
    Type type,
    ui::MenuSeparatorType separator_style)
{
    DCHECK_NE(type, EMPTY);
    DCHECK_LE(0, index);
    if (!GetSubmenu())
        CreateSubmenu();
    DCHECK_GE(GetSubmenu()->child_count(), index);
    if (type == SEPARATOR) {
        GetSubmenu()->AddChildViewAt(new views::MenuSeparator(this, separator_style), index);
        return NULL;
    }
    views::MenuItemView* item = CreateCurrentDevrivedInstance(this, item_id, type);
    if (label.empty() && GetDelegate())
        item->SetTitle(GetDelegate()->GetLabel(item_id));
    else
        item->SetTitle(label);
    item->SetSubtitle(sublabel);
    if (!icon.isNull())
        item->SetIcon(icon);
    if (type == SUBMENU)
        item->CreateSubmenu();
    GetSubmenu()->AddChildViewAt(item, index);
    return item;
}

views::SubmenuView* BililiveMenuItemView::CreateSubmenu()
{
    if (!submenu_)
    {
        submenu_ = new BililiveSubmenuView(this);
        ((BililiveSubmenuView*)submenu_)->set_minimum_preferred_width(minimum_preferred_width());
    }
    return submenu_;
}

const gfx::Font& BililiveMenuItemView::GetFont()
{
    return font();
}

views::MenuItemView* BililiveMenuItemView::CreateCurrentDevrivedInstance(
    BililiveMenuItemView* parent, int command, views::MenuItemView::Type type)
{
    return new BililiveMenuItemView(parent, command, type);
}

gfx::Size BililiveMenuItemView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_width(std::max(size.width(), minimum_preferred_width()));
    if (-1 != maximize_preferred_width())
    {
        size.set_width(std::min(size.width(), maximize_preferred_width()));
    }
    return size;
}

void BililiveMenuItemView::OnPaint(gfx::Canvas* canvas)
{
    //__super::OnPaint(canvas);

    const views::MenuConfig& config = GetMenuConfig();
    bool render_selection =
        (/*mode == PB_NORMAL && */IsSelected() &&
        GetParentMenuItem()->GetSubmenu()->GetShowSelection(this) &&
        (NonIconChildViewsCount() == 0));

    int icon_x = config.item_left_margin + /*left_icon_margin_*/0;
    int top_margin = GetTopMargin();
    int bottom_margin = GetBottomMargin();
    //int icon_y = top_margin + (height() - config.item_top_margin -
    //    bottom_margin - config.check_height) / 2;
    //int icon_height = config.check_height;
    int available_height = height() - top_margin - bottom_margin;
    views::MenuDelegate *delegate = GetDelegate();

    SkColor bk_color = bk_colors_[views::Button::STATE_NORMAL];
    if (enabled())
    {
        bk_color = background_color(render_selection ? views::Button::STATE_HOVERED : views::Button::STATE_NORMAL);
    }
    else
    {
        bk_color = background_color(views::Button::STATE_DISABLED);
    }
    canvas->DrawColor(bk_color);

    // Render the check.
    if (menu_type_ == CHECKBOX && delegate->IsItemChecked(GetCommand()))
    {
        // Don't use config.check_width here as it's padded
        // to force more padding (AURA).
        const gfx::ImageSkia* image = checked_image();
        gfx::Rect check_bounds(
            icon_x, 
            top_margin + (available_height - image->height()) / 2, 
            image->width(), 
            image->height());
        AdjustBoundsForRTLUI(&check_bounds);
        canvas->DrawImageInt(*image, check_bounds.x(), check_bounds.y());
    }
    else if (menu_type_ == RADIO)
    {
        const gfx::ImageSkia* image = radio_image(delegate->IsItemChecked(GetCommand()));
        gfx::Rect radio_bounds(
            icon_x,
            top_margin + (available_height - image->height()) / 2,
            image->width(),
            image->height());
        AdjustBoundsForRTLUI(&radio_bounds);
        canvas->DrawImageInt(*image, radio_bounds.x(), radio_bounds.y());
    }

    // Render the foreground.
    SkColor fg_color = text_colors_[views::Button::STATE_NORMAL];
    if (enabled())
    {
        fg_color = text_color(render_selection ? views::Button::STATE_HOVERED : views::Button::STATE_NORMAL);
    }
    else
    {
        fg_color = text_color(views::Button::STATE_DISABLED);
    }

    const gfx::Font& font = GetFont();
    int accel_width = GetParentMenuItem()->GetSubmenu()->max_minor_text_width();
    int label_start = GetLabelStartForThisItem();

    int width = this->width() - label_start - accel_width -
        (!delegate ||
        delegate->ShouldReserveSpaceForSubmenuIndicator() ?
    /*item_right_margin_*/0 : config.arrow_to_edge_padding);
    gfx::Rect text_bounds(label_start, top_margin, width, available_height);
    text_bounds.set_x(GetMirroredXForRect(text_bounds));
    int flags = GetDrawStringFlags();
    /*if (mode == PB_FOR_DRAG)
        flags |= gfx::Canvas::NO_SUBPIXEL_RENDERING;*/
    canvas->DrawStringInt(title(), font, fg_color,
        text_bounds.x(), text_bounds.y(), text_bounds.width(),
        text_bounds.height(), flags);

    PaintMinorText(canvas, render_selection);

    // Render the submenu indicator (arrow).
    if (HasSubmenu())
    {
        const gfx::ImageSkia* image = arrow_image(render_selection);
        gfx::Rect arrow_bounds(
            this->width() - image->width() - config.arrow_to_edge_padding,
            top_margin + (available_height - image->height()) / 2,
            image->width(), 
            image->height());
        AdjustBoundsForRTLUI(&arrow_bounds);
        canvas->DrawImageInt(*image, arrow_bounds.x(), arrow_bounds.y());
    }
}

void BililiveMenuItemView::PaintMinorText(gfx::Canvas* canvas, SkColor color)
{
    string16 minor_text = GetMinorText();
    if (minor_text.empty())
        return;

    const gfx::Font& font = GetFont();
    int available_height = height() - GetTopMargin() - GetBottomMargin();
    int max_accel_width =
        GetParentMenuItem()->GetSubmenu()->max_minor_text_width();
    const views::MenuConfig& config = GetMenuConfig();
    int accel_right_margin = config.align_arrow_and_shortcut ?
        config.arrow_to_edge_padding : /*item_right_margin_*/0;
    gfx::Rect accel_bounds(width() - accel_right_margin - max_accel_width,
        GetTopMargin(), max_accel_width, available_height);
    accel_bounds.set_x(GetMirroredXForRect(accel_bounds));
    int flags = GetDrawStringFlags();
    flags &= ~(gfx::Canvas::TEXT_ALIGN_RIGHT | gfx::Canvas::TEXT_ALIGN_LEFT);
    if (base::i18n::IsRTL())
        flags |= gfx::Canvas::TEXT_ALIGN_LEFT;
    else
        flags |= gfx::Canvas::TEXT_ALIGN_RIGHT;
    canvas->DrawStringInt(
        minor_text,
        font,
        color,
        accel_bounds.x(),
        accel_bounds.y(),
        accel_bounds.width(),
        accel_bounds.height(),
        flags);
}

void BililiveMenuItemView::SetTextColor(views::Button::ButtonState for_state, SkColor color)
{
    text_colors_[for_state] = color;
}

void BililiveMenuItemView::SetBackgroundColor(views::Button::ButtonState for_state, SkColor color)
{
    bk_colors_[for_state] = color;
}

void BililiveMenuItemView::SetBorderColor(SkColor color)
{
    border_color_ = color;
}

const BililiveMenuItemView* BililiveMenuItemView::GetRootBililiveMenuItem() const
{
    BililiveMenuItemView *root_menu = nullptr;
    const views::MenuItemView *root_item = GetRootMenuItem();
    if (strcmp(((views::View*)root_item)->GetClassName(), BililiveMenuItemView::kViewClassName) == 0)
    {
        root_menu = (BililiveMenuItemView*)root_item;
    }
    return root_menu;
}

const gfx::Font& BililiveMenuItemView::font()
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->font_;
    }
    return font_;
}

SkColor BililiveMenuItemView::text_color(views::Button::ButtonState state)
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->text_colors_[state];
    }
    return text_colors_[state];
}

SkColor BililiveMenuItemView::border_color()
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->border_color_;
    }
    return border_color_;
}

SkColor BililiveMenuItemView::background_color(views::Button::ButtonState state)
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->bk_colors_[state];
    }
    return bk_colors_[state];
}

const gfx::ImageSkia* BililiveMenuItemView::arrow_image(bool render_selection)
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return render_selection ? root_menu->arrow_selection_image_ : root_menu->arrow_image_;
    }
    return views::GetSubmenuArrowImage();
}

const gfx::ImageSkia* BililiveMenuItemView::checked_image()
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->checked_image_;
    }
    return views::GetMenuCheckImage();
}

const gfx::ImageSkia* BililiveMenuItemView::radio_image(bool selected)
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return selected ? root_menu->radio_checked_image_ : root_menu->radio_unchecked_image_;
    }
    return views::GetRadioButtonImage(selected);
}

void BililiveMenuItemView::SetRadioImage(gfx::ImageSkia *image, bool selected)
{
    if (selected)
    {
        radio_checked_image_ = image;
    }
    else
    {
        radio_unchecked_image_ = image;
    }
}

int BililiveMenuItemView::minimum_preferred_width()
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->minimum_preferred_width_;
    }
    return minimum_preferred_width_;
}

int BililiveMenuItemView::maximize_preferred_width()
{
    const BililiveMenuItemView *root_menu = GetRootBililiveMenuItem();
    if (root_menu)
    {
        return root_menu->maximize_preferred_width_;
    }
    return maximize_preferred_width_;
}

void BililiveMenuItemView::SetArrowImage(gfx::ImageSkia *image, gfx::ImageSkia *sel_image)
{
    arrow_image_ = image;
    arrow_selection_image_ = sel_image;
}
