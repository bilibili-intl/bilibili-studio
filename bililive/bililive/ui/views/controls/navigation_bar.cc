#include "navigation_bar.h"

#include "base/strings/utf_string_conversions.h"

#include "ui/base/win/dpi.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/box_layout.h"

//#include "bililive/bililive/livehime/vtuber/vtuber_pk_ui_presenter.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/controls/average_layout.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"

#include "bililive/public/bililive/bililive_process.h"

namespace
{
    const int group_thickness = 40 * ui::GetDPIScale();
    const SkColor clrGroupText = SkColorSetRGB(153, 162, 170);

    class NavigationGroupView
        : public BililiveRecursivePreferredSizeChangedContainerView
    {
    public:
        NavigationGroupView(const base::string16 &title, NavigationBarTabStripPos pos,
            int inside_border_horizontal_spacing,
            int inside_border_vertical_spacing,
            int between_child_spacing,
            bool spread)
            : tabstrip_pos_(pos)
            , title_(title)
        {
            static volatile long inc_group_id = -1;
            group_id_ = ::InterlockedIncrement(&inc_group_id);

            views::FillLayout *fill_layout = new views::FillLayout();
            SetLayoutManager(fill_layout);

            main_view_ = new BililiveRecursivePreferredSizeChangedContainerView();
            {
                views::BoxLayout *layout = new views::BoxLayout(tabstrip_pos_ == NAVIGATIONBAR_TABSTRIP_LEFT ?
                    views::BoxLayout::kVertical : views::BoxLayout::kHorizontal,
                    inside_border_horizontal_spacing, inside_border_vertical_spacing, between_child_spacing);
                layout->set_spread_blank_space(spread);
                main_view_->SetLayoutManager(layout);

                if (!title_.empty())
                {
                    views::View *label_view = new views::View();
                    {
                        views::BoxLayout *box_layout = new views::BoxLayout(views::BoxLayout::kHorizontal, 15, 8, 0);
                        label_view->SetLayoutManager(box_layout);

                        BililiveLabel *label = new BililiveLabel(title_);
                        label->SetTextColor(clrGroupText);

                        label_view->AddChildView(label);
                    }
                    main_view_->AddChildView(label_view);
                }

                tabstrip_container_ = new BililiveRecursivePreferredSizeChangedContainerView();
                {
                    /*views::BoxLayout *box_layout = new views::BoxLayout(tabstrip_pos_ == NAVIGATIONBAR_TABSTRIP_LEFT ?
                        views::BoxLayout::kVertical : views::BoxLayout::kHorizontal, 0, 0, 0);
                    box_layout->set_spread_blank_space(spread);*/
                    auto avg_layout = new bililive::AverageLayout(
                        tabstrip_pos_ == NAVIGATIONBAR_TABSTRIP_LEFT ?
                        bililive::AverageLayout::VERTICAL : bililive::AverageLayout::HORIZONTAL, spread);
                    tabstrip_container_->SetLayoutManager(avg_layout);
                }
                main_view_->AddChildView(tabstrip_container_);
            }
            AddChildView(main_view_);
        };

        virtual ~NavigationGroupView() = default;

        int group_id() const { return group_id_; }
        base::string16 group_title() const { return title_; }

        int AddTab(NavigationStripView *tab_trip)
        {
            tabstrip_container_->AddChildView(tab_trip);
            tabstrip_container_->InvalidateLayout();
            InvalidateLayout();
            return tab_trip->strip_id();
        };

        NavigationStripView * GetSelectedTab()
        {
            for (int i = 0; i < tabstrip_container_->child_count(); i++)
            {
                NavigationStripView *strip_view = static_cast<NavigationStripView*>(tabstrip_container_->child_at(i));
                if (strip_view)
                {
                    if (strip_view->selected())
                    {
                        return strip_view;
                    }
                }
            }
            return nullptr;
        };

        NavigationStripView * GetTab(int strip_id)
        {
            for (int i = 0; i < tabstrip_container_->child_count(); i++)
            {
                NavigationStripView *strip_view = static_cast<NavigationStripView*>(tabstrip_container_->child_at(i));
                if (strip_view)
                {
                    if (strip_view->strip_id() == strip_id)
                    {
                        return strip_view;
                    }
                }
            }
            return nullptr;
        };

    protected:
        // View
        /*virtual void OnPaintBackground(gfx::Canvas* canvas) OVERRIDE
        {
            if (tabstrip_pos_ == NAVIGATIONBAR_TABSTRIP_TOP)
            {
                canvas->DrawLine(gfx::Point(0, height()), gfx::Point(main_view_->bounds().right(), height()),
                    bililive_uploader::clrCtrlGrayBorder);
            }
        };*/

    private:
        int group_id_;
        NavigationBarTabStripPos tabstrip_pos_;
        base::string16 title_;
        views::View *main_view_;
        views::View *tabstrip_container_;

        DISALLOW_COPY_AND_ASSIGN(NavigationGroupView);
    };

    class NavigationGroupContainer
        //: public views::View
        : public BililiveViewWithFloatingScrollbar
    {
    public:
        NavigationGroupContainer(NavigationBarTabStripPos pos, bool spread)
            : tabstrip_pos_(pos)
            , spread_blank_space_(spread)
        {
            views::BoxLayout *layout = new views::BoxLayout(tabstrip_pos_ == NAVIGATIONBAR_TABSTRIP_LEFT ?
                views::BoxLayout::kVertical : views::BoxLayout::kHorizontal, 0, 0, group_thickness);
            layout->set_spread_blank_space(spread_blank_space_);
            SetLayoutManager(layout);
        };

        NavigationGroupContainer(NavigationBarTabStripPos pos,
            int inside_border_horizontal_spacing,
            int inside_border_vertical_spacing,
            int between_child_spacing,
            bool spread)
            : NavigationGroupContainer(pos, spread)
        {
            inside_border_horizontal_spacing_ = inside_border_horizontal_spacing;
            inside_border_vertical_spacing_ = inside_border_vertical_spacing;
            between_child_spacing_ = between_child_spacing;
        };

        virtual ~NavigationGroupContainer() = default;

        int AddGroup(const base::string16 &title)
        {
            for (int i = 0; i < child_count(); i++)
            {
                NavigationGroupView *group_view = static_cast<NavigationGroupView*>(child_at(i));
                if (group_view)
                {
                    if (group_view->group_title() == title)
                    {
                        return group_view->group_id();
                    }
                }
            }

            NavigationGroupView *group_view = new NavigationGroupView(title, tabstrip_pos_,
                inside_border_horizontal_spacing_, inside_border_vertical_spacing_, between_child_spacing_, spread_blank_space_);
            AddChildView(group_view);
            return group_view->group_id();
        };

        int AddTab(int group_id, NavigationStripView *tab_trip)
        {
            NavigationGroupView *group_view = GetGroupView(group_id);
            if (group_view)
            {
                InvalidateLayout();
                return group_view->AddTab(tab_trip);
            }
            return -1;
        };

        NavigationGroupView *GetGroupView(int group_id)
        {
            for (int i = 0; i < child_count(); i++)
            {
                NavigationGroupView *group_view = static_cast<NavigationGroupView*>(child_at(i));
                if (group_view)
                {
                    if (group_view->group_id() == group_id)
                    {
                        return group_view;
                    }
                }
            }
            return nullptr;
        };

        NavigationGroupView *GetGroupView(const base::string16 &title)
        {
            for (int i = 0; i < child_count(); i++)
            {
                NavigationGroupView *group_view = static_cast<NavigationGroupView*>(child_at(i));
                if (group_view)
                {
                    if (group_view->group_title() == title)
                    {
                        return group_view;
                    }
                }
            }
            return nullptr;
        };

    protected:
        // View
        void ChildPreferredSizeChanged(views::View* child) override
        {
            InvalidateLayout();
            Layout();
        }

    private:
        NavigationBarTabStripPos tabstrip_pos_;
        bool spread_blank_space_;
        int inside_border_horizontal_spacing_ = 0;
        int inside_border_vertical_spacing_ = 0;
        int between_child_spacing_ = 0;

        DISALLOW_COPY_AND_ASSIGN(NavigationGroupContainer);
    };

    class NavigationContentContainer
        : public views::View
    {
    public:
        NavigationContentContainer(const gfx::Insets& content_insets)
            : NavigationContentContainer()
        {
            content_insets_ = content_insets;
        }

        NavigationContentContainer()
        {
        }

        virtual ~NavigationContentContainer() = default;

        virtual gfx::Size GetPreferredSize() OVERRIDE
        {
            gfx::Size size;
            for (int i = 0; i < child_count(); i++)
            {
                size.SetToMax(child_at(i)->GetPreferredSize());
            }
            size.Enlarge(content_insets_.width(), content_insets_.height());
            return size;
        };
    protected:
        // View
        virtual void Layout() OVERRIDE
        {
            for (int i = 0; i < child_count(); i++)
            {
                child_at(i)->SetBounds(content_insets_.left(), content_insets_.top(),
                    width() - content_insets_.width(), height() - content_insets_.height());
            }
        };

    private:
        gfx::Insets content_insets_;

        DISALLOW_COPY_AND_ASSIGN(NavigationContentContainer);
    };

}



// NavigationStripView
NavigationStripView::NavigationStripView()
    : content_view_(nullptr)
    , state_(SS_INACTIVE)
    , navigation_bar_(nullptr)
    , left_button_down_(false)
{
    static volatile long inc_tab_id = -1;
    strip_id_ = ::InterlockedIncrement(&inc_tab_id);
}

void NavigationStripView::set_content_view(views::View *content_view)
{
    content_view_ = content_view;
}

void NavigationStripView::SetSelected(bool selected)
{
    if (content_view_)
    {
        content_view_->SetVisible(selected);
        set_state(selected ? SS_SELECTED : SS_INACTIVE);
    }
}

bool NavigationStripView::selected() const
{
    return content_view_->visible();
}

void NavigationStripView::set_state(StripState state)
{
    if (state_ != state)
    {
        state_ = state;
        OnStateChanged();
        SchedulePaint();
    }
}

bool NavigationStripView::OnMousePressed(const ui::MouseEvent &event)
{
    RequestFocus();

    if (!selected())
    {
        left_button_down_ = true;
        set_state(SS_PRESSED);
    }
    return true;
}

void NavigationStripView::OnMouseReleased(const ui::MouseEvent &event)
{
    if (!selected())
    {
        if (left_button_down_)
        {
            left_button_down_ = false;
            if (GetLocalBounds().Contains(event.location()))
            {
                if (navigation_bar_)
                {
                    navigation_bar_->SelectTab(this);
                }
            }
        }
        else
        {
            set_state(SS_HOVER);
        }
    }
}

void NavigationStripView::OnMouseEntered(const ui::MouseEvent &event)
{
    if (!selected())
    {
        set_state(SS_HOVER);
    }
}

void NavigationStripView::OnMouseExited(const ui::MouseEvent &event)
{
    if (!selected())
    {
        set_state(SS_INACTIVE);
    }
}



// NavigationBar
NavigationBar::NavigationBar(NavigationBarListener *listener, NavigationBarTabStripPos pos, bool tabstrip_spread/* = false*/)
    : tabstrip_pos_(pos)
    , listener_(listener)
    , extend_view_(nullptr)
    , tab_area_color_(SK_ColorWHITE)
{
    tab_strip_container_ = new NavigationGroupContainer(tabstrip_pos_, tabstrip_spread);
    tab_contents_container_ = new NavigationContentContainer();
    AddChildView(tab_strip_container_->Container());
    AddChildView(tab_contents_container_);
}

NavigationBar::NavigationBar(NavigationBarListener *listener, NavigationBarTabStripPos pos,
    int tabstrip_inside_border_horizontal_spacing,
    int tabstrip_inside_border_vertical_spacing,
    int tabstrip_between_child_spacing,
    bool tabstrip_spread,
    const gfx::Insets& content_insets)
    : tabstrip_pos_(pos)
    , listener_(listener)
    , extend_view_(nullptr)
    , tab_area_color_(SK_ColorWHITE)
{
    tab_strip_container_ = new NavigationGroupContainer(tabstrip_pos_,
        tabstrip_inside_border_horizontal_spacing,
        tabstrip_inside_border_vertical_spacing,
        tabstrip_between_child_spacing,
        tabstrip_spread);
    tab_contents_container_ = new NavigationContentContainer(content_insets);
    AddChildView(tab_strip_container_->Container());
    AddChildView(tab_contents_container_);
}

NavigationBar::~NavigationBar()
{
}

gfx::Size NavigationBar::GetPreferredSize()
{
    gfx::Size extend_size;
    if (extend_view_)
    {
        extend_size = extend_view_->GetPreferredSize();
    }
    switch (tabstrip_pos_)
    {
    case NAVIGATIONBAR_TABSTRIP_LEFT:
    {
        gfx::Size strip_size = tab_strip_container_->GetPreferredSize();
        gfx::Size contents_size = tab_contents_container_->GetPreferredSize();
        strip_size.set_width(std::max(strip_size.width(), extend_size.width()));
        strip_size.Enlarge(0, extend_size.height());
        strip_size.set_width(strip_size.width() + contents_size.width());
        strip_size.set_height(std::max(strip_size.height(), contents_size.height()));
        return strip_size;
    }
    break;
    case NAVIGATIONBAR_TABSTRIP_TOP:
    {
        gfx::Size strip_size = tab_strip_container_->GetPreferredSize();
        gfx::Size contents_size = tab_contents_container_->GetPreferredSize();
        strip_size.set_height(std::max(strip_size.height(), extend_size.height()));
        strip_size.Enlarge(extend_size.width(), 0);
        strip_size.set_height(strip_size.height() + contents_size.height());
        strip_size.set_width(std::max(strip_size.width(), contents_size.width()));
        return strip_size;
    }
        break;
    default:
        break;
    }
    return gfx::Size();
}

void NavigationBar::Layout()
{
    gfx::Size extend_size;
    if (extend_view_)
    {
        extend_size = extend_view_->GetPreferredSize();
    }
    switch (tabstrip_pos_)
    {
    case NAVIGATIONBAR_TABSTRIP_LEFT:
    {
        gfx::Size size = tab_strip_container_->GetPreferredSize();
        int cx = std::max(extend_size.width(), size.width());
        tab_strip_container_->Container()->SetBounds(0, 0, cx, height() - extend_size.height());
        tab_contents_container_->SetBounds(cx, 0, width() - cx, height());

        if (extend_view_)
        {
            extend_view_->SetBounds(0, height() - extend_size.height(), cx, extend_size.height());
        }
    }
        break;
    case NAVIGATIONBAR_TABSTRIP_TOP:
    {
        gfx::Size size = tab_strip_container_->GetPreferredSize();
        int cy = std::max(extend_size.height(), size.height());
        tab_strip_container_->Container()->SetBounds(0, 0, width() - extend_size.width(), cy);
        tab_contents_container_->SetBounds(0, cy, width(), height() - cy);

        if (extend_view_)
        {
            extend_view_->SetBounds(width() - extend_size.width(), 0, extend_size.width(), cy);
        }
    }
        break;
    default:
        break;
    }
}

int NavigationBar::AddGroup(const base::string16 &title)
{
    return tab_strip_container_->AddGroup(title);
}

void NavigationBar::AddExtendView(views::View *extend_view)
{
    if (extend_view_ != extend_view)
    {
        if (extend_view_)
        {
            RemoveChildView(extend_view_);
            delete extend_view_;
        }

        extend_view_ = extend_view;
        AddChildView(extend_view_);
        InvalidateLayout();
        Layout();
    }
}

int NavigationBar::AddTab(const base::string16 &group_title,
    NavigationStripView *tab_trip, views::View *content_view, int *r_strip_id/* = nullptr*/)
{
    int group_id = AddGroup(group_title);
    return AddTab(group_id, tab_trip, content_view, r_strip_id);
}

int NavigationBar::AddTab(int group_id,
    NavigationStripView *tab_trip, views::View *content_view, int *r_strip_id/* = nullptr*/)
{
    int strip_id = tab_strip_container_->AddTab(group_id, tab_trip);
    tab_contents_container_->AddChildView(content_view);
    tab_trip->set_content_view(content_view);
    tab_trip->set_navigation_bar(this);
    content_view->SetVisible(false);
    if (r_strip_id)
    {
        *r_strip_id = strip_id;
    }
    if (GetTabCount() == 1)
    {
        SelectTab(tab_trip);
    }
    InvalidateLayout();
    Layout();
    return strip_id;
}

NavigationStripView * NavigationBar::GetSelectedTab()
{
    for (int i = 0; i < tab_strip_container_->child_count(); i++)
    {
        NavigationGroupView *group_view = static_cast<NavigationGroupView*>(tab_strip_container_->child_at(i));
        if (group_view)
        {
            NavigationStripView *strip_view = group_view->GetSelectedTab();
            if (strip_view)
            {
                return strip_view;
            }
        }
    }
    return nullptr;
}

views::View * NavigationBar::GetSelectedContent()
{
    NavigationStripView *strip_view = GetSelectedTab();
    if (strip_view)
    {
        return strip_view->content_view();
    }
    return nullptr;
}

int NavigationBar::selected_tab_id()
{
    NavigationStripView *strip_view = GetSelectedTab();
    if (strip_view)
    {
        return strip_view->strip_id();
    }
    return -1;
}

void NavigationBar::SelectTabAt(int strip_id)
{
    for (int i = 0; i < tab_strip_container_->child_count(); i++)
    {
        NavigationGroupView *group_view = static_cast<NavigationGroupView*>(tab_strip_container_->child_at(i));
        if (group_view)
        {
            NavigationStripView *strip_view = group_view->GetTab(strip_id);
            if (strip_view)
            {
                SelectTab(strip_view);
                return;
            }
        }
    }
}

void NavigationBar::SelectTab(NavigationStripView *tab_trip)
{
    if (!tab_trip)
    {
        return;
    }

    for (int i = 0; i < tab_strip_container_->child_count(); i++)
    {
        NavigationGroupView *group_view = static_cast<NavigationGroupView*>(tab_strip_container_->child_at(i));
        if (group_view)
        {
            NavigationStripView *strip_view = group_view->GetSelectedTab();
            if (strip_view)
            {
                if (strip_view == tab_trip)
                {
                    return;
                }
                strip_view->SetSelected(false);
            }
        }
    }
    tab_trip->SetSelected(true);

    if (listener_)
    {
        listener_->NavigationBarSelectedAt(tab_trip->strip_id());
    }
}

void NavigationBar::EnableTab(bool enable)
{
    for (int i = 0; i < tab_strip_container_->child_count(); i++)
    {
        NavigationGroupView* group_view = static_cast<NavigationGroupView*>(tab_strip_container_->child_at(i));
        if (group_view)
        {
            group_view->SetEnabled(enable);
        }
    }
}

gfx::Rect NavigationBar::strip_bounds() const
{
    return tab_strip_container_->Container()->bounds();
}

gfx::Rect NavigationBar::content_bounds() const
{
    return tab_contents_container_->bounds();
}

int NavigationBar::GetTabCount() const
{
    return tab_contents_container_->child_count();
}

void NavigationBar::SetTabAreaColor(SkColor clr)
{
    tab_area_color_ = clr;
    SchedulePaint();
}

void NavigationBar::OnPaintBackground(gfx::Canvas* canvas)
{
    gfx::Rect rect = tab_strip_container_->Container()->bounds();
    canvas->FillRect(rect, tab_area_color_);
}
