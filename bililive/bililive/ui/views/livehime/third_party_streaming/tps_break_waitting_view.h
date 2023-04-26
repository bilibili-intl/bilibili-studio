#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THIRD_STREAMING_BREAK_WATTING_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THIRD_STREAMING_BREAK_WATTING_VIEW_H_

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "ui/views/controls/button/button.h"


class BililiveLabelButton;
class BililiveLabel;

class TpsBreakWaittingView
    : public BililiveWidgetDelegate
    , public views::ButtonListener
    , LivehimeLiveRoomObserver
{
public:
    static void DoModal(gfx::NativeView parent, const EndDialogSignalHandler* handler);
    static void Close();

protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const { return ui::MODAL_TYPE_WINDOW; }
    views::View* GetContentsView() { return this; }
    void WindowClosing() override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LivehimeLiveRoomObserver
    void OnThirdPartyStreamingBreakCountDown(int sec) override;

private:
    TpsBreakWaittingView();
    ~TpsBreakWaittingView();

    void InitViews();

private:
    bool closed_ = false;

    BililiveLabel* label_ = nullptr;
    BililiveLabelButton* stop_button_ = nullptr;
    BililiveLabelButton* wait_button_ = nullptr;

    base::WeakPtrFactory<TpsBreakWaittingView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TpsBreakWaittingView);
};

#endif