#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_IMAGEBUTTON_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_IMAGEBUTTON_H

#include "base/basictypes.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/help_center/help_center.h"
#include "bililive/bililive/ui/views/controls/image_render/bililive_image_render.h"


class BililiveImageButton :
    public views::ImageButton,
    public BililiveImageRenderDelegate,
    public livehime::HelpSupport,
    public livehime::SupportsEventReportV2
{
public:
    explicit BililiveImageButton(views::ButtonListener* listener);
    virtual ~BililiveImageButton();

    // ָ��ͼƬ�Ƿ����š�
    // �� stretch Ϊ true����ͼƬ�ᱻ�������ʺ� View ��������С��ͼƬ����ʾ������
    // �� stretch Ϊ false����ͼƬ���ᱻ���ţ�����ԭ����С������ʾ��ͼƬ������ʾ��ȫ��
    // Ĭ��Ϊ false��
    void SetStretchPaint(bool stretch);

    void SetPreferredSize(const gfx::Size& preferred_size);

    void SetCursor(gfx::NativeCursor cursor){ cursor_ = cursor; }

    virtual void SetAllStateImage(const gfx::ImageSkia* image);

    bool SetImageFromFile(const base::FilePath &file_path);
    bool SetImageFromBinary(const unsigned char * const data, unsigned int len, bool attach_gif_data = false);
    void SetEnableGif(bool enable);
    void SetButtonListener(views::ButtonListener* listener){ listener_ = listener; }
    void Clone(BililiveImageButton &dst);
    bool IsImageRenderValid() const { return !!gif_.get(); }
    gfx::Size GetRawImageSize() const;

    // View
    gfx::Size GetPreferredSize() override;
protected:
    // View
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    // CustomButton
    void StateChanged() override;

    // Button
    void NotifyClick(const ui::Event& event) override;

private:
    void ResetImageRender();
    void OnImageFrameChanged(const gfx::ImageSkia* image) override;

private:
    bool stretch_;
    bool had_set_preferred_;
    bool enable_gif_;
    gfx::Size preferred_size_;
    gfx::NativeCursor cursor_;

    RefImageRender gif_;

    DISALLOW_COPY_AND_ASSIGN(BililiveImageButton);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
