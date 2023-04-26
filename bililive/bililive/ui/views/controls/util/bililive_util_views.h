#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_UTIL_BILILIVE_UTIL_VIEWS_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_UTIL_BILILIVE_UTIL_VIEWS_H

#include "ui/views/view.h"


// 所有子view堆叠布置在相同位置，哪个要显示自己visibl，并自己把其他子view进行disvisible
class BililiveSingleChildShowContainerView : public views::View
{
public:
    void SetPreferredSize(const gfx::Size& pref_size);

    // View
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;

protected:
    // View
    void Layout() override;
    void ChildPreferredSizeChanged(views::View* child) override;

private:
    gfx::Size pref_size_;
};

//根据子控件是否显示控制控件整体高度
class BililiveSingleChildHightShowContainerView :  public views::View
{
public:
    void SetPreferredSize(const gfx::Size& pref_size);

    gfx::Size GetPreferredSize() override;
protected:
    // View
    void Layout() override;
    void ChildPreferredSizeChanged(views::View* child) override;
private:
    gfx::Size pref_size_;
};

//根据子控件是否显示控制控件宽
class BililiveMutilWidthContainerView : public views::View
{
public:
    BililiveMutilWidthContainerView(int space,int max_hight);

    gfx::Size GetPreferredSize() override;
protected:
    // View
    void Layout() override;
    void ChildPreferredSizeChanged(views::View* child) override;
private:
    gfx::Size pref_size_;
    int space_ = 0;
    int max_hight_ = 0;
};

// 逐级通知子view的PreferSizeChanged
class BililiveRecursivePreferredSizeChangedContainerView : public views::View
{
public:
protected:
    // View
    void ChildPreferredSizeChanged(views::View* child) override;

private:
};


// 自己被隐藏的话就把prefersize置零
class BililiveHideAwareView : public views::View
{
public:
    BililiveHideAwareView() = default;
    BililiveHideAwareView(bool fill_layout, bool auto_hide);
    void SetPlaceholderSize(const gfx::Size& placeholder_size);
    void SetInsets(const gfx::Insets& insets);

    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
    gfx::Insets GetInsets() const override;

protected:
    void ChildVisibilityChanged(View* child) override;
    void VisibilityChanged(View* starting_from, bool is_visible);

private:
    gfx::Size placeholder_size_;
    bool auto_hide_ = false;
    gfx::Insets insets_;
};

#endif