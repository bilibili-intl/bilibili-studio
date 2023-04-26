#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_NAVIGATION_BAR_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_NAVIGATION_BAR_H

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"


namespace
{
    class NavigationGroupView;
    class NavigationGroupContainer;
    class NavigationContentContainer;
}

enum NavigationBarTabStripPos
{
    NAVIGATIONBAR_TABSTRIP_LEFT,
    NAVIGATIONBAR_TABSTRIP_TOP,
};

class NavigationBar;

class NavigationStripView
    : public views::View
{
public:
    NavigationStripView();
    virtual ~NavigationStripView() = default;

    int strip_id() const { return strip_id_; }

    void SetSelected(bool selected);
    bool selected() const;
    views::View* content_view(){ return content_view_; }

protected:
    // Overridden from View:
    virtual bool OnMousePressed(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnMouseReleased(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnMouseEntered(const ui::MouseEvent &event) OVERRIDE;
    virtual void OnMouseExited(const ui::MouseEvent &event) OVERRIDE;

    virtual void OnStateChanged(){}

    enum StripState
    {
        SS_INACTIVE,
        SS_HOVER,
        SS_PRESSED,
        SS_SELECTED,
    };
    StripState state() const { return state_; }

private:
    void set_content_view(views::View *content_view);
    void set_state(StripState state);
    void set_navigation_bar(NavigationBar *navigation_bar){ navigation_bar_ = navigation_bar; }

private:
    friend class NavigationBar;
    NavigationBar *navigation_bar_;
    int strip_id_;
    views::View *content_view_;
    StripState state_;
    bool left_button_down_;

    DISALLOW_COPY_AND_ASSIGN(NavigationStripView);
};


class NavigationBarListener
{
public:
    virtual ~NavigationBarListener() = default;

    virtual void NavigationBarSelectedAt(int strip_id) = 0;
};


// ��ǩ�ؼ�
class NavigationBar
    : public views::View
{
public:
    NavigationBar(NavigationBarListener *listener, NavigationBarTabStripPos pos, bool tabstrip_spread = false);
    // ָ����ǩͷ��ͷ�����������
    NavigationBar(NavigationBarListener *listener, NavigationBarTabStripPos pos, 
        int tabstrip_inside_border_horizontal_spacing,
        int tabstrip_inside_border_vertical_spacing,
        int tabstrip_between_child_spacing,
        bool tabstrip_spread,
        const gfx::Insets& content_insets);
    virtual ~NavigationBar();

    // �����
    int AddGroup(const base::string16 &title);
    // ��tabҳ����һ�������view
    void AddExtendView(views::View *extend_view);

    // ��ǩҳ����
    int AddTab(int group_id, NavigationStripView *tab_trip, views::View *content_view, int *r_strip_id = nullptr);
    int AddTab(const base::string16 &group_title, NavigationStripView *tab_trip, views::View *content_view, int *r_strip_id = nullptr);
    void SetTabAreaColor(SkColor clr);
    int GetTabCount() const;
    NavigationStripView* GetSelectedTab();
    views::View *GetSelectedContent();
    int selected_tab_id();
    void SelectTabAt(int strip_id);
    void SelectTab(NavigationStripView *tab_trip);
    void EnableTab(bool enable);

    gfx::Rect strip_bounds() const;
    gfx::Rect content_bounds() const;

    // Overridden from View:
    void Layout() OVERRIDE;
    gfx::Size GetPreferredSize() OVERRIDE;
    void OnPaintBackground(gfx::Canvas* canvas) OVERRIDE;
    const char *GetClassName() const OVERRIDE { return "NavigationBar"; };

protected:
    NavigationBarTabStripPos tabstrip_pos() const { return tabstrip_pos_; }
    views::View *tab_strip_container() { return (views::View*)tab_strip_container_; }
    views::View *tab_contents_container() { return (views::View*)tab_contents_container_; }
    views::View *extend_view() { return extend_view_; }

private:
    NavigationBarListener *listener_;
    NavigationBarTabStripPos tabstrip_pos_;
    NavigationGroupContainer *tab_strip_container_;
    NavigationContentContainer *tab_contents_container_;
    views::View *extend_view_;
    SkColor tab_area_color_;

    DISALLOW_COPY_AND_ASSIGN(NavigationBar);
};


#endif