#pragma once

#include "ui/views/view.h"

class TabBarView;

class TabBarItemView :
    public views::View
{
public:
    TabBarItemView(TabBarView* parent);
    ~TabBarItemView() {}

    virtual bool OnSelect() = 0;
    virtual void OnSelectChanged(int select_index) = 0;

    TabBarView* GetParentTabBar();
    int GetItemIndex();

private:
    TabBarView* parent_ = nullptr;
};

class TabBarViewListener
{
    friend class TabBarView;

public:
    ~TabBarViewListener() {};

protected:
    TabBarViewListener() {};
    virtual bool TabItemSelect(int index, TabBarView* tab_view) = 0;
    virtual int TabItemTagId() { return -1; };
};

class TabBarView :
    public views::View
{
public:
    TabBarView(bool horiziontal, TabBarViewListener* listener);
    ~TabBarView();

    void EnableNoSelect(bool enable_no_select);
    bool IsHoriziontal();
    void AddTab(TabBarItemView* tab_view);
    void Select(int index,bool init = false);
    int GetSelect();
    void SetInset(const gfx::Insets& insets);
    const gfx::Insets& GetInset();

protected:
    bool OnMousePressed(const ui::MouseEvent& event) override;

private:
    bool is_horiziontal_ = false;
    bool enable_no_select_ = false;
    TabBarViewListener* listener_ = nullptr;
    int select_index_ = -1;
    gfx::Insets insets_;
};
