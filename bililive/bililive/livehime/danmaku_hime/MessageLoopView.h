#pragma once

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "ui/base/animation/linear_animation.h"
#include "ui/gfx/canvas.h"

#include <array>
#include <vector>
#include <string>


class MessageLoopView :
    public BililiveWidgetDelegate,
    public ui::AnimationDelegate
{
public:
    MessageLoopView();
    ~MessageLoopView();

    static void ShowWindow(int x, int y, gfx::NativeView parent_view);
    static void HideWindow();
    static bool IsWindowHide();
	static void MoveWindow(int x, int y);
    static void SetParent(gfx::NativeView parent_view);
    static void CloseWindow();
    static MessageLoopView* GetView();

    void OnPaint(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;

    void SetItem(int index, const std::wstring& name, const std::wstring text);
    void RemoveItem(int index);
    void RollItem(int index, const std::wstring& name, const std::wstring& text);

    void SetBasePosition(const gfx::Point& pt);

protected:
    void AnimationEnded(const ui::Animation* animation) override;
    void AnimationProgressed(const ui::Animation* animation) override;
    void AnimationCanceled(const ui::Animation* animation) override;

    void OnWidgetDestroyed(views::Widget* widget) override;

private:
    struct Item
    {
        bool            is_using = false;
        std::wstring    name, text;
        std::wstring    roll_name, roll_text;
        SkColor         color[2];
        bool            roll_horz = false;
        float           process;
    };

    static views::Widget*       MESSAGE_LOOP_WIDGET;
    static MessageLoopView*     MESSAGE_LOOP_VIEW;

    gfx::ImageSkia              placeholder_image_;
    gfx::ImageSkia              background_image_;
    ui::LinearAnimation         linear_animation_[2];

    const gfx::Size             one_item_pref_size_ = GetSizeByDPIScale({ 278, 90 });
    const gfx::Size             two_item_pref_size_ = GetSizeByDPIScale({ 278, 148 });
    const int                   item_height_ = GetLengthByDPIScale(54);

    std::array<Item, 2>         items_;

    gfx::Point                  base_pt_;
    int GetItemCount();
    bool SplitString(const std::wstring& text, const gfx::Font& font, int length, std::wstring& text1, std::wstring& text2);
    void DrawItem(gfx::Canvas* canvas, const Item& item, const gfx::Rect& rect);
    void DrawItemText(gfx::Canvas* canvas, const std::wstring& name, const std::wstring& text, const gfx::Rect& rect, int offset_x, int offset_y);

    void AdjustItemPos();

    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override
    {
        return nullptr;
    }
};

