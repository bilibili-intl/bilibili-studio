#include "bilibili_search_list_view.h"

#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/screen.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/controls/label.h"

#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"


namespace
{
    const int kMaxDroplistHeight = 300 * ui::GetDPIScale();
    const int kMinShowHeight = 157 * ui::GetDPIScale();
    const unsigned char disable_alpha = 0.3f * 255;

    // BiliSearchListDroplistItemView
    class DefaultListDroplistItemView : public BilibiliSearchListDroplistItemView
    {
    public:
        DefaultListDroplistItemView(const _SEARCH_ITEM &data, BilibiliSearchListView *search_edit)
            : BilibiliSearchListDroplistItemView(data, search_edit)
        {
            int left_inset, right_inset;
            search_edit_->GetHorizontalMargins(left_inset, right_inset);

            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, left_inset);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, right_inset);

            label_ = new views::Label(data.text);
            label_->SetAutoColorReadabilityEnabled(false);
            label_->set_background(nullptr);
            label_->SetEnabledColor(search_edit_->item_words_normal_color());
            label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            label_->SetFont(search_edit_->font());

            layout->StartRow(1.0f, 0);
            layout->AddView(label_);
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
                label_->SetEnabledColor(search_edit_->item_words_hover_color());
            }
            else
            {
                views::Button::ButtonState bs = state();
                switch (bs)
                {
                case Button::STATE_NORMAL:
                    label_->SetEnabledColor(search_edit_->item_words_normal_color());
                    break;
                case Button::STATE_HOVERED:
                case Button::STATE_PRESSED:
                    label_->SetEnabledColor(search_edit_->item_words_hover_color());
                    break;
                default:
                    label_->SetEnabledColor(SkColorSetA(search_edit_->item_words_normal_color(), disable_alpha));
                    break;
                }
            }
            SchedulePaint();
        };

    private:
        views::Label *label_ = nullptr;

        DISALLOW_COPY_AND_ASSIGN(DefaultListDroplistItemView);
    };


    // BiliSearchListDropdownView
    class BilibiliSearchListDropdownView
        : public views::WidgetDelegateView
        , BilibiliSearchListDroplistItemDelegate
    {
    public:
        explicit BilibiliSearchListDropdownView(BilibiliSearchListView *search_edit_)
            : search_edit_(search_edit_)
            , current_choosed_item_(-1)
        {
            contents_ = new BililiveViewWithFloatingScrollbar();
            views::BoxLayout *layout = new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, 0);
            contents_->SetLayoutManager(layout);

            views::FillLayout *fill_layout = new views::FillLayout();
            SetLayoutManager(fill_layout);
            AddChildView(contents_->Container());

            contents_->Container()->set_background(views::Background::CreateSolidBackground(search_edit_->droplist_bk_color_));
            contents_->Container()->set_border(views::Border::CreateSolidBorder(1, search_edit_->droplist_border_color_));

            UpdateDate(false);
        };

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
                    ((BilibiliSearchListDroplistItemView*)contents_->child_at(i))->SetSelected(i == current_choosed_item_);
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

        bool SelectByReturn()
        {
            if (current_choosed_item_ >= 0 && current_choosed_item_ < contents_->child_count())
            {
                OnSearchDroplistItemSelected(((BilibiliSearchListDroplistItemView*)contents_->child_at(current_choosed_item_)),
                    ui::MouseEvent(ui::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(), ui::EF_NONE));
                return true;
            }
            return false;
        };

        void UpdateDate(bool layout = true)
        {
            contents_->RemoveAllChildViews(true);
            for (size_t i = 0; i < search_edit_->items_.size(); i++)
            {
                _SEARCH_ITEM& data = *(search_edit_->items_[i]);
                BilibiliSearchListDroplistItemView* item_view = nullptr;
                if (search_edit_->controller())
                {
                    item_view = search_edit_->controller()->GetDroplistItemView(data, search_edit_);
                }
                else
                {
                    item_view = new DefaultListDroplistItemView(data, search_edit_);
                }
                DCHECK(item_view);
                if (item_view)
                {
                    item_view->SetDelegateInternal(this);
                    contents_->AddChildView(item_view);
                }
            }

            if (layout)
            {
                contents_->InvalidateLayout();
                contents_->Container()->InvalidateLayout();
                InvalidateLayout();
                Layout();
            }
        };

    protected:
        // WidgetDelegate
        views::View *GetContentsView() override { return this; };

        // View
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size = contents_->GetPreferredSize();
            size.Enlarge(2, 2);
            if (!search_edit_->droplist_equal_width())
            {
                size.set_width(std::max(size.width(), search_edit_->width()));
                size.set_height(std::min(kMaxDroplistHeight, size.height()));
            }
            else
            {
                size.set_width(search_edit_->width());
            }
            return size;
        };

        // BilibiliSearchListDroplistItemDelegate
        void OnSearchDroplistItemSelected(BilibiliSearchListDroplistItemView* item_view, const ui::Event& event) override
        {
            if (search_edit_)
            {
                search_edit_->OnSearchResultItemSelected(item_view->data());
            }
            GetWidget()->Hide();
        }

    private:
        BilibiliSearchListView *search_edit_;
        BililiveViewWithFloatingScrollbar *contents_;

        int current_choosed_item_;

        DISALLOW_COPY_AND_ASSIGN(BilibiliSearchListDropdownView);
    };


    // BiliSearchListDropdownWidget
    class BilibiliSearchListDropdownWidget : public views::Widget
    {
    public:
        explicit BilibiliSearchListDropdownWidget(BilibiliSearchListView *expand_button)
            : expand_button_(expand_button)
        {
        };

        void UpdateOrigin()
        {
            gfx::Rect related_rect = expand_button_->GetBoundsInScreen();
            gfx::Rect workarea = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(related_rect.origin()).work_area();
            gfx::Size pref_size = GetRootView()->GetPreferredSize();
            gfx::Rect bounds;
            switch (expand_button_->anchor_position_)
            {
            case BilibiliSearchListView::AnchorPosition::TOPLEFT:
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
            case BilibiliSearchListView::AnchorPosition::BOTTOMLEFT:
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
        };

    protected:
        // Widget
        void OnNativeWidgetVisibilityChanged(bool visible) override
        {
            if (visible)
            {
                UpdateOrigin();
            }
            else
            {
                expand_button_->OnDropdownHide();
            }
            __super::OnNativeWidgetVisibilityChanged(visible);
        };

        void OnNativeWidgetDestroying() override
        {
            if (IsVisible())
            {
                expand_button_->OnDropdownHide();
            }
            expand_button_->OnDropdownClose();
        };

    private:
        void ShowInTopLeft()
        {
            gfx::Rect related_rect = expand_button_->GetBoundsInScreen();
            gfx::Rect workarea = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(related_rect.origin()).work_area();
            gfx::Size pref_size = GetRootView()->GetPreferredSize();
            gfx::Rect bounds;
            int height = std::min(workarea.bottom() - related_rect.bottom(), pref_size.height());
            height = std::min(kMaxDroplistHeight, height);
            bounds.SetRect(related_rect.x(), related_rect.bottom(), pref_size.width(), height);
            SetBounds(bounds);
        };

        void ShowInBottomLeft()
        {
            gfx::Rect related_rect = expand_button_->GetBoundsInScreen();
            gfx::Rect workarea = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(related_rect.origin()).work_area();
            gfx::Size pref_size = GetRootView()->GetPreferredSize();
            gfx::Rect bounds;
            int height = std::min(related_rect.y() - workarea.y(), pref_size.height());
            height = std::min(kMaxDroplistHeight, height);
            bounds.SetRect(related_rect.x(), related_rect.y() - height, pref_size.width(), height);
            SetBounds(bounds);
        };

    private:
        BilibiliSearchListView *expand_button_;

        DISALLOW_COPY_AND_ASSIGN(BilibiliSearchListDropdownWidget);
    };

}

// _SEARCH_ITEM
volatile long _SEARCH_ITEM::inc = 0;



// BilibiliSearchListDroplistItemView
BilibiliSearchListDroplistItemView::BilibiliSearchListDroplistItemView(const _SEARCH_ITEM& data, BilibiliSearchListView* search_edit)
    : views::CustomButton(nullptr)
    , search_edit_(search_edit)
    , data_(data)
{
    SetEnabled(data_.valid);
}

void BilibiliSearchListDroplistItemView::SetSelected(bool selected)
{
    is_selected_ = selected;
    StateChanged();
}

void BilibiliSearchListDroplistItemView::NotifyClick(const ui::Event& event)
{
    if (delegate_)
    {
        delegate_->OnSearchDroplistItemSelected(this, event);
    }
    //__super::NotifyClick(event);
}

void BilibiliSearchListDroplistItemView::SetDelegateInternal(BilibiliSearchListDroplistItemDelegate* delegate)
{
    delegate_ = delegate;
}



// BilibiliSearchListController
BilibiliSearchListDroplistItemView* BilibiliSearchListController::GetDroplistItemView(
    const _SEARCH_ITEM& data, BilibiliSearchListView* search_edit)
{
    return new DefaultListDroplistItemView(data, search_edit);
}



// BilibiliSearchListView
BilibiliSearchListView::BilibiliSearchListView(BilibiliSearchListListener *listener,
    views::View *relate_view/* = nullptr*/, BilibiliNativeEditView::CooperateDirection dir/* = BilibiliNativeEditView::BD_RIGHT*/)
    : dropdown_view_(nullptr)
    , listener_(listener)
    , anchor_position_(TOPLEFT)
    , item_words_normal_color_(clrLabelText)
    , item_bk_normal_color_(SK_ColorWHITE)
    , item_words_hover_color_(clrFrameAndButtonBk)
    , item_bk_hoverd_color_(clrComboboxBorder)
    , droplist_border_color_(clrComboboxBorder)
    , droplist_bk_color_(SK_ColorWHITE)
    , is_edit_ldown_now_(false)
    , is_edit_rdown_now_(false)
    , is_last_lbtn_valid_(false)
    , weakptr_factory_(this)
{
    set_notify_enter_exit_on_child(true);

    search_edit_ = new BilibiliNativeEditView(relate_view, dir);
    search_edit_->SetController(this);
    search_edit_->SetSkipDefaultKeyEventProcessing(true);

    views::FillLayout *layout = new views::FillLayout();
    SetLayoutManager(layout);
    AddChildView(search_edit_);
}

BilibiliSearchListView::~BilibiliSearchListView()
{
}

void BilibiliSearchListView::SetBlurChangeReason(BiliveSearchListBlurChangeReason reason)
{
    blur_reason_ = reason;
}

void BilibiliSearchListView::SetController(BilibiliSearchListController* controller)
{
    controller_ = controller;
}

BilibiliSearchListController* BilibiliSearchListView::controller()
{
    return controller_;
}

void BilibiliSearchListView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();

            if (GetWidget())
            {
                GetWidget()->AddObserver(this);
            }
        }
        else
        {
            if (GetWidget())
            {
                GetWidget()->RemoveObserver(this);
            }

            UninitView();
        }
    }
}

void BilibiliSearchListView::VisibilityChanged(views::View* starting_from, bool is_visible)
{
    if (!visible())
    {
        // 关框
        HideDropdown();
    }
}

void BilibiliSearchListView::OnEnabledChanged()
{
    search_edit_->SetEnabled(enabled());
    if (!enabled())
    {
        // 关框
        HideDropdown();
    }
}

void BilibiliSearchListView::OnFocus()
{
    RequestFocus();
}

void BilibiliSearchListView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    if (dropdown_view_ && dropdown_view_->GetWidget())
    {
        ((BilibiliSearchListDropdownWidget*)dropdown_view_->GetWidget())->UpdateOrigin();
    }
}

void BilibiliSearchListView::InitView()
{
    dropdown_view_ = new BilibiliSearchListDropdownView(this);
    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    params.parent = GetWidget()->GetNativeView();
    params.delegate = dropdown_view_;
    params.can_activate = false;
    params.remove_taskbar_icon = true;
    params.keep_on_top = true;
    BilibiliSearchListDropdownWidget *widget = new BilibiliSearchListDropdownWidget(this);
    widget->Init(params);
    widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    widget->Hide();
}

void BilibiliSearchListView::UninitView()
{
    detect_timer_.Stop();
    if (dropdown_view_)
    {
        dropdown_view_->GetWidget()->Close();
    }
    dropdown_view_ = nullptr;
    search_edit_ = nullptr;
}

void BilibiliSearchListView::ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents)
{
    if (listener_)
    {
        listener_->OnSearchEditContentsChanged(this, new_contents);
    }
}

bool BilibiliSearchListView::PreHandleMSG(BilibiliNativeEditView* edit, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
    {
        is_edit_ldown_now_ = true;
        // 弹框
        DoDropDown();
    }
        break;
    case WM_LBUTTONUP:
        is_edit_ldown_now_ = false;
        break;
    case WM_RBUTTONDOWN:
        is_edit_rdown_now_ = true;
        break;
    case WM_RBUTTONUP:
        is_edit_rdown_now_ = false;
        break;
    case WM_KILLFOCUS:
    {
        // 关框
        HideDropdown();

        if (listener_)
        {
            listener_->OnSearchEditBlur(this, blur_reason_);
        }
    }
        break;
    case WM_SETFOCUS:
    {
        blur_reason_ = BiliveSearchListBlurChangeReason::CHANGED_BY_DEFAULT;
        if (listener_)
        {
            listener_->OnSearchEditFocus(this);
        }
    }
        break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_UP:
        case VK_DOWN:
        {
            if (dropdown_view_ && dropdown_view_->GetWidget()->IsVisible())
            {
                dropdown_view_->SelectByUpOrDown(wParam == VK_DOWN);
            }
        }
            break;
        case VK_RETURN:
        {
            bool clear_focus = true;
            if (dropdown_view_ && dropdown_view_->GetWidget()->IsVisible())
            {
                clear_focus = !dropdown_view_->SelectByReturn();
            }

            if (clear_focus)
            {
                blur_reason_ = BiliveSearchListBlurChangeReason::CHANGED_BY_USER_RETURN;
                GetFocusManager()->ClearFocus();
            }
        }
            break;
        case VK_ESCAPE:
            if (dropdown_view_ && dropdown_view_->GetWidget()->IsVisible())
            {
                // 关框
                HideDropdown();
            }
            else
            {
                blur_reason_ = BiliveSearchListBlurChangeReason::CHANGED_BY_USER_ESC;
                GetFocusManager()->ClearFocus();
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

void BilibiliSearchListView::DoDropDown()
{
    if (dropdown_view_)
    {
        // 显示弹框
        if (items_.size())
        {
            if (!dropdown_view_->GetWidget()->IsVisible())
            {
                if (listener_)
                {
                    listener_->OnSearchListDropDown(this);
                }
            }

            // 更新显示项
            dropdown_view_->UpdateDate();
            // 重置窗口位置
            ((BilibiliSearchListDropdownWidget*)dropdown_view_->GetWidget())->UpdateOrigin();
            dropdown_view_->GetWidget()->ShowInactive();

            if (!detect_timer_.IsRunning())
            {
                detect_timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(40),
                    base::Bind(&BilibiliSearchListView::OnDetectMousePressed, weakptr_factory_.GetWeakPtr()));
            }
        }
        else
        {
            HideDropdown();
        }
    }
    else
    {
        DCHECK(false);
    }
}

void BilibiliSearchListView::RequestFocus()
{
    if (search_edit_)
    {
        search_edit_->RequestFocus();
    }
}

void BilibiliSearchListView::HideDropdown()
{
    detect_timer_.Stop();
    if (dropdown_view_)
    {
        if (dropdown_view_->GetWidget()->IsVisible())
        {
            dropdown_view_->GetWidget()->Hide();
        }
    }
}

void BilibiliSearchListView::OnDropdownHide()
{
    detect_timer_.Stop();
    if (listener_)
    {
        listener_->OnSearchListDropDownClosed(this);
    }
}

void BilibiliSearchListView::OnDropdownClose()
{
    detect_timer_.Stop();
    dropdown_view_ = nullptr;
}

void BilibiliSearchListView::OnSearchResultItemSelected(const _SEARCH_ITEM &item)
{
    base::string16 text = item.text;
    int64 data = item.data;
    if (search_edit_)
    {
        search_edit_->SetText(text);
    }
    if (listener_)
    {
        listener_->OnSearchListResultSelected(this, text, data);
    }
}

void BilibiliSearchListView::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{
    OnBoundsChanged(new_bounds);
}

// 编辑框相关
void BilibiliSearchListView::SetText(const base::string16& text, BilibiliNativeEditView::EnChangeReason reason/* =
    BilibiliNativeEditView::EnChangeReason::ECR_API*/)
{
    search_edit_->SetText(text, reason);
}

base::string16 BilibiliSearchListView::GetText() const
{
    return search_edit_->GetText();
}

void BilibiliSearchListView::SetTextColor(SkColor color)
{
    search_edit_->SetTextColor(color);
}

void BilibiliSearchListView::SetFont(const gfx::Font& font)
{
    search_edit_->SetFont(font);
}

const gfx::Font& BilibiliSearchListView::font() const
{
    return search_edit_->font();
}

void BilibiliSearchListView::SetLimitText(unsigned int max)
{
    search_edit_->LimitText(max);
}

void BilibiliSearchListView::SetPlaceHolderText(const base::string16& text)
{
    search_edit_->set_placeholder_text(text);
}

const string16& BilibiliSearchListView::placeholder_text() const
{
    return search_edit_->placeholder_text();
}

void BilibiliSearchListView::SetPlaceHolderTextColor(SkColor clr)
{
    search_edit_->set_placeholder_text_color(clr);
}

void BilibiliSearchListView::SetHorizontalMargins(int left, int right)
{
    search_edit_->SetHorizontalMargins(left, right);
}

void BilibiliSearchListView::GetHorizontalMargins(int &left, int &right) const
{
    search_edit_->GetHorizontalMargins(left, right);
}

void BilibiliSearchListView::SelectAll()
{
    search_edit_->SelectAll();
}

void BilibiliSearchListView::SetNumberOnly(bool number_only)
{
    search_edit_->SetNumberOnly(number_only);
}

void BilibiliSearchListView::SetBackgroundColor(SkColor color)
{
    //droplist_bk_color_ = color;
    search_edit_->SetBackgroundColor(color);
}

void BilibiliSearchListView::SetBorderColor(SkColor clrNormal, SkColor clrFocus, SkColor clrDroplist)
{
    droplist_border_color_ = clrDroplist;
    search_edit_->SetBorderColor(clrNormal, clrFocus);
}

// combobox数据项相关
int BilibiliSearchListView::AddItem(const base::string16 &str, int64 data/* = 0*/)
{
    items_.push_back(new _SEARCH_ITEM(str, data));
    return items_.size() - 1;
}

int BilibiliSearchListView::FindItem(const base::string16 &str, int start/* = 0*/)
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

int BilibiliSearchListView::FindItemData(int64 data, int start)
{
    for (size_t i = start; i < items_.size(); i++)
    {
        if (items_[i]->data == data)
        {
            return i;
        }
    }
    return -1;
}

int BilibiliSearchListView::DeleteItem(int index)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_.erase(items_.begin() + index);
    }
    return GetItemCount();
}

int BilibiliSearchListView::GetItemCount()
{
    return items_.size();
}

bool BilibiliSearchListView::SetItemData(int index, int64 data)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_[index]->data = data;
        return true;
    }
    return false;
}

int64 BilibiliSearchListView::GetItemData(int index)
{
    if (index >= 0 && index < (int)items_.size())
    {
        return items_[index]->data;
    }
    return 0;
}

bool BilibiliSearchListView::SetItemText(int index, const base::string16& str)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_[index]->text = str;
        return true;
    }
    return false;
}

bool BilibiliSearchListView::EnabledItem(int index, bool enable)
{
    if (index >= 0 && index < (int)items_.size())
    {
        items_[index]->valid = enable;
        return true;
    }
    return false;
}

void BilibiliSearchListView::ClearItems(bool close_drop/* = true*/)
{
    // 关闭下拉框
    if (close_drop)
    {
        HideDropdown();
    }

    items_.clear();
}

base::string16 BilibiliSearchListView::GetItemText(int index)
{
    if (index >= 0 && index < (int)items_.size())
    {
        return items_[index]->text;
    }
    return L"";
}

void BilibiliSearchListView::SetItemColor(SkColor clrWordsNormal, SkColor clrBkNormal, SkColor clrWordsHoverd, SkColor clrBkHoverd)
{
    item_words_normal_color_ = clrWordsNormal;
    item_bk_normal_color_ = clrBkNormal;
    item_words_hover_color_ = clrWordsHoverd;
    item_bk_hoverd_color_ = clrBkHoverd;
}

SkColor BilibiliSearchListView::item_words_normal_color() const
{
    return item_words_normal_color_;
}

SkColor BilibiliSearchListView::item_words_hover_color() const
{
    return item_words_hover_color_;
}

void BilibiliSearchListView::SetDroplistEqualWidth(bool equal) {
    equal_width_ = equal;
}

void BilibiliSearchListView::OnDetectMousePressed()
{
    if (dropdown_view_)
    {
        bool ld = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        bool rd = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        if (ld || rd)
        {
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            gfx::Point point(pt);
            if (!GetBoundsInScreen().Contains(point) &&
                !dropdown_view_->GetBoundsInScreen().Contains(point) &&
                !is_edit_ldown_now_ &&
                !is_edit_rdown_now_)
            {
                if (!is_last_lbtn_valid_)
                {
                    if (listener_)
                    {
                        listener_->PreSearchEditBlur(this, point);
                    }
                    GetFocusManager()->ClearFocus();
                }
            }
            else
            {
                is_last_lbtn_valid_ = true;
            }
        }
        if (!ld)
        {
            is_last_lbtn_valid_ = false;
        }
        if (is_edit_rdown_now_)
        {
            if (!rd)
            {
                is_edit_rdown_now_ = false;
            }
        }
    }
    else
    {
        detect_timer_.Stop();
    }
}
