#ifndef BILILIVE_BILILIVE_UI_OPEN_LIVE_VIDDUP_VIEW_H_
#define BILILIVE_BILILIVE_UI_OPEN_LIVE_VIDDUP_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"

class LivehimeContentLabel;
class LivehimeActionLabelButton;
class BililiveWidgetDelegate;

namespace livehime {
    void ShowOpenLiveDialog(gfx::NativeView parent_widget);

    class OpenLiveViddupView :
        public BililiveWidgetDelegate
        , public views::ButtonListener
        , public BililiveComboboxExListener {
    public:
        OpenLiveViddupView();
        // views::View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
        // WidgetDelegate
        ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
        views::View* GetContentsView() override { return this; }
        // views::ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override;
        // BililiveComboboxExListener
        void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;


    private:
        void InitViews();
        void InitViewsData();

        LivehimeCombobox* combobox_;
        LivehimeContentLabel* label_;
        views::View* buttons_bar_;
        int button_padding_ = 0;
        std::string url_;

        DISALLOW_COPY_AND_ASSIGN(OpenLiveViddupView);
    };

}

#endif  // BILILIVE_BILILIVE_UI_EXAMPLES_MISC_TAB_VIEW_H_