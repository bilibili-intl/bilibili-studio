#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_FRAME_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_FRAME_VIEW_H_

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/settings/settings_contract.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"


class NavigationBar;
class BaseSettingsView;
class LivehimeActionLabelButton;
class BlacklistView;

enum class SettingIndex {
    VideoSettings = 0,
    AudioSettings,
    AcceleratorSettings,
    RoomManager,
    DanmakuHimeSettings,
    RecordingSettings,
    GlobalSettings,
    StreamingSettings,
};

namespace obs_proxy {
    class SceneCollection;
}

// 主界面设置模块
class SettingsFrameView
    : public BililiveWidgetDelegate
    , public views::ButtonListener
    , public contracts::SettingsFrameView
    , public NavigationBarListener
{
public:
    explicit SettingsFrameView(int index = 0);
    ~SettingsFrameView();

    static void ShowForm(views::Widget *parent, int index);

    void SaveOrCheckStreamingSettingsChange(
        bool check, BaseSettingsView::ChangeType &result) override;

    void SaveNormalSettingsChange() override;

    bool ShowRestreamingDialog() override;

    bool CheckExclusive() override;

    void InitViewsForViddup();

    void ShowLiveReplaySetting(bool publish, bool archive) override;
protected:
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void Paint(gfx::Canvas *canvas) override;

    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    void WindowClosing() override;

    //NavigationBarListener
    void NavigationBarSelectedAt(int strip_id) override;

private:
    void InitViews();
    void SelectDefTab(int index);

    void ProcessPositive();
    void ProcessNegative();

    void OnShowRestreamingDialog(const base::string16& result, void* data);

private:
    static SettingsFrameView *instance_;
    NavigationBar *tabbed_pane_;
    std::vector<BaseSettingsView*> sub_views_;
    LivehimeActionLabelButton *ok_button_;
    LivehimeActionLabelButton *cancel_button_;
    BlacklistView* blac_klist_ = nullptr;
    std::vector<int> sub_views_tabbed_ids_;
    int def_tabindex_;
    int global_setting_id_ = 0;
    int room_manager_id_ = -1;
    int download_tap_id_ = -1;
    bool saved_changes_;
    bool window_closing_;
    bool trigger_close_;
    int global_index_ = 0;
    int language_index_ = 0;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    base::WeakPtrFactory<SettingsFrameView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(SettingsFrameView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_SETTINGS_FRAME_VIEW_H_