#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THIRD_STREAMING_OBS_GUIDE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THIRD_STREAMING_OBS_GUIDE_VIEW_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "ui/views/controls/button/button.h"


class BililiveLabelButton;
class LivehimeNativeEditView;
class BililiveLabel;
class BililiveViewWithFloatingScrollbar;

class ThirdPartyStreamingOBSGuideView
    : public BililiveWidgetDelegate
    , public views::ButtonListener
{
public:
    static void ShowForm(gfx::NativeView parent);

protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const { return ui::MODAL_TYPE_WINDOW; }
    views::View* GetContentsView() { return this; }

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    ThirdPartyStreamingOBSGuideView();
    ~ThirdPartyStreamingOBSGuideView();

    void InitViews();
    void InitData();

private:
    BililiveViewWithFloatingScrollbar* content_view_ = nullptr;
    LivehimeNativeEditView* folder_edit_ = nullptr;
    BililiveLabelButton* folder_button_ = nullptr;

    BililiveLabel* obs_folder_tip_ = nullptr;
    BililiveLabelButton* setup_button_ = nullptr;

    base::WeakPtrFactory<ThirdPartyStreamingOBSGuideView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ThirdPartyStreamingOBSGuideView);
};

#endif