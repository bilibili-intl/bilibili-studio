#pragma once

#include "base/timer/timer.h"
#include "ui/base/animation/animation_delegate.h"
#include "ui/views/animation/bounds_animator.h"
#include "ui/views/view.h"

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bilibili_native_edit_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/viddup/danmaku_hime/danmaku_interaction_title_viddup_view.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_view.hpp"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_view_theme.h"
#include "bililive/bililive/ui/views/viddup/danmaku_hime/danmaku_render_viddup_view.h"
#include "bililive/bililive/livehime/danmaku_hime/dmkhime_source_contract.h"
#include "bililive/bililive/viddup/danmaku_hime/send_danmaku_viddup_presenter.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_mp4_player_viewer.h"
#include "bililive/secret/public/live_streaming_service.h"

class DanmakuRenderViddupView;
class MarkednessMessageView;
class DanmakuInteractionViddupViewDelegate;

class DanmakuInteractionViddupView :
    public DanmakuView<DanmakuInteractionViddupView>,
    public contracts::SendDanmakuViddupView,
    public views::ButtonListener,
    public contracts::DmkhimeSourceView
{
public:
    static const int kMergeMiniHeight;

public:
    DanmakuInteractionViddupView();
    ~DanmakuInteractionViddupView();

    void AddDanmaku(const DanmakuData& data);
    void UpdateView();

    void SetDelegate(DanmakuInteractionViddupViewDelegate* delegate);

    void RedrawRenderViews(const DanmakuRenderingParams& param);

    void SetScrollingFps(int fps);
    void SetSuspendingScrollingEnabled(bool enabled);
    void SetGiftEffectsEnabled(bool enabled);

    void ProcessAntiBFree(const DanmakuData& data);
    void ProcessAntiBInteract(const DanmakuData& data);

    gfx::Point GetMarkednessMessageViewPopPosition();
    void AddNewItem(const secret::LiveStreamingService::MarkednessMessageInfo& danmaku);
    void UpdateMMViews();
    void RefreshShadow(int64_t sel_id);
    void CloseDetailsView();

    DanmakuInteractionTitleViddupView* GetTitleView();

    void Layout() override;

    void ReloadInputView();

    void RelocationMessageLoopView();

protected:

    //ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    //bool PreHandleMSG(BilibiliNativeEditView* sender, UINT msg, WPARAM wParam, LPARAM lParam) override;

    //contracts::SendDanmakuView
    void OnDanmakuSent(bool valid_response, bool sending_succeeded, const string16& msg) override;

    //DanmakuThemeInterface
    void SwitchThemeImpl() override;

    //DanmakuShowTypeInterface
    void SwitchShowTypeImpl() override;

    //DanmakuView
    void OnOpacityChanged()override;

    void Paint(gfx::Canvas* canvas) override;

    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    //DmkhimeSourceView
    void OnOutgoing(bool first) override;
    void OnAutism() override;

    //DanmakuView
    void OnLockMouseDect() override;
    //views::View
    bool OnMouseDragged(const ui::MouseEvent& event) override;

    views::View* GetEventHandlerForPoint(const gfx::Point& point) override;

    void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;

    void OnWidgetBoundsChanged(views::Widget* widget,
        const gfx::Rect& new_bounds) override;

private:
    enum class AnimStatus
    {
        None,
        In,
        Out,
    };

    void InitView();
    void InitInputView();
    void InitDanmakuTip();
    void OnFreeTimer();
    void OnFreeAntibVisibilityChanged(bool visible);
    void OnInteractAntibVisibilityChanged(bool visible);
    void OnInteractTimer();
    void SendLiveDanmaku();
    void AddAnchorSendLocalMessage(const string16& msg);

    DanmakuInteractionViddupViewDelegate* delegate_ = nullptr;

    DanmakuInteractionTitleViddupView* title_view_ = nullptr;

    float                                               effect_view_height_rate_ = 0.0f;
    DanmakuRenderViddupView* danmaku_rv_ = nullptr;
    DanmakuRenderViddupView* antib_free_rv_ = nullptr;
    DanmakuRenderViddupView* antib_interact_rv_ = nullptr;
    //DanmakuRenderViddupView* enter_effect_rv_ = nullptr;
    views::View* danmaku_viewport_ = nullptr;
    MarkednessMessageView* message_view_ = nullptr;
    DanmakuRenderingParams                              render_params_ = {};

    bool                                                is_idle_free_timer_ = false;
    std::queue<DanmakuData>                             free_queue_;
    base::OneShotTimer<DanmakuInteractionViddupView>          free_timer_;
    std::unique_ptr<views::BoundsAnimator>              free_anim_;
    AnimStatus                                          free_anim_status_ = AnimStatus::None;

    bool                                                is_idle_interact_timer_ = false;
    std::vector<DanmakuData>                            cur_slice_;
    std::map<int64_t, std::vector<DanmakuData>>         interact_queue_;
    base::OneShotTimer<DanmakuInteractionViddupView>          interact_timer_;
    std::unique_ptr<views::BoundsAnimator>              interact_anim_;
    AnimStatus                                          interact_anim_status_ = AnimStatus::None;

    SplitLineHorizontalLabel* label_split_line_ = nullptr;
    views::View* text_input_view_ = nullptr;
    LivehimeNativeEditView* dmk_edit_view_ = nullptr;
    LivehimeActionLabelButton* send_dmk_button_ = nullptr;
    contracts::SendDanmakuViddupPresenter                     send_dmk_presenter_;

    std::unique_ptr<contracts::DmkhimeSourcePresenter>  source_presenter_;

    base::WeakPtrFactory<DanmakuInteractionViddupView>        wpf_;
};

class DanmakuInteractionViddupViewDelegate
{
public:
    virtual ~DanmakuInteractionViddupViewDelegate() {}
    virtual void OnDanmakuInteractionViddupViewButtonPressed(DanmakuButtonType button_id) = 0;

protected:
    DanmakuInteractionViddupViewDelegate() {}

};
