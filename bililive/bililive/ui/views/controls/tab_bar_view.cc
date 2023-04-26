#include "tab_bar_view.h"

//#include "bililive/bililive/livehime/sources_properties/beauty_camera_controller.h"
//#include "bililive/bililive/livehime/face_effect/face_effect_presenter.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "ui/views/layout/layout_manager.h"

class TabBarViewLayout :
    public views::LayoutManager
{
public:
    void Layout(views::View* host) override
    {
        auto tab_view = static_cast<TabBarView*>(host);

        auto preferred_size = tab_view->GetPreferredSize();

        uint32_t count = tab_view->child_count();
        int x{}, y{};
        for (uint32_t i = 0; i < count; i++)
        {
            views::View* child_view = tab_view->child_at(i);
            auto child_preferred_size = child_view->GetPreferredSize();

            if (tab_view->IsHoriziontal())
            {
                y = (preferred_size.height() - child_preferred_size.height()) / 2.0;
            }
            else
            {
                x = (preferred_size.width() - child_preferred_size.width()) / 2.0;
            }

            child_view->SetBounds(
                tab_view->GetInset().left() + x,
                tab_view->GetInset().top() + y,
                child_preferred_size.width() - tab_view->GetInset().right(),
                child_preferred_size.height() - tab_view->GetInset().bottom()
            );

            if (tab_view->IsHoriziontal())
            {
                x += child_preferred_size.width();
            }
            else
            {
                y += child_preferred_size.height();
            }
        }
    }

    gfx::Size GetPreferredSize(views::View* host) override
    {
        auto tab_view = static_cast<TabBarView*>(host);

        int preferred_width{};
        int preferred_height{};
        uint32_t count = host->child_count();
        for (uint32_t i = 0; i < count; i++)
        {
            views::View* child_view = host->child_at(i);
            auto child_preferred_size = child_view->GetPreferredSize();

            if (tab_view->IsHoriziontal())
            {
                preferred_height = std::max(preferred_height, child_preferred_size.height());
                preferred_width += child_preferred_size.width();
            }
            else
            {
                preferred_height += child_preferred_size.height();
                preferred_width = std::max(preferred_width, child_preferred_size.width());
            }
        }

        return { preferred_width,  preferred_height };
    }
};


TabBarView::TabBarView(bool horiziontal, TabBarViewListener* listener) :
    is_horiziontal_(horiziontal),
    listener_(listener)
{
    SetLayoutManager(new TabBarViewLayout());
}

TabBarView::~TabBarView()
{
}

void TabBarView::EnableNoSelect(bool enable_no_select)
{
    enable_no_select_ = enable_no_select;
}

bool TabBarView::IsHoriziontal()
{
    return is_horiziontal_;
}

void TabBarView::AddTab(TabBarItemView* content_view)
{
    AddChildView(content_view);
    Layout();
}

void TabBarView::Select(int index, bool init)
{
    if (index >= child_count())
    {
        return;
    }

    if (index < -1)
    {
        index = -1;
    }

    //if (listener_ && listener_->TabItemTagId() == kBeautyMaterialSticker && index!=0)
    //{
    //    if (!init) {
    //        if (!livehime::face::FaceEffectPresenter::GetInstance()->FaceEffectIsIdel()) {
    //            HotkeyNotifyView::ShowForm(GetLocalizedString(IDS_CAMERA_FACE_PLAYDING_TIP),
    //                GetBoundsInScreen(), nullptr, &ftPrimary);
    //            return;
    //        }
    //    }
    //}

    int pre_select = select_index_;
    select_index_ = index;

    if (listener_)
    {
        if (!listener_->TabItemSelect(index, this))
        {
            select_index_ = pre_select;
            return;
        }
    }

    if (index >= 0)
    {
        TabBarItemView* select_item = static_cast<TabBarItemView*>(child_at(index));
        if (!select_item->OnSelect())
        {
            select_index_ = pre_select;
            return;
        }
    }

    int count = child_count();
    for (int i = 0; i < count; i++)
    {
        TabBarItemView* item_view = static_cast<TabBarItemView*>(child_at(i));

        if (select_index_ != i)
        {
            item_view->OnSelectChanged(select_index_);
        }
    }
}

int TabBarView::GetSelect()
{
    return select_index_;
}

void TabBarView::SetInset(const gfx::Insets& insets)
{
    insets_ = insets;
}

const gfx::Insets& TabBarView::GetInset()
{
    return insets_;
}

bool TabBarView::OnMousePressed(const ui::MouseEvent& event)
{
    bool ret = __super::OnMousePressed(event);

    int count = child_count();
    for (int i = 0; i < count; i++)
    {
        TabBarItemView*content_view = static_cast<TabBarItemView*>(child_at(i));

        auto pt = event.location();
        views::View::ConvertPointToTarget(this, content_view, &pt);
        if (content_view->HitTestPoint(pt))
        {
            if (GetSelect() != i)
            {
                Select(i);
                SchedulePaint();
                break;
            }
            else if (enable_no_select_)
            {
                Select(-1);
            }
        }
    }

    SchedulePaint();
    return ret;
}

TabBarItemView::TabBarItemView(TabBarView* parent) :
    parent_(parent)
{

}

TabBarView* TabBarItemView::GetParentTabBar()
{
    return parent_;
}

int TabBarItemView::GetItemIndex()
{
    return parent_->GetIndexOf(this);
}

