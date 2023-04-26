#ifndef LIVEHIME_USER_RELATED_CTRLS_H
#define LIVEHIME_USER_RELATED_CTRLS_H

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/image_render/bililive_image_render.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/secret/public/danmaku_hime_service.h"

#include "base/memory/weak_ptr.h"

enum class UserOfficalVerifyType
{
    Unverified = -1,
    Official = 0,
    Enterprise = 1,
};

class LivehimeUserPhotoView : public BililiveImageButton
{
public:
    explicit LivehimeUserPhotoView(views::ButtonListener* listener = nullptr);
    virtual ~LivehimeUserPhotoView();

    void SetAvatarByUrl(const std::string &url);

    void SetAvatar(const std::string& avatar);
    void SetAvatar(const unsigned char* data, size_t size);
    void SetAvatar(const gfx::ImageSkia& avatar_img);

    void SetVerifyType(UserOfficalVerifyType verify_type);

    void SetPreferredSize(const gfx::Size& preferred_size);

protected:
    // View
    virtual void OnPaint(gfx::Canvas* canvas) override;

private:
    void OnAvatarByUrl(bool valid_response, const std::string& data);

private:
    gfx::ImageSkia raw_image_;
    UserOfficalVerifyType verify_type_;

    std::string url_;

    base::WeakPtrFactory<LivehimeUserPhotoView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeUserPhotoView);
};


// 用于显示带头像框和认证信息的圆头像的版本
class LivehimeUserPhotoExView :
    public LivehimeImageView,
    public BililiveImageRenderDelegate
{
public:
    explicit LivehimeUserPhotoExView(views::ButtonListener* listener = nullptr, const gfx::Size& avatar_size = {0,0});
    ~LivehimeUserPhotoExView();

    void SetAvatarUrl(const std::string& url);
    void SetAvatar(const uint8_t* data, uint32_t size);
    void SetAvatarPadding(const gfx::Insets& padding);
    void SetAvatarFrameUrl(const std::string& url);
    void SetVerifyType(UserOfficalVerifyType type);
    void SetNFT(bool nft);
    void RemoveAvatarFrame();
    void SetIconSize(const gfx::Size& icon_size);

protected:
    enum Type {
        AVATAR,
        FRAME,
        VERIFY,
        NFT,
    };

    gfx::ImageSkia OnImageChanging(
        const gfx::ImageSkia& img, int id, bool is_placeholder) override;
    void OnImageDownloaded(int id) override;

private:
    void AdjustPadding();
    void OnImageFrameChanged(const gfx::ImageSkia* image) override;

    bool is_nft_ = false;
    UserOfficalVerifyType verift_type_ = UserOfficalVerifyType::Unverified;
    gfx::Insets padding_;
    gfx::Size icon_size_ = GetSizeByDPIScale({12, 12});
    RefImageRender nft_gif_ = nullptr;
};


class LivehimeUserLevelView : public views::View
{
public:
    explicit LivehimeUserLevelView(bool draw_level_name_bk = false);

    virtual ~LivehimeUserLevelView();

    void SetLevel(int level);
    void SetLevel(int level, SkColor color);
    void SetLevel(const base::string16& level_name, int level);
    void SetLevel(const base::string16& level_name, int level, SkColor color);

protected:
    // View
    void OnPaintBackground(gfx::Canvas* canvas) override;

private:
    bool draw_name_bk_ = false;
    base::string16 level_name_;
    int level_ = 0;
    BililiveLabel* name_label_ = nullptr;
    BililiveLabel* level_label_ = nullptr;
};


#endif
