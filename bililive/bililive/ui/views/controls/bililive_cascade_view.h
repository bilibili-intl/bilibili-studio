#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_CASCADE_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_CASCADE_VIEW_H

#include "ui/views/controls/scroll_view.h"


class BililiveCascadeView : public views::View
{
public:
    explicit BililiveCascadeView(bool cascade_visible = false)
        : cascade_enable_(true)
        , cascade_visible_(cascade_visible){}

    virtual ~BililiveCascadeView() = default;

    void set_cascade_enable(bool cascade_enable){ cascade_enable_ = cascade_enable; }
    bool cascade_enable() const { return cascade_enable_; }

    void set_cascade_visible(bool cascade_visible){ cascade_visible_ = cascade_visible; }
    bool cascade_visible() const { return cascade_visible_; }

    void AddChildView(View* view)
    {
        __super::AddChildView(view);
    };

    void AddChildView(View* view, bool ignore_enable)
    {
        if (ignore_enable)
        {
            ignore_enables_.push_back(view);
        }
        else
        {
            ignore_enables_.remove(view);
        }
        AddChildView(view);
    };

protected:
    // view
    void OnEnabledChanged() override
    {
        if (cascade_enable_)
        {
            for (int i = 0; i < child_count(); i++)
            {
                if (std::find(ignore_enables_.begin(), ignore_enables_.end(), child_at(i)) == ignore_enables_.end())
                {
                    child_at(i)->SetEnabled(enabled());
                }
            }
        }
    };

    void VisibilityChanged(View* starting_from, bool is_visible) override
    {
        if (cascade_visible_)
        {
            for (int i = 0; i < child_count(); i++)
            {
                child_at(i)->SetVisible(visible());
            }
        }
    };

private:
    bool cascade_enable_;
    bool cascade_visible_;
    std::list<views::View*> ignore_enables_;

    DISALLOW_COPY_AND_ASSIGN(BililiveCascadeView);
};


#endif