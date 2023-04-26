#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVE_NOTICE_LIVE_NOTICE_DIALOG_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVE_NOTICE_LIVE_NOTICE_DIALOG_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bilibili_native_edit_view.h"
#include "bililive/secret/services/live_streaming_service_impl.h"

class BililiveLabelButton;
class LivehimeSmallContentLabel;
class LivehimeSubEditView;
class LivehimeActionLabelButton;

class LiveNoticeDialog
    : public BililiveWidgetDelegate
    , public views::ButtonListener
    , public BilibiliNativeEditController
{
public:
    LiveNoticeDialog();
    virtual ~LiveNoticeDialog();

    static void ShowWindow(views::Widget *parent);
    static void Close();

protected:
    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

    // views::View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // BilibiliNativeEditController
    void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) override;

private:
    void InitViews();

    void RequestUpdateNotice(const std::string& content);
    void OnNewRoomSetting(bool, int, const std::string&);

    void ShowToastMsg(const base::string16& box_text);

private:
    views::View* title_area_view_ = nullptr;
    BililiveLabelButton* room_title_button_ = nullptr;
    LivehimeSmallContentLabel* title_num_label_ = nullptr;
    LivehimeSubEditView* text_area_ = nullptr;

    LivehimeActionLabelButton* ok_button_ = nullptr;
    LivehimeActionLabelButton* cancel_button_ = nullptr;

    base::string16 old_content_;
    int old_len_ = 0;

    base::WeakPtrFactory<LiveNoticeDialog> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(LiveNoticeDialog);
};
#endif

