#ifndef BILILIVE_BILILIVE_VIDDUP_DANMAKU_HIME_DANMAKU_RENDERER_H_
#define BILILIVE_BILILIVE_VIDDUP_DANMAKU_HIME_DANMAKU_RENDERER_H_

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
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"

namespace dmkhime
{
    // Normal Danmaku type
    class CommonViddupDmkRenderer : public DanmakuRenderer
    {
    public:
        explicit CommonViddupDmkRenderer(const CommonDmkData& d);
        virtual ~CommonViddupDmkRenderer();

        void GifStop();
        bool SetGifTick(int64_t interval);

        virtual void UpdateWicBitMap(base::win::ScopedComPtr<IWICBitmap> bitmap) override;

        DmkRendererType GetType() const override { return DRT_COMMON; }
        const DmkDataObject* GetData() const override { return &data_; }
        DmkDataObject* GetCopiedData() const override { return new CommonDmkData(data_); }

    protected:
        void OnInit() override;
        void OnRefactor(bool show_icon) override;
        void OnMeasure(int parent_width, int base_font_size) override;
        void OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc) override;
        void OnInvalid() override;
        bool IsOpacitySensitive() const override;

        bool HitTest(const gfx::Point& p) override;

    private:
        void ResetGifPlay();
    private:
        CommonDmkData data_;

        int rank_start_ = 0;
        int medal_start_ = 0;
        int flags_start_ = 0;

        int username_start_;
        int voice_start_;
        int play_img_start_;
        TextLayout plain_text_layout_;
        std::string rank_img_data_;

        base::win::ScopedComPtr<IWICBitmapSource> cur_bitmap_;
        int parent_width_;
        int base_font_;

        bool release_ = false;
        bool has_bg_color_ = false;
        D2D1_COLOR_F bg_color_in_black_;
        D2D1_COLOR_F bg_color_in_white_;
    };

    // Danmaku type with gift
    class GiftComboViddupDmkRenderer : public DanmakuRenderer
    {
    public:
        explicit GiftComboViddupDmkRenderer(const GiftComboDmkData& d);
        virtual ~GiftComboViddupDmkRenderer();
        bool SetGifTick(int64_t interval);
        DmkRendererType GetType() const override { return DRT_COMBO; }
        const DmkDataObject* GetData() const override { return &data_; }
        DmkDataObject* GetCopiedData() const override { return new GiftComboDmkData(data_); }

    protected:
        void OnInit() override;
        void OnRefactor(bool show_icon) override;
        void OnMeasure(int parent_width, int base_font_size) override;
        void OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc) override;
        void OnInvalid() override;

    private:
        GiftComboDmkData data_;

        Range gift_range_;
        Range org_gift_range_;
        Range uname_range_;
        Range receiver_uname_range_;
        TextLayout plain_text_layout_;
        WICBitmaps gift_gif_image_;

        bool release_ = false;
    };

    class AnnocViddupDmkRenderer : public DanmakuRenderer
    {
    public:
        explicit AnnocViddupDmkRenderer(const AnnocDmkData& d);

        DmkRendererType GetType() const override { return DRT_ANNOC; }
        const DmkDataObject* GetData() const override { return &data_; }
        DmkDataObject* GetCopiedData() const override { return new AnnocDmkData(data_); }

    protected:
        void OnInit() override;
        void OnRefactor(bool show_icon) override {}
        void OnMeasure(int parent_width, int base_font_size) override;
        void OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc) override;
        void DrawBackground(Sculptor& sculptor, float x, float y, float right, float bottom, const Paint* painter);
        void OnInvalid() override {}

    private:
        AnnocDmkData data_;

        int uname_start_ = 0;
        int uname_length_ = 0;
        string16 userName;
        string16 real_text_;
        TextLayout plain_text_layout_;
    };

    // The live little assistant shows
    class LiveAsistantViddupDmkRenderer : public DanmakuRenderer
    {
    public:
        explicit LiveAsistantViddupDmkRenderer(const LiveAsistantDanmakuRenderData& d);
        ~LiveAsistantViddupDmkRenderer();

        DmkRendererType GetType() const override { return DRT_ANCHOR_BROADCAST; }
        const DmkDataObject* GetData() const override { return &data_; }
        DmkDataObject* GetCopiedData() const override { return new LiveAsistantDanmakuRenderData(data_); }

    protected:
        void OnInit() override;
        void OnRefactor(bool show_icon) override {}
        void OnMeasure(int parent_width, int base_font_size) override;
        void OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc) override;
        void OnInvalid() override;

        bool HitTest(const gfx::Point& p) override;

        bool IsOpacitySensitive() const override { return true; }

    private:
        LiveAsistantDanmakuRenderData data_;
        ButtonInlineObject* btn_inline_obj_ = nullptr;
        base::string16 btn_text_;
        bool has_button_ = false;

        uint32_t                title_text_size_ = 0;
        uint32_t                content_text_size_ = 0;
        uint32_t                btn_text_start_pos_ = 0;

        int uname_start_ = 0;
        D2D1_COLOR_F btn_bg_color_;
        D2D1_COLOR_F title_font_color_ = {};
        D2D1_COLOR_F btn_font_color_;
        D2D1_COLOR_F content_color_;
        D2D1_COLOR_F content_bg_color_;
        TextLayout plain_text_layout_;
    };
}

#endif  // BILILIVE_BILILIVE_VIDDUP_DANMAKU_HIME_DANMAKU_RENDERER_H_