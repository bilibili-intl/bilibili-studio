#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FLOATING_SCROLL_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FLOATING_SCROLL_VIEW_H

#include "ui/views/view.h"


enum ThumbState
{
    ThumbState_OUTOFFCLT,
    ThumbState_NORMAL,
    ThumbState_HOVERED,
    ThumbState_PRESSED,
    ThumbState_COUNT,
};

enum ScrollReason
{
    NoMove,
    Api,
    User,
    Wheel,
    Drag,
    Key,
    ButtonPress,
    ThumbPress,
};

class ScrollViewWithFloatingScrollbar;
class FloatingScrollBar;

class ScrollPositionChangeListener {
public:
    virtual ~ScrollPositionChangeListener() = default;

    virtual void OnScrollPositionChanged(bool vert, int position, ScrollReason reason) = 0;
};

// 按钮和滑块基类
class FloatingScrollBarBaseCtrl
{
public:
    // View
    void SetBounds(int x, int y, int width, int height);
    void SetBoundsRect(const gfx::Rect& bounds);
    const gfx::Rect& bounds() const { return bounds_; }
    int width() const { return bounds_.width(); }
    int height() const { return bounds_.height(); }
    int x() const { return bounds_.x(); }
    int y() const { return bounds_.y(); }
    const gfx::Size& size() const { return bounds_.size(); }
    ThumbState state() const{ return state_; }
    void SetState(ThumbState states){ state_ = states; }

protected:
    enum CtrlType
    {
        CT_BUTTON_PREV,
        CT_THUMB,
        CT_BUTTON_NEXT,
    };

    explicit FloatingScrollBarBaseCtrl(FloatingScrollBar *scroll_bar, CtrlType ct);
    virtual ~FloatingScrollBarBaseCtrl(){}

    // View
    virtual void OnPaint(gfx::Canvas* canvas) = 0;
    virtual bool OnMousePressed(const ui::MouseEvent& event) = 0;
    virtual bool OnMouseMoved(const ui::MouseEvent& event);

    void SetScrollView(ScrollViewWithFloatingScrollbar *scroll_view){ scroll_view_ = scroll_view; }
    const gfx::Rect& visible_bounds() const { return visible_bounds_; }

protected:
    CtrlType ctrl_type_;
    ThumbState state_;
    FloatingScrollBar *scroll_bar_;
    ScrollViewWithFloatingScrollbar *scroll_view_;

private:
    friend class FloatingScrollBar;
    gfx::Rect bounds_; // 基于ScrollViewWithFloatingScrollbar的坐标
    gfx::Rect visible_bounds_; // 基于ScrollViewWithFloatingScrollbar的坐标

    DISALLOW_COPY_AND_ASSIGN(FloatingScrollBarBaseCtrl);
};

// 滑块两端按钮
class FloatingScrollBarButton : public FloatingScrollBarBaseCtrl
{
public:
    void SetColor(ThumbState state, SkColor color);

protected:
    explicit FloatingScrollBarButton(FloatingScrollBar *scroll_bar, bool prev, bool is_bar_narrow = false);

    // View
    void OnPaint(gfx::Canvas* canvas) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;

private:
    friend class FloatingScrollBar;
    bool is_prev_;
    SkColor thumb_colors_[ThumbState_COUNT];
    bool is_bar_narrow_ = false;
    DISALLOW_COPY_AND_ASSIGN(FloatingScrollBarButton);
};

// 滑块区域
class FloatingScrollBarThumb : public FloatingScrollBarBaseCtrl
{
public:
    void SetColor(ThumbState state, SkColor color);
    void SetThumbBoundsRect(const gfx::Rect& thumb_bound);
    const gfx::Rect& thumb_bounds() const { return thumb_bounds_; }

protected:
    explicit FloatingScrollBarThumb(FloatingScrollBar *scroll_bar);

    // View
    void OnPaint(gfx::Canvas* canvas) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;

private:
    friend class FloatingScrollBar;
    gfx::Rect thumb_bounds_; // 基于ScrollViewWithFloatingScrollbar的坐标
    SkColor thumb_colors_[ThumbState_COUNT];

    DISALLOW_COPY_AND_ASSIGN(FloatingScrollBarThumb);
};

// 整个滚动条区域
class FloatingScrollBar
{
public:
    void SetColor(ThumbState state, SkColor color);
    bool is_horiz() const { return is_horiz_; }

    // View
    void SetBounds(int x, int y, int width, int height);
    void SetBoundsRect(const gfx::Rect& bounds);
    // No transformation is applied on the size or the locations.
    const gfx::Rect& bounds() const { return bounds_; }
    int width() const { return bounds_.width(); }
    int height() const { return bounds_.height(); }
    int x() const { return bounds_.x(); }
    int y() const { return bounds_.y(); }
    const gfx::Size& size() const { return bounds_.size(); }

    // Set whether this view is visible. Painting is scheduled as needed.
    virtual void SetVisible(bool visible);

    // Return whether a view is visible
    bool visible() const { return visible_; }

    // Returns the max and min positions.
    int GetMaxPosition() const;
    int GetMinPosition() const;

    // Returns the position of the scrollbar.
    virtual int GetPosition() const;

    /**
     * 返回滚动条可操作区的宽度。
     * 对于垂直滚动条，返回的值用于设置滚动条可操作区的宽度；
     * 对于水平滚动条，返回的值用于设置滚动条两端按钮的大小。
     */
    virtual int GetScrollBarWidth() const;

    /**
     * 返回滚动条可操作区高度。
     * 对于垂直滚动条，返回的值用于设置滚动条两端按钮的大小。
     * 对于水平滚动条，返回的值用于设置滚动条可操作区的高度。
     */
    virtual int GetScrollBarHeight() const;

    /**
     * 返回滚动条整体区域的宽度。
     * 对于垂直滚动条，返回的值用于设置滚动条整体区域和两端按钮的宽度。
     * 对于水平滚动条，返回的值用于设置滚动条两端按钮的宽度。
     */
    virtual int GetScrollBarVisibleWidth() const;

    /**
     * 返回滚动条整体区域的宽度。
     * 对于垂直滚动条，返回的值用于设置滚动条两端按钮的高度。
     * 对于水平滚动条，返回的值用于设置滚动条整体区域和两端按钮的高度。
     */
    virtual int GetScrollBarVisibleHeight() const;

    /**
     * 返回添加在滚动条上、下和右边的边距。
     */
    virtual int GetScrollBarBorderThickness() const;

protected:
    explicit FloatingScrollBar(bool is_horiz, bool is_bar_narrow = false);
    virtual ~FloatingScrollBar();

    // View
    virtual void OnPaint(gfx::Canvas* canvas);
    virtual bool OnMousePressed(const ui::MouseEvent& event);
    virtual void OnMouseMoved(const ui::MouseEvent& event);
    virtual bool OnMouseDragged(const ui::MouseEvent& event);

    void GetOperateRect(const gfx::Rect &bound, gfx::Rect *prev, gfx::Rect *middle, gfx::Rect *next) const;
    ThumbState thumb_state() const;
    int thumb_range() const;

    // ScrollBar
    // Update the scrollbar appearance given a viewport size, content size and
    // current position
    virtual void Update(int viewport_size, int content_size, int current_pos);

    void SetScrollView(ScrollViewWithFloatingScrollbar *scroll_view);

private:
    void ResetState(ThumbState state);

protected:
    ScrollViewWithFloatingScrollbar *scroll_view_;
    FloatingScrollBarButton *prev_button_;
    FloatingScrollBarThumb *thumb_;
    FloatingScrollBarButton *next_button_;

private:
    friend class ScrollViewWithFloatingScrollbar;
    bool is_horiz_;
    gfx::Rect bounds_; // 基于ScrollViewWithFloatingScrollbar的坐标
    bool visible_;
    int max_pos_;
    int current_pos_;
    int viewport_size_;
    int content_size_;

    int drag_start_pos_ = 0;
    gfx::Point drag_thumb_down_point_;
    bool is_bar_narrow_ = false;
    DISALLOW_COPY_AND_ASSIGN(FloatingScrollBar);
};

// 带浮动滚动条的滚动视图
class ScrollViewWithFloatingScrollbar : public views::View
{
    class Viewport;

public:
    static char kViewClassName[];
    explicit ScrollViewWithFloatingScrollbar(views::View *contents,bool is_bar_narrow = false);
    virtual ~ScrollViewWithFloatingScrollbar();

    void SetScrollThumbColor(ThumbState state, SkColor color);
    void SetScrollPositionChangeListener(ScrollPositionChangeListener* l);

    ScrollReason last_scroll_reason() const { return scroll_reason_; }
    ThumbState current_scrollbar_state(bool vert) const;

    void set_hide_horizontal_scrollbar(bool visible) {
        hide_horizontal_scrollbar_ = visible;
    };

    // Set the contents. Any previous contents will be deleted. The contents
    // is the view that needs to scroll.
    void SetContents(views::View* content, bool delete_old = true);
    const View* contents() const { return contents_; }
    View* contents() { return contents_; }

    // Sets the header, deleting the previous header.
    void SetHeader(views::View* header, bool delete_old = true);

    // Returns the visible region of the content View.
    gfx::Rect GetVisibleRect() const;

    // Retrieves the width/height of scrollbars. These return 0 if the scrollbar
    // has not yet been created.
    int GetScrollBarWidth() const;
    int GetScrollBarHeight() const;

    // ScrollBarController overrides:
    void ScrollToPosition(bool vert, int position, ScrollReason reason = User);
    int GetScrollIncrement(bool vert, bool is_page, bool is_positive);

    // Returns the horizontal/vertical scrollbar. This may return NULL.
    const FloatingScrollBar* horizontal_scroll_bar() const { return horiz_sb_; }
    const FloatingScrollBar* vertical_scroll_bar() const { return vert_sb_; }

    // Customize the scrollbar design. ScrollView takes the ownership of the
    // specified ScrollBar. |horiz_sb| and |vert_sb| cannot be NULL.
    void SetHorizontalScrollBar(FloatingScrollBar* horiz_sb);
    void SetVerticalScrollBar(FloatingScrollBar* vert_sb);

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;
    void Paint(gfx::Canvas* canvas) override;
    View* GetEventHandlerForPoint(const gfx::Point& point) override;
    bool OnKeyPressed(const ui::KeyEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseMoved(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    bool OnMouseWheel(const ui::MouseWheelEvent& event) override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    const char* GetClassName() const override { return kViewClassName; };
    void ChildPreferredSizeChanged(views::View* child) override;

private:
    // Shows or hides the scrollbar/resize_corner based on the value of
    // |should_show|.
    void SetControlVisibility(bool vert, bool should_show);

    // Used internally by SetHeader() and SetContents() to reset the view.  Sets
    // |member| to |new_view|. If |new_view| is non-null it is added to |parent|.
    void SetHeaderOrContents(Viewport* parent, views::View* new_view, views::View** member, bool delete_old);

    // Scrolls the minimum amount necessary to make the specified rectangle
    // visible, in the coordinates of the contents view. The specified rectangle
    // is constrained by the bounds of the contents view. This has no effect if
    // the contents have not been set.
    void ScrollContentsRegionToBeVisible(const gfx::Rect& rect);

    // Computes the visibility of both scrollbars, taking in account the view port
    // and content sizes.
    void ComputeScrollBarsVisibility(const gfx::Size& viewport_size,
        const gfx::Size& content_size,
        bool* horiz_is_shown,
        bool* vert_is_shown);

    // Update the scrollbars positions given viewport and content sizes.
    void UpdateScrollBarPositions();

private:
    bool hide_horizontal_scrollbar_;

    // The current header and its viewport. |header_| is contained in
    // |header_viewport_|.
    views::View* header_;
    Viewport* header_viewport_;

    // The current contents and its viewport. |contents_| is contained in
    // |contents_viewport_|.
    views::View* contents_;
    Viewport* contents_viewport_;

    // Horizontal scrollbar.
    FloatingScrollBar* horiz_sb_;

    // Vertical scrollbar.
    FloatingScrollBar* vert_sb_;

    ScrollReason scroll_reason_;
    ScrollPositionChangeListener* listener_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(ScrollViewWithFloatingScrollbar);
};

// 会自动出现滚动条的view，大多数情况下我们将内容view直接继承此类
// 使用此类的原则是内容区的内容最好不要能直接贴边显示，因如果能贴边显示则浮动滚动条在鼠标hover窗口的时候
// 显示出来会挡住部分内容，在这种情况下用常规的滚动视图BililiveViewWithScrollbar会好一些
class BililiveViewWithFloatingScrollbar : public views::View
{
public:
    explicit BililiveViewWithFloatingScrollbar(bool hide_horiz = true);

    ScrollViewWithFloatingScrollbar* Container(){ return scroll_view_; }

    // View
    const char* GetClassName() const override { return "BililiveViewWithFloatingScrollbar"; }
    void Layout() override;
    void ChildPreferredSizeChanged(views::View* child) override;
    void PreferredSizeChanged() override;

private:
    ScrollViewWithFloatingScrollbar *scroll_view_;

    DISALLOW_COPY_AND_ASSIGN(BililiveViewWithFloatingScrollbar);
};

#endif