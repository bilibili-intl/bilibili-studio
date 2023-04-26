#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_DANMAKU_HIME_DANMAKU_RENDER_VIDDUP_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_DANMAKU_HIME_DANMAKU_RENDER_VIDDUP_VIEW_H_

#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"
#include "base/timer/timer.h"

#include "ui/views/controls/menu/menu_delegate.h"
#include "ui/views/view.h"

//#include "bililive/bililive/ui/views/livehime/danmaku_hime/dmkhime_effect_stub.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_list.h"
#include "bililive/bililive/livehime/audio_player/audio_player_presenter_imp.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/viddup/server_broadcast/broadcast_viddup_service.h"
#include "bililive/bililive/viddup/danmaku_hime/danmaku_viddup_renderer.h"

//class DanmakuMenu;

namespace dmkhime
{
    class DanmakuList;
    class DanmakuListAdapter;
    class DanmakuRenderer;
}

class EnterEffectViddupDelegate
{
public:
    virtual ~EnterEffectViddupDelegate() {};
    virtual void OnAnimationStart() = 0;
    virtual void OnEnterEffectPlay(bool play) = 0;
    virtual void OnAnimationEnd() = 0;
protected:
    EnterEffectViddupDelegate() {}
};

class DanmakuRenderViddupView :
    public views::View,
    public dmkhime::ListStatusChangeListener
{
public:
    explicit DanmakuRenderViddupView(DanmakuViewType type, int max_count);
    DanmakuRenderViddupView(int max_count, bool can_interactive, float speed_factor, dmkhime::DanmakuListAdapter* adapter = nullptr);
    virtual ~DanmakuRenderViddupView();

    void InitViews();
    void UninitViews();
    void StartGifTimer(bool start = false);
    void SetEnterEffectDelegate(EnterEffectViddupDelegate* delegate);
    void Clear();

    // View
    //void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    void OnPaint(gfx::Canvas* canvas) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    views::View* GetEventHandlerForPoint(const gfx::Point& point) override;
    bool OnMouseWheel(const ui::MouseWheelEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    void OnMouseCaptureLost() override;
    void OnMouseMoved(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    bool HitTestRect(const gfx::Rect& rect) const override;

    virtual void AddDanmaku(const DanmakuData& data);
    virtual void UpdateDanmaku(const DanmakuData& data);
    void UpdateVoiceImgState(const AudioPlayInfo& data);
    void UpdateView();

    void RedrawAllElements(const DanmakuRenderingParams& params);
    const DanmakuRenderingParams& GetRenderingParams();

    void SetScrollingFps(int fps);
    void SetSuspendingScrollingEnabled(bool enabled);

    bool GetLastDanmakuData(DanmakuData* out) const;
    void ShowNoDataNotes(const string16& text);

protected:
    enum class HWRenderResult
    {
        Succeeded,   // Hardware accelerated rendering successful ^_^
        NeedRetry,   // An error occurred, but the error has been fixed and needs to be retried *_*
        Failed       // It's hopeless. Let's go to software rendering x_x
    };

    //using DmkRendererPtrPtr = std::shared_ptr<dmkhime::DanmakuRendererPtr>;
    using DmkRendererPtrVectorPtr = std::shared_ptr<std::vector<dmkhime::DanmakuRendererPtr>>;

    void RedrawAllElementsSync(bool force, bool redraw_effects);
    void ForceRedrawAllElements();
    void FallbackToSoftwareRender();

    // dmkhime::DmkhimeEffectRenderCallback
    //void OnEffectRender() override;
    //void OnEffectAnimationStart() override;

    // dmkhime::ListStatusChangeListener
    void OnListAnimationStart() override;
    void OnListAutoScrollStateChanged(bool auto_scroll) override;

    void AddTipDanmaku();
    virtual dmkhime::TipsDmkData OnGetTipDanmakuInfo();
    virtual void SetNavDownText(bool has_new);

    void SoftwareRender(gfx::Canvas* canvas);
    HWRenderResult HardwareRender(gfx::Canvas* canvas);
    void CreateHardwareSurface();
    void SuspendScrolling(bool suspend);

    void OnDanmakuAdded(DmkRendererPtrVectorPtr vec_renderer_ptr_ptr);

    void OnDraw(Sculptor& s);
    void OnRenderTimer();
    void OnGifRenderTimer();
    void OnRenderCallback();
    void OnAnimationStartCallback();

    /* --------------- drawing_thread -----------------*/
    // Creates an off-screen buffer merge column for a new bullet screen
    virtual void RenderOneDanmakuRaw(const DanmakuData& data, DmkRendererPtrVectorPtr vec_renderer_ptr_ptr, int width);

    void RenderOneDanmakuRawForward(const DanmakuData& data, DmkRendererPtrVectorPtr vec_renderer_ptr_ptr, int width);

    bool IsCallConnectStatus();

    DanmakuRenderingParams& RenderParam();

    dmkhime::DanmakuListAdapter* GetDanmakuListAdapter();
    void SetDanmakuListAdapter(dmkhime::DanmakuListAdapter* adapter);

    dmkhime::DanmakuList* GetDanmakuList();

private:
    int max_count_;
    int new_count_ = 0;
    DanmakuViewType type_ = DanmakuViewType::NONE;

    bool is_suspend_scrolling_ = false;
    bool is_suspend_scrolling_enabled_ = false;
    int scrolling_interval_ = 40;
    DanmakuRenderingParams rendering_params_;
    base::RepeatingTimer<DanmakuRenderViddupView> timer_;
    base::RepeatingTimer<DanmakuRenderViddupView> gif_timer_;
    std::vector<dmkhime::DanmakuRendererPtr> com_dmk_list_;
    std::vector<dmkhime::DanmakuRendererPtr> gift_dmk_list_;

    std::mutex com_dmk_list_mtx_;
    std::mutex gift_dmk_list_mtx_;
    //base::RepeatingTimer<DanmakuRenderViddupView> gif_timer_;

    gfx::Point down_pt_;
    gfx::Rect down_rect_;

    dmkhime::DanmakuListAdapter* dmk_adapter_;
    std::unique_ptr<dmkhime::DanmakuList> dmk_list_;

    // The thread used to render the danmaku
    std::unique_ptr<base::Thread> drawing_thread_;
    base::win::ScopedComPtr<ID2D1RenderTarget> hw_rt_;

    bool has_last_data_ = false;
    DanmakuData last_data_;
    base::WeakPtrFactory<DanmakuRenderViddupView> weak_ptr_factory_;

    BililiveLabel* label_no_data_notes_ = nullptr;
    base::TimeTicks start_time_;

    EnterEffectViddupDelegate* delegate_ = nullptr;
    DISALLOW_COPY_AND_ASSIGN(DanmakuRenderViddupView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_DANMAKU_HIME_DANMAKU_RENDER_VIDDUP_VIEW_H_