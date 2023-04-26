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
     * �� UI �ϵ���������ָʾ��λ��ʱ�����á�
     * Ӧ�� x ֵ�����ڵײ���ʹ������Ч��
     * gfx::RectF�е�x��y��cx��cy�Ǳ���ֵ�����Ǿ��������ֵ
     */
    virtual void SetCropRegion(const gfx::RectF& region) = 0;

    /**
     * ��ȡ��ǰ�Ŀ�����λ�á�
     * ��λ�ý���������ָʾ����λ�á�
     * gfx::RectF�е�x��y��cx��cy�Ǳ���ֵ�����Ǿ��������ֵ
     */
    virtual gfx::RectF GetCropRegion() const = 0;
};

/**
 * ���ڵ������󳡾���������С���ڡ�
 * ��ʹ������϶������е���������ָʾ��������ÿ�������λ�����ݡ�
 * ʹ�� PreviewCropDelegate ������罻����
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