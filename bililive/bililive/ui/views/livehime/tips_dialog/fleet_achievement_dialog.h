#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TIPS_DIALOG_FLEET_ACHIEVEMENT_DIALOG_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TIPS_DIALOG_FLEET_ACHIEVEMENT_DIALOG_H_

#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"

#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/secret/public/danmaku_hime_service.h"


class LivehimeImageView;
class LivehimeTipLabel;
class LivehimeTitleLabel;
class BililiveImageButton;

class FleetAchievementDialog :
    public BililiveWidgetDelegate,
    public BililiveNonTitleBarFrameViewDelegate,
    public views::ButtonListener
{
public:
    struct FleetAchievementInfo {
        std::string bg_url;
        std::string avatar_url;
        std::string avatar_frame_url;
        int level = 0;
        std::wstring first_line_text;
        std::wstring second_line_text;
        std::string highligh_color;
    };

    static void ShowDialog(views::Widget* parent, const FleetAchievementInfo& info);

    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_NONE; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget *widget) override;

    // views::View
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // BililiveNonTitleBarFrameViewDelegate
    int NonClientHitTest(const gfx::Point &point) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    explicit FleetAchievementDialog(const FleetAchievementInfo& info);
    ~FleetAchievementDialog();

    void InitViews();
    void DownloadBgImage();
    void OnCountdownTimer();
    void OnGetImageByUrl(
        bool valid_response, const std::string& data);
    void ProcessFirstLineText(
        const string16& org_text, string16* plain_text, string16* highlight_text, bool* left);

    static void AdjustWindowPosition(views::Widget* widget);

    LivehimeTitleLabel* left_label_ = nullptr;
    LivehimeTitleLabel* right_label_ = nullptr;
    LivehimeTipLabel* tip_label_ = nullptr;
    LivehimeImageView* bg_view_ = nullptr;
    LivehimeImageView* avatar_view_ = nullptr;
    BililiveImageButton* button_view_ = nullptr;
    FleetAchievementInfo info_;

    int countdown_;
    base::RepeatingTimer<FleetAchievementDialog> timer_;
    base::WeakPtrFactory<FleetAchievementDialog> weak_ptr_factory_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_TIPS_DIALOG_FLEET_ACHIEVEMENT_DIALOG_H_