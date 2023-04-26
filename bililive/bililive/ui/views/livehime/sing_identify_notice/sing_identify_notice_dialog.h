#ifndef BILILIVE_BILILIVE_UI_VIEWS_SING_IDENTIFY_NOTICE_DIALOG_H_
#define BILILIVE_BILILIVE_UI_VIEWS_SING_IDENTIFY_NOTICE_DIALOG_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "ui/views/controls/button/button.h"

class LivehimeImageView;
class LivehimeCheckbox;
class LivehimeActionLabelButton;
namespace views {
    class Label;
}

class SingIdentifyNoticeDialog
    : public BililiveWidgetDelegate
    , public views::ButtonListener
{
public:
    SingIdentifyNoticeDialog();
    virtual ~SingIdentifyNoticeDialog();

    static void ShowWindow(views::Widget *parent,const std::string& image_url);
    static void Close();
    void SetImageUrl(const std::string& url) { image_url_ = url; }

protected:
    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }

    // views::View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitViews();

private:
    std::string                     image_url_;
    views::Label*                   notice_title_lab = nullptr;
    views::Label*                   notice_lab_ = nullptr;
    LivehimeImageView*              image_view_ = nullptr;
    LivehimeActionLabelButton*      know_button_ = nullptr;
    LivehimeCheckbox*               not_notice_checkbox_ = nullptr;
    base::WeakPtrFactory<SingIdentifyNoticeDialog> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(SingIdentifyNoticeDialog);
};
#endif //BILILIVE_BILILIVE_UI_VIEWS_SING_IDENTIFY_NOTICE_DIALOG_H_

