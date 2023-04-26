#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LIVE_HANDON_NOTIFY_WINDOW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LIVE_HANDON_NOTIFY_WINDOW_H_

#include "ui/views/controls/button/button.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"

class LivehimeContentLabel;
class BililiveLabelButton;

class LiveHandonNotifyWindow
    : public BililiveWidgetDelegate,
    public views::ButtonListener{

public:
    // WidgetDelegate
    views::View* GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_NONE; }

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    void UpdateNotifyInfo();

protected:
    void WindowClosing() override;
private:
    LiveHandonNotifyWindow();
    ~LiveHandonNotifyWindow();

    void InitViews();
    void UninitViews();
    void OnCountDownTimer();
    void SetTimeLabStr(int count);
    //���ֵ���ԭ�� 1 �������ȷ�ϲ��ز�  2����û�е�������ϳɹ��ز�
    void HttpUpdateNotifyResult(int flag); 
    void OnUpdateNotifyResult(bool succ,int code);

    LivehimeContentLabel* title_label_;
    LivehimeContentLabel* tips_label_;
    BililiveLabelButton* confirm_button_;
    LivehimeContentLabel* time_label_;

    int countdown_ = 0;
    base::RepeatingTimer<LiveHandonNotifyWindow> timer_;
    base::WeakPtrFactory<LiveHandonNotifyWindow> weak_ptr_factory_;
};

#endif // !#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_LIVE_HANDON_NOTIFY_WINDOW_H_
