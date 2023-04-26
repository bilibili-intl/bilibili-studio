#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THIRD_STREAMING_MODE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THIRD_STREAMING_MODE_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_button.h"

class LivehimeImageView;
class LivehimeLinkButton;

//class BililiveLabelButton;
class LivehimeTextfield;

class ThirdPartyStreamingView
    : public views::View
    , public views::ButtonListener
    , LivehimeLiveRoomObserver
{
public:
    ThirdPartyStreamingView(bool preset_material = false);
    ~ThirdPartyStreamingView();

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LivehimeLiveRoomObserver
    void OnThirdPartyStreamingListened(const std::vector<std::string>& ip_list, int port) override;

private:
    void InitViews();
    void ChangeQRImage();

    void QuitEndDialog(const base::string16& btn, void* data);

private:
    bool preset_material_ = false;
    LivehimeTextfield* addr_textfield_ = nullptr;
    LivehimeTextfield* code_textfield_ = nullptr;
    LivehimeImageView* qr_image_view_ = nullptr;

    CustomStyleButton* bt_copy_addr_ = nullptr;
    CustomStyleButton* bt_copy_code_ = nullptr;
    LivehimeLinkButton* change_button_ = nullptr;

    std::vector<std::string> ip_list_;
    int port_ = 0;
    int index_ = 0;
    
    base::WeakPtrFactory<ThirdPartyStreamingView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ThirdPartyStreamingView);
};

#endif