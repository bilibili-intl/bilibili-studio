#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/strings/string16.h"
#include "ui/views/view.h"

namespace
{
	const SkColor kTabColor_Inactive = SkColorSetRGB(221, 221, 221);
	const SkColor kTabColor_Active = SkColorSetRGB(247, 247, 247);
}  // namespace

class TabContentView;
class TabbedPane;
class TabbedPaneListener;
class TabStrip;

enum TabState
{
    TAB_INACTIVE,
    TAB_ACTIVE, // 选中
    TAB_PRESSED,
    TAB_HOVERED,
};

enum TabbedPaneTabStripPos
{
    TABBEDPANE_TABSTRIP_TOP,
    TABBEDPANE_TABSTRIP_LEFT,
};

// 标签页头部
class Tab
    : public views::View
{
public:
    Tab(TabbedPane *tabbed_pane, const string16 &title, TabContentView *tab_contents,
		views::View *contents, SkColor clrActive = kTabColor_Active, SkColor clrInActive = kTabColor_Inactive);
    virtual ~Tab();

    View *contents() const { return contents_; }
    TabContentView *tab_contents() const { return tab_contents_; }

	base::string16& TabHeadText(){ return tabpane_text_; }

    bool selected() const
    {
        return contents_->visible();
    }
    void SetSelected(bool selected);

    // Overridden from View:
    virtual bool OnMousePressed(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnMouseReleased(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnMouseCaptureLost() OVERRIDE;
    virtual void OnMouseEntered(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnMouseExited(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnGestureEvent(ui::GestureEvent *event) OVERRIDE;
    virtual void OnPaintBackground(gfx::Canvas *canvas) OVERRIDE;

protected:
    TabState tab_state_;
    SkColor active_clr_;
    SkColor inactive_clr_;
    views::View *contents_;
    TabContentView *tab_contents_;

private:
    void SetState(TabState tab_state);

    TabbedPane *tabbed_pane_;
	base::string16 tabpane_text_;

    friend class TabContentView;

    DISALLOW_COPY_AND_ASSIGN(Tab);
};

class TabContentView
    : public views::View
{
public:
	TabContentView()
		: tab_(NULL) { }
    ~TabContentView() { }

    TabState tab_state() {
        return tab_->tab_state_;
    }

	Tab* tab() const { return (Tab* const)tab_; } 
    void set_tab(Tab *tab) {
        tab_ = tab;
    }

    virtual base::string16 GetClassName(){ return L"TabContentView"; }

protected:
    // view
    virtual void OnMouseEntered(const ui::MouseEvent &event) OVERRIDE
    {
        ui::MouseEvent tab_event(event.native_event());
        gfx::Point pt = event.location();
        ConvertPointToTarget(this, tab_, &pt);
        tab_event.set_location(pt);
        tab_->OnMouseEntered(tab_event);
    }

    virtual void OnMouseExited(const ui::MouseEvent &event) OVERRIDE
    {
        ui::MouseEvent tab_event(event.native_event());
        gfx::Point pt = event.location();
        ConvertPointToTarget(this, tab_, &pt);
        tab_event.set_location(pt);
        tab_->OnMouseExited(tab_event);
    }

protected:
    virtual void OnTabStateChanged(){}

private:
    friend class Tab;
    Tab *tab_;
};

class TabbedPane
    : public views::View
{
public:
    static const char kViewClassName[];

    explicit TabbedPane(TabbedPaneTabStripPos tabstrip_pos, bool spread_blank_space);
    explicit TabbedPane(TabbedPaneTabStripPos tabstrip_pos, views::View *tab_strip);
    virtual ~TabbedPane();

    TabbedPaneListener *listener() const
    {
        return listener_;
    }
    void set_listener(TabbedPaneListener *listener)
    {
        listener_ = listener;
    }

    int selected_tab_index() const;

    TabbedPaneTabStripPos tabstrip_pos() const { return tabstrip_pos_; }

    int GetTabCount();
    views::View *GetSelectedTab();

    void AddTab(const string16 &title, TabContentView *tab_contents, views::View *contents);
    void AddTabAtIndex(int index, const string16 &title, TabContentView *tab_contents, views::View *contents);

    virtual void SelectTabAt(int index);
    void SelectTab(Tab *tab);

    void SetTabColor(SkColor clrActive, SkColor clrInActive);
    void SetTabEmptyAreaColor(SkColor clrEmpty);
    void SetTabLineColor(SkColor clrLine);

    // Overridden from View:
    virtual void Layout() OVERRIDE;
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual const char *GetClassName() const OVERRIDE;

protected:
    friend class TabStrip;

    Tab *GetTabAt(int index);
    virtual Tab *CreateTab(TabbedPane *tabbed_pane, const string16 &title, TabContentView *tab_contents,
        views::View *contents, SkColor clrActive, SkColor clrInActive);

    // Overridden from View:
    virtual void ViewHierarchyChanged(
        const ViewHierarchyChangedDetails &details) OVERRIDE;
    virtual bool AcceleratorPressed(const ui::Accelerator &accelerator) OVERRIDE;
    virtual void OnFocus() OVERRIDE;
    virtual void GetAccessibleState(ui::AccessibleViewState *state) OVERRIDE;
    virtual void OnPaintBackground(gfx::Canvas* canvas) OVERRIDE;

    TabbedPaneListener *listener_;

    View *tab_strip_;
    View *contents_;
	SkColor active_clr_;
    SkColor inactive_clr_;
    TabbedPaneTabStripPos tabstrip_pos_;

    // 标签区域外的区域颜色，不设置将采用未激活色
    bool is_set_empty_;
    SkColor empty_clr_;
    // 标签区域标签底部或右部的边线
    bool is_set_line_clr_;
    SkColor line_clr_;

private:

    DISALLOW_COPY_AND_ASSIGN(TabbedPane);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H