#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_CROP_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_CROP_VIEW_H_

#include "base/memory/weak_ptr.h"

#include "ui/views/widget/widget_delegate.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_view.h"


class LivehimeSmallContentLabel;
class LivehimeOBSPreviewControl;

class PreviewCropDelegate {
public:
    virtual ~PreviewCropDelegate() = default;

    /**
     * 在 UI 上调整可视区指示器位置时被调用。
     * 应将 x 值设置于底层以使调整生效。
     * gfx::RectF中的x、y、cx、cy是比例值，不是具体的像素值
     */
    virtual void SetCropRegion(const gfx::RectF& region) = 0;

    /**
     * 获取当前的可视区位置。
     * 该位置将用于设置指示器的位置。
     * gfx::RectF中的x、y、cx、cy是比例值，不是具体的像素值
     */
    virtual gfx::RectF GetCropRegion() const = 0;
};

/**
 * 用于调整连麦场景可视区的小窗口。
 * 可使用鼠标拖动窗口中的明亮区域（指示器）来获得可视区的位置数据。
 * 使用 PreviewCropDelegate 来与外界交互。
 */
class PreviewCropView
    : public views::WidgetDelegateView
    , public BililiveFloatingViewDelegate
{
public:
    static void ShowForm(View *related_view, PreviewCropDelegate* d);

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;

    // WidgetDelegate
    View *GetContentsView() override { return this; }

    // BililiveFloatingViewDelegate
    void OnFloatingViewKeyEvent(BililiveFloatingView *floating_view, ui::KeyEvent* event) override;
    void OnFloatingViewFocus(BililiveFloatingView *floating_view) override;
    void OnFloatingViewBlur(BililiveFloatingView *floating_view) override;
    void OnFloatingViewDestroying(BililiveFloatingView *floating_view) override {}

private:
    explicit PreviewCropView(PreviewCropDelegate* d);
    ~PreviewCropView();

    void StartCheckActive();

private:
    static PreviewCropView *instance_;
    LivehimeOBSPreviewControl *obs_preview_control_;
    BililiveFloatingView *capture_view_;
    LivehimeSmallContentLabel *tip_label_;
    bool start_check_active_;

    PreviewCropDelegate* delegate_;
    base::WeakPtrFactory<PreviewCropView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(PreviewCropView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_CROP_VIEW_H_