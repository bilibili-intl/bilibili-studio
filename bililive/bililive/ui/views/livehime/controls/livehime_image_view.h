#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_IMAGE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_IMAGE_VIEW_H_

#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/button/custom_button.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/secret/public/danmaku_hime_service.h"


// 用于显示单/多张图片的控件，支持从 url 获取图片，同时可指定占位图。
// 图片依照 id 升序重叠显示，id 越大，显示越靠前。
// 关于图片的缩放方式及显示方式，参见 SetPreferredSize() 方法和 SetScaleType() 方法。
class LivehimeImageView :
    public views::CustomButton,
    public livehime::SupportsEventReportV2
{
public:
    enum Alignment {
        LEADING = 0,
        CENTER,
        TRAILING
    };

    enum ScaleType {
        ST_NONE,
        ST_FILL,
        ST_SHOW_ALL
    };

    explicit LivehimeImageView(views::ButtonListener* listener = nullptr);

    // 取第一张不为空的图片，图片为空就取占位图。所有图片都为空的话就返回空。
    // 这个方法取到的图是实际绘制的图（已经过压缩）。
    gfx::ImageSkia GetImage();

    // 取对应 id 的原图
    gfx::ImageSkia GetOrgImage(bool is_placeholder = false, int id = -1) const;

    bool IsURLImage(int id = -1);

    bool SetRound(bool isRound) { return  is_round_ = isRound; }

    int  SetRadius(int radius) { return radius_ = radius; }

    // 设置图片的二进制数据。
    // 该图片将立即显示，除非传入的图片为空，这时会显示对应 id 的占位图。
    // 对同样 id 进行多次设置（包括 SetImageUrl()）的话，最后一次设置生效。
    void SetImageBinary(const unsigned char* data, unsigned int len, int id = -1);

    // 设置图片。
    // 该图片将立即显示，除非传入的图片为空，这时会显示对应 id 的占位图。
    // 对同样 id 进行多次设置（包括 SetImageUrl()）的话，最后一次设置生效。
    void SetImage(const gfx::ImageSkia& img, int id = -1);

    // 设置图片 URL。
    // 将首先显示占位图，等待图片下载完成后显示图片。
    // 若 URL 传空，则将一直显示对应 id 的占位图。
    // 对同样 id 进行多次设置（包括 SetImage()）的话，最后一次设置生效。
    void SetImageUrl(const std::string& url, int id = -1);

    // 设置占位图。
    void SetImagePlaceholder(const gfx::ImageSkia& img, int id = -1);

    // 设置图片上下左右的空隙。
    // 根据布局参数不同，该空隙可能向外扩展，也可能挤压图片
    void SetImagePadding(const gfx::Insets& padding, int id = -1);

    // 设置指定图片的显示大小。
    // 除非实际显示区域不够大，否则图片大小将一直是 {size}。
    void SetImageBoundSize(bool enabled, const gfx::Size& size, int id = -1);

    void SetImageHoriAlign(Alignment align, int id = -1);

    void SetImageVertAlign(Alignment align, int id = -1);

    // 设置图片填充方式，该设置对所有图片生效。
    // ST_NONE     不进行缩放
    // ST_FILL     指定图片填充控件的所有空间，图片可能会被裁剪；
    // ST_SHOW_ALL 指定图片在显示完全的情况下尽可能大的显示，默认为此方式
    void SetScaleType(ScaleType type);

    // 移除图片。
    // 如果有占位图，占位图也会一并被移除。
    // 若 id 不存在，则什么也不做。
    void RemoveImage(int id = -1);

    // 移除所有图片。
    // 如果有占位图，占位图也会一并被移除。
    // 如果当前未设置任何图片，则什么也不做。
    void RemoveAllImages();

    // 清除指定图片的缓存，并重新生成缓存（包括占位图）。
    // 该调用会触发指定图片的 OnImageChanging() 回调。
    // 若 id 不存在，则什么也不做。
    void RefreshCache(int id = -1);

    // 清除所有图片的缓存，并重新生成缓存。
    // 该调用会触发所有图片的 OnImageChanging() 回调。
    void RefreshAllCaches();

    // 设置 View 的建议大小。
    // 未调用该方法时，View 的大小由最大的图片（包括占位图）大小决定。
    // 推荐调用该方法，因为可免去图片改变时的重布局。
    void SetPreferredSize(const gfx::Size& size);
    gfx::Size preferred_size() const { return preferred_size_; }

    void SetCursor(gfx::NativeCursor cursor) { cursor_ = cursor; }

    // views::View
    gfx::Size GetPreferredSize() override;

protected:
    // views::View
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    bool HitTestRect(const gfx::Rect& rect) const override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;

    // Button
    void NotifyClick(const ui::Event& event) override;

    /**
     * 在图片被更改之前调用。
     * 有时可能需要对设置的图片（包括从 URL 获取的图片）进行一些处理，比如圆化。
     * @param img 新图片；
     * @param id 指明将被更改的图片 id；
     * @param is_placeholder 指明将更改的图片是否为占位图。
     * @return 返回处理后的结果即可。默认直接返回 img。
     */
    virtual gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder);

    /**
     * 在图片 URL 下载成功时被调用。
     * @param id 指明下载完成图片的 id。
     */
    virtual void OnImageDownloaded(int id) {}

    struct ImageCache {
        gfx::ImageSkia scaled_img; // 缩放后的图片

        ImageCache() {}

        void Set(const gfx::ImageSkia& img) {
            scaled_img = img;
        }

        bool Hit(const gfx::Size& s) const {
            return !scaled_img.isNull() && scaled_img.size() == s;
        }

        void Reset() {
            scaled_img = gfx::ImageSkia();
        }
    };

    struct ImageLayerEntity {
        // id 的智能指针，用于引出弱指针，
        // 方便回调过期时将其吊销。
        std::shared_ptr<int> id;

        bool using_url = false;
        std::string url;
        gfx::ImageSkia org_ph_img;
        ImageCache ph_img_cache;

        gfx::ImageSkia org_img;
        ImageCache img_cache;
        gfx::Insets padding;

        bool using_size = false;
        gfx::Size size;
        Alignment hori_align = Alignment::CENTER;
        Alignment vert_align = Alignment::CENTER;
    };

    struct DrawImageDetails
    {
        gfx::ImageSkia image;
        gfx::Point point;
    };

    DrawImageDetails GetImageToPaint(ImageLayerEntity& entity);

    std::map<int, ImageLayerEntity> img_layer_ents_;

private:
    ImageLayerEntity* MakeImageLayerEntity(int id);
    gfx::Point GetImagePoint(const gfx::Size& img_size, const gfx::Rect& bounds, Alignment ha, Alignment va) const;
    void ResizeImages();

    // 图片下载完成回调。
    // weak_id 指向对应的 ImageLayerEntity 中的 id 共享指针。
    void OnGetImageByUrl(
        bool valid_response, const std::string& data,
        const std::weak_ptr<int>& weak_id);

    ScaleType s_type_;
    gfx::NativeCursor cursor_;
    bool is_spec_preferred_size_;
    gfx::Size preferred_size_;
    base::WeakPtrFactory<LivehimeImageView> weakptr_factory_;
    bool  is_round_ = false;
    int    radius_ = 4;

    DISALLOW_COPY_AND_ASSIGN(LivehimeImageView);
};

// 用于显示圆图片的版本
class LivehimeCircleImageView : public LivehimeImageView
{
public:
    explicit LivehimeCircleImageView(views::ButtonListener* listener = nullptr);

protected:
    gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder) override;
};

// 用于显示带大航海头像框的圆头像的版本
class LivehimeGuardPhotoView : public LivehimeImageView {
public:
    explicit LivehimeGuardPhotoView(views::ButtonListener* listener = nullptr);

    void SetGuardFrame(int guard, int size);

protected:
    gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder) override;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_IMAGE_VIEW_H_