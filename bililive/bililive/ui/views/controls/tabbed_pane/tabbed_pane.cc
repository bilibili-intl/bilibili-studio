#include "tabbed_pane.h"
#include "tabbed_pane_listener.h"

#include "base/logging.h"
#include "ui/base/accessibility/accessible_view_state.h"
#include "ui/base/keycodes/keyboard_codes.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"

// static
const char TabbedPane::kViewClassName[] = "TabbedPane";


// TabStrip，标签页头部容器
class TabStrip
    : public views::View
{
public:
    explicit TabStrip(TabbedPane *tabbed_pane, bool horizontal, bool spread_blank_space)
        : tabbed_pane_(tabbed_pane)
    {
        views::BoxLayout *box_layout = new views::BoxLayout(
            horizontal ? views::BoxLayout::kHorizontal : views::BoxLayout::kVertical, 0, 0, 0);
        SetLayoutManager(box_layout);
        box_layout->set_spread_blank_space(spread_blank_space);
    };
    virtual ~TabStrip() = default;

private:
    TabbedPane *tabbed_pane_;
    bool spread_blank_space_;

    DISALLOW_COPY_AND_ASSIGN(TabStrip);
};



// Tab
Tab::Tab(TabbedPane *tabbed_pane, const string16 &title, TabContentView *tab_contents, views::View *contents,
    SkColor clrActive/* = kTabColor_Active*/, SkColor clrInActive/* = kTabColor_Inactive*/)
    : tabbed_pane_(tabbed_pane),
    tab_state_(TAB_ACTIVE),
    contents_(contents),
    tabpane_text_(title),
    active_clr_(clrActive),
    inactive_clr_(clrInActive),
    tab_contents_(tab_contents)
{
    tab_contents_->set_tab(this);
    SetLayoutManager(new views::FillLayout());
    AddChildView(tab_contents_);
    SetState(TAB_INACTIVE);
}

Tab::~Tab() {}

void Tab::SetSelected(bool selected)
{
    contents_->SetVisible(selected);
    SetState(selected ? TAB_ACTIVE : TAB_INACTIVE);
}

bool Tab::OnMousePressed(const ui::MouseEvent &event)
{
    SetState(TAB_PRESSED);
    return true;
}

void Tab::OnMouseReleased(const ui::MouseEvent &event)
{
    SetState(selected() ? TAB_ACTIVE : TAB_HOVERED);
    if (GetLocalBounds().Contains(event.location()))
    {
        tabbed_pane_->SelectTab(this);
        SetSelected(true);
    }
}

void Tab::OnMouseCaptureLost()
{
    SetState(TAB_INACTIVE);
}

void Tab::OnMouseEntered(const ui::MouseEvent &event)
{
    SetState(selected() ? TAB_ACTIVE : TAB_HOVERED);
}

void Tab::OnMouseExited(const ui::MouseEvent &event)
{
    SetState(selected() ? TAB_ACTIVE : TAB_INACTIVE);
}

void Tab::OnGestureEvent(ui::GestureEvent *event)
{
    switch (event->type())
    {
    case ui::ET_GESTURE_TAP_DOWN:
        SetState(TAB_PRESSED);
        break;
    case ui::ET_GESTURE_TAP:
        // SelectTab also sets the right tab color.
        tabbed_pane_->SelectTab(this);
        break;
    case ui::ET_GESTURE_TAP_CANCEL:
        SetState(selected() ? TAB_ACTIVE : TAB_INACTIVE);
        break;
    default:
        break;
    }
    event->SetHandled();
}

void Tab::OnPaintBackground(gfx::Canvas *canvas)
{
    switch (tab_state_)
    {
    case TAB_ACTIVE:
    case TAB_HOVERED:
        canvas->FillRect(gfx::Rect(0, 0, width(), height()), active_clr_);
        break;
    default:
        canvas->FillRect(gfx::Rect(0, 0, width(), height()), inactive_clr_);
        break;
    }
}

void Tab::SetState(TabState tab_state)
{
    if (tab_state == tab_state_)
    {
        return;
    }
    tab_state_ = tab_state;
    tab_contents_->OnTabStateChanged();

    SchedulePaint();
}



// TabbedPane
TabbedPane::TabbedPane(TabbedPaneTabStripPos tabstrip_pos, bool spread_blank_space)
    : listener_(NULL)
    , contents_(new View())
    , tabstrip_pos_(tabstrip_pos)
    , active_clr_(kTabColor_Active)
    , inactive_clr_(kTabColor_Inactive)
    , is_set_empty_(false)
    , empty_clr_(SK_ColorWHITE)
    , is_set_line_clr_(false)
    , line_clr_(SK_ColorWHITE)
{
    tab_strip_ = new TabStrip(
        this,
        tabstrip_pos_ == TABBEDPANE_TABSTRIP_TOP,
        spread_blank_space);
    set_focusable(true);
    AddChildView(tab_strip_);
    AddChildView(contents_);
}

TabbedPane::TabbedPane(TabbedPaneTabStripPos tabstrip_pos, views::View *tab_strip)
    : listener_(NULL)
    , contents_(new View())
    , tabstrip_pos_(tabstrip_pos)
    , active_clr_(kTabColor_Active)
    , inactive_clr_(kTabColor_Inactive)
    , is_set_empty_(false)
    , empty_clr_(SK_ColorWHITE)
    , is_set_line_clr_(false)
    , line_clr_(SK_ColorWHITE)
{
    tab_strip_ = tab_strip;
    set_focusable(true);
    AddChildView(tab_strip_);
    AddChildView(contents_);
}

TabbedPane::~TabbedPane() {}

int TabbedPane::GetTabCount()
{
    DCHECK_EQ(tab_strip_->child_count(), contents_->child_count());
    return contents_->child_count();
}

views::View *TabbedPane::GetSelectedTab()
{
    return selected_tab_index() < 0 ?
        NULL : GetTabAt(selected_tab_index())->contents();
}

void TabbedPane::SetTabColor(SkColor clrActive, SkColor clrInActive)
{
    active_clr_ = clrActive;
    inactive_clr_ = clrInActive;
}

void TabbedPane::SetTabEmptyAreaColor(SkColor clrEmpty)
{
    is_set_empty_ = true;
    empty_clr_ = clrEmpty;
}

void TabbedPane::SetTabLineColor(SkColor clrLine)
{
    is_set_line_clr_ = true;
    line_clr_ = clrLine;
}

void TabbedPane::AddTab(const string16 &title, TabContentView *tab_contents, views::View *contents)
{
    AddTabAtIndex(tab_strip_->child_count(), title, tab_contents, contents);
}

void TabbedPane::AddTabAtIndex(int index,
    const string16 &title,
    TabContentView *tab_contents,
    views::View *contents)
{
    DCHECK(index >= 0 && index <= GetTabCount());
    contents->SetVisible(false);

    tab_strip_->AddChildViewAt(CreateTab(this, title, tab_contents, contents, active_clr_, inactive_clr_), index);
    contents_->AddChildViewAt(contents, index);
    if (selected_tab_index() < 0) {
        SelectTabAt(index);
    }

    PreferredSizeChanged();
}

Tab *TabbedPane::CreateTab(TabbedPane *tabbed_pane, const string16 &title, TabContentView *tab_contents,
    views::View *contents, SkColor clrActive, SkColor clrInActive)
{
    Tab *tab = new Tab(this, title, tab_contents, contents, active_clr_, inactive_clr_);
    return tab;
}

int TabbedPane::selected_tab_index() const
{
    Tab *tab = NULL;
    TabContentView *tab_content = NULL;
    for (int i = 0; i < tab_strip_->child_count(); i++)
    {
        tab = (Tab*)tab_strip_->child_at(i);
        tab_content = tab->tab_contents();
        if (tab_content->tab_state() == TAB_ACTIVE)
        {
            return i;
        }
    }
    return -1;
}

void TabbedPane::SelectTabAt(int index)
{
    DCHECK(index >= 0 && index < GetTabCount());
    if (index == selected_tab_index())
    {
        return;
    }

    if (selected_tab_index() >= 0)
    {
        GetTabAt(selected_tab_index())->SetSelected(false);
    }

    //selected_tab_index_ = index;
    Tab *tab = GetTabAt(index);
    tab->SetSelected(true);
    tab_strip_->SchedulePaint();

    views::FocusManager *focus_manager = tab->contents()->GetFocusManager();
    if (focus_manager)
    {
        const View *focused_view = focus_manager->GetFocusedView();
        if (focused_view && contents_->Contains(focused_view) &&
            !tab->contents()->Contains(focused_view))
        {
            focus_manager->SetFocusedView(tab->contents());
        }
    }

    if (listener())
    {
        listener()->TabSelectedAt(index);
    }
}

void TabbedPane::SelectTab(Tab *tab)
{
    const int index = tab_strip_->GetIndexOf(tab);
    if (index >= 0)
    {
        SelectTabAt(index);
    }
}

gfx::Size TabbedPane::GetPreferredSize()
{
    gfx::Size size;
    for (int i = 0; i < contents_->child_count(); ++i)
    {
        size.SetToMax(contents_->child_at(i)->GetPreferredSize());
    }
    if (tabstrip_pos_ == TABBEDPANE_TABSTRIP_TOP)
    {
        size.Enlarge(0, tab_strip_->GetPreferredSize().height());
        size.set_width(std::max(size.width(), tab_strip_->GetPreferredSize().width()));
    }
    else
    {
        size.Enlarge(tab_strip_->GetPreferredSize().width(), 0);
        size.set_height(std::max(size.height(), tab_strip_->GetPreferredSize().height()));
    }
    return size;
}

Tab *TabbedPane::GetTabAt(int index)
{
    return static_cast<Tab *>(tab_strip_->child_at(index));
}

void TabbedPane::Layout()
{
    if (!IsDrawn())
    {
        return;
    }
    switch (tabstrip_pos_)
    {
    case TABBEDPANE_TABSTRIP_TOP:
    {
        const gfx::Size size = tab_strip_->GetPreferredSize();
        tab_strip_->SetBounds(0, 0, width(), size.height());
        contents_->SetBounds(0, tab_strip_->bounds().bottom(), width(),
            std::max(0, height() - size.height()));
        for (int i = 0; i < contents_->child_count(); ++i)
        {
            contents_->child_at(i)->SetSize(contents_->size());
        }
    }
        break;
    case TABBEDPANE_TABSTRIP_LEFT:
    {
        const gfx::Size size = tab_strip_->GetPreferredSize();
        tab_strip_->SetBounds(0, 0, size.width(), height());
        contents_->SetBounds(size.width(), 0, std::max(0, width() - size.width()), height());
        for (int i = 0; i < contents_->child_count(); ++i)
        {
            contents_->child_at(i)->SetSize(contents_->size());
        }
    }
        break;
    default:
        break;
    }
}

void TabbedPane::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails &details)
{
    if (details.is_add)
    {
        // Support navigating tabs by Ctrl+Tab and Ctrl+Shift+Tab.
        AddAccelerator(ui::Accelerator(ui::VKEY_TAB,
            ui::EF_CONTROL_DOWN | ui::EF_SHIFT_DOWN));
        AddAccelerator(ui::Accelerator(ui::VKEY_TAB, ui::EF_CONTROL_DOWN));
    }
}

bool TabbedPane::AcceleratorPressed(const ui::Accelerator &accelerator)
{
    // Handle Ctrl+Tab and Ctrl+Shift+Tab navigation of pages.
    DCHECK(accelerator.key_code() == ui::VKEY_TAB && accelerator.IsCtrlDown());
    const int tab_count = GetTabCount();
    if (tab_count <= 1)
    {
        return false;
    }
    const int increment = accelerator.IsShiftDown() ? -1 : 1;
    int next_tab_index = (selected_tab_index() + increment) % tab_count;
    // Wrap around.
    if (next_tab_index < 0)
    {
        next_tab_index += tab_count;
    }
    SelectTabAt(next_tab_index);
    return true;
}

const char *TabbedPane::GetClassName() const
{
    return kViewClassName;
}

void TabbedPane::OnFocus()
{
    View::OnFocus();

    View *selected_tab = GetSelectedTab();
    if (selected_tab)
    {
        selected_tab->NotifyAccessibilityEvent(
            ui::AccessibilityTypes::EVENT_FOCUS, true);
    }
}

void TabbedPane::GetAccessibleState(ui::AccessibleViewState *state)
{
    state->role = ui::AccessibilityTypes::ROLE_PAGETABLIST;
}

void TabbedPane::OnPaintBackground(gfx::Canvas* canvas)
{
    gfx::Rect rect = tab_strip_->bounds();
    gfx::Size size = tab_strip_->GetPreferredSize();
    SkColor clrBk;
    if (is_set_empty_)
    {
        clrBk = empty_clr_;
    }
    else
    {
        clrBk = inactive_clr_;
    }
    if (tabstrip_pos_ == TABBEDPANE_TABSTRIP_TOP)
    {
        canvas->FillRect(gfx::Rect(0, 0, rect.x(), rect.height()), clrBk);
        canvas->FillRect(gfx::Rect(rect.x() + size.width(), 0, width() - (rect.x() + size.width()), rect.height()), clrBk);
    }
    else
    {
        canvas->FillRect(gfx::Rect(0, 0, rect.width(), rect.y()), clrBk);
        canvas->FillRect(gfx::Rect(0, rect.y() + size.height(), rect.width(), height() - (rect.y() + size.height())), clrBk);
    }

    if (is_set_line_clr_)
    {
        if (tabstrip_pos_ == TABBEDPANE_TABSTRIP_TOP)
        {
            canvas->DrawLine(gfx::Point(0, rect.bottom() - 1), gfx::Point(width(), rect.bottom() - 1), line_clr_);
        }
        else
        {
            canvas->DrawLine(gfx::Point(rect.width() - 1, 0), gfx::Point(rect.width() - 1, height()), line_clr_);
        }
    }
}
