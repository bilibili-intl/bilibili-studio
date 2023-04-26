
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_SEQUENCE_IMAGE_BUTTON_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_SEQUENCE_IMAGE_BUTTON_H

#include "base/basictypes.h"

#include "ui/views/controls/button/button.h"
#include "ui/gfx/text_constants.h"

#include "bililive/bililive/ui/views/controls/image_render/bililive_sequence_images_util.h"

class BililiveSequenceImageButton;
class BililiveSequenceImageButtonDelegate
{
public:
    virtual ~BililiveSequenceImageButtonDelegate() = default;

    virtual void OnSequenceImageFrameChanged(BililiveSequenceImageButton* button,
        size_t frame_index, size_t frame_counts) = 0;
};


class BililiveSequenceImageButton
    : public views::View
    , public BililiveSequenceImageDelegate
{
public:
    explicit BililiveSequenceImageButton(views::ButtonListener* listener);
    virtual ~BililiveSequenceImageButton();

    void SetStretchPaintSeq(bool stretch);
    void SetStretchPaintBase(bool stretch);

    void SetPreferredSize(const gfx::Size& preferred_size);
    void SetCursor(gfx::NativeCursor cursor) { cursor_ = cursor; }
    void SetButtonListener(views::ButtonListener* listener) { listener_ = listener; }
    void SetImage(const gfx::ImageSkia* image);
    void SetDelegate(BililiveSequenceImageButtonDelegate* delegate) { delegate_ = delegate; }

    // ����֡�������֮���Ƿ��Զ�����View
    void SetHideWhenStoped(bool hide) { hide_when_stop_ = hide; }
    // �ظ����Ŷ���
    void SetRepeating(bool repeating);

    // ����/���ö�Ч����enable=false��ʱ��clear�����ã������Ƿ�Ҫ������һ֡
    // enable=false��ʱ�����û�������Ҫ��ð���Ӧ��RefSequenceImageWrapperҲͣ����
    // ����RefSequenceImageWrapper���ǻ�һֱ�ص�OnSequenceImageFrameChanged
    void EnableAnimation(bool enable, bool clear);

    void AttachSequenceImageWrapper(RefSequenceImageWrapper wrapper, bool owned = false);
    RefSequenceImageWrapper sequence_wrapper() { return sequence_wrapper_; }

    // like ImageButton
    void SetImageAlignment(gfx::HorizontalAlignment h_align,
        gfx::VerticalAlignment v_align);

    // View
    gfx::Size GetPreferredSize() override;
protected:
    // View
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    // BililiveSequenceImageDelegate
    void OnSequenceImageFrameChanged(const gfx::ImageSkia* image, size_t curent_frame, size_t frame_counts) override;
    void OnSequenceMotionStart() override;
    void OnSequenceMotionStop() override;

    void PaintImage(
        gfx::Canvas* canvas, const gfx::ImageSkia &image, bool ignore_align, bool is_stretch);
private:

    void CleanUp();

protected:
    bool is_stretch_seq_ = false;
    bool is_stretch_base_ = false;

    gfx::ImageSkia base_image_;
    gfx::ImageSkia image_;

private:
    views::ButtonListener* listener_;
    BililiveSequenceImageButtonDelegate* delegate_ = nullptr;
    gfx::Size preferred_size_;
    gfx::NativeCursor cursor_;
    bool enable_animation_ = true;
    bool owned_;
    RefSequenceImageWrapper sequence_wrapper_;
    bool hide_when_stop_ = true;
    bool repeating_ = true;
    bool pre_sequence_wrapper_repeating_ = true;

    gfx::HorizontalAlignment h_align_;
    gfx::VerticalAlignment v_align_;

    DISALLOW_COPY_AND_ASSIGN(BililiveSequenceImageButton);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
