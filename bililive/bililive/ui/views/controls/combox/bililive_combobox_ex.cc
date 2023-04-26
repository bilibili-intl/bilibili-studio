#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"

#include "ui/base/win/dpi.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/screen.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/views/uploader/bililive_uploader_type.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "grit/theme_resources.h"


namespace
{
    const int kRadius = 4 * ui::GetDPIScale();
    const int label_arrow_thickness = 5 * ui::GetDPIScale();
    const int label_arrow_thickness_min = 2 * ui::GetDPIScale();
    // 左右缩进
    const int kLeftInsets = 12 * ui::GetDPIScale();
    const int kRightInsets = 12 * ui::GetDPIScale();

    const int kMaxDroplistHeight =300 * ui::GetDPIScale();
    const int kMinShowHeight = 157 * ui::GetDPIScale();
    const U8CPU disable_mask = 0.3f * 255;

    class EditLabelArea : public views::View
    {
    public:
        EditLabelArea()
            : edit_view_(nullptr)
        {
            set_notify_enter_exit_on_child(true);
        }

        void AddChildView(views::View *view, bool edit = false)
        {
            if (edit)
            {
                edit_view_ = view;
            }
            __super::AddChildView(view);
        };
    protected:
        // View
        virtual void Layout() override
        {
            gfx::Rect rc = GetLocalBounds();
            gfx::Insets ist = GetInsets();
            rc.Inset(ist);
            for (int i = 0; i < child_count(); ++i)
            {
                child_at(i)->SetBounds(rc.x(), rc.y(), rc.width(), rc.height());
            }
        };

        virtual gfx::Size GetPreferredSize() override
        {
            gfx::Size size;
            for (int i = 0; i < child_count(); ++i)
            {
                size.SetToMax(child_at(i)->GetPreferredSize());
            }
            return size;
        };

        virtual int GetHeightForWidth(int w)
        {
            return GetPreferredSize().height();
        };

        virtual bool HitTestRect(const gfx::Rect& rect) const override
        {
            /*if (edit_view_ && !edit_view_->visible())
            {
                return false;
            }*/
            return false;
        };

    private:
        views::View *edit_view_;
    };

    // BililiveComboboxDroplistItemView
    class BililiveComboboxDroplistItemView : public views::CustomButton
    {
    public:
        explicit BililiveComboboxDroplistItemView(const BililiveComboboxEx::_COMBOBOX_ITEM &data,
            views::ButtonListener *listener, BililiveComboboxEx *combobox)
            : views::CustomButton(listener)
            , combobox_(combobox)
            , data_(data)
            , is_selected_(false)
        {
            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kLeftInsets);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kRightInsets);

            label_ = new views::Label(data.text);
            label_->SetAutoColorReadabilityEnabled(false);
            label_->set_background(nullptr);
            label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label_->SetEnabledColor(combobox_->item_color_);
            label_->SetFont(combobox_->font());

            layout->StartRow(1.0f, 0);
            layout->AddView(label_);

            SetEnabled(data_.valid);
        }
        virtual ~BililiveComboboxDroplistItemView() = default;

        BililiveComboboxEx::_COMBOBOX_ITEM& data() { return data_; }

        void SetSelected(bool selected)
        {
            is_selected_ = selected;
            StateChanged();
        };

    protected:
        // View
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = __super::GetPreferredSize();
            size.Enlarge(0, size.height() * 0.5f);
            return size;
        };
        int GetHeightForWidth(int w) override
        {
            return GetPreferredSize().height();
        };

        // CustomButton
        void StateChanged() override
        {
            if (is_selected_)
            {
                //set_background(views::Background::CreateSolidBackground(bililive_uploader::clrGrayBackground));
                label_->SetEnabledColor(combobox_->item_focus_color_);
            }
            else
            {
                views::Button::ButtonState bs = state();
                switch (bs)
                {
                case Button::STATE_NORMAL:
                    set_background(nullptr);
                    label_->SetEnabledColor(combobox_->item_color_);
                    break;
                case Button::STATE_HOVERED:
                case Button::STATE_PRESSED:
                    set_background(views::Background::CreateSolidBackground(SkColorSetARGB(0x0c,0x0e,0xbe,0xff)));//bililive_uploader::clrGrayBackground
                    label_->SetEnabledColor(combobox_->item_focus_color_);
                    break;
                default:
                    set_background(nullptr);
                    label_->SetEnabledColor(SkColorSetA(combobox_->item_color_, disable_mask));
                    break;
                }
            }
            SchedulePaint();
        };

    private:
        BililiveComboboxEx *combobox_;
        BililiveComboboxEx::_COMBOBOX_ITEM data_;
        views::Label *label_;
        bool is_selected_;
    };


    // BililiveComboboxDropdownView
    class BililiveComboboxDropdownView
        : public views::WidgetDelegateView
        , public views::ButtonListener
    {
    public:
        explicit BililiveComboboxDropdownView(BililiveComboboxEx *combobox)
            : combobox_(combobox)
            , current_choosed_item_(-1)
        {
            contents_ = new BililiveViewWithFloatingScrollbar();
            views::BoxLayout *layout = new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, 0);
            contents_->SetLayoutManager(layout);

            for (size_t i = 0; i < combobox_->items_.size(); i++)
            {
                contents_->AddChildView(new BililiveComboboxDroplistItemView(*(combobox_->items_[i]),
                    this, combobox_));
            }

            views::FillLayout *fill_layout = new views::FillLayout();
            SetLayoutManager(fill_layout);
            AddChildView(contents_->Container());

            contents_->Container()->set_background(views::Background::CreateSolidBackground(combobox_->droplist_bk_color_));
            contents_->Container()->set_border(views::Border::CreateSolidBorder(1, combobox_->droplist_border_color_));
        }
        virtual ~BililiveComboboxDropdownView() = default;

        void SelectByUpOrDown(bool is_down)
        {
            if (contents_->child_count())
            {
                if (is_down)
                {
                    current_choosed_item_++;
                }
                else
                {
                    current_choosed_item_--;
                }
                current_choosed_item_ = std::max(current_choosed_item_, 0);
                current_choosed_item_ = std::min(current_choosed_item_, contents_->child_count() - 1);

                for (int i = 0; i < contents_->child_count(); i++)
                {
                    ((BililiveComboboxDroplistItemView*)contents_->child_at(i))->SetSelected(i == current_choosed_item_);
                }

                gfx::Size pref_size = contents_->GetPreferredSize();
                gfx::Rect visb_bound = contents_->GetVisibleBounds();
                gfx::Rect item_bound = contents_->child_at(current_choosed_item_)->bounds();
                if (item_bound.y() < visb_bound.y())
                {
                    // 往上滚一页
                    contents_->Container()->ScrollToPosition(true,
                        std::max(0, visb_bound.y() - (visb_bound.height() - item_bound.height())));
                }
                else if (item_bound.bottom() > visb_bound.bottom())
                {
                    // 往下滚一页
                    contents_->Container()->ScrollToPosition(true,
                        std::min(pref_size.height() - visb_bound.height(),
                        visb_bound.y() + (visb_bound.height() - item_bound.height())));
                }
            }
        };

        void SelectByReturn()
        {
            if (current_choosed_item_ >= 0 && current_choosed_item_ < contents_->child_count())
            {
                ButtonPressed(((BililiveComboboxDroplistItemView*)contents_->child_at(current_choosed_item_)),
                    ui::MouseEvent(ui::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(), ui::EF_NONE));
            }
        };

        ThumbState current_scrollbar_state() const
        {
            return contents_->Container()->current_scrollbar_state(true);
        }

    protected:
        // WidgetDelegate
        virtual views::View *GetContentsView() override { return this; };

        // View
        virtual gfx::Size GetPreferredSize() override
        {
            gfx::Size size = contents_->GetPreferredSize();
            size.Enlarge(2, 2);
            if(!combobox_->droplist_equal_width())
            {
                size.set_width(std::max(size.width(), combobox_->width()));
                size.set_height(std::min(kMaxDroplistHeight, size.height()));
            }
            else
            {
                size.set_width(combobox_->width());
            }
            return size;
        };

        // ButtonListener
        virtual void ButtonPressed(views::Button* sender, const ui::Event& event) override
        {
            if (combobox_)
            {
                combobox_->OnSelectedIndexChanged(((BililiveComboboxDroplistItemView*)sender)->data());
            }
            GetWidget()->Close();
        }

    private:
        BililiveComboboxEx *combobox_;
        BililiveViewWithFloatingScrollbar *contents_;

        int current_choosed_item_;

        DISALLOW_COPY_AND_ASSIGN(BililiveComboboxDropdownView);
    };


    // BililiveComboboxDropdownWidget
    class BililiveComboboxDropdownWidget : public views::Widget
    {
    public:
        explicit BililiveComboboxDropdownWidget(BililiveComboboxEx *expand_button, BililiveComboboxDropdownView *dropdown_view)
            : combobox_(expand_button)
            , dropdown_view_(dropdown_view)
        {
        }
        virtual ~BililiveComboboxDropdownWidget(){}

    protected:
        // Widget
        virtual void OnNativeWidgetVisibilityChanged(bool visible) OVERRIDE
        {
            if (visible)
            {
                gfx::Rect related_rect = combobox_->GetBoundsInScreen();
                gfx::Rect workarea = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(related_rect.origin()).work_area();
                switch (combobox_->anchor_position_)
                {
                case BililiveComboboxEx::AnchorPosition::TOPLEFT:
                {
                    int residual_cy = workarea.bottom() - related_rect.bottom();
                    // 如果底部可用的空间大于预期的下拉列表最小高度则TOPLEFT显示，否则BOTTOMLEFT显示
                    if (residual_cy >= kMinShowHeight)
                    {
                        ShowInTopLeft();
                    }
                    else
                    {
                        ShowInBottomLeft();
                    }
                }
                break;
                case BililiveComboboxEx::AnchorPosition::BOTTOMLEFT:
                {
                    int residual_cy = related_rect.y() - workarea.y();
                    // 如果底部可用的空间大于预期的下拉列表最小高度则TOPLEFT显示，否则BOTTOMLEFT显示
                    if (residual_cy >= kMinShowHeight)
                    {
                        ShowInBottomLeft();
                    }
                    else
                    {
                        ShowInTopLeft();
                    }
                }
                break;
                default:
                    break;
                }
            }
            __super::OnNativeWidgetVisibilityChanged(visible);
        };

        virtual void OnNativeWidgetDestroying() override
        {
            combobox_->OnDropdownClose(dropdown_view_);
        };

    private:
        void ShowInTopLeft()
        {
            gfx::Rect related_rect = combobox_->GetBoundsInScreen();
            gfx::Rect workarea = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(related_rect.origin()).work_area();
            gfx::Size pref_size = GetRootView()->GetPreferredSize();
            gfx::Rect bounds;
            int height = std::min(workarea.bottom() - related_rect.bottom(), pref_size.height());
            height = std::min(kMaxDroplistHeight, height);
            bounds.SetRect(related_rect.x(), related_rect.bottom(), pref_size.width(), height);
            SetBounds(bounds);
        }

        void ShowInBottomLeft()
        {
            gfx::Rect related_rect = combobox_->GetBoundsInScreen();
            gfx::Rect workarea = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(related_rect.origin()).work_area();
            gfx::Size pref_size = GetRootView()->GetPreferredSize();
            gfx::Rect bounds;
            int height = std::min(related_rect.y() - workarea.y(), pref_size.height());
            height = std::min(kMaxDroplistHeight, height);
            bounds.SetRect(related_rect.x(), related_rect.y() - height, pref_size.width(), height);
            SetBounds(bounds);
        }

    private:
        BililiveComboboxEx *combobox_;
        BililiveComboboxDropdownView *dropdown_view_;

        DISALLOW_COPY_AND_ASSIGN(BililiveComboboxDropdownWidget);
    };
}

volatile long BililiveComboboxEx::_COMBOBOX_ITEM::inc = 0;

BililiveComboboxEx::BililiveComboboxEx(bool editable/* = false*/, int underline_style/* = false*/)
    : views::CustomButton(nullptr)
    , current_index_(-1)
    , listener_(nullptr)
    , dropdown_view_(nullptr)
    , anchor_position_(TOPLEFT)
    , editable_(editable)
    , edit_blur_reason_(EDIT_BLUR_REASON_DEFAULT)
    , edit_view_(nullptr)
    , is_dropdown_(false)
    , default_item_(L"")
    , border_color_(bililive_uploader::clrGrayBorder)
    , border_focus_color_(bililive_uploader::clrPinkBorder)
    , item_color_(bililive_uploader::clrBlackText)
    , item_focus_color_(bililive_uploader::clrPink)
    , droplist_border_color_(SK_ColorGRAY)
    , droplist_bk_color_(SK_ColorWHITE)
    , default_width_in_chars_(0)
    , default_width_padding_char_(L" ")
    , weakptr_factory_(this)
{
    set_notify_enter_exit_on_child(true);
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    int insert = 0;
    if (underline_style > 1) {
        insert = underline_style;

        arrow_up_hv_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_MATCH_DROP_DOWN);
        arrow_down_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_MATCH_DROP_DOWN);
        arrow_down_hv_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_MATCH_DROP_DOWN);
    }
    else {
        insert = underline_style ? 0 : kLeftInsets;

        arrow_up_hv_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_PULL_UP_HV);
        arrow_down_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_DROP_DOWN);
        arrow_down_hv_img_ = rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_STRIP_DROP_DOWN_HV);
    }

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, insert);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    if (underline_style > 1) {
        column_set->AddPaddingColumn(0, label_arrow_thickness_min);
    }
    else {
        column_set->AddPaddingColumn(0, label_arrow_thickness);
    }
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, insert);

    main_area_ = new EditLabelArea();
    {
        label_ = new views::Label();
        label_->SetAutoColorReadabilityEnabled(false);
        label_->SetDisabledColor(SkColorSetA(item_color_, disable_mask));
        label_->SetEnabledColor(item_color_);
        label_->SetHorizontalAlignment(underline_style ? gfx::ALIGN_CENTER : gfx::ALIGN_LEFT);
        main_area_->AddChildView(label_);
        if (editable_)
        {
            edit_view_ = new BilibiliNativeEditView();
            edit_view_->SetVerticalAlignment(gfx::ALIGN_VCENTER);
            edit_view_->RemoveBorder();
            edit_view_->SetTextColor(item_color_);
            edit_view_->SetSkipDefaultKeyEventProcessing(true);
            edit_view_->SetController(this);
            edit_view_->SetVisible(false);
            main_area_->AddChildView(edit_view_, true);
        }
    }

    arrow_ = new views::ImageView();
    arrow_->set_interactive(false);
    arrow_->SetImage(arrow_down_img_);

    layout->AddPaddingRow(0, kRadius / 2);
    layout->StartRow(1.0f, 0);
    layout->AddView(main_area_);
    layout->AddView(arrow_);
    layout->AddPaddingRow(0, kRadius / 2);

    set_focusable(true);
    set_focus_border(nullptr);
    set_request_focus_on_press(true);
}

BililiveComboboxEx::~BililiveComboboxEx()
{
}

void BililiveComboboxEx::SetArrowImage(gfx::ImageSkia *down_image, gfx::ImageSkia *down_hv_image, gfx::ImageSkia *up_image)
{
    arrow_down_img_ = down_image;
    arrow_down_hv_img_ = down_hv_image;
    arrow_up_hv_img_ = up_image;
}

void BililiveComboboxEx::SetBackgroundColor(SkColor color)
{
    droplist_bk_color_ = color;
}

void BililiveComboboxEx::SetBorderColor(SkColor clrBlur, SkColor clrFocus, SkColor clrDroplist)
{
    border_color_ = clrBlur;
    border_focus_color_ = clrFocus;
    droplist_border_color_ = clrDroplist;
    SchedulePaint();
}

void BililiveComboboxEx::SetTextColor(SkColor clrNor, SkColor clrHov)
{
    label_->SetEnabledColor(clrNor);
    label_->SetDisabledColor(SkColorSetA(clrNor, disable_mask));
    if (edit_view_)
    {
        edit_view_->SetTextColor(clrNor);
    }
    SetItemTextColor(clrNor, clrHov);
}

void BililiveComboboxEx::SetItemTextColor(SkColor clrNor, SkColor clrHov)
{
	item_color_ = clrNor;
	item_focus_color_ = clrHov;
}

void BililiveComboboxEx::set_default_width_in_chars(int default_width, const base::string16 &padding_char)
{
    default_width_in_chars_ = default_width;
    default_width_padding_char_ = padding_char;
    InvalidateLayout();
}

void BililiveComboboxEx::SetDroplistEqualWidth(bool equal)
{
    equal_width_ = equal;
}

void BililiveComboboxEx::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            //////////////////////////////////////////////////////////////////////////
        }
        else
        {
            CloseDropDown();
        }
    }
}

gfx::Size BililiveComboboxEx::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    if (fixed_width_ > 0)
    {
        size.set_width(fixed_width_);
    }
    else
    {
		gfx::Size label_size = label_->GetPreferredSize();
		int eng_cx = std::max(0, font_.GetStringWidth(default_width_padding_char_) * default_width_in_chars_ + 2 - label_size.width());
		size.set_width(size.width() + eng_cx);
    }
    //size.set_height(std::max(size.height(), font_.GetHeight() + kRadius));
    return size;
}

int BililiveComboboxEx::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void BililiveComboboxEx::OnPaintBackground(gfx::Canvas* canvas)
{
    SkColor clr = border_color_;
    views::Button::ButtonState st = state();
    if (st == views::Button::STATE_NORMAL)
    {
        clr = border_color_;
    }
    else if (st == views::Button::STATE_DISABLED)
    {
        clr = SkColorSetA(border_color_, 0.3f * 255);
    }
    else
    {
        clr = border_focus_color_;
    }
    if (is_dropdown_)
    {
        clr = border_focus_color_;
    }
    if (edit_view_ && edit_view_->visible() && edit_view_->IsFocus())
    {
        clr = border_focus_color_;
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(clr); 

    paint.setColor(SkColorSetRGB(56, 57, 70)); // 海外的颜色统一
    // 海外填充绘制
    bililive::FillRoundRect(canvas, 0, 0, width(), height(), kRadius, paint);
}

bool BililiveComboboxEx::HitTestRect(const gfx::Rect& rect) const
{
    if (!enabled())
    {
        return false;
    }
    return __super::HitTestRect(rect);
}

void BililiveComboboxEx::VisibilityChanged(views::View* starting_from, bool is_visible)
{
    if (!visible())
    {
        CloseDropDown();
    }
}

void BililiveComboboxEx::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    CloseDropDown();
}

void BililiveComboboxEx::OnVisibleBoundsChanged()
{
    CloseDropDown();
}

bool BililiveComboboxEx::OnMousePressed(const ui::MouseEvent& event)
{
    if (editable_)
    {
        if (main_area_->bounds().Contains(event.location()))
        {
            label_->SetVisible(false);
            base::string16 edit_text = label_->text();
            if (listener_)
            {
                edit_text = listener_->OnBililiveComboboxExEditShow(this, edit_text);
            }
            edit_view_->SetText(edit_text);
            edit_view_->SetSel(0, -1);
            edit_view_->SetVisible(true);
            edit_view_->RequestFocus();

            return true;
        }
    }
    if (listener_)
    {
        listener_->OnBililiveComboboxPressed(this);
    }
    return __super::OnMousePressed(event);
}

void BililiveComboboxEx::OnMouseReleased(const ui::MouseEvent& event)
{
    __super::OnMouseReleased(event);

    if (edit_view_ && edit_view_->IsFocus())
    {
        if (!arrow_->bounds().Contains(event.location()))
        {
            return;
        }
    }

    if (is_dropdown_)
    {
        CloseDropDown();
    }
    else
    {
        DoDropDown();
    }
}

int BililiveComboboxEx::FindItem(const base::string16 &str, int start/* = 0*/)
{
    for (size_t i = start; i < items_.size(); i++)
    {
        if (items_[i]->text == str)
        {
            return i;
        }
    }
    return -1;
}

int BililiveComboboxEx::DeleteItem(int index)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_.erase(items_.begin() + index);
        if (current_index_ == index)
        {
            SetSelectedIndex(-1);
        }
    }
    return GetItemCount();
}

int BililiveComboboxEx::GetItemCount()
{
    return items_.size();
}

bool BililiveComboboxEx::SetItemText(int index, const base::string16& str)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_[index]->text = str;
        return true;
    }
    return false;
}

bool BililiveComboboxEx::EnabledItem(int index, bool enable)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_[index]->valid = enable;
        return true;
    }
    return false;
}

void BililiveComboboxEx::ClearItems()
{
    // 关闭下拉框
    CloseDropDown();

    items_.clear();
    current_index_ = -1;
    label_->SetText(default_item_.text);
}

base::string16 BililiveComboboxEx::GetItemText(int index)
{
    if (index >= 0 && index < (int)items_.size())
    {
        return items_[index]->text;
    }
    else if (index == -1)
    {
        return default_item_.text;
    }
    return L"";
}

void BililiveComboboxEx::SetSelectedIndex(int index)
{
    if (index >= 0 && index < (int)items_.size())
    {
        current_index_ = index;
        label_->SetText(items_[index]->text);
    }
    else if (index == -1)
    {
        current_index_ = -1;
        label_->SetText(default_item_.text);
    }
    Relayout();
}

void BililiveComboboxEx::OnSelectedIndexChanged(const _COMBOBOX_ITEM &item)
{
    for (size_t i = 0; i < items_.size(); i++)
    {
        if (items_[i]->id == item.id)
        {
            SetSelectedIndex(i);
            break;
        }
    }
    if (listener_)
    {
        listener_->OnSelectedIndexChanged(this);
    }

    if (edit_view_ && edit_view_->IsFocus())
    {
        edit_blur_reason_ = EDIT_BLUR_REASON_SELECT_ITEM;
        ::SetFocus(GetWidget()->GetNativeView());
        RequestFocus();
    }
}

void BililiveComboboxEx::DoDropDown()
{
    // 显示弹框
    if (items_.size())
    {
        if (listener_)
        {
            listener_->OnBililiveComboboxExDropDown(this);
        }

        is_dropdown_ = true;
        StateChanged();

        dropdown_view_ = new BililiveComboboxDropdownView(this);

        BililiveComboboxDropdownWidget *widget_ = new BililiveComboboxDropdownWidget(this, dropdown_view_);
        views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
        if (GetWidget())
        {
            params.parent = GetWidget()->GetNativeView();
        }
        params.delegate = dropdown_view_;
        params.can_activate = false;
        widget_->Init(params);
        widget_->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
        widget_->ShowInactive();

        if (!detect_timer_.IsRunning())
        {
            detect_timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(40),
                base::Bind(&BililiveComboboxEx::OnDetectMousePressed, weakptr_factory_.GetWeakPtr()));
        }
    }
}

void BililiveComboboxEx::CloseDropDown()
{
    detect_timer_.Stop();
    if (dropdown_view_)
    {
        dropdown_view_->GetWidget()->Close();
    }
}

void BililiveComboboxEx::OnDropdownClose(BililiveComboboxDropdownView *dropdown_view)
{
    if (dropdown_view_ == dropdown_view)
    {
        dropdown_view_ = nullptr;
        is_dropdown_ = false;
        StateChanged();
        SchedulePaint();

        if (listener_)
        {
            listener_->OnBililiveComboboxExDropDownClosed(this);
        }
    }
}

void BililiveComboboxEx::Relayout()
{
    InvalidateLayout();
    Layout();
}

bool BililiveComboboxEx::OnKeyPressed(const ui::KeyEvent& event)
{
    switch (event.key_code())
    {
    case ui::VKEY_ESCAPE:
        CloseDropDown();
        break;
    case ui::VKEY_UP:
        if (dropdown_view_)
        {
            dropdown_view_->SelectByUpOrDown(false);
        }
        break;
    case ui::VKEY_DOWN:
        if (dropdown_view_)
        {
            dropdown_view_->SelectByUpOrDown(true);
        }
        break;
    case ui::VKEY_RETURN:
        if (dropdown_view_)
        {
            dropdown_view_->SelectByReturn();
        }
        break;
    default:
        break;
    }
    return __super::OnKeyPressed(event);
}

void BililiveComboboxEx::OnEnabledChanged()
{
    label_->SetEnabled(enabled());
    if (edit_view_)
    {
        edit_view_->SetEnabled(enabled());
    }
}

void BililiveComboboxEx::SetFont(const gfx::Font &font)
{
    font_ = font;
    label_->SetFont(font_);
    if (edit_view_)
    {
        edit_view_->SetFont(font_);
    }
}

gfx::NativeCursor BililiveComboboxEx::GetCursor(const ui::MouseEvent& event)
{
    if (editable_ && enabled())
    {
        if (main_area_->bounds().Contains(event.location()))
        {
            static HCURSOR ibeam = ::LoadCursor(nullptr, IDC_IBEAM);
            return ibeam;
        }
    }
    return __super::GetCursor(event);
}

void BililiveComboboxEx::StateChanged()
{
    views::Button::ButtonState st = state();
    gfx::ImageSkia *img = nullptr;
    bool hover = (st == views::Button::STATE_HOVERED || st == views::Button::STATE_PRESSED);
    if (is_dropdown_)
    {
        img = arrow_up_hv_img_;
    }
    else
    {
        img = hover ? arrow_down_hv_img_ : arrow_down_img_;
    }
    arrow_->SetImage(img);

    // 这里不做非hovered态下的直接关闭，不然用户没有机会将鼠标划到帮助bubble上bubble就关闭了
    if (enable_help() && state() == Button::STATE_HOVERED)
    {
        ShowHelp(this);
    }
}

bool BililiveComboboxEx::PreHandleMSG(BilibiliNativeEditView* edit, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SETFOCUS:
        SchedulePaint();
        break;
    case WM_KILLFOCUS:
    {
        label_->SetVisible(true);
        edit_view_->SetVisible(false);
        if (listener_)
        {
            listener_->OnBililiveComboboxExEditBlur(this, edit_view_->GetText(), edit_blur_reason_);
        }
        edit_blur_reason_ = EDIT_BLUR_REASON_DEFAULT;
        SchedulePaint();
    }
        break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_TAB:
        {
            if (wParam == VK_RETURN)
            {
                edit_blur_reason_ = EDIT_BLUR_REASON_RETURN;
            }
            else
            {
                edit_blur_reason_ = EDIT_BLUR_REASON_SPECIA_KEY;
            }
            ::SetFocus(GetWidget()->GetNativeView());
            RequestFocus();
        }
        break;
        default:
            break;
        }
    }
        break;
    default:
        break;
    }
    return true;
}

void BililiveComboboxEx::ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents)
{
    if (listener_)
    {
        listener_->OnBililiveComboboxExEditContentsChanged(this, new_contents);
    }
}

void BililiveComboboxEx::SetEditInputType(EditInputType eit)
{
    if (edit_view_)
    {
        if (eit & EDIT_INPUT_TYPE_NUMBER)
        {
            edit_view_->SetNumberOnly(true);
        }
        if (eit & EDIT_INPUT_TYPE_OBSCURED)
        {
            edit_view_->SetObscured(true);
        }
    }
}

void BililiveComboboxEx::SetEditLimitText(unsigned int nMax)
{
    if (edit_view_)
    {
        edit_view_->LimitText(nMax);
    }
}

void BililiveComboboxEx::OnDetectMousePressed()
{
    if (dropdown_view_)
    {
        if (dropdown_view_->current_scrollbar_state() == ThumbState_PRESSED)
        {
            return;
        }

        bool ld = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
        bool rd = ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
        if (ld || rd)
        {
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            gfx::Point point(pt);
            if (!GetBoundsInScreen().Contains(point) &&
                !dropdown_view_->GetBoundsInScreen().Contains(point))
            {
                CloseDropDown();
            }
        }
    }
    else
    {
        detect_timer_.Stop();
    }
}

void BililiveComboboxEx::OnBlur()
{
    if (is_dropdown_ && dropdown_view_)
    {
        CloseDropDown();
    }
    __super::OnBlur();
}

bool BililiveComboboxEx::SkipDefaultKeyEventProcessing(const ui::KeyEvent& event)
{
    return true;
}

base::string16 BililiveComboboxEx::GetText() const
{
    if (!edit_view_)
    {
        return L"";
    }
    return edit_view_->GetText();
}

bool BililiveComboboxEx::IsEditFocus()
{
    if (!edit_view_)
    {
        return false;
    }
    return edit_view_->IsFocus();
}