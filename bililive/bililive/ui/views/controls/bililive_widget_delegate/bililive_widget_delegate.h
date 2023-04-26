#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_H_
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_H_

#include "bililive_widget_delegate_types.h"

#include "ui/base/view_prop.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/controls/messagebox/message_box.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

#include "base/timer/timer.h"

#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_titlebar.h"

namespace livehime
{
    base::string16 ShowModalMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia,
        gfx::HorizontalAlignment align);
}

class BililiveFrameView;
class BililiveWidgetDelegate;
class BililiveWidgetDelegateObserver
{
public:
    virtual ~BililiveWidgetDelegateObserver() = default;

    virtual void OnWidgetEnabledChanged(BililiveWidgetDelegate*, BOOL enabled) {}
};

class BililiveWidgetDelegate
    : public views::WidgetDelegateView
    , public views::WidgetObserver
{
public:
    static char kWidgetDelegateClassName[];

public:
    // 以“假模态”的形式弹出
    static void DoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
        views::Widget::InitParams params, const EndDialogSignalHandler* handler = nullptr);
    // 以普通形式弹出
    static void ShowWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
        views::Widget::InitParams params, bool bShow = true, bool use_custom_shadow = false,
        views::InsureShowCorner anchor_type = views::InsureShowCorner::ISC_NEARBY);
    // 指定窗口是否处于模态，不区分模态机制
    static bool IsWidgetInModalNow(gfx::NativeView hwnd);
    // 若指定的窗口上有模态弹窗（不区分模态机制），则闪烁最顶层的那个
    static void FlashModalWidget(gfx::NativeView hwnd);

    // 检测当前目标模态窗口是否已存在，避免重复打开，返回查询到的目标模态窗口
    static views::Widget* GetCurrentWidget();

    // 监听一些框架的WidgetObserver没有监听的窗口状态变更
    void AddObserver(BililiveWidgetDelegateObserver *obs);
    void RemoveObserver(BililiveWidgetDelegateObserver *obs);

    // 当前窗口里是否弹了系统的模态框
    bool IsInSystemModalNow();
    bool IsInOwnedModalNow();
    bool IsInModalNow();
    // 模态操作结果
    void SetResultCode(int result_code);
    int GetResultCode() const;
    void SetResultData(void* data);
    void* GetResultData() const;

    void EnableResize(bool can_resize) { can_resize_ = can_resize; }

    // 闪烁自身，阴影边框hide/show交替
    void FlashFrame();
    //是否禁用窗口抖动
	void SetDisableFlashFrame(bool disable) { disable_flash_frame_ = disable;}
    bool IsFlashFrameDisabled() { return disable_flash_frame_; }

    virtual void SetWindowTitle(const base::string16& title);

protected:
    BililiveWidgetDelegate(const gfx::ImageSkia& skia, const base::string16 &caption,
                           int TitleBarButtonCombination = TBB_CLOSE);
    BililiveWidgetDelegate();
    ~BililiveWidgetDelegate();

    // WidgetObserver
    void OnWidgetCreated(views::Widget* widget) override;
    void OnWidgetDestroying(views::Widget* widget) override;
    void OnWidgetDestroyed(views::Widget* widget) override;
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    views::NonClientFrameView *CreateNonClientFrameView(views::Widget *widget) override;
    string16 GetWindowTitle() const override{ return caption_; };
    const char* GetWidgetDelegateClassName() const override { return kWidgetDelegateClassName; }
    bool CanMaximize() const override { return CanResize(); }
    bool CanResize() const override { return can_resize_; };

    // View
    void OnThemeChanged() override;

    virtual void OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view) {}

    // 是否缓存最大最小化尺寸，以便于拖拽移动时减少递归层级调用GetPrefersize
    void SetCacheMaxminSize(bool cache);

    // 当前窗体进行模态弹窗时，是否为owner窗体覆一层蒙板
    void SetShowMaskWndWhenDoModal(bool show_mask) { show_mask_hwnd_when_domodal_ = show_mask; }
    bool show_mask_hwnd_when_domodal() const { return show_mask_hwnd_when_domodal_; }
    // 在该窗口上获取系统弹窗的句柄
    HWND GetSystemModalWnd();
    // 在该窗口上获取以我们自己的窗口机制弹出的最顶层模态窗
    HWND GetOwnedTopModalWnd();
    // 在窗口上获取最顶层弹窗，不区分弹出机制
    HWND GetTopModalWnd();

    // 以“真模态”的形式弹出
    static int RealDoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
        views::Widget::InitParams params, bool obs_core_not_init = false);

private:
    void InitParam();
    void set_frame_details(BililiveFrameView* frame_view);
    void OnClosing();
    // 检测窗口状态，不一致就进行observer通知
    void CheckStatus();
    // 显示模态标识mask窗
    void ShowMaskWidget(bool show);
    // 模态结束回调
    void SetEndDialogSignalHandler(const EndDialogSignalHandler& handler);
    void SetRealModalResult(const std::shared_ptr<int>& modal_result);

    static LRESULT CALLBACK BililiveWidgetDelegateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    int titlebar_button_combination_ = 1;
    gfx::ImageSkia skia_;
    base::string16 caption_;
    bool cache_maxmin_size_ = true;
    EndDialogSignalHandler modal_end_dialog_handler_;

private:
    friend base::string16 livehime::ShowModalMessageBox(
        gfx::NativeView parent_widget,
        const base::string16& title,
        const base::string16& message,
        const base::string16& buttons,
        const gfx::ImageSkia* image_skia,
        gfx::HorizontalAlignment align);

    // 继承BililiveWidgetDelegate的view必须采用ShowWidget进行Widget::Init，否则其将无法提供统一的功能特性
    bool init_by_specified_way_ = false;

    scoped_ptr<ui::ViewProp> prop_;
    WNDPROC original_handler_ = nullptr;
    ObserverList<BililiveWidgetDelegateObserver> observers_;
    BOOL widget_enabled_ = TRUE;
    gfx::Point origin_before_minimize_in_screen_;

    bool show_mask_hwnd_when_domodal_ = false;
    views::Widget* modal_status_mask_widget_ = nullptr;

    std::shared_ptr<int> modal_result_;
    bool real_modal_ = false;

    bool can_resize_ = false;

    bool disable_flash_frame_ = false;//禁用窗口抖动

    DISALLOW_COPY_AND_ASSIGN(BililiveWidgetDelegate);
};



class BilibiliUploaderWidgetDelegate : public BililiveWidgetDelegate
{
public:
    BilibiliUploaderWidgetDelegate();
    explicit BilibiliUploaderWidgetDelegate(gfx::ImageSkia skia, const base::string16 &caption);
    virtual ~BilibiliUploaderWidgetDelegate() = default;

protected:
    // WidgetDelegate
    views::NonClientFrameView *CreateNonClientFrameView(views::Widget *widget) override;

    void set_transparent(bool transparent)
    {
        is_transparent_ = transparent;
    }

    bool is_transparent() const
    {
        return is_transparent_;
    }

private:
    // 远古时期为UGC的窗体附上窗口阴影的开关，使用的是框架自带的BubbleFrame
    bool is_transparent_;

    DISALLOW_COPY_AND_ASSIGN(BilibiliUploaderWidgetDelegate);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_H_