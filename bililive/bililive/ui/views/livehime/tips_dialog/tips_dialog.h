#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TIPS_DIALOG_TIPS_DIALOG_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TIPS_DIALOG_TIPS_DIALOG_H_

#include "ui/views/controls/button/button.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"


class TipsDialog:
    public BililiveWidgetDelegate,
    public views::ButtonListener
{
public:
    enum TipsType
    {
        CameraTips = 0,
        LiveReplayTips
    };
    //rect是tips窗口的大小，目前只用到rect的宽高，因为x,y是需要计算的；refer是参照物，目前摄像头源的tips在用，可以传空
    static void ShowWindow(views::Widget* parent,const gfx::Rect& rect, TipsType type,const gfx::Rect* refer,const EndDialogSignalHandler* handler = nullptr,int fade_duration_ms = 0);

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void MoveDialog();
private:
    TipsDialog(views::Widget* widget_,const gfx::Rect& rect, TipsType type,const gfx::Rect* refer);
    ~TipsDialog();

    void InitViews();

    void StartFade(int fade_duration_ms);
    void FadeOut();

private:
    views::View* left_view_ = nullptr;
    views::ImageView* bubble_img_view_ = nullptr;
    LivehimeFunctionLabelButton* close_button_ = nullptr;
    views::Widget* widget_ = nullptr;
    views::ImageView* img_view_ = nullptr;
    gfx::Rect rect_;
    gfx::Rect refer_;
    TipsType type_;

    base::WeakPtrFactory<TipsDialog> weak_ptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(TipsDialog);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TIPS_DIALOG_TIPS_DIALOG_H_