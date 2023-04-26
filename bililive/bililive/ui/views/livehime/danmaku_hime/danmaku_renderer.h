#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_RENDERER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_RENDERER_H_

#include "base/timer/timer.h"
#include "ui/base/range/range.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmakuhime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/button_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_layout.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_render.h"
#include "bililive/bililive/livehime/gift_image/image_fetcher.h"

class ImageStub;
class NumberImageStub;

namespace dmkhime {

class DanmakuRenderer;
using DanmakuRendererPtr = std::shared_ptr<DanmakuRenderer>;


struct Range {
    int start = 0;
    int length = 0;

    void Set(int start, int length) {
        this->start = start;
        this->length = length;
    }

    void Reset() {
        start = 0;
        length = 0;
    }

    bool Valid() const {
        return length > 0;
    }
};


class DanmakuRenderer {
public:
    DanmakuRenderer();
    virtual ~DanmakuRenderer() = default;

    void Init(int width, bool show_icon);
    void Show();
    void Refactor(bool show_icon);
    void Measure(int parent_width);
    void Measure(int parent_width, int base_font_size);
    void Render(int bk_opacity, Theme theme, bool hw_acc);
    void Invalid();
    virtual void UpdateWicBitMap(base::win::ScopedComPtr<IWICBitmap> bitmap) {}
    /**
     * ���Ը���������ֵ p ���Ƿ����ĳЩ���͵��ı���
     * ���� p �ĵ�λΪ���أ�����ڵ�ǰ��Ļλͼ���Ͻǡ�
     */
    virtual bool HitTest(const gfx::Point& p) { return false; }

    // ��ȡ��ǰ�������Ŀ�ȣ���λΪ����
    int GetWidth() const { return width_; }
    // ��ȡ��ǰ�������ĸ߶ȣ���λΪ����
    int GetHeight() const { return height_; }
    int GetParentWidth() const { return parent_width_; }
    base::win::ScopedComPtr<ID2D1Bitmap> GetBitmap() const;
    bool IsRendered() const { return is_rendered_; }
    bool IsHardwareAccelerated() const { return is_hw_acc_rendered_; }

    virtual DmkRendererType GetType() const = 0;
    virtual const DmkDataObject* GetData() const = 0;
    virtual DmkDataObject* GetCopiedData() const = 0;

protected:
    virtual void OnInit() = 0;
    virtual void OnShow() {}
    virtual void OnRefactor(bool show_icon) = 0;
    virtual void OnMeasure(int parent_width, int base_font_size) = 0;
    virtual void OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc) = 0;
    virtual void OnInvalid() = 0;

    virtual bool IsAutoWidth() const { return false; }
    virtual bool IsThemeSensitive() const { return true; }
    virtual bool IsOpacitySensitive() const { return false; }
    virtual bool IsFontSizeSensitive() const { return true; }

    int ScaleI(int x) const { return int(x * sx_); }

    // ��Ļ��ȣ����ڴ������ݿ�ȣ���λΪ����
    int width_;

    // ��Ļ�߶ȣ��ɵ�Ļ���ݼ���õ�����λΪ����
    int height_;

    int parent_width_ = 0;

    float sx_ = 1.f;

    bool is_rendered_;
    int rendered_opacity_;
    Theme rendered_theme_;
    bool is_hw_acc_rendered_;

    bool is_measured_;
    bool is_icon_showing_;
    int base_font_size_;
    bool force_refactor_ = false;
    base::win::ScopedComPtr<ID2D1Bitmap> d2d_bmp_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKU_RENDERER_H_