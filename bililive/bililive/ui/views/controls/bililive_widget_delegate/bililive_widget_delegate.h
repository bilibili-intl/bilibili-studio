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
    // �ԡ���ģ̬������ʽ����
    static void DoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
        views::Widget::InitParams params, const EndDialogSignalHandler* handler = nullptr);
    // ����ͨ��ʽ����
    static void ShowWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
        views::Widget::InitParams params, bool bShow = true, bool use_custom_shadow = false,
        views::InsureShowCorner anchor_type = views::InsureShowCorner::ISC_NEARBY);
    // ָ�������Ƿ���ģ̬��������ģ̬����
    static bool IsWidgetInModalNow(gfx::NativeView hwnd);
    // ��ָ���Ĵ�������ģ̬������������ģ̬���ƣ�������˸�����Ǹ�
    static void FlashModalWidget(gfx::NativeView hwnd);

    // ��⵱ǰĿ��ģ̬�����Ƿ��Ѵ��ڣ������ظ��򿪣����ز�ѯ����Ŀ��ģ̬����
    static views::Widget* GetCurrentWidget();

    // ����һЩ��ܵ�WidgetObserverû�м����Ĵ���״̬���
    void AddObserver(BililiveWidgetDelegateObserver *obs);
    void RemoveObserver(BililiveWidgetDelegateObserver *obs);

    // ��ǰ�������Ƿ���ϵͳ��ģ̬��
    bool IsInSystemModalNow();
    bool IsInOwnedModalNow();
    bool IsInModalNow();
    // ģ̬�������
    void SetResultCode(int result_code);
    int GetResultCode() const;
    void SetResultData(void* data);
    void* GetResultData() const;

    void EnableResize(bool can_resize) { can_resize_ = can_resize; }

    // ��˸������Ӱ�߿�hide/show����
    void FlashFrame();
    //�Ƿ���ô��ڶ���
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

    // �Ƿ񻺴������С���ߴ磬�Ա�����ק�ƶ�ʱ���ٵݹ�㼶����GetPrefersize
    void SetCacheMaxminSize(bool cache);

    // ��ǰ�������ģ̬����ʱ���Ƿ�Ϊowner���帲һ���ɰ�
    void SetShowMaskWndWhenDoModal(bool show_mask) { show_mask_hwnd_when_domodal_ = show_mask; }
    bool show_mask_hwnd_when_domodal() const { return show_mask_hwnd_when_domodal_; }
    // �ڸô����ϻ�ȡϵͳ�����ľ��
    HWND GetSystemModalWnd();
    // �ڸô����ϻ�ȡ�������Լ��Ĵ��ڻ��Ƶ��������ģ̬��
    HWND GetOwnedTopModalWnd();
    // �ڴ����ϻ�ȡ��㵯���������ֵ�������
    HWND GetTopModalWnd();

    // �ԡ���ģ̬������ʽ����
    static int RealDoModalWidget(BililiveWidgetDelegate *delegate_view, views::Widget *widget,
        views::Widget::InitParams params, bool obs_core_not_init = false);

private:
    void InitParam();
    void set_frame_details(BililiveFrameView* frame_view);
    void OnClosing();
    // ��ⴰ��״̬����һ�¾ͽ���observer֪ͨ
    void CheckStatus();
    // ��ʾģ̬��ʶmask��
    void ShowMaskWidget(bool show);
    // ģ̬�����ص�
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

    // �̳�BililiveWidgetDelegate��view�������ShowWidget����Widget::Init�������佫�޷��ṩͳһ�Ĺ�������
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

    bool disable_flash_frame_ = false;//���ô��ڶ���

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
    // Զ��ʱ��ΪUGC�Ĵ��帽�ϴ�����Ӱ�Ŀ��أ�ʹ�õ��ǿ���Դ���BubbleFrame
    bool is_transparent_;

    DISALLOW_COPY_AND_ASSIGN(BilibiliUploaderWidgetDelegate);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_H_